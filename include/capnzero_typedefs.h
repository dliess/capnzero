#ifndef CAPNZERO_TYPEDEFS_H
#define CAPNZERO_TYPEDEFS_H

#include <string>
#include <string_view>
#include <array>
#include <vector>
#include <cstdint>
#include "tcb/span.hpp"

namespace capnzero{

using Text = std::string;
using TextView = std::string_view;
using Span = tcb::span<const uint8_t, tcb::dynamic_extent>;
template<std::size_t SIZE>
using SpanCL = tcb::span<const uint8_t, SIZE>;
template<std::size_t SIZE>
using Data = std::array<uint8_t, SIZE>;


using Int8 = int8_t;
using Int16 = int16_t;
using Int32 = int32_t;
using Int64 = int64_t;
using UInt8 = uint8_t;
using UInt16 = uint16_t;
using UInt32 = uint32_t;
using UInt64 = uint64_t;
using Float32 = float;
using Float64 = double;

} // namespace capnzero

#endif