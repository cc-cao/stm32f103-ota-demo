#pragma once

#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

// 整数类型（带符号）
typedef signed char i8;
typedef short i16;
typedef int i32;
typedef long long i64;

// 整数类型（无符号）
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

// 浮点类型
typedef float f32;
typedef double f64;

// 指针大小类型（平台相关）
typedef ssize_t isize;
typedef size_t usize;

#ifdef __cplusplus
}
#endif
