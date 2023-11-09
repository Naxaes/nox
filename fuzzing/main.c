#include "lexer/lexer.h"

#include <stdlib.h>

int LLVMFuzzerTestOneInput(const u8* data, size_t size) {
    size = size >= 1024 ? 1023 : size;

    char source[1024] = { 0 };
    memcpy(source, data, size);
    source[size] = '\0';

    TokenArray token_array = lexer_lex(STR("<fuzzer>"), (Str) { size, source });

    if (token_array.tokens) free(token_array.tokens);
    if (token_array.identifiers) free(token_array.identifiers);
    if (token_array.interned_strings) free(token_array.interned_strings);

    return 0;  // Values other than 0 and -1 are reserved for future use.
}

