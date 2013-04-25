# Enginio Qt Library development branch
Client library for accessing Enginio service from Qt and QML code. 

# System Requirements
* Qt 5.0 or newer
* OpenSSL library 
  * Windows: Get the installer from http://slproweb.com/products/Win32OpenSSL.html (light version is enough, copy DLLs to windows system directory when asked).
  * Linux: Seach for `libssl` in your distribution's package repository.
* To install the library on a Windows system ActivePerl (http://www.activestate.com/activeperl/downloads) is needed which is also a dependency for building Qt
  (For additional information see http://qt-project.org/wiki/Building_Qt_Desktop_for_Windows_with_MSVC).

# Install

* `qmake && make install` will install the client library as a Qt5 module (enginio) to the Qt installation.
* In C++ applications include headers as usual (for example: `#include <Enginio/enginioclient.h>`) and add 'Qt += enginio' to the project file of your application.
* In QML applications import Enginio components with 'import Enginio 1.0'


# Contributing
* Fork the repository on [GitHub](https://github.com/enginio/enginio-qt)
* Create a [feature branch](http://nvie.com/posts/a-successful-git-branching-model/) (i.e. create a branch which is named like `my-cool-feature`)
* Implement the new feature and automated tests for it
* Make sure all tests pass
* Submit a pull request that includes only your feature branch


# Copyright
Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
Contact http://qt.digia.com/contact-us 


# License
Enginio Qt library is tri-licensed under the Commercial Qt License, the GNU General Public License 3.0, and the GNU Lesser General Public License 2.1.

**Commercial License Usage**
Licensees holding valid Commercial Qt Licenses may use this library in accordance with the commercial license agreement provided with the Software or, alternatively, in accordance with the terms contained in a written agreement between you and Digia. For licensing terms and conditions see http://qt.digia.com/licensing. For further information use the contact form at http://qt.digia.com/contact-us.

**GNU Lesser General Public License Usage**
Alternatively, this library may be used under the terms of the GNU Lesser General Public License version 2.1 as published by the Free Software Foundation and appearing in the file LICENSE.LGPL included in the packaging of this library. Please review the following information to ensure the GNU Lesser General Public License version 2.1 requirements will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html. In addition, as a special exception, Digia gives you certain additional rights. These rights are described in the Digia Qt LGPL Exception version 1.1, included in the file LGPL_EXCEPTION.txt.

**GNU General Public License Usage**
Alternatively, this library may be used under the terms of the GNU General Public License version 3.0 as published by the Free Software Foundation and appearing in the file LICENSE.GPL included in the packaging of this library. Please review the following information to ensure the GNU General Public License version 3.0 requirements will be met: http://www.gnu.org/copyleft/gpl.html.
