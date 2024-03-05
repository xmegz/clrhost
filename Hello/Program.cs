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
            var str = JsonSerializer.Serialize(new Data(0, "Name"));

            var logger = Program._loggerFactory.CreateLogger<Data>();

            logger?.LogInformation("Test Start");
            logger?.LogInformation(str);
            logger?.LogInformation("Test End!");
        }
    };

    public class Program
    {
        public static ILoggerFactory _loggerFactory = null;

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
            _loggerFactory = LoggerFactory.Create(builder =>
            {
                builder.AddConsole();
            });
            var logger = _loggerFactory.CreateLogger<Program>();

            logger?.LogInformation($"Ctor {Assembly.GetExecutingAssembly().FullName}");
        }

        public static void Main(string[] arg)
        {
            var logger = _loggerFactory.CreateLogger<Program>();

            logger?.LogInformation($"Start Modified{DateTime.Now}");
            
            foreach (var i in arg)
                logger?.LogInformation($"Arg: {i}");

            Data.Test();

            logger?.LogInformation($"End {DateTime.Now}");
            Environment.ExitCode = 11;
        }
    }
}
