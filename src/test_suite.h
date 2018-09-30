#ifndef __TEST_SUITE_H__
#define __TEST_SUITE_H__

#include <iostream>
#include <sstream>
#include <string>

namespace test {

void init();
void fini();

// Run some of the tests registered by the TEST() macro.
// TEST(Foo, Hello) { ... }
// TEST(Foo, World) { ... }
//
// Returns 0 if all tests pass.
// Dies or returns a non-zero value if some test fails.
int run_all_tests();


// An instance of Tester is allocated to hold temporary state during
// the execution of an assertion.
class Tester {
public:
    Tester(const char *fname, int line) : __ok(true), __fname(fname), __line(line) {}
    ~Tester()
    {
        if (!__ok) {
            std::cerr << "[     NG ]" << __fname << ":" << __line << __ss.str() << std::endl;
            exit(-1);
        }
    }

    Tester &is(bool b, const char *msg)
    {
        if (!b) {
            __ss << " failed: " << msg;
            __ok = false;
        }
        return *this;
    }

    Tester &isNear(double val1, double val2, double abs_error, const char *msg1,
                   const char *msg2)
    {
        const double diff = std::fabs(val1 - val2);
        if (diff > abs_error) {
            __ss << "The difference between (" << msg1 << ") and (" << msg2 << ") is "
                 << diff << ", which exceeds " << abs_error << ", where\n"
                 << msg1 << " evaluates to " << val1 << ",\n"
                 << msg2 << " evaluates to " << val2;
            __ok = false;
        }
        return *this;
    }

#define BINARY_OP(name, op)                                                        \
    template <class X, class Y>                                                    \
    Tester &name(const X &x, const Y &y, const char *msg1, const char *msg2) {     \
        if (!(x op y)) {                                                           \
            __ss << " failed: " << msg1 << (" " #op " ") << msg2;                  \
            __ok = false;                                                          \
        }                                                                          \
        return *this;                                                              \
    }

    BINARY_OP(isEq, == )
    BINARY_OP(isNe, != )
    BINARY_OP(isGe, >= )
    BINARY_OP(isGt, >)
    BINARY_OP(isLe, <= )
    BINARY_OP(isLt, <)
#undef BINARY_OP

    // Attach the specified value to the error message if an error has occured
    template <class V>
    Tester &operator<<(const V &value)
    {
        if (!__ok) {
            __ss << " " << value;
        }
        return *this;
    }

private:
    bool __ok;
    const char *__fname;
    int __line;
    std::stringstream __ss;
};

#define EXPECT_TRUE(c) \
  test::Tester(__FILE__, __LINE__).is((c), #c)
#define EXPECT_FALSE(c) \
  test::Tester(__FILE__, __LINE__).is((!(c)), #c)
#define EXPECT_STREQ(a, b)                        \
  test::Tester(__FILE__, __LINE__) \
      .isEq(std::string(a), std::string(b), #a, #b)
#define EXPECT_EQ(a, b) \
  test::Tester(__FILE__, __LINE__).isEq((a), (b), #a, #b)
#define EXPECT_NE(a, b) \
  test::Tester(__FILE__, __LINE__).isNe((a), (b), #a, #b)
#define EXPECT_GE(a, b) \
  test::Tester(__FILE__, __LINE__).isGe((a), (b), #a, #b)
#define EXPECT_GT(a, b) \
  test::Tester(__FILE__, __LINE__).isGt((a), (b), #a, #b)
#define EXPECT_LE(a, b) \
  test::Tester(__FILE__, __LINE__).isLe((a), (b), #a, #b)
#define EXPECT_LT(a, b) \
  test::Tester(__FILE__, __LINE__).isLt((a), (b), #a, #b)
#define EXPECT_NEAR(a, b, c) \
  test::Tester(__FILE__, __LINE__).isNear((a), (b), (c), #a, #b)


#define TCONCAT(a,b,c) a##b##c

#define TEST(base, name)                                                        \
    class TCONCAT(base, _Test_, name) {                                         \
    public:                                                                     \
        void _run();                                                            \
        static void _runIt()                                                    \
        {                                                                       \
            TCONCAT(base, _Test_, name) t;                                      \
            t._run();                                                           \
        }                                                                       \
    };                                                                          \
    bool TCONCAT(base, _Test_ignored_, name) =                                  \
        test::register_test(#base, #name, &TCONCAT(base, _Test_, name)::_runIt);\
    void TCONCAT(base, _Test_, name)::_run()

// register the specified test.
extern bool register_test(const char *base, const char *name, void(*func)());
}
#endif // !__TEST_SUITE_H__
