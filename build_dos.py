#!/usr/bin/env python

from __future__ import print_function

import os
import shutil
import sys
import subprocess
import platform
from colorama import Fore, Back, Style

include_paths = ["src", "dos"]

def compile(source):
    print(Fore.GREEN + os.path.basename(source))

    command = ["../djgpp/bin/i586-pc-msdosdjgpp-gcc", "-c", "-DNDEBUG", "-march=i386", "-mtune=pentiumpro", "-O3", "-funroll-loops", "-ffast-math", "-fomit-frame-pointer", "-fstrength-reduce", "-fforce-addr"]
    #command = ["/Users/aarni/dev/djgpp/bin/i586-pc-msdosdjgpp-gcc", "-c", "-DDEBUG", "-march=i486", "-mtune=pentiumpro"]

    for include_path in include_paths:
        command.append("-I" + include_path)

    command.append(source)
    r = subprocess.call(command)
    if r != 0:
        print(Fore.RED + "Compile failed: " + source)

    return r

def link(sources, output_filename):
    command = ["../djgpp/bin/i586-pc-msdosdjgpp-gcc", "-o", output_filename]

    for source in sources:
        command.append(os.path.splitext(os.path.basename(source))[0] + ".o")

    return subprocess.call(command)

def strip(input_filename, output_filename):
    command = ["../djgpp/bin/i586-pc-msdosdjgpp-strip", "-s", "-o", output_filename, input_filename]
    return subprocess.call(command)

#-------------------------------------------------------------------------------

sources = [
    "src/demo.c",
    "src/triangle.c",

    "dos/dosmain.c",
    "dos/vga.c",
]

for source in sources:
    if compile(source) != 0:
        exit()

if link(sources, "demo_sym.exe") != 0:
    exit()

if strip("demo_sym.exe", "demo.exe") != 0:
    exit()

shutil.copyfile("demo.exe", os.path.join("../../dos", "demo.exe"))
