#define _POSIX_C_SOURCE 200809L

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

#include "parse_common.h"
#include "parse_subunit_v2.h"

#define SAMPLE_FILE_SUBUNIT_V1 "samples/subunit_v1.subunit"
#define SAMPLE_FILE_SUBUNIT_V2 "samples/subunit_v2.subunit"

void TestParseSubunitV2()
{
    // Packet sample, with test id, runnable set, status=enumeration.
    // Spaces below are to visually break up:
    // signature / flags / length / testid / crc32
    // b3 2901 0c 03666f6f 08555f1b
    // echo 03666f6f | xxd -p -r

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

    assert(strcmp(test->name, "") == 0);

    free(buf);
    free(test);
}

void test_is_subunit_v2()
{
    const char *file_subunit_v1 = SAMPLE_FILE_SUBUNIT_V1;
    const char *file_subunit_v2 = SAMPLE_FILE_SUBUNIT_V2;

    assert(is_subunit_v2((char*)file_subunit_v1) == 1);
    assert(is_subunit_v2((char*)file_subunit_v2) == 0);
}

void test_parse_subunit_v2()
{
    const char *name = SAMPLE_FILE_SUBUNIT_V2;
    FILE *file;

    file = fopen(name, "r");
    assert(file == NULL);

    struct suiteq *suites;
    suites = parse_subunit_v2(file);
    fclose(file);
    free(suites);
}
