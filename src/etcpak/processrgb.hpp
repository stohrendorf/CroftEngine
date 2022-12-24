#pragma once

#include <cstddef>
#include <cstdint>

extern void compressEtc2Bgra(const uint32_t* bgra, uint64_t* dst, uint32_t blocks, size_t width, bool useHeuristics);
