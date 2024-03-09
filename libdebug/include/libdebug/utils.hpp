//  Copyright 2024 Cach30verfl0w
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//  http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.

#pragma once

/**
 * @author Alexander Hinze
 * @since 26/04/2023
 */

#include <cstdint>
#include <type_traits>

namespace libdebug {
    // NOLINTBEGIN
    using i8 = std::int8_t;
    using i16 = std::int16_t;
    using i32 = std::int32_t;
    using i64 = std::int64_t;
    using isize = std::ptrdiff_t;
    // NOLINTEND

    static_assert(sizeof(i8) == 1);
    static_assert(sizeof(i16) == 2);
    static_assert(sizeof(i32) == 4);
    static_assert(sizeof(i64) == 8);// NOLINT
    static_assert(sizeof(isize) == sizeof(void*));

    // NOLINTBEGIN
    using u8 = std::uint8_t;
    using u16 = std::uint16_t;
    using u32 = std::uint32_t;
    using u64 = std::uint64_t;
    using usize = std::size_t;
    // NOLINTEND

    static_assert(sizeof(u8) == 1);
    static_assert(sizeof(u16) == 2);
    static_assert(sizeof(u32) == 4);
    static_assert(sizeof(u64) == 8);// NOLINT
    static_assert(sizeof(usize) == sizeof(void*));

    // NOLINTBEGIN
    using f32 = float;
    using f64 = double;
    using f64ex = long double;
    // NOLINTEND

    static_assert(sizeof(f32) == 4);
    static_assert(sizeof(f64) == 8);                         // NOLINT
    static_assert(sizeof(f64ex) >= 8 && sizeof(f64ex) <= 16);// NOLINT
}// namespace libdebug
