#pragma once

#include "gl/pixel.h"

#include <memory>
#include <string>

namespace cimg_library
{
template<typename T>
struct CImg;
}

namespace util
{
class CImgWrapper
{
private:
    std::unique_ptr<cimg_library::CImg<uint8_t>> m_image;
    bool m_interleaved = false;

public:
    explicit CImgWrapper(const std::string& filename);

    explicit CImgWrapper(const uint8_t* data, int width, int height, bool shared);

    CImgWrapper(const CImgWrapper& other);

    CImgWrapper& operator=(const CImgWrapper& other);

    ~CImgWrapper();

    void interleave();

    void deinterleave();

    int width() const;

    int height() const;

    void resize(int width, int height);

    void crop(int x0, int y0, int x1, int y1);

    uint8_t& operator()(int x, int y, int c);

    gameplay::gl::RGBA8& operator()(int x, int y);

    uint8_t operator()(int x, int y, int c) const;

    const uint8_t* data() const;

    void savePng(const std::string& filename);

private:
    void unshare();
};
}