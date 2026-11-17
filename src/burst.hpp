#pragma once

#include <cstddef>

namespace burst {

constexpr size_t LENGTH_BUFFERS = 10;
constexpr size_t GAP_BUFFERS    = 10;
constexpr size_t CYCLE_BUFFERS  = LENGTH_BUFFERS + GAP_BUFFERS;

inline bool isBurst(size_t bufferIndex)
{
    return (bufferIndex % CYCLE_BUFFERS) < LENGTH_BUFFERS;
}

} // namespace burst