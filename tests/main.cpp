#include "gtest/gtest.h"

#include "arithmetic.cpp"
#include "logic.cpp"
#include "if-stmt.cpp"
#include "while-stmt.cpp"
#include "functions.cpp"
#include "struct.cpp"


int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}