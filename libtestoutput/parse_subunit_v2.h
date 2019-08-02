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

#ifndef PARSE_SUBUNIT_V2_H
#define PARSE_SUBUNIT_V2_H

#include <stdint.h>

#include "parse_common.h"

#define SUBUNIT_SIGNATURE 	0xB3
#define SUBUNIT_VERSION 	0x02
#define PACKET_MAX_LENGTH 	4194303

#define FLAG_TEST_ID		0x0800
#define FLAG_ROUTE_CODE		0x0400
#define FLAG_TIMESTAMP		0x0200
#define FLAG_RUNNABLE		0x0100
#define FLAG_TAGS		0x0080
#define FLAG_MIME_TYPE		0x0020
#define FLAG_EOF		0x0010
#define FLAG_FILE_CONTENT	0x0040

#define _STATUS_UNDEFINED 	0x000
#define _STATUS_ENUMERATION 	0x001
#define _STATUS_INPROGRESS	0x002
#define _STATUS_SUCCESS		0x003
#define _STATUS_UXSUCCESS	0x004
#define _STATUS_SKIPPED		0x005
#define _STATUS_FAILED		0x006
#define _STATUS_UXFAILURE	0x007

typedef struct subunit_packet {
    uint8_t  signature;
    uint16_t flags;
    uint8_t status;
    uint32_t length;
    uint32_t timestamp;
    uint8_t version;
    char *testid;
    char *tags;
    char *mime;
    char *content;
    char *routing_code;
    char *crc32;
} __attribute__ ((packed)) subunit_packet;

int read_subunit_v2_packet(const void *buf, subunit_packet *p);
struct suiteq *parse_subunit_v2_from_file(const char *path);
int is_subunit_v2(char* path);

const void* read_uint8(const void* buffer, uint8_t* value);
const void* read_uint16(const void* buffer, uint16_t* value);
const void* read_uint32(const void* buffer, uint32_t* value);
const void* read_varint(const void* buffer, uint8_t *n_bytes);

#endif				/* PARSE_SUBUNIT_V2_H */
