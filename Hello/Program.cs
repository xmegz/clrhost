// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
using Microsoft.Extensions.Logging;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Text.Json;

namespace Hello
{
    public record Data(int Id, string Name)
    {
        public static void Test()
        {
            var logger = Program.LoggerFactory.CreateLogger<Data>();

            logger?.LogInformation($"Test Json Serializer {JsonSerializer.Serialize(new Data(0, "Name"))}");
        }
    };

    public class Program
    {
        public static ILoggerFactory LoggerFactory { get; set; } = null;

        [UnmanagedCallersOnly]
        public static void NativeEntryPoint(int argc, IntPtr argv)
        {
            static string[] MarshalArgv(int argc, IntPtr argv)
            {
                string[] args = new string[argc];

                for (int i = 0; i < argc; i++, argv += IntPtr.Size)
                    args[i] = Marshal.PtrToStringAnsi(Marshal.ReadIntPtr(argv));

                return args;
            }

            string[] args = MarshalArgv(argc, argv);

            Main(args);
        }
        static Program()
        {
            LoggerFactory = Microsoft.Extensions.Logging.LoggerFactory.Create(builder =>
            {
                builder.AddConsole();
            });

            var logger = LoggerFactory.CreateLogger<Program>();

            logger?.LogInformation($"Ctor {Assembly.GetExecutingAssembly().FullName}");
        }

        public static void DebugAppContextData(string str)
        {
            var logger = LoggerFactory.CreateLogger<Program>();

            var obj = System.AppContext.GetData(str);

            if (obj != null)
                logger?.LogInformation($"{str} : {obj.ToString()}");
        }


        public static void Main(string[] arg)
        {
            var logger = LoggerFactory.CreateLogger<Program>();

            logger?.LogInformation($"Start {DateTime.Now}");

            logger?.LogInformation($"Args: {String.Join(',', arg)}");

            DebugAppContextData("APP_PATHS");
            DebugAppContextData("APP_CONTEXT_BASE_DIRECTORY");
            DebugAppContextData("APP_CONTEXT_DEPS_FILES");
            DebugAppContextData("APP_NAME");
            DebugAppContextData("APPBASE");
            DebugAppContextData("FX_DEPS_FILE");
            DebugAppContextData("HOST_RUNTIME_CONTRACT");
            DebugAppContextData("PROBING_DIRECTORIES");
            DebugAppContextData("NATIVE_DLL_SEARCH_DIRECTORIES");
            DebugAppContextData("PROBING_DIRECTORIES");
            DebugAppContextData("RUNTIME_IDENTIFIER");
            DebugAppContextData("STARTUP_HOOKS");
            //DebugAppContextData("TRUSTED_PLATFORM_ASSEMBLIES");

            Data.Test();

            logger?.LogInformation($"End {DateTime.Now}");

            Environment.ExitCode = 11;
        }
    }
}
