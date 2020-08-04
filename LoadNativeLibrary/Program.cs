//-----------------------------------------------------------------------------
// Filename: Program.cs
//
// Description: Test program to load a native dll and that works cross platform.

// Author(s):
// Aaron Clauson (aaron@sipsorcery.com)
//
// History:
// 04 Aug 2020	Aaron Clauson	Created, Dublin, Ireland.
//
// License: 
// BSD 3-Clause "New" or "Revised" License, see included LICENSE.md file.
//-----------------------------------------------------------------------------
// Background info:
// https://www.mono-project.com/docs/advanced/pinvoke/
//
//-----------------------------------------------------------------------------
// Usage:
// To load from vcpkg directory (adjust x86/x64 as required):
//
// dotnet run C:\Tools\vcpkg\installed\x64-windows\bin
// dotnet run -r win-x86 C:\Tools\vcpkg\installed\x86-windows\bin
//
//-----------------------------------------------------------------------------

using System;
using System.IO;
using System.Reflection;
using System.Runtime.InteropServices;

namespace LoadNativeLibrary
{
    class Program
    {
        public const string LIB_AVCODEC_BASE_NAME = "avcodec";
        public const string LIB_AVCODEC_VERSION = "58";

        public const string LIB_AVCODEC_WINDOWS_NAME = LIB_AVCODEC_BASE_NAME + "-" + LIB_AVCODEC_VERSION;

        [DllImport("kernel32.dll", CharSet = CharSet.Auto, SetLastError = true)]
        static extern bool SetDllDirectory(string lpPathName);

        [DllImport(LIB_AVCODEC_WINDOWS_NAME)]
        static extern uint avcodec_version();

        static void Main(string[] args)
        {
            Console.WriteLine("Load Native Library Console!");
            Console.WriteLine($"Is 64bit process {Environment.Is64BitProcess}.");

            if (args?.Length > 0)
            {
                string searchDir = args[0].Trim();

                if (!Directory.Exists(searchDir))
                {
                    Console.WriteLine($"Specified search directory does not exist, {searchDir}.");
                }
                else
                {
                    bool res = SetDllDirectory(searchDir);
                    Console.WriteLine($"SetDllDirectory result {res} for {searchDir}.");
                }
            }

            NativeLibrary.SetDllImportResolver(Assembly.GetExecutingAssembly(), ImportResolver);

            Console.WriteLine($"avcodec version {avcodec_version()}.");

            Console.WriteLine("press any key to exit...");
            Console.ReadLine();
        }

        private static IntPtr ImportResolver(string libraryName, Assembly assembly, DllImportSearchPath? searchPath)
        {
            IntPtr libHandle = IntPtr.Zero;
            if (libraryName == LIB_AVCODEC_WINDOWS_NAME && Environment.OSVersion.Platform == PlatformID.Unix)
            {
                // Try using the system library 'libmylibrary.so.5'
                NativeLibrary.TryLoad($"{LIB_AVCODEC_BASE_NAME}.so.{LIB_AVCODEC_VERSION}", assembly, DllImportSearchPath.SafeDirectories, out libHandle);
            }
            return libHandle;
        }
    }
}
