#ifndef HEADER_DEFINITIONS_H
#define HEADER_DEFINITIONS_H

#include <cstdint>

using i8  = std::int8_t;
using i16 = std::int16_t;
using i32 = std::int32_t;
using i64 = std::int64_t;

using u8  = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

using f32 = float;
using f64 = double;

static_assert(sizeof(i8) == 1, "Size of i8 is not 1 byte.");
static_assert(sizeof(i16) == 2, "Size of i16 is not 2 bytes.");
static_assert(sizeof(i32) == 4, "Size of i32 is not 4 bytes.");
static_assert(sizeof(i64) == 8, "Size of i64 is not 8 bytes.");

static_assert(sizeof(u8) == 1, "Size of u8 is not 1 byte.");
static_assert(sizeof(u16) == 2, "Size of u16 is not 2 bytes.");
static_assert(sizeof(u32) == 4, "Size of u32 is not 4 bytes.");
static_assert(sizeof(u64) == 8, "Size of u64 is not 8 bytes.");

static_assert(sizeof(f32) == 4, "Size of f32 is not 4 bytes.");
static_assert(sizeof(f64) == 8, "Size of f64 is not 8 bytes.");

#define COV_API __declspec(dllexport)

#endif// HEADER_DEFINITIONS_H
