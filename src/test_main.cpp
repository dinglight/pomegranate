#include "test_suite.h"

int main(int argc, char **argv)
{
    test::init();
    test::run_all_tests();
    test::fini();
    return 0;
}