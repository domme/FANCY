#pragma once

#include "FancyCoreDefines.h"
#include "Debug/Log.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  template<uint BufferSize>
  struct StaticString
  {
    StaticString()
      : myBuffer{ 0u }
      , myNextFree(0u)
    {
    }

    explicit StaticString(const char* aFormatString, ...)
      : myBuffer{ 0u }
      , myNextFree(0u)
    {
      va_list args;
      va_start(args, aFormatString);
      AppendInternal(aFormatString, args);
      va_end(args);
    }

    const char* Format(const char* aFormatString, ...)
    {
      myNextFree = 0u;

      va_list args;
      va_start(args, aFormatString);
      AppendInternal(aFormatString, args);
      va_end(args);
      return myBuffer;
    }

    const char* Append(const char* aFormatString, ...)
    {
      va_list args;
      va_start(args, aFormatString);
      AppendInternal(aFormatString, args);
      va_end(args);
      return myBuffer;
    }

    StaticString<BufferSize>& operator=(const char* aStr)
    {
      myNextFree = 0;
      myBuffer[myNextFree] = '\0';

      const char* currStr = aStr;
      while (*currStr != '\0')
      {
        myBuffer[myNextFree] = *currStr;
        ++currStr;
        ++myNextFree;
        ASSERT(!IsFull());
      }

      myBuffer[myNextFree] = '\0';

      return *this;
    }

    template<uint BufferSize2>
    StaticString<BufferSize>& operator=(const StaticString<BufferSize2>& aStr)
    {
      if (BufferSize < aStr.Size() + 1)
        LOG_WARNING("Truncating StaticString on copy-assignment");

      uint numCharsToCopy = glm::min(BufferSize - 1, aStr.Size());
      memcpy(myBuffer, aStr.myBuffer, numCharsToCopy);

      myNextFree = numCharsToCopy;
      myBuffer[myNextFree] = '\0';
      return *this;
    }

    bool operator==(const char* aStr) const 
    {
      uint len = (uint) strlen(aStr);
      if (len != Size())
        return false;

      for (uint i = 0u; i < len; ++i)
      {
        if (aStr[i] != myBuffer[i])
          return false;
      }

      return true;
    }

    template<uint BufferSize2>
    bool operator==(const StaticString<BufferSize2>& aStr) const
    {
      uint len = aStr.Size();
      if (len != Size())
        return false;

      for (uint i = 0u; i < len; ++i)
      {
        if (aStr[i] != myBuffer[i])
          return false;
      }

      return true;
    }

    uint Size() const { return myNextFree; }
    bool IsEmpty() const { return myNextFree == 0u; }
    bool IsFull() const { return myNextFree == BufferSize; }

    const char* GetBuffer() const { return myBuffer; }
    char* GetBuffer() { return myBuffer; }

    operator const char*() const { return myBuffer; }

  private:
    void AppendInternal(const char* aFormatString, va_list args)
    {
      const uint remainingSize = BufferSize - myNextFree;
      ASSERT(remainingSize > 0);
      const int numCharsNeeded = vsnprintf(myBuffer + myNextFree, static_cast<size_t>(remainingSize), aFormatString, args) + 1;
      ASSERT(numCharsNeeded-1 <= (int)remainingSize);
      myNextFree += numCharsNeeded-1;
    }

    char myBuffer[BufferSize];
    uint myNextFree;
  };
//---------------------------------------------------------------------------//
  template<uint PoolSize>
  struct CircularStringBufferSized
  {
    const char* Format(const char* aFmt, ...)
    {
      if (myNextFree >= PoolSize)
        myNextFree = 0;  // Wrap around

      const uint freeSize = PoolSize - myNextFree;
      char* bufferStart = myBuffer + myNextFree;

      va_list args;
      va_start(args, aFmt);
      const int numCharsNeeded = vsnprintf(bufferStart, freeSize, aFmt, args) + 1;
      va_end(args);

      myNextFree += numCharsNeeded;
      return bufferStart;
    }

    void Reset() { myNextFree = 0; }

  private:
    uint myNextFree = 0;
    char myBuffer[PoolSize] = {};
  };
//---------------------------------------------------------------------------//
  using CircularStringBufferSmall = CircularStringBufferSized<1024>;
  using CircularStringBuffer = CircularStringBufferSized<4096>;
  using CircularStringBufferLarge = CircularStringBufferSized<8192>;
  using StaticFilePath = StaticString<260>;

}
//---------------------------------------------------------------------------//
