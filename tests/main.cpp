#include "gtest/gtest.h"

#include "arithmetic.cpp"
#include "logic.cpp"
#include "if-stmt.cpp"
#include "while-stmt.cpp"
#include "functions.cpp"
#include "struct.cpp"

extern "C" {
#include "logger.h"
}


int main(int argc, char **argv) {
    logger_init(LOG_LEVEL_DEBUG);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}