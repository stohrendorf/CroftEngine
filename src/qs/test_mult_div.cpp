#include "mult_div.h"

#include "test_util.h"

using namespace qs;

BEGIN_TEST

    TEST_SAME(
            TPL( fraction_unit_t<std::tuple<int, int, int, int>, std::tuple<short, int>> ),
            TPL( fraction_unit<std::tuple<int, int, int>, std::tuple<short>> )
    );

    TEST_SAME(
            TPL( fraction_unit_t<std::tuple<int, char>, std::tuple<char>> ),
            int
    );

    TEST_SAME(
            TPL( fraction_unit_t<std::tuple<int, int, int>, std::tuple<int>> ),
            TPL( product_unit<int, int> )
    );

    TEST_SAME(
            TPL( fraction_unit_t<std::tuple<int, int>, std::tuple<int>> ),
            int
    );

END_TEST
