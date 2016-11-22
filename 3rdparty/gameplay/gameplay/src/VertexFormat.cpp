#include "Base.h"
#include "VertexFormat.h"


namespace gameplay
{
    VertexFormat::VertexFormat(const Element* elements, size_t elementCount)
        : _vertexSize(0)
    {
        BOOST_ASSERT(elements);

        _elements.assign(elements + 0, elements + elementCount);

        // Copy elements and compute vertex size
        for( const Element& element : _elements )
        {
            _vertexSize += element.size * sizeof(float);
        }
    }


    VertexFormat::~VertexFormat() = default;


    const VertexFormat::Element& VertexFormat::getElement(size_t index) const
    {
        BOOST_ASSERT(index < _elements.size());
        return _elements[index];
    }


    size_t VertexFormat::getElementCount() const
    {
        return _elements.size();
    }


    size_t VertexFormat::getVertexSize() const
    {
        return _vertexSize;
    }


    bool VertexFormat::operator ==(const VertexFormat& f) const
    {
        if( _elements.size() != f._elements.size() )
            return false;

        for( size_t i = 0, count = _elements.size(); i < count; ++i )
        {
            if( _elements[i] != f._elements[i] )
                return false;
        }

        return true;
    }


    bool VertexFormat::operator !=(const VertexFormat& f) const
    {
        return !(*this == f);
    }


    VertexFormat::Element::Element() = default;


    VertexFormat::Element::Element(Usage usage, size_t size)
        : usage(usage)
        , size(size)
    {
    }


    bool VertexFormat::Element::operator ==(const VertexFormat::Element& e) const
    {
        return (size == e.size && usage == e.usage);
    }


    bool VertexFormat::Element::operator !=(const VertexFormat::Element& e) const
    {
        return !(*this == e);
    }
}
