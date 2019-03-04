#include "engine/engine.h"

int main()
{
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
