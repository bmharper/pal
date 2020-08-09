#pragma once

#include "../Error/Error.h"

namespace bmhpal {
namespace net {
namespace http {

enum class ProgressPhase {
	Send,
	Receive,
};

// Progress callback. If you return false, then the operation is cancelled
typedef std::function<bool(ProgressPhase phase, size_t bytesDone, size_t bytesTotal)> ProgressCallback;

// HTTP Request
class BMHPAL_API Request {
public:
	std::string                                      Method;
	std::string                                      URI;
	std::vector<std::pair<std::string, std::string>> Headers;
	std::string                                      Body;
	bool                                             AllowExpect100  = false;
	bool                                             FollowRedirects = false;
	ProgressCallback                                 OnProgress;

	Request();
	Request(const std::string& method, const std::string& uri, const std::string& body = "");
	~Request();

	void SetHeader(const std::string& key, const std::string& val);
};

// HTTP Response
class BMHPAL_API Response {
public:
	std::string                                      ConErr; // If not empty, then failed to reach HTTP server, and Status = 0
	int                                              Status = 0;
	std::string                                      Body;
	std::vector<std::pair<std::string, std::string>> Headers;
	bool                                             RedirectLoop = false; // True if we detect a 302 redirect loop
	bool                                             Cancelled    = false; // True if you called Cancel() on the Client, or returned false from your progress callback

	std::string              Header(const std::string& header);     // Returns first matching header
	std::vector<std::string> AllHeaders(const std::string& header); // Returns all matching headers

	bool Is2xx() const {
		return Status >= 200 && Status < 300;
	}
	std::string StatusAndBody() const;
	Error       ToError() const; // Returns Error() if Is2xx(). Otherwise, returns ConErr if ConErr is not empty. If ConErr is empty, returns StatusAndBody()
};

struct BMHPAL_API RobustOptions {
	int   MaxRetries = 5;
	float Backoff    = 1.4; // Every sleep is Backoff ^ N, where N is number of attempts
};

// Wrapper around libcurl
class BMHPAL_API Client {
public:
	std::string CACertificatesFile; // If not empty, this is sent via CURLOPT_CAINFO

	Client();
	~Client();

	// Call this once, at application close, so that we can call curl_global_cleanup
	static void AppClose();

	void     Perform(const Request& req, Response& resp);
	Response Perform(const Request& req);

	// PerformRobust keeps retrying until it gets a 2xx response code,
	// or it reaches it's maximum number of retries.
	Response PerformRobust(const Request& req, RobustOptions robust = RobustOptions());

	void Cancel();

private:
	void*             Curl = nullptr;
	std::atomic<bool> IsCancelled;
	const char*       ReadPtr         = nullptr;
	const char*       ReadPtrStart    = nullptr;
	const char*       ReadPtrEnd      = nullptr;
	const Request*    CurrentRequest  = nullptr;
	Response*         CurrentResponse = nullptr;

	void PerformInternal(const Request& req, Response& resp);

	static size_t CurlWriteCallback(char* ptr, size_t size, size_t nmemb, void* userdata);
	static size_t CurlReadCallback(char* buffer, size_t size, size_t nitems, void* instream);
	static size_t CurlHeaderCallback(char* buffer, size_t size, size_t nitems, void* userdata);
};

} // namespace http
} // namespace net
} // namespace bmhpal