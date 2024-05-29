using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;
using Sharpmake;

namespace Fancy
{
  [Sharpmake.Generate]
  public class FancyLibProject : Project
  {
    public string ExternalBasePath;

    public FancyLibProject()
    {
      AddTargets(FancyTargets.LibTargets);

      SourceRootPath = @"[project.SharpmakeCsPath]";
      ExternalBasePath = Path.Combine(SourceRootPath, "/../external/");
    }
    public virtual void ConfigureAll(Configuration conf, Target target)
    {
      if (target.Optimization == Optimization.Debug)
        conf.Options.Add(Options.Vc.Compiler.RuntimeLibrary.MultiThreadedDebugDLL);
      else
        conf.Options.Add(Options.Vc.Compiler.RuntimeLibrary.MultiThreadedDLL);

      conf.Output = Configuration.OutputType.Lib;
      conf.ProjectPath = @"[project.SharpmakeCsPath]";
      conf.TargetLibraryPath = @"[project.SharpmakeCsPath]/../lib/[target.Platform]/[target.Optimization]/[project.Name]";
      conf.IntermediatePath = @"[project.SharpmakeCsPath]/../_build_temp/[target.Platform]/[target.Optimization]/[project.Name]";

      conf.IncludePaths.Add("[project.SourceRootPath]");
      conf.IncludePaths.Add(ExternalBasePath);
      conf.LibraryPaths.Add(ExternalBasePath);

      // WinPixEventRuntime
      conf.IncludePaths.Add(ExternalBasePath + "WinPixEventRuntime/Include/");

      conf.AddPrivateDependency<Assimp>(target);
      conf.AddPrivateDependency<EASTL>(target);
      conf.AddPrivateDependency<GLM>(target);
      conf.AddPrivateDependency<XXHash>(target);
      conf.AddPrivateDependency<DXC>(target);
      conf.AddPrivateDependency<Direct3D12AgilitySdk>(target);
    }
  }
}

