//==============================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
// =============================================================
#ifndef __UNROLLED_LOOP_HPP__
#define __UNROLLED_LOOP_HPP__
#pragma once

#include <type_traits>
#include <utility>

///////////////////////////////////////////////////////////////////////////////
//
// This header defines the UnrolledLoop utility to create unrolled loops in
// DPC++ FPGA designs.
//
// Note:
// DPC++ also supports #pragma unroll <k> to fully or partially unroll
// a loop.
// The UnrolledLoop approach should be used instead of that pragma,
// wherever the loop induction variable must be constexpr, e.g.,
// when indexing into a PipeArray (see pipe_utils.hpp).
//
// UnrolledLoop is a constexpr template function.
// The code for the body of the loop is passed to UnrolledLoop as a lambda function.
// The lambda function must take a single argument, of type auto, which acts as the
// loop's induction variable.
// The iterations of the loop are either passed to UnrolledLoop as a std::integer_sequence,
// or described by explicit template parameters of UnrolledLoop.
//
// Example usage for UnrolledLoop:
//
// Base: iterations are passed as an argument to UnrolledLoop
//    /* i is a std::integral_constant wrapping a static constant of type int */
//    UnrolledLoop(std::integer_sequence<int,5,2,7,8>{},[&](auto i) {
//      /* i = 5,2,7,8 */
//    });
//
// Cases A-D: iterations are described by explicit template parameters
//
// Case A
//    /* i is a std::integral_constant wrapping a static constant of type int */
//    UnrolledLoop<int, 10>([&](auto i) {
//      /* i = 0,1,...,9 */
//    });
//
// Case B
//    /* i is a std::integral_constant wrapping a static constant of
//       type std::size_t */
//    UnrolledLoop<10>([&](auto i) {
//      /* i = 0,1,...,9 */
//    });
//
// Case C
//    /* note iterations start at 1 rather than 0 */
//    UnrolledLoop<char, 1, 10>([&](auto i) {
//      /* i = 1,2,...,9 */
//    });
//    /* note 'end' is always exclusive, so final iteration is i=2 */
//    UnrolledLoop<char, 10 /*start*/, 1 /*end*/>([&](auto i) {
//      /* i = 10,9,...,2 */
//    });
//
// Case D
//    UnrolledLoop<1, 10>([&](auto i) {
//      /* i = 1,2,...,9 */
//    });
//    UnrolledLoop<10, 1>([&](auto i) {
//      /* i = 10,9,...,2 */
//    });
//
///////////////////////////////////////////////////////////////////////////////

namespace fpga_tools {

// =============================================================
// Internal Helper Functions/Structs
// =============================================================

namespace detail {

template <typename T, typename, T begin, bool increase>
struct integer_range_impl;

// incrementing case
template <typename T, T... N, T begin>
struct integer_range_impl<T, std::integer_sequence<T, N...>, begin, true> {
  using type = std::integer_sequence<T, N + begin...>;
};

// decrementing case
template <typename T, T... N, T begin>
struct integer_range_impl<T, std::integer_sequence<T, N...>, begin, false> {
  using type = std::integer_sequence<T, begin - N...>;
};

// integer_range
template <typename T, T begin, T end>
using integer_range = typename integer_range_impl<T,
    std::make_integer_sequence<T, (begin < end) ? end - begin : begin - end>,
    begin,
    (begin < end)>::type;

/*
 make_integer_range and make_index_range are akin to
 std::make_integer_sequence and std::make_index_sequence, respectively.
 However they represent a range, rather than an arbitrary sequence. The
 range can either be increasing or decreasing.

 1. make_integer_range (choose an integral type)

 USAGE:
    make_integer_range<int,1,10>{} ==> 1,2,...,9
    make_integer_range<int,10,1>{} ==> 10,9,...,2

 2. make_index_range (std::size_t)

 USAGE:
    make_index_range<1,10>{} ==> 1,2,...,9
    make_index_range<10,1>{} ==> 10,9,...,2

*/
template <class T, T begin, T end>
using make_integer_range = integer_range<T, begin, end>;

template <std::size_t begin, std::size_t end>
using make_index_range = integer_range<std::size_t, begin, end>;

} // namespace detail

//
// Base implementation
// Templated on:
//    ItType    - the type of the iterator (size_t, int, char, ...)
//    ItType... - the indices to iterate on
//    F         - the function to run for each index (i.e. the lamda)
//
template <class ItType, ItType... inds, class F>
constexpr void UnrolledLoop(std::integer_sequence<ItType, inds...>, F&& f) {
  (f(std::integral_constant<ItType, inds>{}), ...);
}

//
// Case A
// performs UnrolledLoop in range [0,n) with iterator of type ItType
//
template <class ItType, ItType n, class F>
constexpr void UnrolledLoop(F&& f) {
  UnrolledLoop(std::make_integer_sequence<ItType, n>{}, std::forward<F>(f));
}

//
// Case B
// performs UnrolledLoop in range [0,n) with an iterator of type std::size_t
//
template <std::size_t n, class F>
constexpr void UnrolledLoop(F&& f) {
  UnrolledLoop(std::make_index_sequence<n>{}, std::forward<F>(f));
}

//
// Case C
// performs UnrolledLoop from start...end with an iterator of type ItType
// NOTE:  start is INCLUSIVE, end is EXCLUSIVE
// NOTE:  if start<=end, sequence is start,start+1,...,end-1
//        if end<=start, sequence is start,start-1,...,end+1
//
template <class ItType, ItType start, ItType end, class F>
constexpr void UnrolledLoop(F&& f) {
  UnrolledLoop(detail::make_integer_range<ItType, start, end>{}, std::forward<F>(f));
}

//
// Case D
// performs UnrolledLoop from start...end with an iterator of type std::size_t
// NOTE:  start is INCLUSIVE, end is EXCLUSIVE
// NOTE:  if start<=end, sequence is start,start+1,...,end-1
//        if end<=start, sequence is start,start-1,...,end+1
//
template <std::size_t start, std::size_t end, class F>
constexpr void UnrolledLoop(F&& f) {
  UnrolledLoop(detail::make_index_range<start, end>{}, std::forward<F>(f));
}

} // namespace fpga_tools
#endif /* __UNROLLED_LOOP_HPP__ */
