Image gallery example application for QML
=========================================

Simple image gallery example application using Enginio service. Demonstrates how file upload and download operations can be executed with Enginio QML API. Application is data compatible with other Enginio Image Gallery examples.


# System Requirements
* Qt 5.0 or newer (Qt 5.1 recommended)
* OpenSSL library
  * Mac OS X: OpenSSL library should be preinstalled.
  * Linux: Most distributions have preinstalled OpenSSL library. If yours doesn't, seach for `libssl` in package repository.
  * Windows: Get the installer from http://slproweb.com/products/Win32OpenSSL.html (light version is enough, copy DLLs to windows system directory when asked).
* Perl 5.10 or newer
  * Mac and Linux: Perl should be preinstalled.
  * Windows: http://www.perl.org/get.html


 # Backend configuration
* Go to Enginio Dashboard and create new backend with `Image Gallery` template
* Or in existing or blank backend:
  * In *Object Types* > *Add Object Type* create new object type `objects.image`
  * Add new property with *name* `name` and *type* `String`
  * Add new property with *name* `file`, *type* `Ref` and *target* `files`
  * Add new file processor by clicking the *file* property. Choose `Image processor` and in *Variants* field enter:

        ```
        {
            "thumbnail": {
                "crop":"100x100"
            }
        }
        ```

  * Apply and Save

# Running the application
* Sign up to Enginio service on [Enginio website](https://www.engin.io/)
* Then either:
  * Go to Enginio Dashboard, create new backend and in 'backend type' selection choose 'Image gallery'
  * Download example application source code package from Enginio Dashboard (for QML)  
  * Extract package and open `enginio-qt-qml-image-gal.pro` file in Qt Creator
  * Run the project
* Or:
  * Clone the repository (and its submodules) from [GitHub](https://github.com/enginio/enginio-qt-qml-image-gal) (i.e. use `git clone --recursive`)
  * Open `enginio-qt-qml-image-gal.pro` file in Qt Creator
  * Go to Enginio Dashboard and create new blank backend
  * Copy Backend Id and Backend Secret from Enginio Dashboard to `applicationconfig.js` file (Qt Creator > image_gallery > Other files)
  * Run the project 


# Copyright and License
See LICENSE file. 
