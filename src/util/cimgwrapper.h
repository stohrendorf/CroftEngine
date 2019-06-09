#pragma once

#include "render/gl/pixel.h"

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

    explicit CImgWrapper(int w, int h);

    explicit CImgWrapper(int size);

    explicit CImgWrapper();

    CImgWrapper(const CImgWrapper& other);

    CImgWrapper(CImgWrapper&& other) noexcept;

    CImgWrapper& operator=(const CImgWrapper& other);

    CImgWrapper& operator=(CImgWrapper&& other);

    ~CImgWrapper();

    void interleave();

    void deinterleave();

    int width() const;

    int height() const;

    void resize(int width, int height);

    void crop(int x0, int y0, int x1, int y1);

    void crop(const glm::vec2& uv0, const glm::vec2& uv1);

    uint8_t& operator()(int x, int y, int c);

    render::gl::SRGBA8& operator()(int x, int y);

    uint8_t operator()(int x, int y, int c) const;

    const uint8_t* data() const;

    void savePng(const std::string& filename);

    void replace(int x, int y, const CImgWrapper& other);

private:
    void unshare();
};
}