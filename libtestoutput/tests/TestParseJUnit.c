#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "parse_common.h"
#include "parse_junit.h"

#define SAMPLE_FILE_JUNIT "samples/junit.xml"

void TestParseJUnit()
{
    FILE *file;
    const char *name = SAMPLE_FILE_JUNIT;

    file = fopen(name, "r");
    assert(file != NULL);
    struct suiteq *report = parse_junit(file);
    assert(report != NULL);
    fclose(file);
}
