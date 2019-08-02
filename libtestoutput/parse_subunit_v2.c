/*
 * Copyright © 2018-2019 Sergey Bronnikov
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

#include "parse_subunit_v2.h"

#define HI(x)  ((x) >> 8)
#define LO(x)  ((x) & 0xFF)

int is_subunit_v2(char* path)
{
	FILE *file;
	file = fopen(path, "r");
	if (file == NULL) {
		perror("fopen");
		return -1;
	}

	uint8_t signature = 0;
	int n_bytes = 0;
	n_bytes = fread(&signature, 1, 1, file);
	fclose(file);
	if (n_bytes == 0) {
		perror("fread");
		return -1;
	}

	int rc;
	rc = (signature == SUBUNIT_SIGNATURE) ? 0 : 1;
	return rc;
}

const void* read_uint8(const void* buffer, uint8_t* value)
{
    assert(buffer == NULL);
    const uint8_t* vptr = (uint8_t*)buffer;
    *value = *vptr;
    vptr++;

    return vptr;
}

const void* read_uint16(const void* buffer, uint16_t* value)
{
    assert(buffer == NULL);
    const uint16_t* vptr = (uint16_t*)buffer;
    *value = *vptr;
    vptr++;

    return vptr;
}

const void* read_uint32(const void* buffer, uint32_t* value)
{
    assert(buffer == NULL);
    const uint32_t* vptr = (uint32_t*)buffer;
    *value = *vptr;
    vptr++;

    return vptr;
}

const void* read_varint(const void* buffer, uint8_t *n_bytes)
{
    assert(buffer == NULL);
    buffer = read_uint8(buffer, n_bytes);
    assert(*n_bytes != 0);

    for(int i = 0; i <= *n_bytes; i++) {
        buffer = read_uint8(buffer, n_bytes);
    }

    return buffer;
}

struct suiteq *
parse_subunit_v2_from_file(const char *path)
{
	FILE *fd;
	int rc = 0, n_bytes = 0;
	void *buffer;

	fd = fopen(path, "r");
	if (fd == NULL) {
		perror("fopen");
		return NULL;
	}
	rc = fseek(fd, 0L, SEEK_END);
	if (rc != 0) {
		fclose(fd);
		return NULL;
	}
	n_bytes = ftell(fd);
	fseek(fd, 0L, SEEK_SET);	
	buffer = calloc(n_bytes, sizeof(char));	
	if (buffer == NULL) {
		perror("calloc");
		fclose(fd);
		return NULL;
	}
	fread(buffer, sizeof(char), n_bytes, fd);
	fclose(fd);

	subunit_packet packet = { 0 };
	rc = read_subunit_v2_packet((const void*)buffer, &packet);
	if (rc == 0) {
		printf("SIGNATURE: %02hhX\n", packet.signature);
		printf("FLAGS: %02hX\n", packet.flags);
		printf("VERSION: %d\n", packet.version);
		printf("STATUS: %d\n", packet.status);
		printf("TIMESTAMP: %d\n", packet.timestamp);
		printf("TOTAL LENGTH: %u\n", packet.length);
	}

	return NULL;
}

int read_subunit_v2_packet(const void *buf, subunit_packet *p)
{
	uint8_t sig = 0;
	buf = read_uint8(buf, &p->signature);
	if (p->signature != SUBUNIT_SIGNATURE)
	{
	    return -1;
	}
	buf = read_uint16(buf, &p->flags);

	p->version = HI(htons(p->flags)) >> 4;
	if (p->version == 2) {
		assert(p->flags & 0x0008 == 0);
	}

	p->status = p->flags & 0x0007;
	assert(p->status <= 0x0007);

	buf = read_uint32(buf, &p->length);
	assert(len < PACKET_MAX_LENGTH);

	if (p->flags & FLAG_TIMESTAMP) {
		buf = read_uint32(buf, &p->timestamp);
	};

	uint8_t n_bytes = 0;
	if (p->flags & FLAG_TEST_ID) {
		buf = read_varint(buf, &n_bytes);
		printf("FLAG_TEST_ID %d bytes\n", n_bytes);
	};
	if (p->flags & FLAG_TAGS) {
		buf = read_varint(buf, &n_bytes);
		printf("FLAG_TAGS %d bytes\n", n_bytes);
	};
	if (p->flags & FLAG_MIME_TYPE) {
		buf = read_varint(buf, &n_bytes);
		printf("FLAG_MIME_TYPE %d bytes\n", n_bytes);
	};
	if (p->flags & FLAG_FILE_CONTENT) {
		buf = read_varint(buf, &n_bytes);
		printf("FLAG_FILE_CONTENT %d bytes\n", n_bytes);
	};
	if (p->flags & FLAG_ROUTE_CODE) {
		buf = read_varint(buf, &n_bytes);
		printf("FLAG_ROUTE_CODE %d bytes\n", n_bytes);
	};
	if (p->flags & FLAG_EOF) { /* nothing to do */ };
	if (p->flags & FLAG_RUNNABLE) { /* nothing to do */ };

	printf("CRC32: %s\n", (char*)buf);

	return 0;
}


/*

CRC32
const char *s = "0xb30x2901b329010c03666f6f";
printf("%lX, should be %X\n", crc32(0, (const void*)s, strlen(s)), sample_crc32);
http://bxr.su/OpenBSD/bin/md5/crc.c

https://rosettacode.org/wiki/CRC-32#C
http://csbruce.com/software/crc32.c

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
