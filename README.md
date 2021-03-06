
About LibISF-Qt:
============

LibISF-Qt is a Qt library made to encode and decode Microsoft's
[Ink Serialized Format (ISF)](https://docs.microsoft.com/en-us/uwp/specifications/ink-serialized-format) files.

Q. What is ISF?
A. Ink Serialized Format is an open specification initially
   made by Microsoft, and then freed up with their "Open
   Specification Promise" initiative. ISF is a file format
   made to store handwriting very efficiently: an ISF file
   with a somewhat complex handwritten drawing takes less than
   1000 bytes. ISF supports metrics to adapt drawings to
   different form factors, bezier smoothing of drawn strokes,
   coloring, stroke size varying by pen pressure, linking
   in static data, and more.

Q. How does this library differ from aMSN's or Emesene's?
A. It's written with Qt, which makes it easily portable on
   a lot of different architectures; also Qt's new licensing
   terms (LGPL) make it possible for everyone to use it
   easily.
   The library supports more ISF features than those two,
   since we've written it using the official specification.
   Moreover, aMSN's and Emesene's are not distributed by
   theirselves, ours is.

Q. What's the status of completion of the library, and which
   ISF features does/will support?
A. Please see the TODO file to have an overview.



Compiling and installing:
=================

LibISF-Qt uses CMake, a quick and clean alternative to Autotools.

First create a new directory within the one which contains this
file (we'll have the library built there):
```
$ mkdir build
```
Enter it and run CMake:
```
$ cd build
$ cmake -D CMAKE_INSTALL_PREFIX=/usr ..
```
Then, if everything went smoothly, build the library itself:
```
$ make
```
If everything went right again, install it:
```
$ make install
```
Aaand now you're set. :)

LibISF-Qt natively supports being bundled into other applications.
Bundling means that the library does not get installed system-wide
along with your application, but instead it gets statically linked
into the application.
The idea is that you copy the Isf-Qt source tree (with an SVN export
or a Git submodule) into your project's tree, and then set up your
CMake listfile to add this library as a subdirectory:
```
SET( ISFQT_IS_BUNDLED TRUE )
ADD_SUBDIRECTORY( contrib/isf-qt )
```
And again, you're done!


Documentation:
=================

The ISF format is (albeit poorly*) documented: here's the
sources we used.

- Original [Microsoft ISF specification](http://download.microsoft.com/download/0/B/E/0BE8BDD7-E5E8-422A-ABFD-4342ED7AD886/InkSerializedFormat(ISF)Specification.pdf) made open with their Open Specification Promise
- [Microsoft's ISF specification in the UWP references](https://docs.microsoft.com/en-us/uwp/specifications/ink-serialized-format)
- Original code from [KMess on gitlab.com](https://gitlab.com/kmess/libisf-qt)
- Original code from [aMSN](https://amsn.svn.sourceforge.net/svnroot/amsn/trunk/amsn/utils/tclISF/src)
- Original code from [emesene](https://emesene.svn.sourceforge.net/svnroot/emesene/trunk/emesene/pyisf)

The ISF specification was probably not meant for use by developers wanting to write an alternative implementation; there's a lot of guesswork to be done.



Licensing
=========

Only the Oxygen icon, draw-eraser, is released under LGPL 3.0 (see http://www.oxygen-icons.org/). See COPYING_ICONS for full license text.

The remainder of ISF-qt is released under LGPL2.1. See COPYING for full license text.


More Information
================

For more information about LibISF-Qt, ask us:
- forum board at http://www.kmess.org/board/
- e-mail at project@kmess.org
- IRC at #kmess on irc.freenode.net
