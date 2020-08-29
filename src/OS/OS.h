#pragma once

#include "../Time/Time_.h"
#include "../Error/Error.h"
#include "../Error/CommonErrors.h"

namespace bmhpal {
namespace os {

enum class Platforms {
	Linux,
	Windows,
};

enum class ExecFlags {
	None          = 0,
	UseSearchPath = 1, // Use PATH environment variable to find executable (otherwise, assume absolute path)
};
inline ExecFlags operator|(ExecFlags a, ExecFlags b) {
	return ExecFlags((uint32_t) a | (uint32_t) b);
}
inline uint32_t operator&(ExecFlags a, ExecFlags b) {
	return (uint32_t) a & (uint32_t) b;
}

enum class MkDirFlags {
	None    = 0,
	Private = 1, // When not private, then mode |= S_IROTH | S_IXOTH
	ExistOK = 2, // Don't fail if the directory already exists
};
inline MkDirFlags operator|(MkDirFlags a, MkDirFlags b) {
	return MkDirFlags((uint32_t) a | (uint32_t) b);
}
inline uint32_t operator&(MkDirFlags a, MkDirFlags b) {
	return (uint32_t) a & (uint32_t) b;
}

struct FileAttributes {
	double   TimeCreate = 0; // Creation time (unix seconds)
	double   TimeModify = 0; // Last modification time (unix seconds)
	uint64_t Size       = 0;
	bool     IsDir      = false;
};

struct FindFileItem {
	std::string Name;
	std::string FullPath;
	bool        IsDir = false;

	bool operator<(const FindFileItem& b) const {
		return FullPath < b.FullPath;
	}
};

// When you are finished using a ProcessHandle, you must call IsProcessAlive() on it, until
// the process has exited.
// This is necessary for linux, because if we don't want on PID, we end up with zombie processes: https://linux.die.net/man/2/waitpid
struct BMHPAL_API ProcessHandle {
#if defined(BMHPAL_PLATFORM_WINDOWS)
	HANDLE HProc  = nullptr;
	DWORD  ProcID = 0;
#else
	pid_t PID = 0;
#endif
	// This is set by IsProcessAlive(), when it returns true with 'alive' = false.
	int ExitCode = 0;
	// If you call this, and alive is false, then the handle is made null. This behaviour is driven by linux behaviour.
	// If you call waitpid and the process has exited, then the OS will get rid of the process, and the pid can then be
	// reused for a new process.
	// Returns true if the call succeeded.
	bool IsProcessAlive(bool& alive);
	bool IsNull() const;
	// On Windows this just calls TerminateProcess. On Linux, we do kill(SIGTERM)
	bool Kill();
#if defined(BMHPAL_PLATFORM_WINDOWS)
	int ProcessID() const {
		return ProcID;
	}
#else
	int   ProcessID() const {
        return PID;
	}
#endif

private:
	void Close();
};

enum class ReadFlags {
	None = 0,
	// Do not first read the length of the file, and pre-allocate memory. Read chunk by chunk.
	// This is necessary when reading virtual files such as /proc/pid/cmdline
	Stream = 1,
};
inline ReadFlags operator|(ReadFlags a, ReadFlags b) {
	return ReadFlags((uint32_t) a | (uint32_t) b);
}
inline uint32_t operator&(ReadFlags a, ReadFlags b) {
	return (uint32_t) a & (uint32_t) b;
}

extern BMHPAL_API StaticError ErrEACCESS;
extern BMHPAL_API StaticError ErrEEXIST;
extern BMHPAL_API StaticError ErrEINVAL;
extern BMHPAL_API StaticError ErrEMFILE;
extern BMHPAL_API StaticError ErrENOENT;

BMHPAL_API void Sleep(time::Duration d);
BMHPAL_API bool MkDir(const std::string& dir, MkDirFlags flags = MkDirFlags::None);
BMHPAL_API std::string Cwd(); // Get current working directory
BMHPAL_API Error       Stat(const std::string& path, FileAttributes& attribs);
BMHPAL_API Error       FindFiles(std::string dir, std::vector<FindFileItem>& result, const std::string& wc = "*"); // Finds all entries in the given directory that match the wildcard
BMHPAL_API bool        DirExists(const std::string& path);                                                         // Returns true if this is directory
BMHPAL_API bool        FileExists(const std::string& path);                                                        // Returns true if this can be Stat() and is not a directory. It could be a socket, or anything weird like that though.
BMHPAL_API Error       ReadFile(const std::string& filename, std::string& content, ReadFlags flags = ReadFlags::None);
BMHPAL_API Error       WriteFile(const std::string& filename, const std::string& buf);
BMHPAL_API Error       WriteFile(const std::string& filename, const void* buf, size_t len);
BMHPAL_API Error       Rename(const std::string& src, const std::string& dst);
BMHPAL_API Error       ErrorFrom_errno(int errno_);
BMHPAL_API Error       Remove(const std::string& path);
BMHPAL_API Error       RemoveAll(const std::string& path);
BMHPAL_API std::string UserHomeDir();        // On Windows, return C:\Users\<username>
BMHPAL_API std::string UserLocalAppData();   // On Windows, return C:\Users\<username>\AppData\Local
BMHPAL_API std::string UserRoamingAppData(); // On Windows, return C:\Users\<username>\AppData\Roaming
BMHPAL_API std::string ProgramFilesx64();    // On linux returns empty string
BMHPAL_API std::string ProgramFilesx86();    // On linux returns empty string
BMHPAL_API std::string GetEnv(const std::string& var);
BMHPAL_API void        TraceStr(const std::string& msg);
BMHPAL_API Error       Exec(const std::string& path, const std::vector<std::string>& args, ExecFlags flags, ProcessHandle& handle);
BMHPAL_API Error       Terminate(ProcessHandle& handle);
BMHPAL_API bool        IsNotExist(const Error& err);
BMHPAL_API std::string ProcessPath();
BMHPAL_API Error       CPUTime(double& self, double& children); // Return seconds of CPU time. Only implemented on Linux. Child processes must have exited and been waited on.
BMHPAL_API bool        IsInsideLinuxContainer();                // Returns true if we believe we're running under docker or lxc

inline Platforms Platform() {
#ifdef _WIN32
	return Platforms::Windows;
#else
	return Platforms::Linux;
#endif
}

inline bool IsLinux() {
	return os::Platform() == Platforms::Linux;
}

inline bool IsWindows() {
	return os::Platform() == Platforms::Windows;
}

template <typename... Args>
void Trace(const char* fs, const Args&... args) {
	bmhpal::os::TraceStr(tsf::fmt(fs, args...));
}

} // namespace os
} // namespace bmhpal