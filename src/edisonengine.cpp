#include "engine/engine.h"

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>

int main()
{
    boost::log::core::get()->set_filter( boost::log::trivial::severity >= boost::log::trivial::info );

    try
    {
        engine::Engine engine;
        engine.run();

        return EXIT_SUCCESS;
    }
    catch( std::exception& ex )
    {
        BOOST_LOG_TRIVIAL( error ) << "Error: " << ex.what();
        return EXIT_FAILURE;
    }
    catch( ... )
    {
        BOOST_LOG_TRIVIAL( error ) << "Unexpected error";
        return EXIT_FAILURE;
    }
}
