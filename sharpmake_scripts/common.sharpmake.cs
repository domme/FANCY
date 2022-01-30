using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Sharpmake;

namespace Fancy
{
  public static class Util
  {
    public static string PlatformToArchitecture(Sharpmake.Platform aPlatform)
    {
      switch(aPlatform)
      {
        case Platform.win64:
          return "x64";
        case Platform.win32:
          return "x86";
        default:
          return aPlatform.ToString();
      }
    }
  }
  
}
