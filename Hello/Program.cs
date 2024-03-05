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

            logger?.LogInformation($"Test Data {JsonSerializer.Serialize(new Data(0, "Name"))}");
        }
    };

    public class Program
    {
        public static ILoggerFactory LoggerFactory { get; set; } = null;

        [UnmanagedCallersOnly]
        private static void NativeEntryPoint(int argc, IntPtr argv)
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

        public static void Main(string[] arg)
        {
            var logger = LoggerFactory.CreateLogger<Program>();

            logger?.LogInformation($"Start {DateTime.Now}");

            foreach (var i in arg)
                logger?.LogInformation($"Arg: {i}");

            Data.Test();

            logger?.LogInformation($"End {DateTime.Now}");
            Environment.ExitCode = 11;
        }
    }
}
