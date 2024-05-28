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

  public class FancyTargets 
  {
    public static Target[] ApplicationTargets
    {
        get
        {
          return new Target[]{new Target( Platform.win64, DevEnv.vs2022, Optimization.Debug | Optimization.Release)};
        }
    }

    public static Target[] LibTargets
    {
        get
        {
          return new Target[]{new Target( Platform.win64, DevEnv.vs2022, Optimization.Debug | Optimization.Release, OutputType.Lib)};
        }
    }
  }
}
