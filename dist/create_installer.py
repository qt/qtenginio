#!/usr/bin/python
# -*- coding: utf-8 -*-

# Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
# Contact: http://engin.io
#
# You may use this file under the terms of the 3-clause BSD license.
# See the file LICENSE from this package for details.
#

from __future__ import print_function
import os
import shutil
import subprocess
import sys

# Qt
qmake = "qmake"

make = "make"
if sys.platform == "win32":
    make = "jom"


# from the installer framework
binarycreator = "binarycreator"

# visual studio
#setEnv = "c:\\Program Files\\Microsoft SDKs\\Windows\\v7.1\\Bin\\SetEnv.Cmd"

# should be extracted from .qmake.conf
VERSION = "unknown_version"

# Build Enginio

subprocess.check_call(["git", "clean", "-xdf"])

os.mkdir("build")
os.chdir("build")

# this is evil - keep the process alive so that we can set the right sdk env here
# and it may not work, at least currently I have conflicts with VS 7.1 and 8
#process = subprocess.Popen('cmd.exe /k\n', shell=True, stdin=subprocess.PIPE)
#process.stdin.write('call "' + setEnv + '"\n')
#process.stdin.write(qmake + " ../..\r\n")
#process.stdin.write(jom + "\n")

# docs build currently errors out!
# process.stdin.write("nmake docs\n")
#process.stdin.write("exit\n")
#process.communicate()

subprocess.check_call([qmake, "../.."])
print("Compiling Enginio...")

try:
    from subprocess import DEVNULL # py3k
except ImportError:
    DEVNULL = open(os.devnull, 'wb')

subprocess.check_call([make,], stdout=DEVNULL)

if sys.platform == "win32": # bug with jom subtargets
    subprocess.check_call(["nmake", "docs"])
else:
    subprocess.check_call([make, "docs"])

# Copy files around
os.chdir("../..")

packages = {
    "com.digia.enginio": ["include", "lib", "qml", ], #"doc/enginio-qt.qch", ],
    "com.digia.enginioExamples": ["examples",],
    #"com.digia.enginioDocumentation": ["doc/enginio-qt",],
    "com.digia.enginioSources": ["src",],
    }

print("Creating installer...")

for package in packages:
    for path in packages[package]:
        sourcePath = path
        if not os.path.exists(sourcePath):
            sourcePath = "dist/build/" + path
        if not os.path.exists(sourcePath):
            print("ERROR: Could not find path '" + path + "' in source or build directory.")
            exit(1)
        print("Creating ", path, " dir.")
        dest = "dist/packages/" + package + "/data/" + path
        if os.path.exists(dest):
            print("ERROR: ", dest, " exists already.")
            exit(1)
        if os.path.isfile(sourcePath):
            os.mkdir(os.path.dirname(dest))
            shutil.copyfile(sourcePath, dest)
        else:
            shutil.copytree(sourcePath, dest)

# copy the real headers
# src/enginio_client
# src/enginio_plugin
headerPath = "dist/packages/com.digia.enginio/data/include/Enginio/"

import fileinput
for line in fileinput.input(".qmake.conf"):
    if line.startswith("MODULE_VERSION"):
        import re
        VERSION = re.search(r'[0-9.]+', line).group(0)


privateHeaderPath = headerPath + VERSION + "/Enginio/private"
import glob
allHeaders = glob.glob("src/*/*.h")
for header in allHeaders:
    fileName = header[header.rindex(os.sep):]
    if header.endswith("_p.h"):
        print("Copy ", header, " to ", privateHeaderPath + fileName)
        shutil.copyfile(header, privateHeaderPath + fileName)
    else:
        print("Copy ", header, " to ", headerPath + fileName)
        shutil.copyfile(header, headerPath + fileName)


os.chdir("dist")



# the Module .pri file is special - take the one from mkspecs/modules_inst
modulesPath = "packages/com.digia.enginio/data/mkspecs/modules"
os.mkdir("packages/com.digia.enginio/data/mkspecs")
os.mkdir(modulesPath)
shutil.copyfile("build/mkspecs/modules-inst/qt_lib_enginio.pri", modulesPath + "/qt_lib_enginio.pri")


subprocess.check_call([binarycreator, "-c", "config" + os.sep + "config.xml", "-p", "packages", "EnginioInstaller_" + VERSION])

print("Installer created.")
