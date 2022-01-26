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
  public class FancyExternalApplication : Project
  {
    public string FancyExternalBasePath;

    public FancyExternalApplication()
    {
      AddTargets(new Target(Platform.win64,
        DevEnv.vs2019,
        Optimization.Debug | Optimization.Release));
      
      SourceRootPath = @"[project.SharpmakeCsPath]";
      FancyExternalBasePath = Path.Combine(SourceRootPath, "/../Fancy/external/");
    }
    public virtual void ConfigureAll(Configuration conf, Target target)
    {
      conf.ProjectPath = SourceRootPath;

      conf.Output = Configuration.OutputType.Exe;
      conf.TargetPath = @"[conf.ProjectPath]/../bin/[target.Platform]/[target.Optimization]/[project.Name]";
      conf.TargetCopyFilesPath = conf.TargetPath;
      conf.IntermediatePath = @"[conf.ProjectPath]/../_build_temp/[target.Platform]/[target.Optimization]/[project.Name]";

      conf.IncludePaths.Add("[project.SourceRootPath]");
      conf.IncludePaths.Add(FancyExternalBasePath);
      conf.LibraryPaths.Add(FancyExternalBasePath);

      // DXC
      conf.LibraryPaths.Add(FancyExternalBasePath + "dxc/lib/");
      conf.LibraryFiles.Add("dxcompiler.lib");
      conf.TargetCopyFiles.Add(FancyExternalBasePath + @"dxc/bin/" + Util.PlatformToArchitecture(target.Platform) + "/dxcompiler.dll");
      conf.TargetCopyFiles.Add(FancyExternalBasePath + @"dxc/bin/" + Util.PlatformToArchitecture(target.Platform) + "/dxil.dll");

      // GLM
      conf.IncludePaths.Add(FancyExternalBasePath + "glm/");

      // Vulkan
      conf.LibraryPaths.Add("%VK_SDK_PATH%/Lib/");
      conf.LibraryFiles.Add(new string[] { "vulkan-1.lib", "VkLayer_utils.lib", "shaderc_combined.lib" });

      conf.AddPrivateDependency<FancyCoreProject>(target);
      conf.AddPrivateDependency<FancyImGuiProject>(target);
    }
  }
}

