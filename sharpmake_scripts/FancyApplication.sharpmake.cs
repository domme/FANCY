using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;
using Sharpmake;

namespace Fancy
{
  public abstract class FancyApplication : Project
  {
    public string FancyRootPath;
    public string FancyExternalBasePath;

    public FancyApplication()
    {
      AddTargets(FancyTargets.ApplicationTargets);

      SourceRootPath = @"[project.SharpmakeCsPath]";
    }
    public virtual void ConfigureAll(Configuration conf, Target target)
    {
      conf.Options.Add(Options.Vc.Linker.SubSystem.Windows);

      conf.ProjectPath = SourceRootPath;

      if (target.Optimization == Optimization.Debug)
        conf.Options.Add(Options.Vc.Compiler.RuntimeLibrary.MultiThreadedDebugDLL);
      else
        conf.Options.Add(Options.Vc.Compiler.RuntimeLibrary.MultiThreadedDLL);

      conf.Output = Configuration.OutputType.Exe;

      conf.IncludePaths.Add("[project.SourceRootPath]");
      conf.IncludePaths.Add(FancyExternalBasePath);
      conf.LibraryPaths.Add(FancyExternalBasePath);

      // WinPixEventRuntime
      conf.LibraryPaths.Add(FancyExternalBasePath + @"WinPixEventRuntime/bin/" + Util.PlatformToArchitecture(target.Platform));
      conf.LibraryFiles.Add(new string[] { "WinPixEventRuntime.lib" });
      conf.TargetCopyFiles.Add(FancyExternalBasePath + @"WinPixEventRuntime/bin/" + Util.PlatformToArchitecture(target.Platform) + "/WinPixEventRuntime.dll");

      conf.AddPrivateDependency<FancyCoreProject>(target);
      conf.AddPrivateDependency<FancyImGuiProject>(target);

      conf.AddPrivateDependency<Assimp>(target);
      conf.AddPrivateDependency<EASTL>(target);
      conf.AddPrivateDependency<GLM>(target);
      conf.AddPrivateDependency<XXHash>(target);
      conf.AddPrivateDependency<DXC>(target);
      conf.AddPrivateDependency<Direct3D12AgilitySdk>(target);
    }
  }
}

