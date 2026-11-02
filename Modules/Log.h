#ifndef __LOG_H__
#define __LOG_H__

#include <cstdio>
#include <cstdarg>
#include <vector>
#include <string>
std::vector<std::string> GetLogMessages();
void ClearLogMessages();

#define LOG(format, ...) Log(__FILE__, __LINE__, format, ##__VA_ARGS__)

void Log(const char file[], int line, const char* format, ...);

#endif  // __LOG_H__