// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
using LibObjectFile.Elf;
using System;
using System.ComponentModel.DataAnnotations;
using System.IO;
using System.Reflection;
using System.Text;

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


var ExeFileInfo = new FileInfo(ExeFileName);
var DllFileData = File.ReadAllBytes(DllFileName);

using var inStream = File.OpenRead(ExeFileName);
var elf = ElfObjectFile.Read(inStream);

var codeStream = new MemoryStream();
codeStream.Write(DllFileData);
codeStream.Position = 0;

// Create a .text code section
var codeSection = new ElfBinarySection(codeStream).ConfigureAs(ElfSectionSpecialType.ReadOnlyData);
codeSection.Name = ".idr_rcdata1";
codeSection.Type = ElfSectionType.ProgBits;
codeSection.Alignment = 1;
codeSection.Flags = ElfSectionFlags.None;


var lastSection = elf.Sections[elf.Sections.Count - 1];
codeSection.Offset = lastSection.Offset+lastSection.Size;

elf.AddSection(codeSection);


var diag = new LibObjectFile.DiagnosticBag();

codeSection.UpdateLayout(diag);
elf.UpdateLayout(diag);

//using ResourceUpdaterPE updater = new(ExeFileInfo);
//updater.AddBinaryResource(SectionName, DllFileData);

elf.Print(Console.Out);
elf.Verify();

using var outStream = File.OpenWrite("helloworld2");
elf.Write(outStream);

Console.WriteLine($"{Assembly.GetExecutingAssembly().GetName().Name} End!");
