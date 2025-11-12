koi
===

[stb](https://github.com/nothings/stb)-like single-file, public domain (or MIT-licensed) image processing libraries for C/C++

# This project discusses security-relevant bugs in public in Github Issues and Pull Requests, and it may take significant time for security fixes to be implemented or merged. If this poses an unreasonable risk to your project, do not use koi libraries.

Noteworthy:

* image loader: [koi_image.h](https://github.com/Muppetsg2/koi/blob/main/koi_image.h)
* image writer: [koi_image_write.h](https://github.com/Muppetsg2/koi/blob/main/koi_image_write.h)

<a name="koi_libs"></a>

library    | lastest version | category | LoC | description
--------------------- | ---- | -------- | --- | --------------------------------
**[koi_image.h](https://github.com/Muppetsg2/koi/blob/main/koi_image.h)** | 1.00 | graphics | 1331 | image loading/decoding from file/memory: QOI
**[koi_image_write.h](https://github.com/Muppetsg2/koi/blob/main/koi_image_write.h)** | 1.00 | graphics | 803 | image writing to disk: QOI

Total libraries: 2
Total lines of C code: 2134


FAQ
---

#### What's the license?

These libraries are in the public domain. You can do anything you
want with them. You have no legal obligation
to do anything else, although I appreciate attribution.

They are also licensed under the MIT open source license, if you have lawyers
who are unhappy with public domain. Every source file includes an explicit
dual-license for you to choose from.

#### How do I use these libraries?

The idea behind single-header file libraries is that they're easy to distribute and deploy
because all the code is contained in a single file. By default, the .h files in here act as
their own header files, i.e. they declare the functions contained in the file but don't
actually result in any code getting compiled.

So in addition, you should select _exactly one_ C/C++ source file that actually instantiates
the code, preferably a file you're not editing frequently. This file should define a
specific macro (this is documented per-library) to actually enable the function definitions.
For example, to use koi_image, you should have exactly one C/C++ file that doesn't
include koi_image.h regularly, but instead does

    #define KOI_IMAGE_IMPLEMENTATION
    #include "koi_image.h"

The right macro to define is pointed out right at the top of each of these libraries.

#### <a name="other_libs"></a> Are there other single-file public-domain/open source libraries with minimal dependencies out there?

[Yes.](https://github.com/nothings/single_file_libs)

#### If I wrap an koi library in a new library, does the new library have to be public domain/MIT?

No, because it's public domain you can freely relicense it to whatever license your new
library wants to be.

#### Some of these libraries seem redundant to existing open source libraries. Are they better somehow?

Generally they're only better in that they're easier to integrate,
easier to use, and easier to release (single file; good API; no
attribution requirement). They may be less featureful, slower,
and/or use more memory. If you're already using an equivalent
library, there's probably no good reason to switch.

#### Can I link directly to the table of koi libraries?

You can use [this URL](https://github.com/Muppetsg2/koi#koi_libs) to link directly to that list.

#### Why do you list "lines of code"? It's a terrible metric.

Just to give you some idea of the internal complexity of the library,
to help you manage your expectations, or to let you know what you're
getting into. While not all the libraries are written in the same
style, they're certainly similar styles, and so comparisons between
the libraries are probably still meaningful.

Note though that the lines do include both the implementation, the
part that corresponds to a header file, and the documentation.

#### Why single-file headers?

Windows doesn't have standard directories where libraries
live. That makes deploying libraries in Windows a lot more
painful than open source developers on Unix-derivates generally
realize. (It also makes library dependencies a lot worse in Windows.)

There's also a common problem in Windows where a library was built
against a different version of the runtime library, which causes
link conflicts and confusion. Shipping the libs as headers means
you normally just compile them straight into your project without
making libraries, thus sidestepping that problem.

Making them a single file makes it very easy to just
drop them into a project that needs them. (Of course you can
still put them in a proper shared library tree if you want.)

Why not two files, one a header and one an implementation?
The difference between 10 files and 9 files is not a big deal,
but the difference between 2 files and 1 file is a big deal.
You don't need to zip or tar the files up, you don't have to
remember to attach *two* files, etc.

#### Why not fork the koi repository and add new image formats there?

First, why fork a large repository when you're only 
interested in modifying two files? Second, I don’t want this project
to be tightly coupled to changes in the koi repository. The main idea
behind this repo is to add support for additional image formats — both decoders
and encoders — to [koi_image.h](https://github.com/Muppetsg2/koi/blob/main/koi_image.h) and [koi_image_write.h](https://github.com/Muppetsg2/koi/blob/main/koi_image_write.h).
