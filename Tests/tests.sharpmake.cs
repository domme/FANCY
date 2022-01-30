using Sharpmake;

namespace Fancy
{
  [Sharpmake.Generate]
  public class TestsProject : FancyInternalApplication
  {
    public TestsProject()
    {
      Name = "Tests";
    }

    [Sharpmake.Configure]
    public override void ConfigureAll(Configuration conf, Target target)
    {
      base.ConfigureAll(conf, target);
    }
  }
}

