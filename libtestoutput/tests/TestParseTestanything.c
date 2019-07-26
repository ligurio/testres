#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "parse_common.h"
#include "parse_testanything.h"

#define SAMPLE_FILE_TESTANYTHING "samples/testanything.tap"

void TestParseTestanything()
{
    const char *name = SAMPLE_FILE_TESTANYTHING;
    FILE *file;
    file = fopen(name, "r");
    assert(file == NULL);
    parse_testanything(file);
    fclose(file);
}
