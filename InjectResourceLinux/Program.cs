// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
using System;
using System.ComponentModel.DataAnnotations;
using System.IO;
using System.Reflection;

Console.WriteLine($"{Assembly.GetExecutingAssembly().GetName().Name} Start!");

string ExeFileName = args.Length > 0 ? args[0] : "AppHostWindows.exe";
string DllFileName = args.Length > 1 ? args[1] : "Hello.dll";
string SectionName = args.Length > 2 ? args[2] : ".sname";

Console.WriteLine("Inject dotnet assemby as section to native linux ELF exe file");
Console.WriteLine($"Usage: InjectResource [{nameof(ExeFileName)}] [{nameof(DllFileName)}] [{nameof(SectionName)}]");
Console.WriteLine($"Example: InjectResource AppHostWindows.exe Hello.dll .sname");
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


var ExeFileInfo = new FileInfo(ExeFileName);
var DllFileData = File.ReadAllBytes(DllFileName);

//using ResourceUpdaterPE updater = new(ExeFileInfo);
//updater.AddBinaryResource(SectionName, DllFileData);

Console.WriteLine($"{Assembly.GetExecutingAssembly().GetName().Name} End!");
