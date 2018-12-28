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

#ifndef PARSE_TESTANYTHING_H
#define PARSE_TESTANYTHING_H

enum ast_status {
	AST_OK,
	AST_NOTOK,
	AST_MISSING,
	AST_TODO,
	AST_SKIP
};

struct ast_line {
	/* TODO: comment flag */
	const char *text;
	struct ast_line *next;
};

struct ast_test {
	const char *name;
	enum ast_status status;
	unsigned int rep;
	struct ast_line *line;
	struct ast_test *next;
};

const char *
ast_status(enum ast_status status);

struct ast_line *
ast_line(struct ast_line **head, const char *text);

struct ast_test *
ast_test(struct ast_test **head, enum ast_status status, const char *name);

void
print(FILE *f, const struct ast_test *tests);

struct ast_test *parse_testanything_raw(FILE *f);
struct suiteq *parse_testanything(FILE *f);

#endif				/* PARSE_TESTANYTHING_H */
