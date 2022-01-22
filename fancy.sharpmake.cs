using Sharpmake;
using Fancy;

namespace Fancy
{
  [Sharpmake.Generate]
  public class FancySolution : Solution
  {
    public FancySolution()
    {
      Name = "Fancy_sharpmake";

      AddTargets(new Target(Platform.win64,
        DevEnv.vs2019,
        Optimization.Debug | Optimization.Release));
    }

    [Sharpmake.Configure]
    public void ConfigureAll(Configuration conf, Target target)
    {
      conf.SolutionPath = @"[solution.SharpmakeCsPath]";
      Globals.SolutionPath = conf.SolutionPath;

      conf.AddProject<FancyCoreProject>(target);
    }
  }
}

public static class Main
{
  [Sharpmake.Main]
  public static void SharpmakeMain(Sharpmake.Arguments arguments)
  {
    // Tells Sharpmake to generate the solution described by
    // BasicsSolution.
    arguments.Generate<Fancy.FancySolution>();
  }
}