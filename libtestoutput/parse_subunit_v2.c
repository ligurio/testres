/*
 * Copyright © 2018 Sergey Bronnikov
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <arpa/inet.h>
#include <zlib.h>

#include "parse_subunit_v2.h"

// https://github.com/testing-cabal/subunit/blob/master/python/subunit/v2.py#L412
// https://github.com/testing-cabal/subunit
// https://gist.github.com/ligurio/c173e9af1b766cf6626f7037cbb78d72

#define HI(x)  ((x) >> 8)
#define LO(x)  ((x) & 0xFF)

int is_subunit_v2(char* path)
{
	FILE *file;
	file = fopen(path, "r");
	if (file == NULL) {
		printf("failed to open file %s\n", path);
		return -1;
	}

	uint8_t signature = 0;
	int n_bytes = 0;
	n_bytes = fread(&signature, 1, 1, file);
	fclose(file);
	if (n_bytes == 0) {
		return -1;
	}
	if (signature == SUBUNIT_SIGNATURE) {
		return 0;
	} else {
		return 1;
	}
}

const void* read_uint8(const void* buffer, uint8_t* value)
{
    const uint8_t* vptr = (uint8_t*)buffer;
    *value = *vptr;
    vptr++;

    return vptr;
}

const void* read_uint16(const void* buffer, uint16_t* value)
{
    /* FIXME: network byte order */
    const uint16_t* vptr = (uint16_t*)buffer;
    *value = *vptr;
    vptr++;

    return vptr;
}

uint32_t read_field(FILE * stream)
{

	uint32_t field_value = 0;
	uint8_t byte = 0, byte0 = 0;
	uint16_t buf = 0;
	uint8_t prefix = 0;

	int n_bytes = 0;

	n_bytes = fread(&byte, 1, 1, stream);
	if (n_bytes == 0) {
		return 0;
	}
	prefix = byte >> 6;
	byte0 = byte & 0x3f;
	if (prefix == 0x00) {
		field_value = byte0;
	} else if (prefix == 0x40) {
		n_bytes = fread(&byte, 1, 1, stream);
		if (n_bytes == 0) {
			return 0;
		}
		field_value = (byte0 << 8) | byte;
	} else if (prefix == 0x80) {
		n_bytes = fread(&buf, 2, 1, stream);
		if (n_bytes == 0) {
			return 0;
		}
		field_value = (byte << 16) | buf;
	} else {
		n_bytes = fread(&buf, 1, 2, stream);
		if (n_bytes == 0) {
			return 0;
		}
		field_value = (byte0 << 24) | buf << 8;

		n_bytes = fread(&byte, 1, 1, stream);
		if (n_bytes == 0) {
			return 0;
		}
		field_value = field_value | byte;
	};

	return field_value;
}

struct suiteq *
parse_subunit_v2(FILE * stream)
{
	tailq_suite *suite_item;
	suite_item = calloc(1, sizeof(tailq_suite));
	if (suite_item == NULL) {
		perror("malloc failed");
		return NULL;
	}

	suite_item->tests = calloc(1, sizeof(struct testq));
	if (suite_item->tests == NULL) {
		perror("malloc failed");
		free_suite(suite_item);
		return NULL;
	}

	TAILQ_INIT(suite_item->tests);
	tailq_test *test_item = NULL;

	test_item = read_subunit_v2_packet(stream);
	if (test_item != NULL)
		TAILQ_INSERT_TAIL(suite_item->tests, test_item, entries);

	/*
	while (!feof(stream)) {
		test_item = read_packet(stream);
		if (test_item != NULL)
			TAILQ_INSERT_TAIL(suite_item->tests, test_item, entries);
		else
		{
			free_tests(suite_item->tests);
			free_suite(suite_item);
			return NULL;
		}
	}
	*/

	struct suiteq *suites = NULL;
	suites = calloc(1, sizeof(struct suiteq));
	if (suites == NULL) {
		perror("malloc failed");
		free_suite(suite_item);
	}
	TAILQ_INIT(suites);
	TAILQ_INSERT_TAIL(suites, suite_item, entries);

	return suites;
}

