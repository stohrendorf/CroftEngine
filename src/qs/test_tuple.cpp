#include "tuple_util.h"

#include "test_util.h"

using namespace qs::detail;

BEGIN_TEST

    TEST_SAME( TPL( drop_if_same_t<int, int> ), std::tuple<> );
    TEST_SAME( TPL( drop_if_same_t<int, short> ), std::tuple<short> );

    TEST_SAME( TPL( drop_first_t<int, std::tuple<>> ), std::tuple<> );
    TEST_SAME( TPL( drop_first_t<int, std::tuple<int>> ), std::tuple<> );
    TEST_SAME( TPL( drop_first_t<int, std::tuple<int, int>> ), std::tuple<int> );
    TEST_SAME( TPL( drop_first_t<int, std::tuple<short>> ), std::tuple<short> );
    TEST_SAME( TPL( drop_first_t<int, std::tuple<short, int>> ), std::tuple<short> );

    TEST_SAME( TPL( first_tuple_t<int, short, char> ), std::tuple<int> );
    TEST_SAME( TPL( first_tuple_t<short> ), std::tuple<short> );
    TEST_SAME( TPL( first_tuple_t<short, int> ), std::tuple<short> );

    TEST_SAME( TPL( first_type_t<int, short, char> ), int );
    TEST_SAME( TPL( first_type_t<short> ), short );
    TEST_SAME( TPL( first_type_t<short, int> ), short );

    TEST_SAME( TPL( except_first_tuple_t<std::tuple<int, short, char>> ), TPL( std::tuple<short, char> ) );
    TEST_SAME( except_first_tuple_t<std::tuple<short>>, std::tuple<> );
    TEST_SAME( TPL( except_first_tuple_t<std::tuple<short, int>> ), std::tuple<int> );

    TEST_SAME( TPL( drop_all_once_t<std::tuple<int, short>, std::tuple<char>> ), std::tuple<char> );
    TEST_SAME( TPL( drop_all_once_t<std::tuple<short>, std::tuple<short, int>> ), std::tuple<int> );
    TEST_SAME( TPL( drop_all_once_t<std::tuple<short>, std::tuple<short, short, int>> ),
               TPL( std::tuple<short, int> ) );
    TEST_SAME( TPL( drop_all_once_t<std::tuple<short>, std::tuple<>> ), std::tuple<> );

    TEST_SAME(
            TPL( typename tuple_drop_common<std::tuple<int, short>, std::tuple<char>>::reduced_l ),
            TPL( std::tuple<int, short> )
    );
    TEST_SAME(
            TPL( typename tuple_drop_common<std::tuple<int, short>, std::tuple<char>>::reduced_r ),
            std::tuple<char>
    );
    TEST_SAME(
            TPL( typename tuple_drop_common<std::tuple<short>, std::tuple<short, int>>::reduced_l ),
            std::tuple<>
    );
    TEST_SAME(
            TPL( typename tuple_drop_common<std::tuple<short>, std::tuple<short, int>>::reduced_r ),
            std::tuple<int>
    );
    TEST_SAME(
            TPL( typename tuple_drop_common<std::tuple<short, short, int>, std::tuple<short, int>>::reduced_l ),
            std::tuple<short>
    );
    TEST_SAME(
            TPL( typename tuple_drop_common<std::tuple<short, short, int>, std::tuple<short, int>>::reduced_r ),
            std::tuple<>
    );
    TEST_SAME(
            TPL( typename tuple_drop_common<std::tuple<short>, std::tuple<>>::reduced_l ),
            std::tuple<short>
    );
    TEST_SAME(
            TPL( typename tuple_drop_common<std::tuple<short>, std::tuple<>>::reduced_r ),
            std::tuple<>
    );

END_TEST
