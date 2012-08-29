jemalloc
===========

Summary
-------
The version of _jemalloc_ used in Open webOS

Description
-----------
This is a minimal-effort stand-alone _jemalloc_ distribution for Linux.  The main
rough spots are:

* __isthreaded must be hard-coded, since the pthreads library really needs to
  be involved in order to toggle it at run time.  Therefore, this distribution
  builds two separate libraries:

  + libjemalloc_mt.so.0 : Use for multi-threaded applications.
  + libjemalloc.so.0 : Use for single-threaded applications.

  Both libraries link against libpthread, though with a bit more code hacking,
  this dependency could be removed for the single-threaded version.

* MALLOC_MAG (thread-specific caching, using magazines) is disabled, because
  special effort is required to avoid memory leaks when it is enabled.  To make
  cleanup automatic, we would need help from the pthreads library.  If you
  enable MALLOC_MAG, be sure to call _malloc_thread_cleanup() in each thread
  just before it exits.

* The code that determines the number of CPUs is sketchy.  The trouble is that
  we must avoid any memory allocation during early initialization.

Dependencies
============

Below are the tools (and their minimum versions) required to build _jemalloc_:

- cmake (version required by cmake-modules-webos)
- gcc 4.6.3
- make (any version)
- openwebos/cmake-modules-webos 1.0.0 RC2

How to Build on Linux
=====================

## Building

Once you have downloaded the source, enter the following to build it (after
changing into the directory under which it was downloaded):

    $ mkdir BUILD
    $ cd BUILD
    $ cmake ..
    $ make
    $ sudo make install

The directory under which the files are installed defaults to `/usr/local/webos`.
You can install them elsewhere by supplying a value for `WEBOS_INSTALL_ROOT`
when invoking `cmake`. For example:

    $ cmake -D WEBOS_INSTALL_ROOT:PATH=$HOME/projects/openwebos ..
    $ make
    $ make install

will install the files in subdirectories of `$HOME/projects/openwebos`.

Specifying `WEBOS_INSTALL_ROOT` also causes `pkg-config` to look in that tree
first before searching the standard locations. You can specify additional
directories to be searched prior to this one by setting the `PKG_CONFIG_PATH`
environment variable.

If not specified, `WEBOS_INSTALL_ROOT` defaults to `/usr/local/webos`.

To configure for a debug build, enter:

    $ cmake -D CMAKE_BUILD_TYPE:STRING=Debug ..

To see a list of the make targets that `cmake` has generated, enter:

    $ make help

To enable HEAP_TRACKING feature pass the value of CMAKE_C_FLAGS set as 
-DHEAP_TRACKING as follows:

    $ cmake -D CMAKE_C_FLAGS=-DHEAP_TRACKING ..

## Uninstalling

From the directory where you originally ran `make install`, enter:

    $ [sudo] make uninstall

You will need to use `sudo` if you did not specify `WEBOS_INSTALL_ROOT`.

# Copyright and License Information

Copyright (C) 2008 Jason Evans <jasone@FreeBSD.org>.
Copyright (c) 2008-2012 Hewlett-Packard Development Company, L.P.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
 1. Redistributions of source code must retain the above copyright
    notice(s), this list of conditions and the following disclaimer as
    the first lines of this file unmodified other than the possible
    addition of one or more copyright notices.
 2. Redistributions in binary form must reproduce the above copyright
    notice(s), this list of conditions and the following disclaimer in
    the documentation and/or other materials provided with the
    distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER(S) ``AS IS'' AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER(S) BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; O
BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

