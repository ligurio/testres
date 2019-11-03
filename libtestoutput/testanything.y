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

%{
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <parse_common.h>
#include "parse_testanything.tab.h"

#define YYDEBUG 1

struct suiteq *parse_testanything(FILE *f);
void yyerror(const char *);
int yylex(void);
static void set_missed_status(long tc_missed);

static tailq_test *create_new_test(void);
static tailq_suite *create_new_suite(void);

static tailq_suite *cur_suite = NULL;
static tailq_test *cur_test = NULL;
struct suiteq *suites = NULL;

static bool is_bailout = false;
static bool is_test = false;

static long tc_planned = 0;
static long tc_current = 0;
static long tc_processed = 0;

static char *string = NULL, *word = NULL, *number = NULL;

static tailq_test *create_new_test(void) {
   tailq_test *test_item = NULL;
   test_item = calloc(1, sizeof(tailq_test));
   if (test_item == NULL) {
      perror("calloc");
      return NULL;
   }

   return test_item;
}

static tailq_suite *create_new_suite(void) {
   tailq_suite *test_suite = NULL;
   test_suite = calloc(1, sizeof(tailq_suite));
   if (!test_suite) {
      perror("calloc");
      return NULL;
   }

   test_suite->tests = calloc(1, sizeof(struct testq));
   if (!test_suite->tests) {
      perror("calloc");
      free_suite(test_suite);
      return NULL;
   }
   TAILQ_INIT(test_suite->tests);

   return test_suite;
}

static void set_missed_status(long tc_missed) {
   long i = 0;
   for(i = 0; i <= tc_missed; i++) {
      tailq_test *test = create_new_test();
      test->name = NULL;
      test->status = STATUS_SKIP;
      TAILQ_INSERT_TAIL(cur_suite->tests, test, entries);
   }
}

%}

%token NOT OK BAILOUT SKIP TODO
%token HASH DASH PLAN TAP_VERSION
%token WORD NUMBER NL YAML_START YAML_END

%union {
	long long_val;
	char *string;
};

%type <long_val>	NUMBER
%type <long_val>	test_number
%type <string> 		WORD
%type <string> 		PLAN
%type <string> 		string
%type <string> 		status

%%
program		: program test_line
		| error NL { yyerrok; }
		|
		;

test_line	: TAP_VERSION NUMBER NL {
			int version = $2;
			assert(version == 13);
		}
		| PLAN comment NL {
			if (sscanf($1, "1..%ld", &tc_planned) != 1) {
			   perror("sscanf");
			   return -1;
			}
		}
		| status test_number description comment NL {
			cur_test->time = NULL;
			TAILQ_INSERT_TAIL(cur_suite->tests, cur_test, entries);
			cur_test = NULL;
			is_test = false;
		}
		| comment NL {
			fprintf(stderr, "COMMENT\n");
		}
		| BAILOUT string NL {
			fprintf(stderr, "BAIL OUT!\n");
			is_bailout = true;
			set_missed_status(tc_planned - tc_current);
			string = NULL;
		}
		| YAML_START NL yaml_strings YAML_END NL {
			fprintf(stderr, "YAML\n");
		}
		;

comment	: HASH directive string {
			cur_test->comment = string;
			string = NULL;
		}
		|
		;

test_number	: NUMBER {
		tc_current = $1;
		assert(tc_current == tc_processed);
		}
		;

description	: string {
		cur_test->name = string;
		string = NULL;
		}
		| DASH string {
		cur_test->name = string;
		string = NULL;
		}
		|
		;

status	: OK {
		cur_test = create_new_test();
		cur_test->status = STATUS_PASS;
		tc_processed++;
		is_test = true;
		}
		| NOT OK {
		cur_test = create_new_test();
		cur_test->status = STATUS_FAILED;
		tc_processed++;
		is_test = true;
		}
		;

directive	: TODO {
		cur_test->status = STATUS_TODO;
		}
		| SKIP {
		cur_test->status = STATUS_SKIP;
		}
		|
		;

string	: string WORD {
		word = $2;
		if (string == NULL) {
		    string = strdup(word);
		} else {
		    string = realloc(string, strlen(string) + strlen(word) + 2);
		    sprintf(string, "%s %s", string, word);
                    free(word);
                }
		}
		| string NUMBER {
/*
		sprintf(number, "%ld\n", $2);
		if (string == NULL) {
		    string = strdup(number);
		} else {
		    string = realloc(string, strlen(string) + strlen(number) + 2);
		    sprintf(string, "%s %s\n", string, number);
                }
                free(number);
*/
		}
		|
		;

yaml_strings: yaml_strings string NL
		|
		;
%%

#include <ctype.h>
#include <sys/queue.h>

extern int yylex();
extern int yyparse();
extern int yylineno;
extern FILE *yyin;

void yyerror(const char *s)
{
    fprintf(stderr, "Warning: %s, line %d\n", s, yylineno);
}

struct suiteq *parse_testanything(FILE *f) {

  if (f == NULL) {
	return NULL;
  }

  is_bailout = false;
  is_test = false;

  tc_planned = 0;
  tc_current = 0;
  tc_processed = 0;

  cur_suite = create_new_suite();
  if (!cur_suite) {
      return NULL;
  }

  yyin = f;
  yylineno = 1;
  yyparse();

  suites = calloc(1, sizeof(struct suiteq));
  if (!suites) {
    perror("calloc");
    free_tests(cur_suite->tests);
    free_suite(cur_suite);
    return NULL;
  }
  TAILQ_INIT(suites);
  TAILQ_INSERT_TAIL(suites, cur_suite, entries);
  free(string);

  return suites;
}
