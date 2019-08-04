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
    const uint8_t* vptr = (uint8_t*)buffer;
    *value = *vptr;
    vptr++;

    return vptr;
}

const void* read_uint16(const void* buffer, uint16_t* value)
{
    const uint16_t* vptr = (uint16_t*)buffer;
    *value = *vptr;
    vptr++;

    return vptr;
}

const void* read_uint32(const void* buffer, uint32_t* value)
{
    const uint32_t* vptr = (uint32_t*)buffer;
    *value = *vptr;
    vptr++;

    return vptr;
}

const void* read_varint(const void* buffer, uint32_t content)
{
    uint8_t byte_1 = 0, n_bytes = 0, value_0 = 0;
    uint16_t byte_2 = 0;
    buffer = read_uint8(buffer, &byte_1);
    n_bytes = byte_1 & 0xc0;
    value_0 = byte_1 & 0x3f;

    switch (n_bytes) {
    case 0x00:
		content = value_0;
        break;
    case 0x40:
		buffer = read_uint8(buffer, &byte_1);
		content = value_0 << 8 | byte_1;
        break;
    case 0x80:
		buffer = read_uint16(buffer, &byte_2);
		content = value_0 << 16 | byte_2;
        break;
    default:
		buffer = read_uint8(buffer, &byte_1);
		buffer = read_uint16(buffer, &byte_2);
		content = (value_0 << 24) | byte_1 << 8 | byte_2;
    };

    return buffer;
}

struct suiteq *
parse_subunit_v2_from_file(const char *path)
{
	FILE *fd;
	int rc = 0, n_bytes = 0;
	const void *buffer;

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
	while (rc == 0) {
			rc = read_subunit_v2_packet(&buffer, &packet);
			if (rc == 0) {
				printf("SubUnit v2 packet =================\n");
				printf("SIGNATURE: %02hhX\n", packet.signature);
				printf("FLAGS: %02hX\n", packet.flags);
				printf("VERSION: %d\n", packet.version);
				printf("STATUS: %d\n", packet.status);
				printf("TIMESTAMP: %d\n", packet.timestamp);
				printf("TOTAL LENGTH: %u\n", packet.length);
			}
	}

	return NULL;
}

int read_subunit_v2_packet(const void **buf, subunit_packet *p)
{
	const void* buffer = buf;
	buffer = read_uint8(buffer, &p->signature);
	if (p->signature != SUBUNIT_SIGNATURE)
	{
	    return -1;
	}
	buffer = read_uint16(buffer, &p->flags);
	p->version = HI(htons(p->flags)) >> 4;
	p->status = p->flags & 0x0007;
	assert((p->status) <= 0x0007);

	buffer = read_varint(buffer, p->length);

	if (p->flags & FLAG_TIMESTAMP) {
		buffer = read_uint32(buffer, &p->timestamp);
	};

	if (p->flags & FLAG_TEST_ID) { };
	if (p->flags & FLAG_TAGS) { };
	if (p->flags & FLAG_MIME_TYPE) { };
	if (p->flags & FLAG_FILE_CONTENT) { };
	if (p->flags & FLAG_ROUTE_CODE) { };
	if (p->flags & FLAG_EOF) { /* nothing to do */ };
	if (p->flags & FLAG_RUNNABLE) { /* nothing to do */ };

	buf = buffer;

	return 0;
}


/*
 * UTF-8
 * https://www.json.org/JSON_checker/utf8_decode.c
 * https://github.com/douglascrockford/JSON-c/blob/master/utf8_decode.c
 */
