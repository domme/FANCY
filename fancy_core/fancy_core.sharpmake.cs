using Sharpmake;
using System;
using System.IO;

namespace Fancy
{
  [Sharpmake.Generate]
  public class FancyCoreProject : FancyLibProject
  {
    public FancyCoreProject()
    {
      Name = "fancy_core_sharpmake";
    }

    [Sharpmake.Configure]
    public override void ConfigureAll(Configuration conf, Target target)
    {
      base.ConfigureAll(conf, target);

      conf.PrecompHeader = "fancy_core_precompile.h";
      conf.PrecompSource = "fancy_core_precompile.cpp";
    }
  }
}