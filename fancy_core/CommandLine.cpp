#include "fancy_core_precompile.h"
#include "CommandLine.h"

#include <ctype.h>

namespace Fancy
{
//---------------------------------------------------------------------------//
  CommandLine* CommandLine::ourInstance = nullptr;
//---------------------------------------------------------------------------//
  const CommandLine* CommandLine::CreateInstance(const char** someArguments, uint aNumArguments)
  {
    ASSERT(ourInstance == nullptr);
    ourInstance = new CommandLine(someArguments, aNumArguments);
    return ourInstance;
  }
//---------------------------------------------------------------------------//
  void CommandLine::ParseValue(const char* aValue, Argument& anArgument)
  {
    bool isNumber = true;
    bool hasDecimalPoint = false;
    const char* currVal = aValue;
    while (*currVal != '\0' && isNumber)
    {
      const char c = *currVal;
      const bool isDecimalPoint = c == '.';
      isNumber &= (isdigit(c) != 0) || (!hasDecimalPoint && isDecimalPoint);
      hasDecimalPoint |= isDecimalPoint;
      ++currVal;
    }

    if (isNumber)
    {
      const float number = static_cast<float>(atof(aValue));
      anArgument.myType = ARGTYPE_NUMBER;
      anArgument.myNumber = number;
    }
    else
    {
      anArgument.myType = ARGTYPE_STRING;
    }

    anArgument.myString = aValue;
  }
//---------------------------------------------------------------------------//
  CommandLine::CommandLine(const char** someArguments, uint aNumArguments)
  {
    if (aNumArguments > 1)  // First argument will be the exe path
    {
      for (uint i = 1u; i < aNumArguments; ++i)
      {
        const char* argStr = someArguments[i];
        if (argStr[0] == '-')
        {
          argStr++;  // skip '-'
          ASSERT(strlen(argStr) <= kMaxArgSize);

          Argument argument;
          argument.myName = argStr;
          myArguments.push_back(argument);
        }
        else  // This is a value for a preceeding argument
        {
          ASSERT(!myArguments.empty());
          ASSERT(strlen(argStr) <= kMaxValSize);

          Argument& lastArg = myArguments[myArguments.size() - 1];
          ASSERT(lastArg.myType == ARGTYPE_NO_VALUE, "Malformed command line string. Two values encountered for the same type. Did you forget to add a '-' before the next argument?");

          ParseValue(argStr, lastArg);
        }
      }
    }
  }
//---------------------------------------------------------------------------//
  bool CommandLine::FindArgument(const char* aArgument, Argument& anArgument) const
  {
    if (myArguments.empty())
      return false;

    for (uint i = 0u; i < myArguments.size(); ++i)
    {
      if (myArguments[i].myName == aArgument)
      {
        anArgument = myArguments[i];
        return true;
      }
    }

    return false;
  }
//---------------------------------------------------------------------------//
  bool CommandLine::HasArgument(const char* aArgument) const
  {
    Argument arg;
    return FindArgument(aArgument, arg);
  }
//---------------------------------------------------------------------------//
  bool CommandLine::HasStringValue(const char* anArgument) const
  {
    Argument arg;
    if (!FindArgument(anArgument, arg))
      return false;

    return arg.myType == ARGTYPE_STRING;
  }
//---------------------------------------------------------------------------//
  bool CommandLine::HasFloatValue(const char* anArgument) const
  {
    Argument arg;
    if (!FindArgument(anArgument, arg))
      return false;

    return arg.myType == ARGTYPE_NUMBER;
  }
//---------------------------------------------------------------------------//
  const char* CommandLine::GetStringValue(const char* anArgument) const
  {
    Argument arg;
    if (!FindArgument(anArgument, arg))
      return "";

    if (arg.myType != ARGTYPE_STRING)
      return "";

    return arg.myString.GetBuffer();
  }
//---------------------------------------------------------------------------//
  float CommandLine::GetFloatValue(const char* anArgument) const
  {
    Argument arg;
    if (!FindArgument(anArgument, arg))
      return 0.0f;

    if (arg.myType != ARGTYPE_NUMBER)
      return 0.0f;

    return arg.myNumber;
  }
//---------------------------------------------------------------------------//
}

