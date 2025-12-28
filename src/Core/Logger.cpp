#include “Logger.hpp”
#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <mutex>
#include <cstdio>
#include <cstdarg>

namespace Titan {
namespace Logger {

namespace {
struct LoggerState {
Level minLevel = Level::Info;
LogCallback callback;
std::ofstream fileStream;
std::mutex mutex;
bool fileLoggingEnabled = false;
};

```
LoggerState* g_logger = nullptr;

const char* LevelToString(Level level) {
    switch (level) {
        case Level::Trace:   return "TRACE";
        case Level::Debug:   return "DEBUG";
        case Level::Info:    return "INFO";
        case Level::Warning: return "WARN";
        case Level::Error:   return "ERROR";
        case Level::Fatal:   return "FATAL";
        default:             return "UNKNOWN";
    }
}

std::string GetTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%H:%M:%S");
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return ss.str();
}

void LogMessage(Level level, const std::string& message) {
    if (!g_logger) return;
    if (level < g_logger->minLevel) return;
    
    std::lock_guard<std::mutex> lock(g_logger->mutex);
    
    std::string timestamp = GetTimestamp();
    std::string levelStr = LevelToString(level);
    std::string formatted = "[" + timestamp + "] [" + levelStr + "] " + message;
    
    std::cout << formatted << std::endl;
    
    if (g_logger->fileLoggingEnabled && g_logger->fileStream.is_open()) {
        g_logger->fileStream << formatted << std::endl;
        g_logger->fileStream.flush();
    }
    
    if (g_logger->callback) {
        g_logger->callback(level, message);
    }
}
```

}

void Init() {
if (g_logger == nullptr) {
g_logger = new LoggerState();
}
}

void Shutdown() {
if (g_logger) {
if (g_logger->fileStream.is_open()) {
g_logger->fileStream.close();
}
delete g_logger;
g_logger = nullptr;
}
}

void SetLevel(Level minLevel) {
if (g_logger) {
g_logger->minLevel = minLevel;
}
}

void SetCallback(LogCallback callback) {
if (g_logger) {
g_logger->callback = callback;
}
}

void EnableFileLogging(const std::string& filepath) {
if (!g_logger) return;

```
std::lock_guard<std::mutex> lock(g_logger->mutex);

if (g_logger->fileStream.is_open()) {
    g_logger->fileStream.close();
}

g_logger->fileStream.open(filepath, std::ios::out | std::ios::app);
g_logger->fileLoggingEnabled = g_logger->fileStream.is_open();
```

}

void Trace(const std::string& message) {
LogMessage(Level::Trace, message);
}

void Debug(const std::string& message) {
LogMessage(Level::Debug, message);
}

void Info(const std::string& message) {
LogMessage(Level::Info, message);
}

void Warning(const std::string& message) {
LogMessage(Level::Warning, message);
}

void Error(const std::string& message) {
LogMessage(Level::Error, message);
}

void Fatal(const std::string& message) {
LogMessage(Level::Fatal, message);
}

template<typename… Args>
void TraceF(const char* format, Args… args) {
char buffer[1024];
snprintf(buffer, sizeof(buffer), format, args…);
Trace(buffer);
}

template<typename… Args>
void DebugF(const char* format, Args… args) {
char buffer[1024];
snprintf(buffer, sizeof(buffer), format, args…);
Debug(buffer);
}

template<typename… Args>
void InfoF(const char* format, Args… args) {
char buffer[1024];
snprintf(buffer, sizeof(buffer), format, args…);
Info(buffer);
}

template<typename… Args>
void WarningF(const char* format, Args… args) {
char buffer[1024];
snprintf(buffer, sizeof(buffer), format, args…);
Warning(buffer);
}

template<typename… Args>
void ErrorF(const char* format, Args… args) {
char buffer[1024];
snprintf(buffer, sizeof(buffer), format, args…);
Error(buffer);
}

template<typename… Args>
void FatalF(const char* format, Args… args) {
char buffer[1024];
snprintf(buffer, sizeof(buffer), format, args…);
Fatal(buffer);
}

}
}
