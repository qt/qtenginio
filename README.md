# Enginio Qt Library development branch
Client library for accessing Enginio service from Qt and QML code. 

# System Requirements
* Qt 5.0 or newer (Qt 5.1 recommended)
* OpenSSL library
  * Mac OS X: OpenSSL library should be preinstalled.
  * Linux: Most distributions have preinstalled OpenSSL library. If yours doesn't, seach for `libssl` in package repository.
  * Windows: Get the installer from http://slproweb.com/products/Win32OpenSSL.html (light version is enough, copy DLLs to windows system directory when asked).
* Perl 5.10 or newer
  * Mac and Linux: Perl should be preinstalled.
  * Windows: http://www.perl.org/get.html

# Build & Install
* Two build configurations provided: *standalone static library config* and *shared library config* 
* Both configurations can install the Enginio Library as Qt5 module

1. Standalone static library config 
    * The default build config for Enginio Library
    * Produces a static library on Linux and Mac
    * `qmake && make` - Builds the library to local target under the source folder   
 * `make install` - Installs the static library as a globally available Qt5 module 


2. Shared library config
	* `qmake CONFIG+=no-package && make install`
	* Produces shared library and installs it as a globally available Qt5 module

# Usage
* In C++ applications 
    * Use Enginio module by adding `QT += enginio` to application `.pro` file
    * Include Enginio headers with `<Enginio/...>` (for example: `#include <Enginio/enginioclient.h>`) 
* In QML applications 
    * Import Enginio components with `import Enginio 1.0`

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
