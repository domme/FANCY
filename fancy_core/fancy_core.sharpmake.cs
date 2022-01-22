using Sharpmake;
using System;
using System.IO;

namespace Fancy
{
  [Sharpmake.Generate]
  public class FancyCoreProject : Project
  {
    public FancyCoreProject()
    {
      Name = "fancy_core_sharpmake";

      AddTargets(new Target(Platform.win64, 
        DevEnv.vs2019, 
        Optimization.Debug | Optimization.Release, 
        OutputType.Lib));

      SourceRootPath = @"[project.SharpmakeCsPath]";
    }

    [Sharpmake.Configure]
    public void ConfigureAll(Configuration conf, Target target)
    {
      conf.Output = Configuration.OutputType.Lib;
      conf.ProjectPath = @"[project.SharpmakeCsPath]";
      conf.TargetLibraryPath = Path.Combine(Globals.SolutionPath, @"/lib/[target.Platform]/[target.Optimization]");
      conf.IntermediatePath = @"[project.SharpmakeCsPath]/../_build_temp/[target.Platform]/[target.Optimization]/[project.Name]";
    }
  }
}