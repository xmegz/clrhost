using Microsoft.Extensions.Logging;
using System;
using System.Reflection;
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
        static Program()
        {
            Console.WriteLine($"Program {Assembly.GetExecutingAssembly().FullName}");
        }

        public static void Main()
        {
            Console.WriteLine("Main Start!");
            Data.Test();
            Console.WriteLine("Main End!");
        }
    }
}
