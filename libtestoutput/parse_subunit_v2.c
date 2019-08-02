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
    printf("Bytes %d\n", *n_bytes);
    assert(*n_bytes != 0);
    uint8_t value = 0;
    for(int i = 0; i <= *n_bytes; i++) {
        buffer = read_uint8(buffer, &value);
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
	buf = read_uint8(buf, &p->signature);
	if (p->signature != SUBUNIT_SIGNATURE)
	{
	    return -1;
	}
	buf = read_uint16(buf, &p->flags);

	p->version = HI(htons(p->flags)) >> 4;
	if (p->version == 2) {
		assert((p->flags & 0x0008) == 0);
	}

	p->status = p->flags & 0x0007;
	assert((p->status) <= 0x0007);

	buf = read_uint32(buf, &p->length);
	assert((p->length) < PACKET_MAX_LENGTH);

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

	/* printf("CRC32: %s\n", (char*)buf); */

	return 0;
}


/*
 * UTF-8
 * https://www.json.org/JSON_checker/utf8_decode.c
 * https://github.com/douglascrockford/JSON-c/blob/master/utf8_decode.c
 */
