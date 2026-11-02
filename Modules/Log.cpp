#include "Log.h"
#include <iostream>
#include <deque>
#include <mutex>
namespace {
std::deque<std::string>& Messages() { static std::deque<std::string> messages; return messages; }
std::mutex& LogMutex() { static std::mutex mutex; return mutex; }
}
void Log(const char file[], int line, const char* format, ...) {
    char buffer[4096]; va_list args;
    va_start(args, format); vsnprintf(buffer, sizeof(buffer), format, args); va_end(args);
    std::lock_guard<std::mutex> guard(LogMutex());
    auto& messages = Messages();
    if (messages.size() >= 2000) messages.pop_front();
    std::string message(buffer);
    for (char& c : message) if (c == '\n' || c == '\r') c = ' ';
    messages.push_back(std::move(message));
    std::cerr << file << "(" << line << "): " << buffer << std::endl;
}
std::vector<std::string> GetLogMessages() {
    std::lock_guard<std::mutex> guard(LogMutex());
    return {Messages().begin(), Messages().end()};
}
void ClearLogMessages() {
    std::lock_guard<std::mutex> guard(LogMutex()); Messages().clear();
}
