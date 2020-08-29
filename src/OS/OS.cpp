#include "pch.h"
#include "OS.h"
#include "../Path.h"
#include "../Text/ConvertUTF.h"
#include "../Text/StringUtils.h"

#ifdef _WIN32
#include <intsafe.h>
#include <io.h>
#else
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#endif
#include <fcntl.h>

#if defined(BMHPAL_PLATFORM_LINUX)
#include <sys/time.h>
#include <sys/resource.h>
#include <dirent.h>
#endif

#ifdef BMHPAL_PLATFORM_APPLE
#include <errno.h>
#include <time.h>
#include <signal.h>
#endif

#if defined(BMHPAL_PLATFORM_APPLE)
#define STAT_TIME(st, x) (st.st_##x##timespec.tv_sec) + ((st.st_##x##timespec.tv_nsec) * (1.0 / 1000000000))
#else
#define STAT_TIME(st, x) (st.st_##x##tim.tv_sec) + ((st.st_##x##tim.tv_nsec) * (1.0 / 1000000000))
#endif

#ifdef BMHPAL_PLATFORM_WINDOWS
#define write _write
#define close _close
#define open _open
#endif

using namespace std;

namespace bmhpal {
namespace os {

static const int64_t DaysFrom1601To1970    = 370 * 365 - 276;
static const int64_t SecondsFrom1601To1970 = DaysFrom1601To1970 * (int64_t) 86400;

BMHPAL_API StaticError ErrEACCESS("Tried to open a read-only file for writing, file's sharing mode does not allow the specified operations, or the given path is a directory");
BMHPAL_API StaticError ErrEEXIST("File already exists");
BMHPAL_API StaticError ErrEINVAL("Invalid oflag or pmode argument");
BMHPAL_API StaticError ErrEMFILE("No more file descriptors are available (too many files are open)");
BMHPAL_API StaticError ErrENOENT("File or path not found");

bool ProcessHandle::IsNull() const {
#if defined(BMHPAL_PLATFORM_WINDOWS)
	return HProc == nullptr;
#else
	return PID == 0;
#endif
}

bool ProcessHandle::Kill() {
#if defined(BMHPAL_PLATFORM_WINDOWS)
	return TerminateProcess(HProc, 1) != 0;
#else
	return kill(PID, SIGTERM) == 0;
#endif
}

bool ProcessHandle::IsProcessAlive(bool& alive) {
	if (IsNull()) {
		alive = false;
		return false;
	}

#if defined(BMHPAL_PLATFORM_WINDOWS)
	DWORD code = 0;
	if (!GetExitCodeProcess(HProc, &code))
		return false;
	alive = code == STILL_ACTIVE;
	if (!alive) {
		ExitCode = (int) code;
		Close();
	}
	return true;
#else
	int   status = 0;
	pid_t pwait  = waitpid(PID, &status, WNOHANG);
	if (pwait == 0) {
		// child still running
		alive = true;
		return true;
	} else if (pwait != PID) {
		// waitpid failed
		return false;
	} else {
		// child has exited
		alive    = false;
		ExitCode = (int8_t) WEXITSTATUS(status);
		Close();
		return true;
	}
#endif
}

void ProcessHandle::Close() {
#if defined(BMHPAL_PLATFORM_WINDOWS)
	if (HProc)
		CloseHandle(HProc);
	HProc  = nullptr;
	ProcID = 0;
#else
	PID           = 0;
#endif
}

BMHPAL_API Error ErrorFrom_errno(int errno_) {
	switch (errno_) {
	case EACCES: return ErrEACCESS;
	case EEXIST: return ErrEEXIST;
	case EINVAL: return ErrEINVAL;
	case EMFILE: return ErrEMFILE;
	case ENOENT: return ErrENOENT;
	}
	return Error(tsf::fmt("OS errno = %v", errno_));
}

#if defined(BMHPAL_PLATFORM_WINDOWS)
static Error ErrorFrom_GetLastError(DWORD e) {
	switch (e) {
	case ERROR_ACCESS_DENIED: return ErrEACCESS;
	case ERROR_ALREADY_EXISTS:
	case ERROR_FILE_EXISTS: return ErrEEXIST;
	case ERROR_FILE_NOT_FOUND:
	case ERROR_PATH_NOT_FOUND:
	case ERROR_NO_MORE_FILES: return ErrENOENT;
	default: break;
	}
	return Error::Fmt("Error %v", e);
}

static int64_t FileTimeTo100NanoSeconds(const FILETIME& ft) {
	uint64_t t = ((uint64_t) ft.dwHighDateTime << 32) | ft.dwLowDateTime;
	return (int64_t) t; // 100-nanoseconds
}

static double FileTimeToUnix(const FILETIME& ft) {
	auto t = FileTimeTo100NanoSeconds(ft);
	t -= SecondsFrom1601To1970 * (int64_t) 10000000;
	return (double) t / 10000000.0;
}
#endif

BMHPAL_API void Sleep(time::Duration d) {
	if (d.Nanoseconds() < 0) {
		// We don't want to underflow and become a giant unsigned number.
		// Since time travel doesn't exist, a negative sleep time is never what the user wanted.
		return;
	}
#if defined(BMHPAL_PLATFORM_WINDOWS)
	::Sleep((DWORD) d.Milliseconds());
#elif defined(BMHPAL_PLATFORM_LINUX)
	int64_t  nano = (int64_t) d.Nanoseconds();
	timespec t;
	t.tv_nsec = nano % 1000000000;
	t.tv_sec  = (nano - t.tv_nsec) / 1000000000;
	nanosleep(&t, nullptr);
#elif defined(BMHPAL_PLATFORM_APPLE)
	int64_t  nano = (int64_t) d.Nanoseconds();
	timespec t;
	t.tv_nsec = nano % 1000000000;
	t.tv_sec  = (nano - t.tv_nsec) / 1000000000;
	nanosleep(&t, nullptr);
#else
	BMHPAL_TODO_STATIC
#endif
}

BMHPAL_API bool MkDir(const std::string& dir, MkDirFlags flags) {
	bool isPrivate = !!(flags & MkDirFlags::Private);
	bool existOK   = !!(flags & MkDirFlags::ExistOK);

#if defined(BMHPAL_PLATFORM_WINDOWS)
	auto err = _wmkdir(towide(dir).c_str());
	if (err == -1 && errno == EEXIST && existOK)
		return true;
	return err == 0;
#elif defined(BMHPAL_PLATFORM_LINUX) || defined(BMHPAL_PLATFORM_APPLE)
	int mode = S_IRWXU | S_IRWXG;
	if (!isPrivate)
		mode |= S_IROTH | S_IXOTH;
	auto err = mkdir(dir.c_str(), mode);
	if (err == -1 && errno == EEXIST && existOK)
		return true;
	return err == 0;
#else
	BMHPAL_TODO_STATIC
#endif
}

BMHPAL_API std::string Cwd() {
#if defined(BMHPAL_PLATFORM_WINDOWS)
	const wchar_t* dir = _wgetcwd(nullptr, 0);
	if (dir == nullptr)
		return "";
	std::string copy = toutf8(dir);
	free((void*) dir);
	return copy;
#elif defined(BMHPAL_PLATFORM_LINUX) || defined(BMHPAL_PLATFORM_APPLE)
	const char* dir = getcwd(nullptr, 0);
	if (!dir)
		return "";
	std::string copy = dir;
	free((void*) dir);
	return copy;
#else
	BMHPAL_TODO_STATIC
#endif
}

BMHPAL_API Error Stat(const std::string& path, FileAttributes& attribs) {
#ifdef _WIN32
	// FILE_FLAG_BACKUP_SEMANTICS is necessary for opening a directory
	HANDLE h = CreateFileW(towide(path).c_str(), FILE_READ_ATTRIBUTES, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
	if (h == INVALID_HANDLE_VALUE) {
		return ErrorFrom_GetLastError(GetLastError());
	}
	BY_HANDLE_FILE_INFORMATION inf;
	if (!GetFileInformationByHandle(h, &inf)) {
		CloseHandle(h);
		return ErrorFrom_GetLastError(GetLastError());
	}
	attribs.IsDir      = !!(inf.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
	attribs.TimeCreate = FileTimeToUnix(inf.ftCreationTime);
	attribs.TimeModify = FileTimeToUnix(inf.ftLastWriteTime);
	attribs.Size       = (uint64_t) inf.nFileSizeHigh << 32 | (uint64_t) inf.nFileSizeLow;
	CloseHandle(h);
	return Error();
#else
	struct stat s;
	if (stat(path.c_str(), &s) != 0)
		return ErrorFrom_errno(errno);
	attribs.IsDir      = S_ISDIR(s.st_mode);
	attribs.TimeCreate = STAT_TIME(s, c); // st.st_mtim.tv_sec + st.st_mtim.tv_nsec * (1.0 / 1000000000);
	attribs.TimeModify = STAT_TIME(s, m); // st.st_mtim.tv_sec + st.st_mtim.tv_nsec * (1.0 / 1000000000);
	attribs.Size       = s.st_size;
	return Error();
#endif
}

BMHPAL_API Error FindFiles(std::string dir, std::vector<FindFileItem>& result, const std::string& wc) {
#ifdef _WIN32
	WIN32_FIND_DATAA fd;
	memset(&fd, 0, sizeof(fd));
	HANDLE fh = FindFirstFileA((dir + "\\" + wc).c_str(), &fd);
	if (fh == INVALID_HANDLE_VALUE)
		return ErrorFrom_GetLastError(GetLastError());
	do {
		if (strcmp(fd.cFileName, ".") == 0)
			continue;
		if (strcmp(fd.cFileName, "..") == 0)
			continue;
		FindFileItem item;
		item.IsDir    = !!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
		item.Name     = fd.cFileName;
		item.FullPath = path::Join(dir, item.Name);
		result.push_back(std::move(item));
	} while (FindNextFileA(fh, &fd));
	FindClose(fh);
	return Error();
#else
	DIR* d = opendir(dir.c_str());
	if (!d)
		return ErrorFrom_errno(errno);

	struct dirent* iter = nullptr;
	while (true) {
		FindFileItem item;
		iter = readdir(d);
		if (iter == nullptr) {
			// end of iteration
			break;
		}
		if (strcmp(iter->d_name, ".") == 0)
			continue;
		if (strcmp(iter->d_name, "..") == 0)
			continue;
		if (!strings::MatchWildcard(iter->d_name, wc.c_str()))
			continue;
		item.IsDir    = iter->d_type == DT_DIR;
		item.Name     = iter->d_name;
		item.FullPath = path::Join(dir, item.Name);
		result.push_back(std::move(item));
	}
	closedir(d);
	return Error();
#endif
}

BMHPAL_API bool DirExists(const std::string& path) {
	FileAttributes at;
	auto           err = Stat(path, at);
	return err.OK() && at.IsDir;
}

BMHPAL_API bool FileExists(const std::string& path) {
	FileAttributes at;
	auto           err = Stat(path, at);
	return err.OK() && !at.IsDir;
}

BMHPAL_API Error ReadFile(const std::string& filename, std::string& content, ReadFlags flags) {
	bool stream = !!(flags & ReadFlags::Stream);

#if defined(BMHPAL_PLATFORM_WINDOWS)
	if (!!(flags & ReadFlags::Stream)) {
		// Implement Stream read
		BMHPAL_ASSERT(false);
	}
	HANDLE h = CreateFileW(towide(filename).c_str(), FILE_GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (h == INVALID_HANDLE_VALUE)
		return ErrorFrom_GetLastError(GetLastError());

	BY_HANDLE_FILE_INFORMATION inf;
	if (!GetFileInformationByHandle(h, &inf)) {
		CloseHandle(h);
		return ErrorFrom_GetLastError(GetLastError());
	}

	uint64_t size = (uint64_t) inf.nFileSizeHigh << 32 | inf.nFileSizeLow;
	if (size >= SSIZE_T_MAX) {
		CloseHandle(h);
		return Error("File is too large to read into memory");
	}

	content.resize((size_t) size);

	size_t pos = 0;
	while (pos < size) {
		uint32_t maxRead = std::min(uint32_t(size - pos), 0x7fffffffu);
		DWORD    nread   = 0;
		if (!::ReadFile(h, &content[pos], maxRead, &nread, nullptr)) {
			auto e = GetLastError();
			CloseHandle(h);
			return ErrorFrom_GetLastError(e);
		}
		pos += nread;
	}

	CloseHandle(h);
	return Error();
#else
	int f = open(filename.c_str(), O_RDONLY);
	if (f == -1)
		return ErrorFrom_errno(errno);

	if (stream) {
		char buf[4096];
		while (true) {
			int nread = read(f, buf, sizeof(buf));
			if (nread == 0)
				break;
			if (nread < 0) {
				auto e = errno;
				close(f);
				return ErrorFrom_errno(e);
			}
			content.append(buf, (size_t) nread);
		}
	} else {
		auto size = lseek(f, 0, SEEK_END);
		if (size == -1) {
			auto e = errno;
			close(f);
			return ErrorFrom_errno(e);
		}
		if (size >= ((size_t) -1) / 2) {
			close(f);
			return Error("File is too large to read into memory");
		}
		if (lseek(f, 0, SEEK_SET) == -1) {
			auto e = errno;
			close(f);
			return ErrorFrom_errno(e);
		}

		content.resize((size_t) size);

		size_t pos = 0;
		while (pos < size) {
			uint32_t maxRead = std::min(uint32_t(size - pos), 0x7fffffffu);
			int      nread   = read(f, &content[0], maxRead);
			if (nread < 0) {
				auto e = errno;
				close(f);
				return ErrorFrom_errno(e);
			}
			pos += nread;
		}
	}

	close(f);
	return Error();
#endif
}

BMHPAL_API Error WriteFile(const std::string& filename, const std::string& buf) {
	return WriteFile(filename, buf.data(), buf.size());
}

BMHPAL_API Error WriteFile(const std::string& filename, const void* buf, size_t len) {
#ifdef BMHPAL_PLATFORM_WINDOWS
	int f = _open(filename.c_str(), _O_CREAT | _O_TRUNC | _O_WRONLY | _O_BINARY, _S_IREAD | _S_IWRITE);
#else
	int f = open(filename.c_str(), O_CREAT | O_TRUNC | O_WRONLY, 0644);
#endif
	if (f == -1)
		return ErrorFrom_errno(errno);
	size_t remain = len;
	while (remain) {
		size_t n = write(f, buf, (unsigned) remain);
		if (n == -1) {
			int e = errno;
			close(f);
			return ErrorFrom_errno(e);
		}
		(char*&) buf += n;
		remain -= n;
	}
	if (close(f) == -1)
		return ErrorFrom_errno(errno);
	return Error();
}

BMHPAL_API Error Rename(const std::string& src, const std::string& dst) {
	if (rename(src.c_str(), dst.c_str()) == 0)
		return Error();
	else
		return ErrorFrom_errno(errno);
}

static Error RemoveFile(const std::string& path) {
#if defined(BMHPAL_PLATFORM_WINDOWS)
	return ::DeleteFileA(path.c_str()) ? Error() : os::ErrorFrom_GetLastError(GetLastError());
#else
	if (remove(path.c_str()) == 0)
		return Error();
	else
		return ErrorFrom_errno(errno);
#endif
}

static Error RemoveDir(const std::string& path) {
#if defined(BMHPAL_PLATFORM_WINDOWS)
	return ::RemoveDirectoryA(path.c_str()) ? Error() : os::ErrorFrom_GetLastError(GetLastError());
#else
	if (remove(path.c_str()) == 0)
		return Error();
	else
		return ErrorFrom_errno(errno);
#endif
}

BMHPAL_API Error Remove(const std::string& path) {
#if defined(BMHPAL_PLATFORM_WINDOWS)
	DWORD attribs = GetFileAttributes(path.c_str());
	if (attribs == INVALID_FILE_ATTRIBUTES)
		return os::ErrorFrom_GetLastError(GetLastError());

	if (!!(attribs & FILE_ATTRIBUTE_DIRECTORY))
		return ::RemoveDirectoryA(path.c_str()) ? Error() : os::ErrorFrom_GetLastError(GetLastError());
	else
		return ::DeleteFileA(path.c_str()) ? Error() : os::ErrorFrom_GetLastError(GetLastError());
#else
	if (remove(path.c_str()) == 0)
		return Error();
	else
		return ErrorFrom_errno(errno);
#endif
}

BMHPAL_API Error RemoveAll(const std::string& path) {
	vector<FindFileItem> items;
	auto                 err = FindFiles(path, items);
	if (!err.OK()) {
		if (IsNotExist(err))
			return Error();
		return Error::Fmt("Error enumerating files in %v: %v", path, err.Message());
	}
	Error firstErr;
	for (const auto& item : items) {
		Error err;
		if (item.IsDir)
			err = RemoveAll(item.FullPath);
		else
			err = RemoveFile(item.FullPath);

		if (!err.OK() && firstErr.OK())
			firstErr = firstErr;
	}
	Error lastErr = RemoveDir(path);
	if (firstErr.OK())
		firstErr = lastErr;
	return firstErr;
}

#if defined(BMHPAL_PLATFORM_WINDOWS)
static std::string GetKnownFolder(GUID g) {
	wchar_t* str = nullptr;
	SHGetKnownFolderPath(g, 0, NULL, &str);
	auto copy = toutf8(str);
	CoTaskMemFree(str);
	return copy;
}
#endif

BMHPAL_API std::string UserHomeDir() {
#if defined(BMHPAL_PLATFORM_WINDOWS)
	return GetKnownFolder(FOLDERID_Profile);
#else
	return GetEnv("HOME");
#endif
}

BMHPAL_API std::string UserLocalAppData() {
#if defined(BMHPAL_PLATFORM_WINDOWS)
	return GetKnownFolder(FOLDERID_LocalAppData);
#else
	return UserHomeDir() + "/.config";
#endif
}

BMHPAL_API std::string UserRoamingAppData() {
#if defined(BMHPAL_PLATFORM_WINDOWS)
	return GetKnownFolder(FOLDERID_RoamingAppData);
#else
	return UserHomeDir() + "/.config";
#endif
}

BMHPAL_API std::string ProgramFilesx64() {
#if defined(BMHPAL_PLATFORM_WINDOWS)
	return GetKnownFolder(FOLDERID_ProgramFilesX64);
#else
	return "";
#endif
}

BMHPAL_API std::string ProgramFilesx86() {
#if defined(BMHPAL_PLATFORM_WINDOWS)
	return GetKnownFolder(FOLDERID_ProgramFilesX86);
#else
	return "";
#endif
}

BMHPAL_API std::string GetEnv(const std::string& var) {
#if defined(BMHPAL_PLATFORM_WINDOWS)
	std::string val;
	size_t      size = GetEnvironmentVariable(var.c_str(), nullptr, 0);
	if (size != 0) {
		val.resize(size - 1);
		GetEnvironmentVariable(var.c_str(), &val[0], (DWORD) val.size() + 1);
	}
	return val;
#else
	const char* e = getenv(var.c_str());
	return e ? e : "";
#endif
}

BMHPAL_API void TraceStr(const std::string& msg) {
#if defined(BMHPAL_PLATFORM_WINDOWS)
	OutputDebugStringA(msg.c_str());
#else
	fputs(msg.c_str(), stdout);
#endif
}

BMHPAL_API Error Exec(const std::string& path, const std::vector<std::string>& args, ExecFlags flags, ProcessHandle& handle) {
#if defined(BMHPAL_PLATFORM_WINDOWS)
	auto         wpath = towide(path);
	std::wstring allarg;
	allarg += L'"';
	allarg += wpath;
	allarg += L'"';
	for (const auto& a : args) {
		allarg += L" ";
		if (a.find('"') != -1) {
			// might need a more sophisticated quoter here
			allarg += L'"';
			allarg += towide(a);
			allarg += L'"';
		} else {
			allarg += towide(a);
		}
	}

	wchar_t* cmd = new wchar_t[allarg.size() + 1];
	memcpy(cmd, allarg.c_str(), (allarg.size() + 1) * sizeof(wchar_t));
	STARTUPINFOW si;
	memset(&si, 0, sizeof(si));
	si.cb = sizeof(si);
	PROCESS_INFORMATION pi;
	memset(&pi, 0, sizeof(pi));
	const wchar_t* app = wpath.c_str();
	if (!!(flags & ExecFlags::UseSearchPath)) {
		// By making app null, the behaviour changes, and Windows uses the search path
		app = nullptr;
	}
	bool ok = !!CreateProcessW(app, cmd, nullptr, nullptr, false, 0, nullptr, nullptr, &si, &pi);
	delete[] cmd;
	if (!ok)
		return ErrorFrom_GetLastError(GetLastError());
	CloseHandle(pi.hThread);
	handle.HProc  = pi.hProcess;
	handle.ProcID = pi.dwProcessId;
	return Error();
#else
	std::vector<const char*> argv;
	argv.push_back(path.c_str());
	for (size_t i = 0; i < args.size(); i++)
		argv.push_back(args[i].c_str());
	argv.push_back(nullptr);

	pid_t childid = vfork();
	if (childid == -1)
		return Error::Fmt("Unable to start child process: %d", errno);
	if (childid != 0) {
		// parent
		handle.PID = childid;
		return Error();
	} else {
		// child
		if (!!(flags & ExecFlags::UseSearchPath))
			execvp(path.c_str(), (char* const*) &argv[0]);
		else
			execv(path.c_str(), (char* const*) &argv[0]);
		// Since we're using vfork, the only thing we're allowed to do now is call  __exit
		_exit(1);
		return Error("How can _exit(1) fail!!!"); // unreachable
	}
#endif
}

BMHPAL_API Error Terminate(ProcessHandle& handle) {
	if (handle.IsNull())
		return Error();
#if defined(BMHPAL_PLATFORM_WINDOWS)
	if (!TerminateProcess(handle.HProc, 0))
		return ErrorFrom_GetLastError(GetLastError());
	return Error();
#else
	if (0 != kill(handle.PID, SIGKILL))
		return ErrorFrom_errno(errno);
	return Error();
#endif
}

BMHPAL_API bool IsNotExist(const Error& err) {
	return err == ErrENOENT;
}

BMHPAL_API std::string ProcessPath() {
#if defined(BMHPAL_PLATFORM_WINDOWS)
	wchar_t buf[4096];
	GetModuleFileNameW(NULL, buf, arraysize(buf));
	buf[arraysize(buf) - 1] = 0;
	return toutf8(buf);
#else
	char buf[4096];
	buf[0] = 0;
	int r  = readlink("/proc/self/exe", buf, arraysize(buf) - 1);
	if (r < 0)
		return buf;

	if (r < sizeof(buf))
		buf[r] = 0;
	else
		buf[sizeof(buf) - 1] = 0;
	return buf;
#endif
}

#if defined(BMHPAL_PLATFORM_LINUX)
static double TimeValToSeconds(timeval t) {
	return (double) t.tv_sec + (double) t.tv_usec / 1000000.0;
}
#endif

BMHPAL_API Error CPUTime(double& self, double& children) {
#if defined(BMHPAL_PLATFORM_LINUX)
	rusage rself;
	rusage rchildren;
	int    r = getrusage(RUSAGE_SELF, &rself);
	if (r == -1)
		return ErrorFrom_errno(errno);
	r = getrusage(RUSAGE_CHILDREN, &rchildren);
	if (r == -1)
		return ErrorFrom_errno(errno);
	self     = TimeValToSeconds(rself.ru_utime) + TimeValToSeconds(rself.ru_stime);
	children = TimeValToSeconds(rchildren.ru_utime) + TimeValToSeconds(rchildren.ru_stime);
	return Error();
#else
	return Error("CPU time not supported on this architecture");
#endif
}

BMHPAL_API bool IsInsideLinuxContainer() {
#ifdef __linux__
	int fd = open("/proc/1/cgroup", O_RDONLY);
	if (fd == -1) {
		return false;
	}
	char buf[512];
	auto n      = read(fd, buf, sizeof(buf) - 1);
	n           = n < sizeof(buf) ? n : sizeof(buf) - 1;
	buf[n]      = 0;
	bool inside = strstr(buf, "/docker/") != nullptr || strstr(buf, "/lxc/") != nullptr;
	close(fd);
	return inside;
#else
	return false;
#endif
}

} // namespace os
} // namespace bmhpal
