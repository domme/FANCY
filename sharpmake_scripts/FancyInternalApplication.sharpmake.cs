using System.IO;
using Sharpmake;

namespace Fancy
{
  public abstract class FancyInternalApplication : FancyApplication
  {
    public FancyInternalApplication()
    {
      FancyRootPath = Path.Combine(SourceRootPath, "/../");
      FancyExternalBasePath = Path.Combine(SourceRootPath, "/../external/");
    }

    public override void ConfigureAll(Configuration conf, Target target)
    {
      base.ConfigureAll(conf, target);

      conf.TargetPath = @"[conf.ProjectPath]/../bin/[target.Platform]/[target.Optimization]/[project.Name]";
      conf.TargetCopyFilesPath = conf.TargetPath;
      conf.IntermediatePath = @"[conf.ProjectPath]/../_build_temp/[target.Platform]/[target.Optimization]/[project.Name]";
    }
  }
}

