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
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>

#include "parse_subunit_v1.h"

const char *
directive_string(enum directive dir) {

	switch (dir) {
	case DIR_TEST:
		return "DIR_TEST";
	case DIR_SUCCESS:
		return "DIR_SUCCESS";
	case DIR_FAILURE:
		return "DIR_FAILURE";
	case DIR_ERROR:
		return "DIR_ERROR";
	case DIR_SKIP:
		return "DIR_SKIP";
	case DIR_XFAIL:
		return "DIR_XFAIL";
	case DIR_UXSUCCESS:
		return "DIR_UXSUCCESS";
	case DIR_PROGRESS:
		return "DIR_PROGRESS";
	case DIR_TAGS:
		return "DIR_TAGS";
	case DIR_TIME:
		return "DIR_TIME";
	default:
		return "DIR_UNKNOWN";
	}
}

enum directive
resolve_directive(char * string) {

	assert(string != (char*)NULL);

	if ((strcasecmp(string, "test") == 0) ||
	    (strcasecmp(string, "testing") == 0) ||
	    (strcasecmp(string, "test:") == 0) ||
	    (strcasecmp(string, "testing:") == 0)) {
		return DIR_TEST;
	} else if ((strcasecmp(string, "success") == 0) ||
		   (strcasecmp(string, "success:") == 0) ||
		   (strcasecmp(string, "successful") == 0) ||
		   (strcasecmp(string, "successful:") == 0)) {
		return DIR_SUCCESS;
	} else if (strcasecmp(string, "failure:") == 0) {
		return DIR_FAILURE;
	} else if (strcasecmp(string, "error:") == 0) {
		return DIR_ERROR;
	} else if ((strcasecmp(string, "skip") == 0) ||
		   (strcasecmp(string, "skip:") == 0)) {
		return DIR_SKIP;
	} else if ((strcasecmp(string, "xfail") == 0) ||
		   (strcasecmp(string, "xfail:") == 0)) {
		return DIR_XFAIL;
	} else if ((strcasecmp(string, "uxsuccess") == 0) ||
		   (strcasecmp(string, "uxsuccess:") == 0)) {
		return DIR_UXSUCCESS;
	} else if (strcasecmp(string, "progress:") == 0) {
		return DIR_PROGRESS;
	} else if (strcasecmp(string, "tags:") == 0) {
		return DIR_TAGS;
	} else if (strcasecmp(string, "time:") == 0) {
		return DIR_TIME;
	} else {
		/* unknown directive */
	}

	return DIR_TEST;
}

struct tm* parse_iso8601_time(char* date_str, char* time_str) {
	assert(date_str != (char*)NULL);
	assert(time_str != (char*)NULL);

	struct tm * t;
	t = malloc(sizeof(struct tm));
	if (t == NULL) {
		perror("failed to malloc");
		return NULL;
	}
	if (sscanf(date_str, "%d-%d-%d", &t->tm_year, &t->tm_mon, &t->tm_mday) == 3) {
		assert(t->tm_year > 2000);
		assert((t->tm_mon <= 12) && (t->tm_mon >= 0));
		assert((t->tm_mday <= 31) && (t->tm_mday >= 0));
	}

	if (sscanf(time_str, "%d:%d:%dZ", &t->tm_hour, &t->tm_min, &t->tm_sec) == 3) {
		assert((t->tm_hour <= 23) && (t->tm_hour >= 0));
		assert((t->tm_min <= 60) && (t->tm_min >= 0));
		assert((t->tm_sec <= 60) && (t->tm_sec >= 0));
	}

	return t;
}

void read_tok() {
	char* token = (char*)NULL;
	while (token != NULL) { token = strtok(NULL, " \t"); };
}

tailq_test* read_test() {

	tailq_test *test_item = NULL;
	test_item = calloc(1, sizeof(tailq_test));
	if (test_item == NULL) {
		perror("failed to malloc");
		return NULL;
	}

	char *token, *name;
	token = strtok(NULL, " \t");
	if (strcmp(token, "test") == 0) {
	   token = strtok(NULL, " \t");
	   assert(token != NULL);
	}
	name = (char*)calloc(strlen(token) + 1, sizeof(char));
	strcpy(name, token);
	test_item->name = name;

	read_tok();

	return test_item;
}

tailq_test* parse_line_subunit_v1(char* string) {

	assert(string != (char*)NULL);

	char *dir;
	char buffer[1024];
	strcpy(buffer, string);
	dir = strtok(buffer, " \t");

	tailq_test *test_item = NULL;
	enum directive d;
	switch (d = resolve_directive(dir)) {
	case DIR_TEST:
		/* testline is useless, but we should check conformance to spec */
		read_tok();
		break;
	case DIR_SUCCESS:
		test_item = read_test();
		test_item->status = STATUS_SUCCESS;
		break;
	case DIR_FAILURE:
		test_item = read_test();
		test_item->status = STATUS_FAILURE;
		break;
	case DIR_ERROR:
		test_item = read_test();
		test_item->status = STATUS_FAILED;
		break;
	case DIR_SKIP:
		test_item = read_test();
		test_item->status = STATUS_SKIPPED;
		break;
	case DIR_XFAIL:
		test_item = read_test();
		test_item->status = STATUS_XFAILURE;
		break;
	case DIR_UXSUCCESS:
		test_item = read_test();
		test_item->status = STATUS_UXSUCCESS;
		break;
	case DIR_PROGRESS:
		/* testline is useless, but we should check conformance to spec */
		read_tok();
		break;
	case DIR_TAGS:
		/* testline is useless, but we should check conformance to spec */
		read_tok();
		break;
	case DIR_TIME:
		/* testline is useless, but we should check conformance to spec */
		/*
		char *date = strtok(NULL, " \t");
		assert(date != (char*)NULL);
		char *time = strtok(NULL, " \t");
		assert(time != (char*)NULL);
		struct tm *t = parse_iso8601_time(date, time);
		printf("Time: %s\n", asctime(t));
		*/

		read_tok();
		break;
	default:
		read_tok();
		return NULL;
	}

	return test_item;
}

struct suiteq* parse_subunit_v1(FILE *stream) {

	tailq_suite *suite_item;
	suite_item = calloc(1, sizeof(tailq_suite));
	if (suite_item == NULL) {
		perror("malloc failed");
		return NULL;
	}
	/* TODO: n_errors, n_failures */
	suite_item->tests = calloc(1, sizeof(struct testq));
	if (suite_item->tests == NULL) {
		perror("malloc failed");
		free(suite_item);
		return NULL;
	};
	TAILQ_INIT(suite_item->tests);

    	char line[1024];
	tailq_test *test_item = NULL;
    	while (fgets(line, sizeof(line), stream)) {
		test_item = parse_line_subunit_v1(line);
		if (test_item != NULL) {
		   TAILQ_INSERT_TAIL(suite_item->tests, test_item, entries);
		}
		if (feof(stream)) {
			break;
		}
    	}

	struct suiteq *suites = NULL;
	suites = calloc(1, sizeof(struct suiteq));
	if (suites == NULL) {
		perror("malloc failed");
	};
	TAILQ_INIT(suites);
	TAILQ_INSERT_TAIL(suites, suite_item, entries);

	return suites;
}
