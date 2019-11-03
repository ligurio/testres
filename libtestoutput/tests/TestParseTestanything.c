#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <parse_common.h>

#define SAMPLE_FILE_TESTANYTHING "samples/testanything.tap"

extern struct suiteq *parse_testanything(FILE *f);

void TestParseTestanything()
{
    const char *name = SAMPLE_FILE_TESTANYTHING;
    FILE *file;
    file = fopen(name, "r");
    assert(file == NULL);
    parse_testanything(file);
    fclose(file);
}
