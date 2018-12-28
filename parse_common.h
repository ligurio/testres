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

#ifndef PARSE_COMMON_H
#define PARSE_COMMON_H

#include <fcntl.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/queue.h>

enum test_format {
	FORMAT_UNKNOWN,
	FORMAT_TAP13,
	FORMAT_JUNIT,
	FORMAT_SUBUNIT_V1,
	FORMAT_SUBUNIT_V2
};

enum test_status {
	STATUS_OK,			/* TestAnythingProtocol	*/
	STATUS_NOTOK,		/* TestAnythingProtocol	*/
	STATUS_MISSING,		/* TestAnythingProtocol	*/
	STATUS_TODO,		/* TestAnythingProtocol	*/
	STATUS_SKIP,		/* TestAnythingProtocol	*/

	STATUS_UNDEFINED,	/* Subunit */
	STATUS_ENUMERATION,	/* Subunit */
	STATUS_INPROGRESS,	/* Subunit */
	STATUS_SUCCESS,		/* Subunit */
	STATUS_UXSUCCESS,	/* Subunit */
	STATUS_SKIPPED,		/* Subunit */
	STATUS_FAILED,		/* Subunit */
	STATUS_XFAILURE,	/* Subunit */

	STATUS_ERROR,		/* JUnit */
	STATUS_FAILURE,		/* JUnit */
	STATUS_PASS			/* JUnit */
};

enum test_status_class {
	STATUS_CLASS_PASS,
	STATUS_CLASS_FAIL,
	STATUS_CLASS_SKIP
};

struct tailq_test {
    const char *name;
    const char *time;
    const char *comment;
    const char *error;
    const char *system_out;
    const char *system_err;
    enum test_status status;
    TAILQ_ENTRY(tailq_test) entries;
};

TAILQ_HEAD(testq, tailq_test);

struct tailq_suite {
    const char *name;
    const char *hostname;
    const char *timestamp;
    int n_failures;
    int n_errors;
    double time;
    struct testq *tests;
    TAILQ_ENTRY(tailq_suite) entries;
};

TAILQ_HEAD(suiteq, tailq_suite);

struct tailq_report {
    enum test_format format;
    struct suiteq *suites;
    time_t ctime;
    unsigned char *id;
    unsigned char *path;
    TAILQ_ENTRY(tailq_report) entries;
};

TAILQ_HEAD(reportq, tailq_report);

typedef struct tailq_test tailq_test;
typedef struct tailq_suite tailq_suite;
typedef struct tailq_report tailq_report;

char *get_filename_ext(const char *filename);
enum test_format detect_format(char *path);
struct reportq *process_dir(char *path);
tailq_report *process_file(char *path);
tailq_test *make_test(char *name, char *time, char *comment);
unsigned char *digest_to_str(unsigned char *str, unsigned char digest[], unsigned int n);
struct tailq_report *is_report_exists(struct reportq *reports, const char* report_id);

/*
static int cmp_date(const void *p1, const void *p2);
struct reportq *sort_reports(struct reportq *reports);
*/

int num_by_status_class(struct tailq_report *report, enum test_status_class c);
enum test_status_class class_by_status(enum test_status status);
double calc_success_perc(struct tailq_report *report);

/* cleanup */
void free_reports(struct reportq *reports);
void free_suites(struct suiteq *suites);
void free_tests(struct testq *tests);

void free_report(tailq_report * report);
void free_suite(tailq_suite * suite);
void free_test(tailq_test * test);

#endif				/* PARSE_COMMON_H */
