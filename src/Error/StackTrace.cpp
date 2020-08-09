// EMSCRIPTEN
#include "pch.h"
#include "StackTrace.h"

#ifdef BMHPAL_PLATFORM_WINDOWS
#include "../OS/OS.h"
#include "../Text/ConvertUTF.h"

#pragma warning(push)
#pragma warning(disable : 4091)
#include <Dbghelp.h>
#pragma warning(pop)
#endif

#ifdef BMHPAL_PLATFORM_LINUX
#include "../OS/OS.h"
#define UNW_LOCAL_ONLY
#include <libunwind.h>
#include <signal.h>
#include <execinfo.h>
#endif

namespace bmhpal {

#ifdef BMHPAL_PLATFORM_LINUX

static std::string PrintStackTrace(bool withAddr2Line, bool saveString) {
	unw_cursor_t  cursor;
	unw_context_t context;

	// Initialize cursor to current frame for local unwinding.
	unw_getcontext(&context);
	unw_init_local(&cursor, &context);

	std::string str;
	if (!saveString)
		fprintf(stderr, "------------------------------------------------------------------\n");

	std::string self = "";
	if (withAddr2Line)
		self = os::ProcessPath();

	// Unwind frames one by one, going up the frame stack.
	while (unw_step(&cursor) > 0) {
		unw_word_t offset, pc;
		unw_get_reg(&cursor, UNW_REG_IP, &pc);
		if (pc == 0) {
			break;
		}

		if (saveString)
			str += tsf::fmt("0x%lx: ", pc);
		else
			fprintf(stderr, "0x%lx: ", pc);

		if (withAddr2Line) {
			char addr[500];
			sprintf(addr, "addr2line 0x%lx -e %s", pc, self.c_str());
			FILE* f = popen(addr, "r");
			if (f) {
				char   buf[512];
				size_t n = 0;
				while ((n = fread(buf, 1, sizeof(buf), f)) != 0) {
					if (saveString)
						str.append(buf, n);
					else
						fwrite(buf, 1, n, stderr);
				}
				pclose(f);
			} else {
				if (saveString)
					str.append("addr2line failed\n");
				else
					fprintf(stderr, "addr2line failed\n");
			}
		} else {
			char sym[256];
			if (unw_get_proc_name(&cursor, sym, sizeof(sym), &offset) == 0) {
				if (saveString)
					str += tsf::fmt("(%s+0x%lx)\n", sym, offset);
				else
					fprintf(stderr, "(%s+0x%lx)\n", sym, offset);
			} else {
				if (saveString)
					str += "-- error: unable to obtain symbol name for this frame\n";
				else
					fprintf(stderr, "-- error: unable to obtain symbol name for this frame\n");
			}
		}
	}
	return str;
}

static void AbortHandler(int sig, siginfo_t* siginfo, void* context) {
	const char* name = "unknown";
	switch (sig) {
	case SIGABRT: name = "SIGABRT"; break;
	case SIGSEGV: name = "SIGSEGV"; break;
	case SIGBUS: name = "SIGBUS"; break;
	case SIGILL: name = "SIGILL"; break;
	case SIGFPE: name = "SIGFPE"; break;
	case SIGPIPE: name = "SIGPIPE"; break;
	}

	fprintf(stderr, "Caught signal %d (%s)\n", sig, name);

	// First print stacktrace without shelling out to addr2line, in case the shelling out somehow kills us.
	PrintStackTrace(false, false);

	// Second stack trace is the "risky" one, where we might end up killing ourselves.
	// But it's very nice to have the line numbers, obviously.
	PrintStackTrace(true, false);

	// Try outputting to log. Here we also do the risky option, which uses addr2line.
	//if (CrashLogger) {
	//	auto str = PrintStackTrace(true, true);
	//	CrashLogger->Error("Caught signal %v (%v). Stack Trace:\n%v", sig, name, str);
	//}

	exit(sig);
}

BMHPAL_API void SetupCrashHandler() {
	struct sigaction sa;
	sa.sa_flags     = SA_SIGINFO;
	sa.sa_sigaction = AbortHandler;
	sigemptyset(&sa.sa_mask);

	sigaction(SIGABRT, &sa, nullptr);
	sigaction(SIGSEGV, &sa, nullptr);
	sigaction(SIGBUS, &sa, nullptr);
	sigaction(SIGILL, &sa, nullptr);
	sigaction(SIGFPE, &sa, nullptr);
	sigaction(SIGPIPE, &sa, nullptr);
}

BMHPAL_API void PrintStackTrace() {
	PrintStackTrace(false, false);
	fprintf(stderr, "-------------------------------------------------------\n");
	fprintf(stderr, "--- with addr2line ------------------------------------\n");
	fprintf(stderr, "-------------------------------------------------------\n");
	PrintStackTrace(true, false);
}

#endif

#ifdef BMHPAL_PLATFORM_WINDOWS

static bool         IsBusy = false;
static std::wstring CrashDumpLocation;
static wchar_t      CrashReporterCmdString[1024]; // Place this memory here so that our stack size can be minimal

static LONG WINAPI MyExceptionHandler(EXCEPTION_POINTERS* exInfo) {
	if (IsBusy)
		return EXCEPTION_EXECUTE_HANDLER;
	// We never set IsBusy to false, so that we preserve the first unhandled exception
	IsBusy = true;

	// First location searched for by CreateProcess is the path of the current executable
	CrashReporterCmdString[0] = 0;
	wcscat(CrashReporterCmdString, L"CrashReporter.exe");
	wcscat(CrashReporterCmdString, L" ");
	wcscat(CrashReporterCmdString, CrashDumpLocation.c_str());

	HANDLE hFile = CreateFileW(CrashDumpLocation.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE) {
		MINIDUMP_EXCEPTION_INFORMATION mExInfo;
		mExInfo.ThreadId          = GetCurrentThreadId();
		mExInfo.ExceptionPointers = exInfo;
		mExInfo.ClientPointers    = TRUE;
		// It's tempting to try things such as MiniDumpWithIndirectlyReferencedMemory, but until I can get some real-world statistics,
		// I'm too afraid to include it, for fear that the size might be too large.
		MINIDUMP_TYPE type    = MiniDumpNormal;
		bool          writeOK = MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, type, &mExInfo, NULL, NULL) == TRUE;
		CloseHandle(hFile);
		if (writeOK) {
			// Launch a child process that will send the crash dump over the network
			STARTUPINFOW si;
			memset(&si, 0, sizeof(si));
			si.cb = sizeof(si);
			PROCESS_INFORMATION pi;
			bool                childOK = CreateProcessW(NULL, CrashReporterCmdString, NULL, NULL, false, 0, NULL, NULL, &si, &pi) == TRUE;
			if (!childOK)
				printf("Unable to launch CrashHelper.exe\n");
			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);
		}
	}

	MessageBoxW(nullptr, L"Something went wrong. We'll investigate this issue and fix it soon.", L"Internal Error", MB_OK);

	return EXCEPTION_EXECUTE_HANDLER;
}

BMHPAL_API void SetupCrashHandler() {
	CrashDumpLocation = towide(os::UserLocalAppData()) + L"\\crash.mdmp";
	SetUnhandledExceptionFilter(MyExceptionHandler);
}

BMHPAL_API void PrintStackTrace() {
}

#endif

#ifdef BMHPAL_PLATFORM_WASM
BMHPAL_API void PrintStackTrace() {
}
BMHPAL_API void SetupCrashHandler() {
}
#endif

} // namespace bmhpal
