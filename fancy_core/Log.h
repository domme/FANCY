#pragma once

#include "WindowsIncludes.h"  // For OutputDebugString()

#include <iostream>
#include <cstdarg>

namespace Fancy {
//---------------------------------------------------------------------------//
  inline void Log(const char* aMessageFormat, ...)
  {
    va_list args;
    va_start(args, aMessageFormat);

    const int messageBufferSize = vsnprintf(nullptr, 0u, aMessageFormat, args) + 1;
    const int logBufferSizeBytes = messageBufferSize + 2u; // \n \0

    char* logBuffer = (char*)alloca(logBufferSizeBytes);

    int offset = 0;
    offset += vsnprintf(logBuffer + offset, messageBufferSize, aMessageFormat, args);
    logBuffer[offset++] = '\n';
    logBuffer[offset] = '\0';

    va_end(args);

    OutputDebugStringA(logBuffer);
    std::cout << logBuffer;
  }

  inline void LogWithLocation(const char* aSeverity, const char* aFile, const int aLine, const char* aMessageFormat, ...)
  {
    va_list args;
    va_start(args, aMessageFormat);

    const int severityBufferSize = snprintf(nullptr, 0u, "%s: ", aSeverity) + 1;
    const int messageBufferSize = vsnprintf(nullptr, 0u, aMessageFormat, args) + 1;
    const int fileBufferSize = snprintf(nullptr, 0u, "File: %s (%i)", aFile, aLine) + 1;

    const int logBufferSizeBytes =
      severityBufferSize + messageBufferSize
      + fileBufferSize + 1u + 1u + 1u + 1u + 1u;  // (...) \0 \n 

    char* logBuffer = (char*)alloca(logBufferSizeBytes);

    int offset = 0;
    offset += snprintf(logBuffer + offset, severityBufferSize, "%s: ", aSeverity);
    offset += vsnprintf(logBuffer + offset, messageBufferSize, aMessageFormat, args);
    logBuffer[offset++] = ' ';
    logBuffer[offset++] = '(';
    offset += snprintf(logBuffer + offset, fileBufferSize, "File: %s (%i)", aFile, aLine);
    logBuffer[offset++] = ')';
    logBuffer[offset++] = '\n';
    logBuffer[offset] = '\0';

    va_end(args);

    OutputDebugStringA(logBuffer);
    std::cout << logBuffer;
  }
//---------------------------------------------------------------------------//
  #define LOG(aFormat, ...)         Log(aFormat, ##__VA_ARGS__)
  #define LOG_DEBUG(aFormat, ...)   Log(aFormat, ##__VA_ARGS__)
  #define LOG_INFO(aFormat, ...)    LogWithLocation("Info", __FILE__, __LINE__,  aFormat, ##__VA_ARGS__)
  #define LOG_WARNING(aFormat, ...) LogWithLocation("Warning", __FILE__, __LINE__, aFormat, ##__VA_ARGS__)
  #define LOG_ERROR(aFormat, ...)   LogWithLocation("Error", __FILE__, __LINE__, aFormat, ##__VA_ARGS__)
//---------------------------------------------------------------------------//
  #define ASSERT(aValue, ...) { if(!(aValue)) { LOG_ERROR("", ##__VA_ARGS__); assert(aValue); } }
  #define STATIC_ASSERT( condition, message ) { static_assert(condition, message); }
//---------------------------------------------------------------------------//
}
