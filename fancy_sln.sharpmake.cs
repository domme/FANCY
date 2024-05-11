using Sharpmake;

[module: Sharpmake.Include("sharpmake_scripts/common.sharpmake.cs")]
[module: Sharpmake.Include("sharpmake_scripts/FancyApplication.sharpmake.cs")]
[module: Sharpmake.Include("sharpmake_scripts/FancyExternalApplication.sharpmake.cs")]
[module: Sharpmake.Include("sharpmake_scripts/FancyInternalApplication.sharpmake.cs")]
[module: Sharpmake.Include("sharpmake_scripts/FancyLibProject.sharpmake.cs")]
[module: Sharpmake.Include("fancy_core/fancy_core.sharpmake.cs")]
[module: Sharpmake.Include("fancy_imgui/fancy_imgui.sharpmake.cs")]
[module: Sharpmake.Include("Tests/Tests.sharpmake.cs")]

namespace Fancy
{
  [Sharpmake.Generate]
  public class FancySln : Solution
  {
    public FancySln()
    {
      Name = "Fancy";

      AddTargets(new Target(
                 Platform.win64,
                 DevEnv.vs2022,
                 Optimization.Debug | Optimization.Release));
    }

    [Sharpmake.Configure]
    public void ConfigureAll(Configuration conf, Target target)
    {
      conf.SolutionFileName = "[solution.Name]_[target.DevEnv]_[target.Platform]";
      conf.SolutionPath = @"[solution.SharpmakeCsPath]";
      conf.AddProject<TestsProject>(target);
      conf.AddProject<FancyCoreProject>(target);
      conf.AddProject<FancyImGuiProject>(target);
    }
  }

  public static class Main
  {
    [Sharpmake.Main]
    public static void SharpmakeMain(Sharpmake.Arguments arguments)
    {
      KitsRootPaths.SetUseKitsRootForDevEnv(DevEnv.vs2022, KitsRootEnum.KitsRoot10, Options.Vc.General.WindowsTargetPlatformVersion.Latest);
      arguments.Generate<Fancy.FancySln>();
    }
  }
}