#pragma once

namespace Fancy
{
  class CommandLine
  {
  public:
    static const CommandLine* CreateInstance(const char** someArguments, uint aNumArguments);
    static const CommandLine* GetInstance() { return ourInstance; }

    bool HasArgument(const char* aArgument) const;

    bool GetValue(const char* aArgument, int& aVal) const;
    bool GetValue(const char* aArgument, float& aVal) const;
    bool GetValue(const char* aArgument, String& aVal) const;

  private:
    enum
    {
      kMaxArgSize = 32,
      kMaxValSize = 64,
    };

    struct Argument
    {
      StaticString<kMaxArgSize> myName;
      StaticString<kMaxValSize> myValue;
    };

    bool FindArgument(const char* aArgument, Argument& anArgument) const;

    CommandLine(const char** someArguments, uint aNumArguments);

    static CommandLine* ourInstance;

    DynamicArray<Argument> myArguments;
  };
}



