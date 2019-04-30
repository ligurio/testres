/*
 * Copyright © 2015-2017 Katherine Flavel <kate@elide.org>
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

#define _POSIX_C_SOURCE 200809L

#include <sys/stat.h>
#include <sys/types.h>

#include <assert.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>

#include <string.h>
#include <strings.h>

#include "parse_common.h"
#include "parse_testanything.h"

const char *
ast_status(enum ast_status status)
{
	switch (status) {
		case AST_OK:return "ok";
	case AST_NOTOK:
		return "not ok";
	case AST_MISSING:
		return "missing";
	case AST_TODO:
		return "todo";
	case AST_SKIP:
		return "skip";

	default:
		return "?";
	}
}

enum test_status 
test_status(enum ast_status status)
{
	switch (status) {
		case AST_OK:return STATUS_OK;
	case AST_NOTOK:
		return STATUS_NOTOK;
	case AST_MISSING:
		return STATUS_MISSING;
	case AST_TODO:
		return STATUS_TODO;
	case AST_SKIP:
		return STATUS_SKIP;
	default:
		return STATUS_MISSING;
	}
}

struct ast_line *
ast_line(struct ast_line ** head, const char *text)
{
	struct ast_line **tail, *new;
	size_t z;

	assert(head != NULL);
	assert(text != NULL);

	z = strlen(text);

	new = malloc(sizeof *new + z + 1);
	if (new == NULL) {
		return NULL;
	}
	new->text = strcpy((char *) new + sizeof *new, text);

	for (tail = head; *tail != NULL; tail = &(*tail)->next);

	new->next = *tail;
	*tail = new;

	return new;
}

struct ast_test *
ast_test(struct ast_test ** head, enum ast_status status, const char *name)
{
	struct ast_test **tail, *new;

	assert(head != NULL);

	new = malloc(sizeof *new +
	    (name == NULL ? 0 : strlen(name) + 1));
	if (new == NULL) {
		return NULL;
	}
	if (name == NULL) {
		new->name = NULL;
	} else {
		new->name = strcpy((char *) new + sizeof *new, name);
	}

	new->rep = 1;
	new->line = NULL;
	new->status = status;

	for (tail = head; *tail != NULL; tail = &(*tail)->next);

	new->next = *tail;
	*tail = new;

	return new;
}

static void
rtrim(char *s)
{
	char *p;

	assert(s != NULL);

	if (*s == '\0') {
		return;
	}
	p = s + strlen(s) - 1;

	assert(strlen(s) > 0);

	while (p >= s && isspace((unsigned char) *p)) {
		*p-- = '\0';
	}
}

static void
plan(const char *line, int *a, int *b)
{
	assert(line != NULL);
	assert(a != NULL);
	assert(b != NULL);

	if (*b != -1) {
		fprintf(stderr, "syntax error: duplicate plan: %s\n", line);
		exit(1);
	}
	if (2 != sscanf(line, "%d..%d", a, b)) {
		fprintf(stderr, "syntax error: missing a..b\n");
		exit(1);
	}
	if (*a < 0 && *b < *a) {
		fprintf(stderr, "error: invalid plan: %d..%d\n", *a, *b);
		exit(1);
	}
}

static void
yaml(struct ast_test * test, const char *line)
{
	assert(test != NULL);

	while (test->next != NULL)
		test = test->next;

	if (!ast_line(&test->line, line)) {
		perror("ast_line");
		exit(1);
	}
}

static void
gap(struct ast_test ** head, int *a, int b)
{
	struct ast_test *new;

	assert(head != NULL);
	assert(a != NULL);
	assert(*a <= b);

	while (*a < b) {
		new = ast_test(head, AST_MISSING, NULL);
		if (new == NULL) {
			perror("ast_test");
			exit(1);
		}
		*a += 1;
	}
}

static void
starttest(struct ast_test ** head, const char *line, int *a, int b)
{
	struct ast_test *new;
	enum ast_status status;
	int i;
	int n;

	assert(a != NULL);
	assert(head != NULL);
	assert(line != NULL);
	assert(b == -1 || *a <= b);

	if (0 == strncmp(line, "not ", 4)) {
		line += 4;
		status = AST_NOTOK;
	} else {
		status = AST_OK;
	}

	if (1 != sscanf(line, "ok %d - %n", &i, &n)) {
		fprintf(stderr, "syntax error: expected 'ok - ': %s\n", line);
		exit(1);
	}
	line += n;

	if (i < *a || (b != -1 && i > b)) {
		fprintf(stderr, "error: test %d out of order; expected %d\n", i, *a);
		exit(1);
	}
	gap(head, a, i);

	new = ast_test(head, status, line);
	if (new == NULL) {
		perror("ast_test");
		exit(1);
	}
	*a += 1;
}

void
print(FILE * f, const struct ast_test * tests)
{
	const struct ast_test *test;
	const struct ast_line *line;
	unsigned int n;

	assert(f != NULL);
	assert(tests != NULL);

	for (test = tests, n = 1; test != NULL; test = test->next, n++) {
		fprintf(f, "\t<test status='%s'",
		    ast_status(test->status));

		fprintf(f, " n='%u'", n);

		if (test->rep > 1) {
			fprintf(f, " rep='%u'", test->rep);
		}
		if (test->name != NULL) {
			fprintf(f, " name='");
			fprintf(f, "%s", test->name);
			fprintf(f, "'");
		}
		fprintf(f, "%s>\n", test->line != NULL ? "" : "/");

		if (test->line == NULL) {
			continue;
		}
		for (line = test->line; line != NULL; line = line->next) {
			fprintf(f, "%s%s",
			    line->text,
			    line->next != NULL ? "\n" : "");
		}
	}
}

struct ast_test *
parse_testanything_raw(FILE * f)
{
	struct ast_test *tests = NULL;
	int a, b;
	int fold = 0;

	{
		char *line, *comment;
		size_t n;

		line = NULL;
		a = 1;
		b = -1;		/* no plan */
		n = 0;

		while (-1 != getline(&line, &n, f)) {
			line[strcspn(line, "\n")] = '\0';

			comment = strchr(line, '#');
			if (comment != NULL) {
				*comment++ = '\0';
			}
			rtrim(line);

			if (comment != NULL) {
				comment += strspn(comment, " \t");

				/* TODO: only if we're actually in a test */

				if (0 == strncasecmp(comment, "todo", 4)) {
					tests->status = AST_TODO;
				}
				if (0 == strncasecmp(comment, "skip", 4)) {
					tests->status = AST_SKIP;
				}
				/* TODO: add comment line anyway */
				/*
				 * TODO: add as yaml line printf("\t<!-- %s
				 * -->\n", comment);
				 */
			}
			switch (line[0]) {
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				plan(line, &a, &b);
				continue;

			case 'n':
			case 'o':
				starttest(&tests, line, &a, b);
				continue;

			case '\v':
			case '\t':
			case '\f':
			case '\r':
			case '\n':
			case ' ':
				if (tests == NULL) {
					fprintf(stderr, "stray text: %s\n", line);
					continue;
				}
				yaml(tests, line);
				continue;
			}
		}

		gap(&tests, &a, b + 1);
	}

	if (fold) {
		struct ast_test *test, *next;

		for (test = tests; test != NULL; test = next) {
			next = test->next;
			if (next == NULL) {
				continue;
			}
			if (0 != strcmp(test->name, next->name)) {
				continue;
			}
			if (test->status != next->status) {
				continue;
			}
			if (test->line != NULL || next->line != NULL) {
				continue;
			}
			test->rep++;
			test->next = next->next;
			next = test;

			/* TODO: free next */
		}
	}
	/* TODO: warn about duplicate test names */
	/* TODO: remove ast_test * tests here */

	/* print(stdout, tests); */
	return tests;
}

