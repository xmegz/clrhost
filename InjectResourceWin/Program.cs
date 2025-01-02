/*-----------------------------------------------------------------------------
 * Project:    CrlHost
 * Repository: https://github.com/xmegz/clrhost
 * Author:     Pádár Tamás
 -----------------------------------------------------------------------------*/

using System;
using System.ComponentModel.DataAnnotations;
using System.IO;
using System.Reflection;

Console.WriteLine($"{Assembly.GetExecutingAssembly().GetName().Name} Start!");

string ExeFileName = args.Length > 0 ? args[0] : "AppHostWindows.exe";
string DllFileName = args.Length > 1 ? args[1] : "Hello.dll";
string ResourceName = args.Length > 2 ? args[2] : "IDR_RCDATA1";

Console.WriteLine("Inject dotnet assemby as resource to native COFF/PE exe file");
Console.WriteLine($"Usage: InjectResource [{nameof(ExeFileName)}] [{nameof(DllFileName)}] [{nameof(ResourceName)}]");
Console.WriteLine($"Example: InjectResource AppHostWindows.exe Hello.dll IDR_RCDATA1");
Console.WriteLine($"Params: [{ExeFileName}] [{DllFileName}] [{ResourceName}]");


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


var OutFileInfo = new FileInfo(OutFileName);
var DllFileData = File.ReadAllBytes(DllFileName);

using ResourceUpdaterPE updater = new(OutFileInfo);
updater.AddBinaryResource(ResourceName, DllFileData);

Console.WriteLine($"{Assembly.GetExecutingAssembly().GetName().Name} End!");
