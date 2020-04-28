#include <stdio.h>

#include "parse_common.h"
#include "parse_subunit_v1.h"

int LLVMFuzzerTestOneInput(const uint8_t * data, size_t size) {
    parse_subunit_v1((const char *)data);
    return 0;
}
