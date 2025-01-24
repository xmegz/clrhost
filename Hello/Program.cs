// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
using Microsoft.Extensions.Logging;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Text;
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


    public static unsafe class HostRuntime
    {
        #pragma warning disable CS0649
        internal struct HostRuntimeContract
        {
            public nint Size;
            public void* Context;
            public delegate* unmanaged[Stdcall]<byte*, byte*, nint, void*, nint> GetRuntimeProperyPtr;
            public IntPtr BundleProbePtr;
            public IntPtr PInvokeOverridePtr;
        }
        #pragma warning restore CS0649 

        private static readonly HostRuntimeContract* ContractPtr = null;

        static HostRuntime()
        {
            Console.WriteLine($"Ctor {nameof(HostRuntime)}");

            ContractPtr = GetContractPtr();
        }

        private static HostRuntimeContract* GetContractPtr()
        {
            string addressHexString = (string)AppContext.GetData("HOST_RUNTIME_CONTRACT");

            if (string.IsNullOrEmpty(addressHexString))
                return null;

            HostRuntimeContract* contractPtr = (HostRuntimeContract*)Convert.ToUInt64(addressHexString, 16);

            if (contractPtr->Size != sizeof(HostRuntimeContract))
                return null;

            return contractPtr;
        }

        public static string GetRuntimeProperty(string nameStr)
        {
            if (ContractPtr == null)
                return null;

            Span<byte> nameSpan = stackalloc byte[Encoding.UTF8.GetMaxByteCount(nameStr.Length)];
            byte* namePtr = (byte*)Unsafe.AsPointer(ref MemoryMarshal.GetReference(nameSpan));
            int nameLength = Encoding.UTF8.GetBytes(nameStr, nameSpan);

            nameSpan[nameLength] = 0;

            nint valueLength = 256;
            byte* valuePrt = stackalloc byte[(int)valueLength];
            nint valueLengthActual = ContractPtr->GetRuntimeProperyPtr(namePtr, valuePrt, valueLength, ContractPtr->Context);

            if (valueLengthActual <= 0)
                return null;

            if (valueLengthActual <= valueLength)
                return Encoding.UTF8.GetString(valuePrt, (int)valueLengthActual);

            valueLength = valueLengthActual;

            byte* expandedValuePtr = stackalloc byte[(int)valueLength];
            valueLengthActual = ContractPtr->GetRuntimeProperyPtr(namePtr, expandedValuePtr, valueLength, ContractPtr->Context);
            return Encoding.UTF8.GetString(expandedValuePtr, (int)valueLengthActual);
        }
    }


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

        public static void DebugAppContextData(string name)
        {
            var logger = LoggerFactory.CreateLogger<Program>();

            var obj = System.AppContext.GetData(name);

            if (obj != null)
                logger?.LogInformation($"{name} : {obj}");
        }

        public static void DebugRuntimeProperty(string name)
        {
            var logger = LoggerFactory.CreateLogger<Program>();

            string str = HostRuntime.GetRuntimeProperty(name);

            if (str != null)
                logger?.LogInformation($"{name} : {str}");
        }


        public static void Main(string[] arg)
        {
            var logger = LoggerFactory.CreateLogger<Program>();

            logger?.LogInformation($"Start {DateTime.Now}");

            logger?.LogInformation($"Args: {String.Join(',', arg)}");

            logger?.LogInformation($"------ {nameof(DebugAppContextData)} -----");

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

            logger?.LogInformation($"------ {nameof(DebugRuntimeProperty)} -----");

            DebugRuntimeProperty("HOST_RUNTIME_CONTRACT");
            DebugRuntimeProperty("APP_PATHS");
            DebugRuntimeProperty("BUNDLE_PROBE");
            DebugRuntimeProperty("ENTRY_ASSEMBLY_NAME");
            DebugRuntimeProperty("NATIVE_DLL_SEARCH_DIRECTORIES");
            DebugRuntimeProperty("PINVOKE_OVERRIDE");
            DebugRuntimeProperty("PLATFORM_RESOURCE_ROOTS");
            DebugRuntimeProperty("TRUSTED_PLATFORM_ASSEMBLIES");

            Data.Test();

            logger?.LogInformation($"End {DateTime.Now}");

            //Console.ReadLine();

            Environment.ExitCode = 11;
        }
    }
}
