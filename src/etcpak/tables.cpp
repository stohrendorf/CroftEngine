#include "tables.hpp"

#include "ssevec.h"

#include <array>
#include <cstdint>
#include <gsl/gsl-lite.hpp>

namespace
{
const std::array<int16_t, 16>& getAlphaRange()
{
  static const std::array<int16_t, 16> alphaRange{{
    gsl::narrow_cast<int16_t>(0x100FF / (1 + getAlpha()[0][7] - getAlpha()[0][3])),
    gsl::narrow_cast<int16_t>(0x100FF / (1 + getAlpha()[1][7] - getAlpha()[1][3])),
    gsl::narrow_cast<int16_t>(0x100FF / (1 + getAlpha()[2][7] - getAlpha()[2][3])),
    gsl::narrow_cast<int16_t>(0x100FF / (1 + getAlpha()[3][7] - getAlpha()[3][3])),
    gsl::narrow_cast<int16_t>(0x100FF / (1 + getAlpha()[4][7] - getAlpha()[4][3])),
    gsl::narrow_cast<int16_t>(0x100FF / (1 + getAlpha()[5][7] - getAlpha()[5][3])),
    gsl::narrow_cast<int16_t>(0x100FF / (1 + getAlpha()[6][7] - getAlpha()[6][3])),
    gsl::narrow_cast<int16_t>(0x100FF / (1 + getAlpha()[7][7] - getAlpha()[7][3])),
    gsl::narrow_cast<int16_t>(0x100FF / (1 + getAlpha()[8][7] - getAlpha()[8][3])),
    gsl::narrow_cast<int16_t>(0x100FF / (1 + getAlpha()[9][7] - getAlpha()[9][3])),
    gsl::narrow_cast<int16_t>(0x100FF / (1 + getAlpha()[10][7] - getAlpha()[10][3])),
    gsl::narrow_cast<int16_t>(0x100FF / (1 + getAlpha()[11][7] - getAlpha()[11][3])),
    gsl::narrow_cast<int16_t>(0x100FF / (1 + getAlpha()[12][7] - getAlpha()[12][3])),
    gsl::narrow_cast<int16_t>(0x100FF / (1 + getAlpha()[13][7] - getAlpha()[13][3])),
    gsl::narrow_cast<int16_t>(0x100FF / (1 + getAlpha()[14][7] - getAlpha()[14][3])),
    gsl::narrow_cast<int16_t>(0x100FF / (1 + getAlpha()[15][7] - getAlpha()[15][3])),
  }};
  return alphaRange;
}
} // namespace

const std::array<std::array<int32_t, 4>, 8>& getTable()
{
  static const std::array<std::array<int32_t, 4>, 8> table{{
    {2, 8, -2, -8},
    {5, 17, -5, -17},
    {9, 29, -9, -29},
    {13, 42, -13, -42},
    {18, 60, -18, -60},
    {24, 80, -24, -80},
    {33, 106, -33, -106},
    {47, 183, -47, -183},
  }};
  return table;
}

const std::array<std::array<uint32_t, 16>, 4>& getId()
{
  static const std::array<std::array<uint32_t, 16>, 4> id{{
    {1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0},
    {3, 3, 2, 2, 3, 3, 2, 2, 3, 3, 2, 2, 3, 3, 2, 2},
    {5, 5, 5, 5, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 4},
    {7, 7, 6, 6, 7, 7, 6, 6, 7, 7, 6, 6, 7, 7, 6, 6},
  }};
  return id;
}

