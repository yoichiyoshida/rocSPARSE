/* ************************************************************************
 * Copyright (c) 2018 Advanced Micro Devices, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * ************************************************************************ */

#pragma once
#ifndef COMMON_H
#define COMMON_H

#include <hip/hip_runtime.h>

// clang-format off
__device__ __forceinline__ float rocsparse_ldg(const float* ptr) { return __ldg(ptr); }
__device__ __forceinline__ double rocsparse_ldg(const double* ptr) { return __ldg(ptr); }
__device__ __forceinline__ rocsparse_int rocsparse_ldg(const rocsparse_int* ptr) { return __ldg(ptr); }

#if defined(__HIP_PLATFORM_HCC__)
__device__ __forceinline__ float rocsparse_nontemporal_load(const float* ptr) { return __builtin_nontemporal_load(ptr); }
__device__ __forceinline__ double rocsparse_nontemporal_load(const double* ptr) { return __builtin_nontemporal_load(ptr); }
__device__ __forceinline__ rocsparse_int rocsparse_nontemporal_load(const rocsparse_int* ptr) { return __builtin_nontemporal_load(ptr); }
#elif defined(__HIP_PLATFORM_NVCC__)
__device__ __forceinline__ float rocsparse_nontemporal_load(const float* ptr) { return *ptr; }
__device__ __forceinline__ double rocsparse_nontemporal_load(const double* ptr) { return *ptr; }
__device__ __forceinline__ rocsparse_int rocsparse_nontemporal_load(const rocsparse_int* ptr) { return *ptr; }
#endif

#if defined(__HIP_PLATFORM_HCC__)
__device__ __forceinline__ void rocsparse_nontemporal_store(float val, float* ptr) { __builtin_nontemporal_store(val, ptr); }
__device__ __forceinline__ void rocsparse_nontemporal_store(double val, double* ptr) { __builtin_nontemporal_store(val, ptr); }
__device__ __forceinline__ void rocsparse_nontemporal_store(rocsparse_int val, rocsparse_int* ptr) { __builtin_nontemporal_store(val, ptr); }
#elif defined(__HIP_PLATFORM_NVCC__)
__device__ __forceinline__ void rocsparse_nontemporal_store(float val, float* ptr) { *ptr = val; }
__device__ __forceinline__ void rocsparse_nontemporal_store(double val, double* ptr) { *ptr = val; }
__device__ __forceinline__ void rocsparse_nontemporal_store(rocsparse_int val, rocsparse_int* ptr) { *ptr = val; }
#endif

__device__ __forceinline__ float rocsparse_fma(float p, float q, float r) { return fma(p, q, r); }
__device__ __forceinline__ double rocsparse_fma(double p, double q, double r) { return fma(p, q, r); }

__device__ __forceinline__ float rocsparse_rcp(float val) { return 1.0f / val; }
__device__ __forceinline__ double rocsparse_rcp(double val) { return 1.0 / val; }

__device__ __forceinline__ int32_t rocsparse_mul24(int32_t x, int32_t y) { return ((x << 8) >> 8) * ((y << 8) >> 8); }
__device__ __forceinline__ int64_t rocsparse_mul24(int64_t x, int64_t y) { return ((x << 40) >> 40) * ((y << 40) >> 40); }

__device__ __forceinline__ rocsparse_int rocsparse_mad24(rocsparse_int x, rocsparse_int y, rocsparse_int z) { return rocsparse_mul24(x, y) + z; }

#if defined(__HIP_PLATFORM_HCC__)
__device__ __forceinline__ rocsparse_int rocsparse_atomic_load(const rocsparse_int* ptr, int memorder) { return __atomic_load_n(ptr, memorder); }
#elif defined(__HIP_PLATFORM_NVCC__)
__device__ __forceinline__ rocsparse_int rocsparse_atomic_load(const rocsparse_int* ptr, int memorder)
{
    const volatile rocsparse_int* vptr = ptr;
    __threadfence();
    rocsparse_int val = *vptr;
    __threadfence();

    return val;
}
#endif

