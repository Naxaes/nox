#include "lexer/lexer.h"
#include "logger.h"

#include "lib.h"


int LLVMFuzzerTestOneInput(const u8* data, size_t size) {
    Logger logger = logger_make_with_memory("fuzzing", LOG_LEVEL_ERROR, NULL, 0);

    size = size >= 1024 ? 1023 : size;

    char source[4096] = { 0 };
    memcpy(source, data, size);
    source[size] = '\0';

    Bytecode code = compile_from_source(STR("<source>"), str_from_c_str(source), &logger);
    if (code.instructions != NULL) {
        dealloc(code.instructions);
    }

    if (memory_in_use() != 0) {
        memory_dump();
        exit(-1);
    }

    return 0;  // Values other than 0 and -1 are reserved for future use.
}

