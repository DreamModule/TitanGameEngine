#pragma once

#include <string>
#include <functional>

namespace Titan {
namespace Logger {

enum class Level {
Trace,
Debug,
Info,
Warning,
Error,
Fatal
};

using LogCallback = std::function<void(Level, const std::string&)>;

void Init();
void Shutdown();

void SetLevel(Level minLevel);
void SetCallback(LogCallback callback);
void EnableFileLogging(const std::string& filepath);

void Trace(const std::string& message);
void Debug(const std::string& message);
void Info(const std::string& message);
void Warning(const std::string& message);
void Error(const std::string& message);
void Fatal(const std::string& message);

template<typename… Args>
void TraceF(const char* format, Args… args);

template<typename… Args>
void DebugF(const char* format, Args… args);

template<typename… Args>
void InfoF(const char* format, Args… args);

template<typename… Args>
void WarningF(const char* format, Args… args);

template<typename… Args>
void ErrorF(const char* format, Args… args);

template<typename… Args>
void FatalF(const char* format, Args… args);

}
}
