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

            if(args?.Length > 0)
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
