##===----------------------------------------------------------------------===##
##  _ _ _      _   ___ ___   ______
## | (_) |__  /_\ | __/ __| / /__  |
## | | | '_ \/ _ \| _|\__ \/ _ \/ /
## |_|_|_.__/_/ \_\___|___/\___/_/
##
## AES67 real-time audio-over-IP streaming library for C/C++.
##
## This source file is part of the libAES67 open source project
##
## Copyright (c) 2025 - 2026 Nevio Hirani. All rights reserved.
##
## Redistribution and use in source and binary forms, with or without
## modification, are permitted provided that the following conditions are met:
##
##  1. Redistributions of source code must retain the above copyright notice, this
##     list of conditions and the following disclaimer.
##
##  2. Redistributions in binary form must reproduce the above copyright notice,
##     this list of conditions and the following disclaimer in the documentation
##     and/or other materials provided with the distribution.
##
##  3. Neither the name of the copyright holder nor the names of its
##     contributors may be used to endorse or promote products derived from
##     this software without specific prior written permission.
##
## THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
## AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
## IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
## DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
## FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
## DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
## SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
## CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
## OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
## OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
##===----------------------------------------------------------------------===##

cmake_minimum_required(VERSION 4.1)
project(libAES67 C CXX)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
enable_testing()

option(BUILD_SHARED_LIBS "Build shared libraries instead of static ones" OFF)

add_library(libAES67 STATIC libAES67.c
        include/libAES67/platform.h
        include/libAES67/endian.h
        include/libAES67/time.h
        src/time.c
        include/libAES67/__tai.h
        include/libAES67/__posix_tz.h
)

target_include_directories(libAES67 PUBLIC include)

if (WIN32)
    message(FATAL_ERROR "Windows builds are not supported yet.")
endif()

if (MSVC)
    message(FATAL_ERROR "MSVC compiler is not supported.")
endif()