const std::array<uint32_t, 64>& getFlags()
{
  static const std::array<uint32_t, 64> flags{
    {0x80800402, 0x80800402, 0x80800402, 0x80800402, 0x80800402, 0x80800402, 0x80800402, 0x8080E002,
     0x80800402, 0x80800402, 0x8080E002, 0x8080E002, 0x80800402, 0x8080E002, 0x8080E002, 0x8080E002,
     0x80000402, 0x80000402, 0x80000402, 0x80000402, 0x80000402, 0x80000402, 0x80000402, 0x8000E002,
     0x80000402, 0x80000402, 0x8000E002, 0x8000E002, 0x80000402, 0x8000E002, 0x8000E002, 0x8000E002,
     0x00800402, 0x00800402, 0x00800402, 0x00800402, 0x00800402, 0x00800402, 0x00800402, 0x0080E002,
     0x00800402, 0x00800402, 0x0080E002, 0x0080E002, 0x00800402, 0x0080E002, 0x0080E002, 0x0080E002,
     0x00000402, 0x00000402, 0x00000402, 0x00000402, 0x00000402, 0x00000402, 0x00000402, 0x0000E002,
     0x00000402, 0x00000402, 0x0000E002, 0x0000E002, 0x00000402, 0x0000E002, 0x0000E002, 0x0000E002}};
  return flags;
}

const std::array<std::array<int16_t, 8>, 16>& getAlpha()
{
  static const std::array<std::array<int16_t, 8>, 16> alpha{{
    {-3, -6, -9, -15, 2, 5, 8, 14},
    {-3, -7, -10, -13, 2, 6, 9, 12},
    {-2, -5, -8, -13, 1, 4, 7, 12},
    {-2, -4, -6, -13, 1, 3, 5, 12},
    {-3, -6, -8, -12, 2, 5, 7, 11},
    {-3, -7, -9, -11, 2, 6, 8, 10},
    {-4, -7, -8, -11, 3, 6, 7, 10},
    {-3, -5, -8, -11, 2, 4, 7, 10},
    {-2, -6, -8, -10, 1, 5, 7, 9},
    {-2, -5, -8, -10, 1, 4, 7, 9},
    {-2, -4, -8, -10, 1, 3, 7, 9},
    {-2, -5, -7, -10, 1, 4, 6, 9},
    {-3, -4, -7, -10, 2, 3, 6, 9},
    {-1, -2, -3, -10, 0, 1, 2, 9},
    {-4, -6, -8, -9, 3, 5, 7, 8},
    {-3, -5, -7, -9, 2, 4, 6, 8},
  }};
  return alpha;
}

const std::array<IVec16, 2>& getTable128_SIMD()
{
  static const std::array<IVec16, 2> table128_SIMD{{
    IVec16{2 * 128, 5 * 128, 9 * 128, 13 * 128, 18 * 128, 24 * 128, 33 * 128, 47 * 128},
    IVec16{8 * 128, 17 * 128, 29 * 128, 42 * 128, 60 * 128, 80 * 128, 106 * 128, 183 * 128},
  }};
  return table128_SIMD;
}

