/*-----------------------------------------------------------------------------
 * Project:    CrlHost
 * Repository: https://github.com/xmegz/clrhost
 * Author:     Pádár Tamás
 -----------------------------------------------------------------------------*/

using System.Diagnostics;
using System.Reflection;

Console.WriteLine($"{Assembly.GetExecutingAssembly().GetName().Name} Start!");

string ExeFileName = args.Length > 0 ? args[0] : "AppHostLinux.out";
string DllFileName = args.Length > 1 ? args[1] : "Hello.dll";
string SectionName = args.Length > 2 ? args[2] : ".idr_rcdata1";

Console.WriteLine("Inject dotnet assemby as section to native linux ELF exe file");
Console.WriteLine($"Usage: InjectResource [{nameof(ExeFileName)}] [{nameof(DllFileName)}] [{nameof(SectionName)}]");
Console.WriteLine($"Example: InjectResource AppHostLinux.out Hello.dll .idr_rcdata1");
Console.WriteLine($"Params: [{ExeFileName}] [{DllFileName}] [{SectionName}]");


if (!File.Exists(ExeFileName))
{
    Console.WriteLine($"{nameof(ExeFileName)} not exist!");
    return;
}

if (!File.Exists(DllFileName))
{
    Console.WriteLine($"{nameof(DllFileName)} not exist!");
    return;
}

string OutPath = Path.Combine(Path.GetDirectoryName(ExeFileName) ?? "", "out");
string OutFileName = Path.Combine(OutPath, Path.GetFileNameWithoutExtension(DllFileName) + ".full.exe");

Directory.CreateDirectory(OutPath);
File.Copy(ExeFileName, OutFileName, true);


var proc = new Process
{
    StartInfo = new ProcessStartInfo
    {
        FileName = @"objcopy",
        Arguments = $"--add-section {SectionName}={DllFileName} --set-section-flags {SectionName}=noload,readonly {OutFileName}",
        UseShellExecute = false,
        RedirectStandardOutput = true,
        CreateNoWindow = true,
        WorkingDirectory = AppContext.BaseDirectory
    }
};

proc.Start();
var ret = proc.StandardOutput.ReadToEnd();
proc.WaitForExit();
Console.WriteLine(ret);

Console.WriteLine($"{Assembly.GetExecutingAssembly().GetName().Name} End!");