#if defined(__HIP_PLATFORM_HCC__)
__device__ __forceinline__ void rocsparse_atomic_store(rocsparse_int* ptr, rocsparse_int val, int memorder) { __atomic_store_n(ptr, val, memorder); }
#elif defined(__HIP_PLATFORM_NVCC__)
__device__ __forceinline__ void rocsparse_atomic_store(rocsparse_int* ptr, rocsparse_int val, int memorder)
{
    volatile rocsparse_int* vptr = ptr;
    __threadfence();
    *vptr = val;
}
#endif

__device__ __forceinline__ void rocsparse_atomic_add(float* ptr, float val)
{
    unsigned int new_val;
    unsigned int prev_val;

    do
    {
        prev_val = __float_as_uint(*ptr);
        new_val  = __float_as_uint(val + *ptr);
    } while(atomicCAS((unsigned int*)ptr, prev_val, new_val) != prev_val);
}

__device__ __forceinline__ void rocsparse_atomic_add(double* ptr, double val)
{
    unsigned long long new_val;
    unsigned long long prev_val;

    do
    {
        prev_val = __double_as_longlong(*ptr);
        new_val  = __double_as_longlong(val + *ptr);
    } while(atomicCAS((unsigned long long*)ptr, prev_val, new_val) != prev_val);
}

// Block reduce kernel computing block sum
template <typename T, unsigned int BLOCKSIZE>
__device__ __forceinline__ void rocsparse_blockreduce_sum(int i, T* data)
{
    if(BLOCKSIZE > 512) { if(i < 512 && i + 512 < BLOCKSIZE) { data[i] += data[i + 512]; } __syncthreads(); }
    if(BLOCKSIZE > 256) { if(i < 256 && i + 256 < BLOCKSIZE) { data[i] += data[i + 256]; } __syncthreads(); }
    if(BLOCKSIZE > 128) { if(i < 128 && i + 128 < BLOCKSIZE) { data[i] += data[i + 128]; } __syncthreads(); }
    if(BLOCKSIZE >  64) { if(i <  64 && i +  64 < BLOCKSIZE) { data[i] += data[i +  64]; } __syncthreads(); }
    if(BLOCKSIZE >  32) { if(i <  32 && i +  32 < BLOCKSIZE) { data[i] += data[i +  32]; } __syncthreads(); }
    if(BLOCKSIZE >  16) { if(i <  16 && i +  16 < BLOCKSIZE) { data[i] += data[i +  16]; } __syncthreads(); }
    if(BLOCKSIZE >   8) { if(i <   8 && i +   8 < BLOCKSIZE) { data[i] += data[i +   8]; } __syncthreads(); }
    if(BLOCKSIZE >   4) { if(i <   4 && i +   4 < BLOCKSIZE) { data[i] += data[i +   4]; } __syncthreads(); }
    if(BLOCKSIZE >   2) { if(i <   2 && i +   2 < BLOCKSIZE) { data[i] += data[i +   2]; } __syncthreads(); }
    if(BLOCKSIZE >   1) { if(i <   1 && i +   1 < BLOCKSIZE) { data[i] += data[i +   1]; } __syncthreads(); }
}

