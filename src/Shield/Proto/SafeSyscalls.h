// FIX #5: Safe syscall resolution with SEH fallback

#ifndef TITAN_SAFE_SYSCALLS_H
#define TITAN_SAFE_SYSCALLS_H

#ifdef _WIN32
#include <windows.h>
#include <winternl.h>

namespace TitanShield {
namespace SafeSyscall {

// Fallback to regular WinAPI if syscall resolution fails
class SafeResolver {
private:
static bool use_direct_syscalls_;

public:
template<typename Func>
static Func try_get_syscall(const char* function_name) {
if (!use_direct_syscalls_) return nullptr;

```
    __try {
        // Try Halo's Gate resolution
        return DirectSyscall::SyscallResolver::get_syscall_func<Func>(function_name);
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        // Resolution failed (incompatible Windows version)
        use_direct_syscalls_ = false;
        return nullptr;
    }
}

static bool are_syscalls_available() {
    return use_direct_syscalls_;
}
```

};

bool SafeResolver::use_direct_syscalls_ = true;

// Safe wrapper for NtQueryInformationProcess
class SafeNtQuery {
private:
static DirectSyscall::pNtQueryInformationProcess syscall_func_;
static bool initialized_;
static std::mutex init_mutex_;

```
// Fallback WinAPI version
typedef NTSTATUS (WINAPI *pNtQueryInformationProcessWinAPI)(
    HANDLE, DWORD, PVOID, ULONG, PULONG
);

static pNtQueryInformationProcessWinAPI winapi_func_;
```

public:
static NTSTATUS query(HANDLE process, DWORD info_class, PVOID buffer, ULONG size, PULONG ret_len) {
std::lock_guard<std::mutex> lock(init_mutex_);

```
    if (!initialized_) {
        // Try direct syscall first
        syscall_func_ = SafeResolver::try_get_syscall<DirectSyscall::pNtQueryInformationProcess>(
            "NtQueryInformationProcess"
        );
        
        // Fallback to WinAPI if syscall failed
        if (!syscall_func_) {
            HMODULE ntdll = GetModuleHandleA("ntdll.dll");
            if (ntdll) {
                winapi_func_ = reinterpret_cast<pNtQueryInformationProcessWinAPI>(
                    GetProcAddress(ntdll, "NtQueryInformationProcess")
                );
            }
        }
        
        initialized_ = true;
    }
    
    // Prefer direct syscall, fallback to WinAPI
    if (syscall_func_) {
        __try {
            return syscall_func_(process, info_class, buffer, size, ret_len);
        }
        __except(EXCEPTION_EXECUTE_HANDLER) {
            // Syscall failed, disable and use WinAPI
            syscall_func_ = nullptr;
        }
    }
    
    if (winapi_func_) {
        return winapi_func_(process, info_class, buffer, size, ret_len);
    }
    
    return STATUS_NOT_IMPLEMENTED;
}
```

};

DirectSyscall::pNtQueryInformationProcess SafeNtQuery::syscall_func_ = nullptr;
SafeNtQuery::pNtQueryInformationProcessWinAPI SafeNtQuery::winapi_func_ = nullptr;
bool SafeNtQuery::initialized_ = false;
std::mutex SafeNtQuery::init_mutex_;

} // namespace SafeSyscall
} // namespace TitanShield

#endif // _WIN32
#endif

// Replace in AntiReverseEngineering::DetectDebuggerNtQuery (proto.cpp):
// OLD: auto NtQueryInformationProcess = DirectSyscall::SyscallCache::get_NtQueryInformationProcess();
//      status = NtQueryInformationProcess(…);
//
// NEW: status = SafeSyscall::SafeNtQuery::query(
//          GetCurrentProcess(), 7, &debugPort, sizeof(debugPort), NULL
//      );
