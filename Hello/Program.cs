using System;
using System.IO;
using System.Runtime.InteropServices.ComTypes;
using System.Runtime.Loader;

namespace Hello
{
    
    public class Program   
    {
        static int num = 0;

        static Program()
        {
            Console.WriteLine("Start");
        }
      
        public static void Main()
        {
            
            Console.WriteLine("Hello World! 2");
            Console.WriteLine(num++);
        }        
    }
}
