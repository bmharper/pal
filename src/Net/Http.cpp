#include "pch.h"
#include "../OS/OS.h"
#include "Http.h"
#include "../Text/StringUtils.h"

#ifdef _MSC_VER
#define itoa _itoa
#endif

namespace bmhpal {
namespace net {
namespace http {

enum {
	Uninitialized    = 0,
	BusyInitializing = 1,
	Initialized      = 2,
};

int InitStatus = Uninitialized;

Request::Request() {
}

Request::Request(const std::string& method, const std::string& uri, const std::string& body) : Method(method), URI(uri), Body(body) {
}

Request::~Request() {
}

void Request::SetHeader(const std::string& key, const std::string& val) {
	for (auto& p : Headers) {
		if (strings::EqualsNoCase(p.first, key)) {
			p.second = val;
			return;
		}
	}
	Headers.push_back({key, val});
}

std::string Request::Dump(size_t truncateBodySize) const {
	std::string s = Method + " " + URI + "\n";
	for (const auto& h : Headers) {
		s += h.first + ": " + h.second + "\n";
	}
	s += "\n";
	if (truncateBodySize == -1) {
		s += Body;
	} else {
		s += Body.substr(0, truncateBodySize);
		if (Body.length() > truncateBodySize)
			s += "...";
	}
	return s;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string Response::Header(const std::string& header) {
	for (const auto& h : Headers) {
		if (strings::EqualsNoCase(h.first, header))
			return h.second;
	}
	return "";
}

std::vector<std::string> Response::AllHeaders(const std::string& header) {
	std::vector<std::string> res;
	for (const auto& h : Headers) {
		if (strings::EqualsNoCase(h.first, header))
			res.push_back(h.second);
	}
	return res;
}

void Response::SetHeader(const std::string& key, const std::string& value) {
	for (auto& h : Headers) {
		if (strings::EqualsNoCase(h.first, key)) {
			h.second = value;
			return;
		}
	}
	Headers.emplace_back(key, value);
}

std::string Response::StatusAndBody() const {
	return ItoA(Status) + " " + Body;
}

Error Response::ToError() const {
	if (Cancelled)
		return Error("Cancelled");
	if (Is2xx())
		return Error();
	return ConErr != "" ? Error(ConErr) : Error(StatusAndBody());
}

std::string Response::Dump(size_t truncateBodySize) const {
	std::string s = ItoA(Status) + "\n";
	for (const auto& h : Headers) {
		s += h.first + ": " + h.second + "\n";
	}
	s += "\n";
	if (truncateBodySize == -1) {
		s += Body;
	} else {
		s += Body.substr(0, truncateBodySize);
		if (Body.length() > truncateBodySize)
			s += "...";
	}
	return s;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Client::Client() {
	std::atomic<int>* ai = (std::atomic<int>*) &InitStatus;

	while (true) {
		int expect = Uninitialized;
		if (ai->compare_exchange_strong(expect, BusyInitializing)) {
			// We got the ticket to initialize
			curl_global_init(CURL_GLOBAL_DEFAULT);
			ai->exchange(Initialized);
			break;
		} else if (expect == Initialized) {
			// Somebody else already initialized
			break;
		} else {
			// Somebody else is busy initializing. Just busy wait.
			os::Sleep(time::Microsecond);
		}
	}
}

Client::~Client() {
	if (Curl)
		curl_easy_cleanup(Curl);
}

void Client::AppClose() {
	if (InitStatus == Initialized)
		curl_global_cleanup();
}

void Client::PerformInternal(const Request& req, Response& resp) {
	if (!Curl)
		Curl = curl_easy_init();
	curl_easy_reset(Curl);

	IsCancelled     = false;
	ReadPtrStart    = (char*) req.Body.c_str();
	ReadPtr         = ReadPtrStart;
	ReadPtrEnd      = ReadPtr + req.Body.size();
	CurrentRequest  = &req;
	CurrentResponse = &resp;

	curl_easy_setopt(Curl, CURLOPT_CUSTOMREQUEST, req.Method.c_str());
	curl_easy_setopt(Curl, CURLOPT_URL, req.URI.c_str());
	curl_easy_setopt(Curl, CURLOPT_READFUNCTION, CurlReadCallback);
	curl_easy_setopt(Curl, CURLOPT_READDATA, this);
	curl_easy_setopt(Curl, CURLOPT_WRITEFUNCTION, CurlWriteCallback);
	curl_easy_setopt(Curl, CURLOPT_WRITEDATA, this);
	curl_easy_setopt(Curl, CURLOPT_HEADERFUNCTION, CurlHeaderCallback);
	curl_easy_setopt(Curl, CURLOPT_HEADERDATA, this);
	if (CACertificatesFile != "")
		curl_easy_setopt(Curl, CURLOPT_CAINFO, CACertificatesFile.c_str());

	if (req.Method == "POST" || req.Method == "PUT") {
		curl_easy_setopt(Curl, CURLOPT_UPLOAD, 1);
		curl_easy_setopt(Curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t) req.Body.size());
	}

	curl_slist* headers           = nullptr;
	bool        haveContentLength = false;
	for (const auto& h : req.Headers) {
		headers = curl_slist_append(headers, (h.first + ": " + h.second).c_str());
	}
	if (!req.AllowExpect100)
		headers = curl_slist_append(headers, "Expect:");

	curl_easy_setopt(Curl, CURLOPT_HTTPHEADER, headers);
	CURLcode res = curl_easy_perform(Curl);
	curl_slist_free_all(headers);
	if (res != CURLE_OK) {
		resp.ConErr = curl_easy_strerror(res);
	}

	CurrentRequest  = nullptr;
	CurrentResponse = nullptr;
}

void Client::Perform(const Request& req, Response& resp) {
	Request                 reqCopy;
	int                     iRedirect   = 0;
	int                     maxRedirect = 7;         // 7 is an arbitrary thumbsuck
	ohash::set<std::string> seenURI     = {req.URI}; // used to detect redirect loops

	for (; iRedirect < maxRedirect; iRedirect++) {
		if (iRedirect == 0)
			PerformInternal(req, resp);
		else
			PerformInternal(reqCopy, resp);

		if (!req.FollowRedirects || resp.Status != 302)
			break;

		reqCopy     = req;
		reqCopy.URI = resp.Header("Location");
		if (seenURI.contains(reqCopy.URI)) {
			resp.RedirectLoop = true;
			break;
		}
		resp = Response();
		seenURI.insert(reqCopy.URI);
	}
	if (iRedirect == maxRedirect)
		resp.RedirectLoop = true;
}

Response Client::Perform(const Request& req) {
	Response resp;
	Perform(req, resp);
	return std::move(resp);
}

Response Client::PerformRobust(const Request& req, RobustOptions robust) {
	Response resp;
	for (int attempt = 0; attempt < robust.MaxRetries; attempt++) {
		resp = Perform(req);
		if (resp.Is2xx())
			break;
		double sleepSeconds = pow((double) robust.Backoff, (double) attempt);
		os::Sleep(time::Millisecond * int(sleepSeconds * 1000));
	}
	return resp;
}

void Client::Cancel() {
	IsCancelled = true;
}

size_t Client::CurlWriteCallback(char* ptr, size_t size, size_t nmemb, void* userdata) {
	auto self = (Client*) userdata;
	if (self->IsCancelled) {
		self->CurrentResponse->Cancelled = true;
		return 0;
	}
	self->CurrentResponse->Body.append(ptr, size * nmemb);
	if (self->CurrentRequest->OnProgress) {
		auto    lenStr = self->CurrentResponse->Header("Content-Length");
		int64_t len    = 0;
		if (lenStr != "")
			len = atoi64(lenStr.c_str());
		bool keepGoing = self->CurrentRequest->OnProgress(ProgressPhase::Receive, self->CurrentResponse->Body.size(), len);
		if (!keepGoing) {
			self->CurrentResponse->Cancelled = true;
			self->IsCancelled                = true;
			return 0;
		}
	}
	return size * nmemb;
}

size_t Client::CurlReadCallback(char* buffer, size_t size, size_t nitems, void* userdata) {
	auto self = (Client*) userdata;
	if (self->CurrentRequest->OnProgress) {
		bool keepGoing = self->CurrentRequest->OnProgress(ProgressPhase::Send, self->ReadPtr - self->ReadPtrStart, self->ReadPtrEnd - self->ReadPtrStart);
		if (!keepGoing) {
			self->CurrentResponse->Cancelled = true;
			self->IsCancelled                = true;
		}
		// It's useful to uncomment this next line, when testing the upload progress UI
		//os::Sleep(250 * time::Millisecond); // DO NOT COMMIT
	}
	if (self->IsCancelled) {
		self->CurrentResponse->Cancelled = true;
		return 0;
	}
	size_t n = size * nitems;
	n        = std::min(n, (size_t)(self->ReadPtrEnd - self->ReadPtr));
	memcpy(buffer, self->ReadPtr, n);
	self->ReadPtr += n;
	return n;
}

size_t Client::CurlHeaderCallback(char* buffer, size_t size, size_t nitems, void* userdata) {
	auto self = (Client*) userdata;
	if (self->IsCancelled) {
		self->CurrentResponse->Cancelled = true;
		return 0;
	}
	auto   w = self->CurrentResponse;
	size_t n = size * nitems;

	// HTTP/1.1 200 OK
	// HTTP/1.0 411 Length Required
	if (n >= 12 && (memcmp(buffer, "HTTP/1.0", 8) == 0 || memcmp(buffer, "HTTP/1.1", 8) == 0)) {
		w->Status = atoi(buffer + 9);
		return n;
	}
	// final (blank) header line
	if (n == 2 && memcmp(buffer, "\r\n", 2) == 0)
		return n;

	size_t keyEnd   = 0;
	size_t valStart = 0;
	for (; keyEnd < n; keyEnd++) {
		if (buffer[keyEnd] == ':') {
			valStart = keyEnd + 1;
			for (; valStart < n && buffer[valStart] == ' '; valStart++) {
			}
			break;
		}
	}
	if (keyEnd == n)
		return 0;
	// the -2 on the value length is to get rid of the \r\n at the end of the header line
	w->Headers.push_back({std::string(buffer, keyEnd), std::string(buffer + valStart, n - valStart - 2)});
	return n;
}

} // namespace http
} // namespace net
} // namespace bmhpal