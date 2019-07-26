#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "parse_common.h"
#include "parse_subunit_v1.h"

#define SAMPLE_FILE_SUBUNIT_V1 "samples/subunit_v1.subunit"

void TestParseSubunitV1()
{
    const char *name = SAMPLE_FILE_SUBUNIT_V1;
    FILE *file;

    file = fopen(name, "r");
    assert(file == NULL);

    struct suiteq *suites;
    suites = parse_subunit_v1(file);

    fclose(file);
    free(suites);
}

void test_parse_subunit_v1_line()
{
	const char *test_sample[] = {
	"test test LABEL",
	"testing test LABEL",
	"test: test LABEL",
	"testing: test LABEL",
	"success test LABEL",
	"success: test LABEL",
	"successful test LABEL",
	"successful: test LABEL",
	"failure: test LABEL",
	"failure: test LABEL DETAILS",
	"error: test LABEL",
	"error: test LABEL DETAILS",
	"skip test LABEL",
	"skip: test LABEL",
	"skip test LABEL DETAILS",
	"skip: test LABEL DETAILS",
	"xfail test LABEL",
	"xfail: test LABEL",
	"xfail test LABEL DETAILS",
	"xfail: test LABEL DETAILS",
	"uxsuccess test LABEL",
	"uxsuccess: test LABEL",
	"uxsuccess test LABEL DETAILS",
	"uxsuccess: test LABEL DETAILS",
	"progress: +10",
	"progress: -14",
	"progress: push",
	"progress: pop",
	"tags: -small +big",
	"time: 2018-09-10 23:59:29Z" };

	const char** qq = test_sample;
	for (int i = 0; i <  (int)(sizeof(test_sample)/sizeof(char*)); ++i) {
		parse_line_subunit_v1((char*)*qq);
		++qq;
	}
}