struct suiteq *
parse_testanything(FILE * f)
{
	tailq_suite *suite_item = NULL;
	suite_item = calloc(1, sizeof(tailq_suite));
	if (suite_item == NULL) {
		perror("malloc failed");
	}
	suite_item->n_errors = 0;
	suite_item->n_failures = 0;
	suite_item->tests = calloc(1, sizeof(struct testq));
	if (suite_item->tests == NULL) {
		perror("malloc failed");
		free(suite_item);
	}
	TAILQ_INIT(suite_item->tests);

	struct ast_test *tests, *current;
	tests = parse_testanything_raw(f);
	tailq_test *test_item;
	current = tests;
	while (current != NULL) {
		test_item = calloc(1, sizeof(tailq_test));
		if (test_item == NULL) {
			perror("malloc failed");
			free_tests(suite_item->tests);
			free(suite_item);
			return NULL;
		}
		char *name = calloc(strlen(current->name) + 1, sizeof(char));
		if (name == NULL) {
			perror("malloc failed");
			free_tests(suite_item->tests);
			free(suite_item);
			free(test_item);
			return NULL;
		}
		test_item->name = strcpy(name, current->name);
		test_item->status = test_status(current->status);
		TAILQ_INSERT_TAIL(suite_item->tests, test_item, entries);
		current = current->next;
		/* TODO: remove ast_test item */
	}

	struct suiteq *suites;
	suites = calloc(1, sizeof(struct suiteq));
	if (suites == NULL) {
		free_tests(suite_item->tests);
		free(suite_item);
		perror("malloc failed");
		return NULL;
	}
	TAILQ_INIT(suites);
	TAILQ_INSERT_TAIL(suites, suite_item, entries);

	return suites;
}
