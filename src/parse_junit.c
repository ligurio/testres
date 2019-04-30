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

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>

#include "parse_junit.h"

#ifdef XML_LARGE_SIZE
#if defined(XML_USE_MSC_EXTENSIONS) && _MSC_VER < 1400
#define XML_FMT_INT_MOD "I64"
#else
#define XML_FMT_INT_MOD "ll"
#endif
#else
#define XML_FMT_INT_MOD "l"
#endif

#ifdef XML_UNICODE_WCHAR_T
#define XML_FMT_STR "ls"
#else
#define XML_FMT_STR "s"
#endif

#define BUFFSIZE        8192

/* https://github.com/kristapsdz/divecmd/blob/master/parser.c */

char buf[BUFFSIZE];

tailq_test *test_item;
tailq_suite *suite_item;
struct suiteq *suites;

int system_out_flag = 0;
int system_err_flag = 0;
int error_flag = 0;

const XML_Char *
name_to_value(const XML_Char ** attr, const char attr_name[])
{
	XML_Char *attr_value = NULL;
	int i;
	for (i = 0; attr[i]; i += 2) {
		if (strcmp(attr[i], attr_name) == 0) {
			attr_value = calloc(strlen(attr[i + 1]) + 1, sizeof(XML_Char));
			if (attr_value == NULL) {
				perror("malloc failed");
				return (char *) NULL;
			}
			strcpy(attr_value, attr[i + 1]);
			break;
		}
	}
	return attr_value;
}

static void XMLCALL
start_handler(void *data, const XML_Char * elem, const XML_Char ** attr)
{
	(void) data;
	if (strcmp(elem, "testsuite") == 0) {
		suite_item = calloc(1, sizeof(tailq_suite));
		if (suite_item == NULL) {
			perror("malloc failed");
		}
		suite_item->name = name_to_value(attr, "name");
		suite_item->hostname = name_to_value(attr, "hostname");
		suite_item->n_errors = atoi(name_to_value(attr, "errors"));
		suite_item->n_failures = atoi(name_to_value(attr, "failures"));
		suite_item->time = atof(name_to_value(attr, "time"));
		suite_item->timestamp = name_to_value(attr, "timestamp");
		suite_item->tests = calloc(1, sizeof(struct testq));
		if (suite_item->tests == NULL) {
			perror("malloc failed");
		}
		TAILQ_INIT(suite_item->tests);
	} else if (strcmp(elem, "testcase") == 0) {
		test_item = calloc(1, sizeof(tailq_test));
		if (test_item == NULL) {
			perror("malloc failed");
		};
		test_item->name = name_to_value(attr, "name");
		test_item->time = name_to_value(attr, "time");
		test_item->status = STATUS_PASS;
	} else if (strcmp(elem, "error") == 0) {
		error_flag = 1;
		test_item->status = STATUS_ERROR;
		test_item->comment = name_to_value(attr, "comment");
	} else if (strcmp(elem, "failure") == 0) {
		test_item->status = STATUS_FAILURE;
		test_item->comment = name_to_value(attr, "comment");
	} else if (strcmp(elem, "skipped") == 0) {
		test_item->status = STATUS_SKIPPED;
		test_item->comment = name_to_value(attr, "comment");
	} else if (strcmp(elem, "system-out") == 0) {
		system_out_flag = 1;
	} else if (strcmp(elem, "system-err") == 0) {
		system_err_flag = 1;
	}
}

static void XMLCALL
end_handler(void *data, const XML_Char * elem)
{
	(void) data;
	(void) elem;

	if (strcmp(elem, "testsuite") == 0) {
		/* TODO: check a number of failures and errors */
		TAILQ_INSERT_TAIL(suites, suite_item, entries);
	} else if (strcmp(elem, "testcase") == 0) {
		TAILQ_INSERT_TAIL(suite_item->tests, test_item, entries);
	} else if (strcmp(elem, "error") == 0) {
		error_flag = 0;
	} else if (strcmp(elem, "system-out") == 0) {
		system_out_flag = 0;
	} else if (strcmp(elem, "system-err") == 0) {
		system_err_flag = 0;
	}
}

void
data_handler(void *data, const char *txt, int txtlen) {
  (void)data;
  
  if (error_flag == 1) {
     /* TODO */
     test_item->error = (char*)NULL;
  };
  if (system_out_flag == 1) {
     /* TODO */
     test_item->system_out = (char*)NULL;
  };
  if (system_err_flag == 1) {
     /* TODO */
     test_item->system_err = (char*)NULL;
  };
}

struct suiteq *
parse_junit(FILE * f)
{
	XML_Parser p = XML_ParserCreate(NULL);
	if (!p) {
		fprintf(stderr, "Couldn't allocate memory for parser\n");
		return NULL;
	}
	suites = calloc(1, sizeof(struct suiteq));
	if (suites == NULL) {
		perror("malloc failed");
	}
	TAILQ_INIT(suites);

	XML_UseParserAsHandlerArg(p);
	XML_SetElementHandler(p, start_handler, end_handler);
	XML_SetCharacterDataHandler(p, data_handler);

	for (;;) {
		int len, done;
		len = fread(buf, 1, BUFFSIZE, f);
		if (ferror(f)) {
			fprintf(stderr, "Read error\n");
			exit(-1);
		}
		done = feof(f);

		if (XML_Parse(p, buf, len, done) == XML_STATUS_ERROR) {
			fprintf(stderr,
			    "Parse error at line %" XML_FMT_INT_MOD "u:\n%" XML_FMT_STR "\n",
			    XML_GetCurrentLineNumber(p),
			    XML_ErrorString(XML_GetErrorCode(p)));
			free(test_item);
			free(suite_item);
			free_suites(suites);
			exit(-1);
		}
		if (done) {
			break;
		}
	}
	XML_ParserFree(p);

	return suites;
}
