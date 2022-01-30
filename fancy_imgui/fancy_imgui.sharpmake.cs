using Sharpmake;
using System;
using System.IO;

namespace Fancy
{
  [Sharpmake.Generate]
  public class FancyImGuiProject : FancyLibProject
  {
    public FancyImGuiProject()
    {
      Name = "fancy_imgui";
    }

    [Sharpmake.Configure]
    public override void ConfigureAll(Configuration conf, Target target)
    {
      base.ConfigureAll(conf, target);

      conf.AddPrivateDependency<FancyCoreProject>(target);
    }
  }
}