// Block reduce kernel computing blockwide maximum entry
template <typename T, unsigned int BLOCKSIZE>
__device__ __forceinline__ void rocsparse_blockreduce_max(int i, T* data)
{
    if(BLOCKSIZE > 512) { if(i < 512 && i + 512 < BLOCKSIZE) { data[i] = max(data[i], data[i + 512]); } __syncthreads(); }
    if(BLOCKSIZE > 256) { if(i < 256 && i + 256 < BLOCKSIZE) { data[i] = max(data[i], data[i + 256]); } __syncthreads(); }
    if(BLOCKSIZE > 128) { if(i < 128 && i + 128 < BLOCKSIZE) { data[i] = max(data[i], data[i + 128]); } __syncthreads(); }
    if(BLOCKSIZE >  64) { if(i <  64 && i +  64 < BLOCKSIZE) { data[i] = max(data[i], data[i +  64]); } __syncthreads(); }
    if(BLOCKSIZE >  32) { if(i <  32 && i +  32 < BLOCKSIZE) { data[i] = max(data[i], data[i +  32]); } __syncthreads(); }
    if(BLOCKSIZE >  16) { if(i <  16 && i +  16 < BLOCKSIZE) { data[i] = max(data[i], data[i +  16]); } __syncthreads(); }
    if(BLOCKSIZE >   8) { if(i <   8 && i +   8 < BLOCKSIZE) { data[i] = max(data[i], data[i +   8]); } __syncthreads(); }
    if(BLOCKSIZE >   4) { if(i <   4 && i +   4 < BLOCKSIZE) { data[i] = max(data[i], data[i +   4]); } __syncthreads(); }
    if(BLOCKSIZE >   2) { if(i <   2 && i +   2 < BLOCKSIZE) { data[i] = max(data[i], data[i +   2]); } __syncthreads(); }
    if(BLOCKSIZE >   1) { if(i <   1 && i +   1 < BLOCKSIZE) { data[i] = max(data[i], data[i +   1]); } __syncthreads(); }
}

#if defined(__HIP_PLATFORM_HCC__)
__device__ int __llvm_amdgcn_readlane(int index, int offset) __asm("llvm.amdgcn.readlane");

// DPP-based wavefront reduction combination of sum and max
template <unsigned int WFSIZE>
__device__ __forceinline__ void rocsparse_wfreduce_sum_max(rocsparse_int* sum,
                                                           rocsparse_int* maximum)
{
    if(WFSIZE > 1)
    {
        // row_shr = 1
        *maximum = max(*maximum, __hip_move_dpp(*maximum, 0x111, 0xf, 0xf, 0));
        *sum += __hip_move_dpp(*sum, 0x111, 0xf, 0xf, 0);
    }

    if(WFSIZE > 2)
    {
        // row_shr = 2
        *maximum = max(*maximum, __hip_move_dpp(*maximum, 0x112, 0xf, 0xf, 0));
        *sum += __hip_move_dpp(*sum, 0x112, 0xf, 0xf, 0);
    }

    if(WFSIZE > 4)
    {
        // row_shr = 4 ; bank_mask = 0xe
        *maximum = max(*maximum, __hip_move_dpp(*maximum, 0x114, 0xf, 0xe, 0));
        *sum += __hip_move_dpp(*sum, 0x114, 0xf, 0xe, 0);
    }

    if(WFSIZE > 8)
    {
        // row_shr = 8 ; bank_mask = 0xc
        *maximum = max(*maximum, __hip_move_dpp(*maximum, 0x118, 0xf, 0xc, 0));
        *sum += __hip_move_dpp(*sum, 0x118, 0xf, 0xc, 0);
    }

    if(WFSIZE > 16)
    {
        // row_bcast = 15 ; row_mask = 0xa
        *maximum = max(*maximum, __hip_move_dpp(*maximum, 0x142, 0xa, 0xf, 0));
        *sum += __hip_move_dpp(*sum, 0x142, 0xa, 0xf, 0);
    }

    if(WFSIZE > 32)
    {
        // row_bcast = 31 ; row_mask = 0xc
        *maximum = max(*maximum, __hip_move_dpp(*maximum, 0x143, 0xc, 0xf, 0));
        *sum += __hip_move_dpp(*sum, 0x143, 0xc, 0xf, 0);
    }
}

