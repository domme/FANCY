using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;
using Sharpmake;

namespace Fancy
{
    public abstract class FancyExternalApplication : FancyApplication
  {
    public FancyExternalApplication()
    {
      FancyRootPath = Path.Combine(SourceRootPath, "/../Fancy/");
      FancyExternalBasePath = Path.Combine(SourceRootPath, "/../Fancy/external/");
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

