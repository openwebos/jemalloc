jemalloc
===========

Summary
-------
The version of jemalloc used in Open webOS

Description
-----------
This is a minimal-effort stand-alone jemalloc distribution for Linux.  The main
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

Below are the tools and libraries (and their minimum versions) required to build jemalloc:

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

The directory under which the files are installed defaults to <tt>/usr/local/webos</tt>.
You can install them elsewhere by supplying a value for <tt>WEBOS\_INSTALL\_ROOT</tt>
when invoking <tt>cmake</tt>. For example, entering the following:

    $ cmake -D WEBOS_INSTALL_ROOT:PATH=$HOME/projects/openwebos ..
    $ make
    $ make install

will install the files in subdirectories of <tt>$HOME/projects/openwebos</tt>.

Specifying <tt>WEBOS\_INSTALL\_ROOT</tt> also causes <tt>pkg-config</tt> to look
in that tree first before searching the standard locations. You can specify
additional directories to be searched prior to this one by setting the
the <tt>PKG\_CONFIG\_PATH</tt> environment variable.

If not specified, <tt>WEBOS\_INSTALL\_ROOT</tt> defaults to <tt>/usr/local/webos</tt>.

To see a list of the make targets that <tt>cmake</tt> has generated, enter:

    $ make help

## Uninstalling

From the directory where you originally ran <tt>make install<tt>, enter:

    $ [sudo] make uninstall

You will need to use <tt>sudo</tt> if you did not specify <tt>WEBOS\_INSTALL\_ROOT</tt>.

To enable HEAP_TRACKING feature pass the value of CMAKE_C_FLAGS set as -DHEAP_TRACKING as follows:

    $ cmake -D CMAKE_C_FLAGS=-DHEAP_TRACKING ..

# Copyright and License Information

Copyright (c) 2008-2012 Hewlett-Packard Development Company, L.P.
All rights reserved.

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

