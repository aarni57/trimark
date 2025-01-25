#!/usr/bin/env python

from __future__ import print_function

import os
import shutil
import subprocess
import optparse

parser = optparse.OptionParser()
parser.set_defaults(clean = False)
parser.add_option("--clean", "-c", action="store_true", dest="clean")

(options, args) = parser.parse_args()

build_dir = "build"
cmake_command = "cmake"
generator_name = "Xcode"

if not os.path.exists(build_dir):
   os.makedirs(build_dir)
elif options.clean:
   if os.path.exists(build_dir):
      shutil.rmtree(build_dir)
   os.makedirs(build_dir)

original_working_dir = os.getcwd()
os.chdir(build_dir)
cmake_args = [cmake_command, "-G", generator_name, ".."]
subprocess.call(cmake_args)
os.chdir(original_working_dir)
