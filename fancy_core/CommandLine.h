#pragma once

namespace Fancy
{
  class CommandLine
  {
  public:
    static const CommandLine* CreateInstance(const char** someArguments, uint aNumArguments);
    static const CommandLine* GetInstance() { return ourInstance; }

    bool HasArgument(const char* aArgument) const;
    bool HasStringValue(const char* anArgument) const;
    bool HasFloatValue(const char* anArgument) const;

    const char* GetStringValue(const char* anArgument) const;
    float GetFloatValue(const char* anArgument) const;

  private:
    enum
    {
      kMaxArgSize = 32,
      kMaxValSize = 64,
    };

    enum ArgumentType
    {
      ARGTYPE_NO_VALUE = 0,
      ARGTYPE_NUMBER,
      ARGTYPE_STRING
    };

    struct Argument
    {
      StaticString<kMaxArgSize> myName;
      StaticString<kMaxValSize> myString;
      ArgumentType myType = ARGTYPE_NO_VALUE;
      float myNumber = 0.0f;
    };

    CommandLine(const char** someArguments, uint aNumArguments);
    const Argument* FindArgument(const char* aArgument) const;
    void ParseValue(const char* aValue, Argument& anArgument);

    static CommandLine* ourInstance;

    DynamicArray<Argument> myArguments;
  };
}



