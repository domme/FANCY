#include "fancy_core_precompile.h"
#include "CommandLine.h"

#include <ctype.h>

namespace Fancy
{
  CommandLine* CommandLine::ourInstance = nullptr;

  const CommandLine* CommandLine::CreateInstance(const char** someArguments, uint aNumArguments)
  {
    ASSERT(ourInstance == nullptr);
    ourInstance = new CommandLine(someArguments, aNumArguments);
    return ourInstance;
  }

  CommandLine::CommandLine(const char** someArguments, uint aNumArguments)
  {
    if (aNumArguments > 1)  // First argument will be the exe path
    {
      for (uint i = 1u; i < aNumArguments; ++i)
      {
        const char* arg = someArguments[i];
        if (arg[0] == '-')
        {
          arg++;
          ASSERT(strlen(arg) <= kMaxArgSize);

          Argument argument;
          argument.myName = StaticString<kMaxArgSize>(arg);
          myArguments.push_back(argument);
        }
        else
        {
          ASSERT(!myArguments.empty());

          Argument& lastArg = myArguments[myArguments.size() - 1];

          ASSERT(strlen(arg) <= kMaxValSize);
          lastArg.myValue = StaticString<kMaxValSize>(arg);
        }
      }
    }
  }

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

  bool CommandLine::HasArgument(const char* aArgument) const
  {
    Argument arg;
    return FindArgument(aArgument, arg);
  }

  bool CommandLine::GetValue(const char* aArgument, int& aVal) const
  {
    Argument arg;
    if (!FindArgument(aArgument, arg))
      return false;

    bool 
    
  }

  bool CommandLine::GetValue(const char* aArgument, String& aVal) const
  {
  }

  bool CommandLine::GetValue(const char* aArgument, float& aVal) const
  {
  }
}

