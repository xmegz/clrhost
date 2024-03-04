using Microsoft.Extensions.Logging;
using System;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Text.Json;

namespace Hello
{
    public record Data(int Id, string Name)
    {
        public static void Test()
        {
            var str = JsonSerializer.Serialize(new Data(0, "Name"));

            var factory = LoggerFactory.Create(builder =>
            {
                builder.AddConsole();
            });
            var logger = factory.CreateLogger<Data>();

            logger?.LogInformation("Test Start");
            logger?.LogInformation(str);
            logger?.LogInformation("Test End!");
        }
    };
    public class Program
    {
        [UnmanagedCallersOnly]
        private static void NativeEntryPoint(int argc, IntPtr argv)
        {
            string[] MarshalArgv(int argc, IntPtr argv)
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
            Console.WriteLine($"Program.Ctor {Assembly.GetExecutingAssembly().FullName}");
        }

        public static void Main(string[] arg)
        {
            Console.WriteLine($"Program.Main! {DateTime.Now}");
            foreach (var i in arg)
            {
                Console.WriteLine($"Arg: {i}");
            }

            Data.Test();
            Environment.ExitCode = 11;
        }
    }
}