// Swizzle-based float wavefront reduction sum
template <unsigned int WFSIZE>
__device__ __forceinline__ float rocsparse_wfreduce_sum(float sum)
{
    typedef union flt_b32
    {
        float val;
        uint32_t b32;
    } flt_b32_t;

    flt_b32_t upper_sum;
    flt_b32_t temp_sum;
    temp_sum.val = sum;

    if(WFSIZE > 1)
    {
        upper_sum.b32 = __hip_ds_swizzle(temp_sum.b32, 0x80b1);
        temp_sum.val += upper_sum.val;
    }

    if(WFSIZE > 2)
    {
        upper_sum.b32 = __hip_ds_swizzle(temp_sum.b32, 0x804e);
        temp_sum.val += upper_sum.val;
    }

    if(WFSIZE > 4)
    {
        upper_sum.b32 = __hip_ds_swizzle(temp_sum.b32, 0x101f);
        temp_sum.val += upper_sum.val;
    }

    if(WFSIZE > 8)
    {
        upper_sum.b32 = __hip_ds_swizzle(temp_sum.b32, 0x201f);
        temp_sum.val += upper_sum.val;
    }

    if(WFSIZE > 16)
    {
        upper_sum.b32 = __hip_ds_swizzle(temp_sum.b32, 0x401f);
        temp_sum.val += upper_sum.val;
    }

    if(WFSIZE > 32)
    {
        upper_sum.b32 = __llvm_amdgcn_readlane(temp_sum.b32, 32);
        temp_sum.val += upper_sum.val;
    }

    sum = temp_sum.val;
    return sum;
}

// Swizzle-based double wavefront reduction
template <unsigned int WFSIZE>
__device__ __forceinline__ double rocsparse_wfreduce_sum(double sum)
{
    typedef union dbl_b32
    {
        double val;
        uint32_t b32[2];
    } dbl_b32_t;

    dbl_b32_t upper_sum;
    dbl_b32_t temp_sum;
    temp_sum.val = sum;

    if(WFSIZE > 1)
    {
        upper_sum.b32[0] = __hip_ds_swizzle(temp_sum.b32[0], 0x80b1);
        upper_sum.b32[1] = __hip_ds_swizzle(temp_sum.b32[1], 0x80b1);
        temp_sum.val += upper_sum.val;
    }

    if(WFSIZE > 2)
    {
        upper_sum.b32[0] = __hip_ds_swizzle(temp_sum.b32[0], 0x804e);
        upper_sum.b32[1] = __hip_ds_swizzle(temp_sum.b32[1], 0x804e);
        temp_sum.val += upper_sum.val;
    }

    if(WFSIZE > 4)
    {
        upper_sum.b32[0] = __hip_ds_swizzle(temp_sum.b32[0], 0x101f);
        upper_sum.b32[1] = __hip_ds_swizzle(temp_sum.b32[1], 0x101f);
        temp_sum.val += upper_sum.val;
    }

    if(WFSIZE > 8)
    {
        upper_sum.b32[0] = __hip_ds_swizzle(temp_sum.b32[0], 0x201f);
        upper_sum.b32[1] = __hip_ds_swizzle(temp_sum.b32[1], 0x201f);
        temp_sum.val += upper_sum.val;
    }

    if(WFSIZE > 16)
    {
        upper_sum.b32[0] = __hip_ds_swizzle(temp_sum.b32[0], 0x401f);
        upper_sum.b32[1] = __hip_ds_swizzle(temp_sum.b32[1], 0x401f);
        temp_sum.val += upper_sum.val;
    }

    if(WFSIZE > 32)
    {
        upper_sum.b32[0] = __llvm_amdgcn_readlane(temp_sum.b32[0], 32);
        upper_sum.b32[1] = __llvm_amdgcn_readlane(temp_sum.b32[1], 32);
        temp_sum.val += upper_sum.val;
    }

    sum = temp_sum.val;
    return sum;
}
#elif defined(__HIP_PLATFORM_NVCC__)
template <unsigned int WFSIZE>
__device__ __forceinline__ void rocsparse_wfreduce_sum_max(rocsparse_int* sum,
                                                           rocsparse_int* maximum)
{
    for(unsigned int i = WFSIZE >> 1; i > 0; i >>= 1)
    {
        *maximum = max(*maximum, __shfl_down_sync(0xffffffff, *maximum, i));
        *sum += __shfl_down_sync(0xffffffff, *sum, i);
    }
}

template <unsigned int WFSIZE, typename T>
__device__ __forceinline__ T rocsparse_wfreduce_sum(T sum)
{
    for(unsigned int i = WFSIZE >> 1; i > 0; i >>= 1)
    {
        sum += __shfl_down_sync(0xffffffff, sum, i);
    }

    return sum;
}
#endif
// clang-format on

#endif // COMMON_H
