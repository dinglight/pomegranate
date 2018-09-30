#include "test_suite.h"
#include <iostream>
#include <vector>
namespace test {
namespace {
struct Test {
    const char *base;
    const char *name;
    void(*func)();
};
std::vector<Test> *tests=nullptr;
} // namespace

bool register_test(const char *base, const char *name, void(*func)())
{
    if (tests == nullptr) {
        tests = new std::vector<Test>;
    }
    Test t;
    t.base = base;
    t.name = name;
    t.func = func;
    tests->emplace_back(t);
    return true;
}

void init()
{
    if (tests == nullptr) {
        tests = new std::vector<Test>;
    }
}
void fini()
{
    if (tests != nullptr) {
        delete tests;
        tests = nullptr;
    }
}
int run_all_tests()
{
    int num = 0;
    if (tests == nullptr) {
        std::cerr << "No tests are found" << std::endl;
        return 0;
    }

    for (const Test &t : *(tests)) {
        std::cerr << "[ RUN     ] " << t.base << "." << t.name << std::endl;
        (*t.func)();
        std::cerr << "[      OK ] " << t.base << "." << t.name << std::endl;
        ++num;
    }
    std::cerr << "==== PASSED " << num << "tests" << std::endl;
    return 0;
}
} // namespace test