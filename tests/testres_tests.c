#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

#include "CUnit/Basic.h"

#include "parse_common.h"
#include "parse_junit.h"
#include "parse_testanything.h"
#include "parse_subunit_v1.h"
#include "parse_subunit_v2.h"
#include "sha1.h"

#define SAMPLE_FILE_JUNIT "samples/junit.xml"
#define SAMPLE_FILE_SUBUNIT_V1 "samples/subunit_v1.subunit"
#define SAMPLE_FILE_SUBUNIT_V2 "samples/subunit_v2.subunit"
#define SAMPLE_FILE_TESTANYTHING "samples/testanything.tap"

static void test_parse_testanything()
{
    char *name = SAMPLE_FILE_TESTANYTHING;
    FILE *file;
    file = fopen(name, "r");
    CU_ASSERT_NOT_EQUAL_FATAL(file, NULL);
    parse_testanything(file);
    fclose(file);
}

static void test_read_subunit_v2_packet()
{
    // Packet sample, with test id, runnable set, status=enumeration.
    // Spaces below are to visually break up:
    // signature / flags / length / testid / crc32
    // b3 2901 0c 03666f6f 08555f1b
    // echo 03666f6f | xxd -p -r

    CU_FAIL_FATAL("not implemented");

    subunit_header sample_header = { .signature = 0xb3, .flags = ntohs(0x2901) };
    uint16_t sample_length = 0x0c;
    uint32_t sample_testid = 0x03666f6f;
    uint32_t sample_crc32 = 0x08555f1b;

    char* buf = NULL;
    size_t buf_size = 0;
    tailq_test * test;
    FILE* stream = open_memstream(&buf, &buf_size);
    fwrite(&sample_header, 1, sizeof(sample_header), stream);
    fwrite(&sample_length, 1, sizeof(sample_length), stream);
    fwrite(&sample_testid, 1, sizeof(sample_testid), stream);
    fwrite(&sample_crc32, 1, sizeof(sample_crc32), stream);

    test = read_subunit_v2_packet(stream);
    fclose(stream);

    CU_ASSERT_STRING_EQUAL(test->name, "");

    free(buf);
    free(test);
}

static void test_is_subunit_v2()
{
    char *file_subunit_v1 = SAMPLE_FILE_SUBUNIT_V1;
    char *file_subunit_v2 = SAMPLE_FILE_SUBUNIT_V2;

    CU_ASSERT(is_subunit_v2(file_subunit_v1) == 1);
    CU_ASSERT(is_subunit_v2(file_subunit_v2) == 0);
}

static void test_parse_subunit_v2()
{
    CU_FAIL_FATAL("not implemented");

    char *name = SAMPLE_FILE_SUBUNIT_V2;
    FILE *file;

    file = fopen(name, "r");
    CU_ASSERT_NOT_EQUAL_FATAL(file, NULL);

    struct suiteq *suites;
    suites = parse_subunit_v2(file);
    fclose(file);
    free(suites);
}

static void test_parse_subunit_v1()
{
    char *name = SAMPLE_FILE_SUBUNIT_V1;
    FILE *file;

    file = fopen(name, "r");
    CU_ASSERT_NOT_EQUAL_FATAL(file, NULL);

    struct suiteq *suites;
    suites = parse_subunit_v1(file);

    fclose(file);
    free(suites);
}

static void test_parse_subunit_v1_line()
{
	char *test_sample[] = {
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

	char** qq = test_sample;
	for (int i = 0; i <  (int)(sizeof(test_sample)/sizeof(char*)); ++i) {
		parse_line_subunit_v1(*qq);
		++qq;
	}
}

static void test_parse_junit()
{
    FILE *file;
    char *name = SAMPLE_FILE_JUNIT;

    file = fopen(name, "r");
    CU_ASSERT_NOT_EQUAL_FATAL(file, NULL);
    parse_junit(file);
    fclose(file);
}

static void test_sha1()
{

    struct {
      char *word;
      char *digest;
      } tests[] = {
        /* Test Vectors (from FIPS PUB 180-1) */
        { "abc", "a9993e364706816aba3e25717850c26c9cd0d89d" },
        { "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq", "84983e441c3bd26ebaae4aa1f95129e5e54670f1" },
        { NULL, NULL },
    };

    int i, length = 20;
    unsigned char digest[length];
    char *str = calloc(length, sizeof(unsigned char*));
    for (i = 0; tests[i].word != NULL; i++) {
	const char *word = tests[i].word;
        SHA1_CTX ctx;
        SHA1Init(&ctx);
        SHA1Update(&ctx, word, strlen(word));
        SHA1Final(digest, &ctx);
		/*
        if (digest_to_str(str, digest, length) != tests[i].digest) {
           CU_get_error();
        }
		*/
    }
    free(str);
}

int main()
{
   CU_pSuite pSuite = NULL;

   /* initialize the CUnit test registry */
   if (CUE_SUCCESS != CU_initialize_registry())
      return CU_get_error();

   /* add a suite to the registry */
   pSuite = CU_add_suite("Suite_1", NULL, NULL);
   if (NULL == pSuite) {
      CU_cleanup_registry();
      return CU_get_error();
   }

   /* add the tests to the suite */
   if ((NULL == CU_add_test(pSuite, "test_parse_junit()", test_parse_junit)) ||
       (NULL == CU_add_test(pSuite, "test_parse_subunit_v1()", test_parse_subunit_v1)) ||
       (NULL == CU_add_test(pSuite, "test_parse_subunit_v1_line()", test_parse_subunit_v1_line)) ||
       (NULL == CU_add_test(pSuite, "test_parse_subunit_v2()", test_parse_subunit_v2)) ||
       (NULL == CU_add_test(pSuite, "test_read_subunit_v2_packet()", test_read_subunit_v2_packet)) ||
       (NULL == CU_add_test(pSuite, "test_is_subunit_v2()", test_is_subunit_v2)) ||
       (NULL == CU_add_test(pSuite, "test_parse_testanything()", test_parse_testanything)) ||
       (NULL == CU_add_test(pSuite, "test_sha1()", test_sha1)))
   {
      CU_cleanup_registry();
      return CU_get_error();
   }

   /* Run all tests using the CUnit Basic interface */
   CU_basic_set_mode(CU_BRM_VERBOSE);
   CU_basic_run_tests();
   CU_cleanup_registry();
   return CU_get_error();
}
