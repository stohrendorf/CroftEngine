#pragma once

#include "quantity.h"

#include <yaml-cpp/yaml.h>

// YAML converters
namespace YAML
{
template<typename Type, typename Unit>
struct convert<qs::quantity<Unit, Type>>
{
    static Node encode(const qs::quantity<Unit, Type>& rhs)
    {
        Node node{ NodeType::Sequence };
        node.SetStyle( YAML::EmitterStyle::Flow );
        node.push_back( Unit::suffix() );
        node.push_back( rhs.get() );
        return node;
    }

    static bool decode(const Node& node, qs::quantity<Unit, Type>& rhs)
    {
        if( !node.IsSequence() )
            return false;
        if( node.size() != 2 )
            return false;
        if( node[0].as<std::string>() != Unit::suffix() )
            return false;

        rhs = qs::quantity<Unit, Type>{ node[1].as<typename qs::quantity<Unit, Type>::type>() };
        return true;
    }
};

template<typename Unit, typename Type>
struct as_if<qs::quantity<Unit, Type>, void>
{
    explicit as_if(const Node& node_)
        : node{ node_ }
    {}

    const Node& node;

    qs::quantity<Unit, Type> operator()() const
    {
        if( !node.m_pNode )
            throw TypedBadConversion<qs::quantity<Unit, Type>>{ node.Mark() };

        qs::quantity<Unit, Type> t{ Type{ 0 } };
        if( convert<qs::quantity<Unit, Type>>::decode( node, t ) )
            return t;
        throw TypedBadConversion<qs::quantity<Unit, Type>>{ node.Mark() };
    }
};
}
