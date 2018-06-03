#pragma once
#include "FC_String.h"

namespace Fancy {
//---------------------------------------------------------------------------//
    // C-style logging (downside: no auto-conversion from std::string to const char*. Argument would need to be str.c_str() always...
  inline void LogC(const char* aSeverity, const char* aFile, const int aLine, const char* aMessageFormat, ...)
  {
    va_list args;
    va_start(args, aMessageFormat);

    const int severityBufferSize = snprintf(nullptr, 0u, "%s: ", aSeverity) + 1;
    const int messageBufferSize = vsnprintf(nullptr, 0u, aMessageFormat, args) + 1;
    const int fileBufferSize = snprintf(nullptr, 0u, "File: %s (%i)", aFile, aLine) + 1;

    const int logBufferSizeBytes =
      severityBufferSize + messageBufferSize + 1u // \n
      + fileBufferSize + 1u + 1u;  // \0 \n 

    char* logBuffer = (char*)alloca(logBufferSizeBytes);

    int offset = 0;
    offset += snprintf(logBuffer + offset, severityBufferSize, "%s: ", aSeverity);
    offset += vsnprintf(logBuffer + offset, messageBufferSize, aMessageFormat, args);
    logBuffer[offset++] = '\n';
    offset += snprintf(logBuffer + offset, fileBufferSize, "File: %s (%i)", aFile, aLine);
    logBuffer[offset++] = '\n';
    logBuffer[offset] = '\0';

    va_end(args);

    OutputDebugStringA(logBuffer);
    std::cout << logBuffer;
  }
//---------------------------------------------------------------------------//
   #define C_LOG_INFO(aFormat, ...)    LogC("Info", __FILE__, __LINE__, aFormat, ##__VA_ARGS__)
   #define C_LOG_WARNING(aFormat, ...) LogC("Warning", __FILE__, __LINE__, aFormat, ##__VA_ARGS__)
   #define C_LOG_ERROR(aFormat, ...)   LogC("Error", __FILE__, __LINE__, aFormat, ##__VA_ARGS__)
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  inline void Log(const char* aSeverity, const char* aFile, const int aLine, bool aBreak, const std::string& aMessage)
  {
    const std::string& logOutput = Fancy::StringFormat("%: % \n % (%) \n", aSeverity, aMessage, aFile, aLine);
    std::cout << logOutput;
    OutputDebugStringA(logOutput.c_str());

    if (aBreak)
      DebugBreak();
  }
//---------------------------------------------------------------------------//
  inline void Log_Debug(const std::string& aMessage)
  {
    std::string logOutput = aMessage + "\n";
    std::cout << logOutput;
    OutputDebugStringA(logOutput.c_str());
  }
//---------------------------------------------------------------------------//
  #define LOG_DEBUG(aFormat, ...)   Log_Debug(StringFormat(aFormat, ##__VA_ARGS__))
  #define LOG_INFO(aFormat, ...)    Log("Info", __FILE__, __LINE__, false, StringFormat(aFormat, ##__VA_ARGS__))
  #define LOG_WARNING(aFormat, ...) Log("Warning", __FILE__, __LINE__, false, StringFormat(aFormat, ##__VA_ARGS__))
  #define LOG_ERROR(aFormat, ...)   Log("Error", __FILE__, __LINE__, true, StringFormat(aFormat, ##__VA_ARGS__))
//---------------------------------------------------------------------------//
  #define ASSERT(aValue, ...) { if(!(aValue)) LOG_ERROR("", ##__VA_ARGS__); }
  #define STATIC_ASSERT( condition, message ) { static_assert(condition, message); }
//---------------------------------------------------------------------------//
}