tailq_test *
read_subunit_v2_packet(FILE * stream)
{
	subunit_header header;
	int n_bytes = 0;
	n_bytes = fread(&header, sizeof(subunit_header), 1, stream);
	if ((n_bytes == 0) || (n_bytes < (int)sizeof(subunit_header))) {
		return NULL;
	}
	tailq_test *test_item;
	test_item = calloc(1, sizeof(tailq_test));
	if (test_item == NULL) {
		perror("malloc failed");
		return NULL;
	}

	uint16_t flags = htons(header.flags);
	printf("SIGNATURE: %02hhX\n", header.signature);
	printf("FLAGS: %02hX\n", flags);
	assert(header.signature == SUBUNIT_SIGNATURE);

	int8_t version;
	version = HI(flags) >> 4;
	printf("\tVERSION: %d\n", version);
	assert(version == SUBUNIT_VERSION);

	/*
	int8_t status;
	status = flags & 0x0007;
	printf("\tSTATUS: %d\n", status);
	test_item->status = status;
	assert(status <= 0x0007);

	uint32_t field_value;
	field_value = read_field(stream);
	printf("TOTAL LENGTH: %u\n", field_value);
	assert(field_value < PACKET_MAX_LENGTH);

	if (flags & FLAG_TIMESTAMP) {
		printf("FLAG_TIMESTAMP ");
		field_value = read_field(stream);
		printf("%08X\n", field_value);
	};
	if (flags & FLAG_TEST_ID) {
		printf("FLAG_TEST_ID ");
		field_value = read_field(stream);
		printf("%08X\n", field_value);
	};
	if (flags & FLAG_TAGS) {
		printf("FLAG_TAGS ");
		field_value = read_field(stream);
		printf("%02X\n", field_value);
	};
	if (flags & FLAG_MIME_TYPE) {
		printf("FLAG_MIME_TYPE ");
		field_value = read_field(stream);
		printf("%02X\n", field_value);
	};
	if (flags & FLAG_FILE_CONTENT) {
		printf("FLAG_FILE_CONTENT ");
		field_value = read_field(stream);
		printf("%08X\n", field_value);
	};
	if (flags & FLAG_ROUTE_CODE) {
		printf("FLAG_ROUTE_CODE ");
		field_value = read_field(stream);
		printf("%08X\n", field_value);
	};
	if (flags & FLAG_EOF) {
		printf("FLAG_EOF\n");
	};
	if (flags & FLAG_RUNNABLE) {
		printf("FLAG_RUNNABLE\n");
	};
	printf("CRC32: ");
	field_value = read_field(stream);
	printf("%08X\n", field_value);
	*/

	return test_item;
}


/*

CRC32
const char *s = "0xb30x2901b329010c03666f6f";
printf("%lX, should be %X\n", crc32(0, (const void*)s, strlen(s)), sample_crc32);
http://bxr.su/OpenBSD/bin/md5/crc.c

https://rosettacode.org/wiki/CRC-32#C
http://csbruce.com/software/crc32.c

Parse timestamp
int y, M, d, h, m;
float sec;
char *dateStr = "2014-11-12T19:12:14.505Z";
sscanf(dateStr, "%d-%d-%dT%d:%d:%fZ", &y, &M, &d, &h, &m, &sec);
https://github.com/mlabbe/c_date_parse

UTF-8
https://github.com/benkasminbullock/unicode-c/blob/master/unicode.c
https://github.com/clibs/cutef8

*/

/* Simple public domain implementation of the standard CRC32 checksum.
 * Outputs the checksum for each file given as a command line argument.
 * Invalid file names and files that cause errors are silently skipped.
 * The program reads from stdin if it is called with no arguments. */

/* http://home.thep.lu.se/~bjorn/crc/ */

/*

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

uint32_t crc32_for_byte(uint32_t r) {
  for(int j = 0; j < 8; ++j)
    r = (r & 1? 0: (uint32_t)0xEDB88320L) ^ r >> 1;
  return r ^ (uint32_t)0xFF000000L;
}

void crc32(const void *data, size_t n_bytes, uint32_t* crc) {
  static uint32_t table[0x100];
  if(!*table)
    for(size_t i = 0; i < 0x100; ++i)
      table[i] = crc32_for_byte(i);
  for(size_t i = 0; i < n_bytes; ++i)
    *crc = table[(uint8_t)*crc ^ ((uint8_t*)data)[i]] ^ *crc >> 8;
}

int main(int ac, char** av) {
  FILE *fp;
  char buf[1L << 15];
  for(int i = ac > 1; i < ac; ++i)
    if((fp = i? fopen(av[i], "rb"): stdin)) { 
      uint32_t crc = 0;
      while(!feof(fp) && !ferror(fp))
	crc32(buf, fread(buf, 1, sizeof(buf), fp), &crc);
      if(!ferror(fp))
	printf("%08x%s%s\n", crc, ac > 2? "\t": "", ac > 2? av[i]: "");
      if(i)
	fclose(fp);
    }
  return 0;
}

*/
