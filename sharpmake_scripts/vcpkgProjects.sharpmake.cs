using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;
using Sharpmake;
    
namespace Fancy {
    public class ExportProject : Project
    {
        public ExportProject()
        {
            AddTargets(FancyTargets.LibTargets);
        }

        [Configure(), ConfigurePriority(1)]
        public virtual void ConfigureAll(Configuration conf, Target target)
        {
        }

        [Configure(Optimization.Debug), ConfigurePriority(2)]
        public virtual void ConfigureDebug(Configuration conf, Target target)
        {
        }

        [Configure(Optimization.Release), ConfigurePriority(3)]
        public virtual void ConfigureRelease(Configuration conf, Target target)
        {
        }
    }

    [Sharpmake.Export]
    public class VCPKG : ExportProject
    {
        public string vcpkgBinPath = @"[project.SharpmakeCsPath]\..\external\vcpkg\installed\x64-windows\bin\";
        public string vcpkgBinPath_debug = @"[project.SharpmakeCsPath]\..\external\vcpkg\installed\x64-windows\debug\bin\";

        public void AddTargetCopyFile( string file, Configuration conf, Target target ) {
            if( target.Optimization == Optimization.Release ) {
                conf.TargetCopyFiles.Add( vcpkgBinPath + file );
            } else if( target.Optimization == Optimization.Debug ) {
                conf.TargetCopyFiles.Add( vcpkgBinPath_debug + file );
            }
        }

        public override void ConfigureRelease(Configuration conf, Target target)
        {
            base.ConfigureRelease(conf, target);

            // Add root include path for vcpkg packages.
            conf.IncludePaths.Add(@"[project.SharpmakeCsPath]\..\external\vcpkg\installed\x64-windows\include");

            // Add root lib path for vcpkg packages.
            conf.LibraryPaths.Add(@"[project.SharpmakeCsPath]\..\external\vcpkg\installed\x64-windows\lib");
        }

        public override void ConfigureDebug(Configuration conf, Target target)
        {
            base.ConfigureDebug(conf, target);

            // Add root include path for vcpkg packages.
            conf.IncludePaths.Add(@"[project.SharpmakeCsPath]\..\external\vcpkg\installed\x64-windows\include");

            // Add root lib path for vcpkg packages.
            conf.LibraryPaths.Add(@"[project.SharpmakeCsPath]\..\external\vcpkg\installed\x64-windows\debug\lib");
        }
    }

    [Sharpmake.Export]
    public class Assimp : VCPKG
    {
        public override void ConfigureDebug(Configuration conf, Target target)
        {
            base.ConfigureDebug(conf, target);

            conf.LibraryFiles.Add(@"assimp-vc143-mtd.lib");
            AddTargetCopyFile( "assimp-vc143-mtd.dll", conf, target );
            AddTargetCopyFile( "draco.dll", conf, target );
            AddTargetCopyFile( "minizip.dll", conf, target );
            AddTargetCopyFile( "poly2tri.dll", conf, target );
            AddTargetCopyFile( "pugixml.dll", conf, target );
            AddTargetCopyFile( "zlibd1.dll", conf, target );
        }

        public override void ConfigureRelease(Configuration conf, Target target)
        {
            base.ConfigureRelease(conf, target);

            conf.LibraryFiles.Add(@"assimp-vc143-mt.lib");
            AddTargetCopyFile( "assimp-vc143-mt.dll", conf, target );
            AddTargetCopyFile( "draco.dll", conf, target );
            AddTargetCopyFile( "minizip.dll", conf, target );
            AddTargetCopyFile( "poly2tri.dll", conf, target );
            AddTargetCopyFile( "pugixml.dll", conf, target );
            AddTargetCopyFile( "zlib1.dll", conf, target );
        }
    }

    [Sharpmake.Export]
    public class EASTL : VCPKG
    {
        public override void ConfigureAll(Configuration conf, Target target)
        {
            base.ConfigureAll(conf, target);
            conf.LibraryFiles.Add(@"EASTL.lib");
        }
    }

    [Sharpmake.Export]
    public class GLM : VCPKG
    {
        public override void ConfigureAll(Configuration conf, Target target)
        {
            base.ConfigureAll(conf, target);
            conf.LibraryFiles.Add(@"glm.lib");
        }
    }

    [Sharpmake.Export]
    public class XXHash : VCPKG
    {
        public override void ConfigureAll(Configuration conf, Target target)
        {
            base.ConfigureAll(conf, target);
            conf.LibraryFiles.Add(@"xxhash.lib");
            AddTargetCopyFile( "xxhash.dll", conf, target );
        }
    }

    [Sharpmake.Export]
    public class DXC : VCPKG
    {
        public override void ConfigureAll(Configuration conf, Target target)
        {
            base.ConfigureAll(conf, target);
            conf.LibraryFiles.Add(@"dxcompiler.lib");
            AddTargetCopyFile( "dxcompiler.dll", conf, target );
            AddTargetCopyFile( "dxil.dll", conf, target );
        }
    }

    [Sharpmake.Export]
    public class Direct3D12AgilitySdk : VCPKG
    {
        public override void ConfigureAll(Configuration conf, Target target)
        {
            base.ConfigureAll(conf, target);
            conf.LibraryFiles.Add("DirectX-Headers.lib", "DirectX-Headers.lib", "dxgi.lib", "d3d12.lib");
            AddTargetCopyFile( "D3D12Core.dll", conf, target );
        }
    }






}
    