const std::array<IVec16, 16>& getAlpha_SIMD()
{
  static const std::array<IVec16, 16> alpha_SIMD{{
    IVec16(getAlpha()[0][0],
           getAlpha()[0][1],
           getAlpha()[0][2],
           getAlpha()[0][3],
           getAlpha()[0][4],
           getAlpha()[0][5],
           getAlpha()[0][6],
           getAlpha()[0][7]),
    IVec16(getAlpha()[1][0],
           getAlpha()[1][1],
           getAlpha()[1][2],
           getAlpha()[1][3],
           getAlpha()[1][4],
           getAlpha()[1][5],
           getAlpha()[1][6],
           getAlpha()[1][7]),
    IVec16(getAlpha()[2][0],
           getAlpha()[2][1],
           getAlpha()[2][2],
           getAlpha()[2][3],
           getAlpha()[2][4],
           getAlpha()[2][5],
           getAlpha()[2][6],
           getAlpha()[2][7]),
    IVec16(getAlpha()[3][0],
           getAlpha()[3][1],
           getAlpha()[3][2],
           getAlpha()[3][3],
           getAlpha()[3][4],
           getAlpha()[3][5],
           getAlpha()[3][6],
           getAlpha()[3][7]),
    IVec16(getAlpha()[4][0],
           getAlpha()[4][1],
           getAlpha()[4][2],
           getAlpha()[4][3],
           getAlpha()[4][4],
           getAlpha()[4][5],
           getAlpha()[4][6],
           getAlpha()[4][7]),
    IVec16(getAlpha()[5][0],
           getAlpha()[5][1],
           getAlpha()[5][2],
           getAlpha()[5][3],
           getAlpha()[5][4],
           getAlpha()[5][5],
           getAlpha()[5][6],
           getAlpha()[5][7]),
    IVec16(getAlpha()[6][0],
           getAlpha()[6][1],
           getAlpha()[6][2],
           getAlpha()[6][3],
           getAlpha()[6][4],
           getAlpha()[6][5],
           getAlpha()[6][6],
           getAlpha()[6][7]),
    IVec16(getAlpha()[7][0],
           getAlpha()[7][1],
           getAlpha()[7][2],
           getAlpha()[7][3],
           getAlpha()[7][4],
           getAlpha()[7][5],
           getAlpha()[7][6],
           getAlpha()[7][7]),
    IVec16(getAlpha()[8][0],
           getAlpha()[8][1],
           getAlpha()[8][2],
           getAlpha()[8][3],
           getAlpha()[8][4],
           getAlpha()[8][5],
           getAlpha()[8][6],
           getAlpha()[8][7]),
    IVec16(getAlpha()[9][0],
           getAlpha()[9][1],
           getAlpha()[9][2],
           getAlpha()[9][3],
           getAlpha()[9][4],
           getAlpha()[9][5],
           getAlpha()[9][6],
           getAlpha()[9][7]),
    IVec16(getAlpha()[10][0],
           getAlpha()[10][1],
           getAlpha()[10][2],
           getAlpha()[10][3],
           getAlpha()[10][4],
           getAlpha()[10][5],
           getAlpha()[10][6],
           getAlpha()[10][7]),
    IVec16(getAlpha()[11][0],
           getAlpha()[11][1],
           getAlpha()[11][2],
           getAlpha()[11][3],
           getAlpha()[11][4],
           getAlpha()[11][5],
           getAlpha()[11][6],
           getAlpha()[11][7]),
    IVec16(getAlpha()[12][0],
           getAlpha()[12][1],
           getAlpha()[12][2],
           getAlpha()[12][3],
           getAlpha()[12][4],
           getAlpha()[12][5],
           getAlpha()[12][6],
           getAlpha()[12][7]),
    IVec16(getAlpha()[13][0],
           getAlpha()[13][1],
           getAlpha()[13][2],
           getAlpha()[13][3],
           getAlpha()[13][4],
           getAlpha()[13][5],
           getAlpha()[13][6],
           getAlpha()[13][7]),
    IVec16(getAlpha()[14][0],
           getAlpha()[14][1],
           getAlpha()[14][2],
           getAlpha()[14][3],
           getAlpha()[14][4],
           getAlpha()[14][5],
           getAlpha()[14][6],
           getAlpha()[14][7]),
    IVec16(getAlpha()[15][0],
           getAlpha()[15][1],
           getAlpha()[15][2],
           getAlpha()[15][3],
           getAlpha()[15][4],
           getAlpha()[15][5],
           getAlpha()[15][6],
           getAlpha()[15][7]),
  }};
  return alpha_SIMD;
}

const IVec16& getAlphaRange_SIMD()
{
  static const IVec16 alphaRange_SIMD = IVec16{getAlphaRange()[0],
                                               getAlphaRange()[1],
                                               getAlphaRange()[4],
                                               getAlphaRange()[5],
                                               getAlphaRange()[8],
                                               getAlphaRange()[14],
                                               0,
                                               0};
  return alphaRange_SIMD;
}
