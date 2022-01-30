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
      AddTargets(new Target(Platform.win64,
        DevEnv.vs2019,
        Optimization.Debug | Optimization.Release));

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

      // DX12 Agility
      conf.TargetCopyFiles.Add(FancyExternalBasePath + @"DX12_Agility_SDK/build/native/bin/" + Util.PlatformToArchitecture(target.Platform) + "/D3D12Core.dll");
      conf.TargetCopyFiles.Add(FancyExternalBasePath + @"DX12_Agility_SDK/build/native/bin/" + Util.PlatformToArchitecture(target.Platform) + "/d3d12SDKLayers.dll");

      // DX12
      conf.LibraryFiles.Add(new string[] { "dxgi.lib", "d3d12.lib" });

      conf.AddPrivateDependency<FancyCoreProject>(target);
      conf.AddPrivateDependency<FancyImGuiProject>(target);
    }
  }
}

