# @@@LICENSE
#
# Copyright (c) 2008-2012 Hewlett-Packard Development Company, L.P.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice(s), this list of conditions and the following disclaimer as
#    the first lines of this file unmodified other than the possible
#    addition of one or more copyright notices.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice(s), this list of conditions and the following disclaimer in
#    the documentation and/or other materials provided with the
#    distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER(S) ``AS IS'' AND ANY
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER(S) BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
# BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
# OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
# EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

cmake_minimum_required(VERSION 2.8.7)

project(jemalloc C)

include(webOS/webOS)
webos_modules_init(1 0 0 QUALIFIER RC2)
webos_upstream_from_repo(0.20080828a 9)

webos_build_pkgconfig(files/pkgconfig/jemalloc)
webos_build_pkgconfig(files/pkgconfig/jemalloc_mt)

webos_add_compiler_flags(ALL -g -DMALLOC_PRODUCTION)

add_library(jemalloc SHARED src/jemalloc_linux.c src/jemalloc_wrapper.c)
set_property(TARGET jemalloc PROPERTY COMPILE_DEFINITIONS __isthreaded=false)
webos_build_library(NAME jemalloc NOHEADERS) 

add_library(jemalloc_mt SHARED src/jemalloc_linux.c src/jemalloc_wrapper.c)
set_property(TARGET jemalloc_mt PROPERTY COMPILE_DEFINITIONS __isthreaded=true)
webos_build_library(NAME jemalloc_mt NOHEADERS) 

