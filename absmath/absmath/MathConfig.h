/**
 * Copyright 2017-2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once

// DirectXMath also works on Windows on ARM and it should also compile on Linux now.

#include <initializer_list>
#include <sa/Compiler.h>

#define HAVE_DIRECTX_MATH
/*
 * BUILD_INTRINSICS_LEVEL 0..3. Try setting different levels and see what compiles.
 * 0 _XM_NO_INTRINSICS_
 * 1 Some
 * 2 More
 * 3 All
 *
 * If not defined it tries to auto detect it.
 * On Linux run `cat /proc/cpuinfo | grep flags` to see what
 * the processor supports, and `g++ -march=native -Q --help=target`
 * what the compiler turns on, on the current processor.
 */

// If BUILD_INTRINSICS_LEVEL not defined try to auto detect.
#if !defined (BUILD_INTRINSICS_LEVEL)
#   ifdef _MSC_VER
#       if _MSC_VER >= 1300
#           define _XM_SSE_INTRINSICS_
#       endif
#       if _MSC_VER >= 1500
#           define _XM_SSE3_INTRINSICS_
#           define _XM_SSE4_INTRINSICS_
#       endif
#       if _MSC_FULL_VER >= 160040219
#           define _XM_AVX_INTRINSICS_
#       endif
#       if _MSC_VER >= 1700
// No support for AVX2 on my processor, Ivy Bridge
//#           define _XM_AVX2_INTRINSICS_
// 16-bit Floating-point conversion instructions
#           define _XM_F16C_INTRINSICS_
#       endif
#   else
#       if defined(__SSE__)
#           define _XM_SSE_INTRINSICS_
#       endif
#       if defined(__SSE3__)
#           define _XM_SSE3_INTRINSICS_
#       endif
#       if defined(__SSE4__) || defined(__SSE4_1__) || defined(__SSE4_2__)
#           define _XM_SSE4_INTRINSICS_
#       endif
#       if defined(__AVX__)
#           define _XM_AVX_INTRINSICS_
#       endif
#       if defined(__AVX2__)
#           define _XM_AVX2_INTRINSICS_
#       endif
#       if defined(__F16C__)
#           define _XM_F16C_INTRINSICS_
#       endif
#   endif
#endif

#if defined(BUILD_INTRINSICS_LEVEL)
#   if defined(SA_ARCH_ARM)
#       if defined(__ARM_NEON) && BUILD_INTRINSICS_LEVEL > 0
#           define _XM_ARM_NEON_INTRINSICS_
#       else
#           define _XM_NO_INTRINSICS_
#       endif
#   else
#       if BUILD_INTRINSICS_LEVEL == 0
#           define _XM_NO_INTRINSICS_
#       endif
#       if BUILD_INTRINSICS_LEVEL > 0
#           define _XM_SSE_INTRINSICS_
#       endif
#       if BUILD_INTRINSICS_LEVEL > 1
#           define _XM_SSE3_INTRINSICS_
#           define _XM_SSE4_INTRINSICS_
#           define _XM_AVX_INTRINSICS_
// No support for AVX2 on my processor, Ivy Bridge
//#           define _XM_AVX2_INTRINSICS_
#       endif
#       if BUILD_INTRINSICS_LEVEL > 2
#           define _XM_F16C_INTRINSICS_
#       endif
#   endif
#endif

#if defined(SA_GCC) || defined(SA_PLATFORM_ANDROID)
#   define _XM_NO_CALL_CONVENTION_
#endif
#if defined(SA_PLATFORM_IOS) || defined(SA_PLATFORM_ANDROID)
#   define _XM_ARM_NEON_NO_ALIGN_
#endif

#if defined(HAVE_DIRECTX_MATH)
    PRAGMA_WARNING_PUSH
        PRAGMA_WARNING_DISABLE_GCC("-Wpadded")
        PRAGMA_WARNING_DISABLE_GCC("-Wunused-but-set-variable")
        PRAGMA_WARNING_DISABLE_CLANG("-Wpadded")
        PRAGMA_WARNING_DISABLE_CLANG("-Wunused-but-set-variable")
#       include <DirectXMath.h>
#       include <DirectXCollision.h>
    PRAGMA_WARNING_POP
namespace XMath = DirectX;
#endif
