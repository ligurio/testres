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

#include <dirent.h>
#include <math.h>
#include <stdlib.h>

#include "parse_common.h"
#include "parse_junit.h"
#include "parse_subunit_v1.h"
#include "parse_subunit_v2.h"
#include "parse_testanything.h"
#include "sha1.h"
#include "testres.h"

void
free_reports(struct reportq * reports)
{
	tailq_report *report_item = NULL;
	while ((report_item = TAILQ_FIRST(reports))) {
		if (report_item->suites != NULL) {
			free_suites(report_item->suites);
		}
		TAILQ_REMOVE(reports, report_item, entries);
		free_report(report_item);
	}
}

void 
free_report(tailq_report *report)
{
	if (report->suites != NULL) {
		free_suites(report->suites);
	}
	free(report->path);
	free(report->id);
	free(report);
}

void 
free_suites(struct suiteq * suites)
{
	tailq_suite *suite_item = NULL;
	while ((suite_item = TAILQ_FIRST(suites))) {
		TAILQ_REMOVE(suites, suite_item, entries);
		free_suite(suite_item);
	}
}

void 
free_suite(tailq_suite * suite)
{
	if (suite->name) {
	   free((char*)suite->name);
        }
	if (suite->hostname) {
	   free((char*)suite->hostname);
        }
	if (suite->timestamp) {
	   free((char*)suite->timestamp);
        }
	if (!TAILQ_EMPTY(suite->tests)) {
		free_tests(suite->tests);
	}

	free(suite);
}

void 
free_tests(struct testq * tests)
{
	tailq_test *test_item;
	while ((test_item = TAILQ_FIRST(tests))) {
		TAILQ_REMOVE(tests, test_item, entries);
		free_test(test_item);
	}
}

void 
free_test(tailq_test * test)
{
	if (test->name) {
	   free((char*)test->name);
        }
	if (test->time) {
	   free((char*)test->time);
        }
	if (test->comment) {
	   free((char*)test->comment);
        }
	if (test->error) {
	   free((char*)test->error);
        }
	if (test->system_out) {
	   free((char*)test->system_out);
        }
	if (test->system_err) {
	   free((char*)test->system_err);
        }
	free(test);
}

char *
get_filename_ext(const char *filename)
{
	char *dot = strrchr(filename, '.');
	if (!dot || dot == filename)
	   return (char *) NULL;

	return dot + 1;
}

enum test_format 
detect_format(char *path)
{
	char *file_ext;
	file_ext = get_filename_ext(basename(path));
	if (file_ext == NULL) {
	   return FORMAT_UNKNOWN;
	}

	if (strcasecmp("xml", file_ext) == 0) {
		return FORMAT_JUNIT;
	} else if (strcasecmp("tap", file_ext) == 0) {
		return FORMAT_TAP13;
	} else if (strcasecmp("subunit", file_ext) == 0) {
		if (is_subunit_v2(path) == 0) {
		   return FORMAT_SUBUNIT_V2;
		} else {
		   return FORMAT_SUBUNIT_V1;
		}
	} else {
		return FORMAT_UNKNOWN;
	}
}

unsigned char *digest_to_str(unsigned char *str, unsigned char digest[], unsigned int n) {
	int r;
	if (n == 0) return 0;
	if (n == 1) r = sprintf((char*)str, "%x", digest[0]);
	else        r = sprintf((char*)str, "%x", digest[0]);
	digest_to_str(str + r, digest + 1, n - 1);

	return str;
}

tailq_report *
process_file(char *path)
{
	FILE *file;
	file = fopen(path, "r");
	if (file == NULL) {
		printf("failed to open file %s\n", path);
		return NULL;
	}
	tailq_report *report = NULL;
	report = calloc(1, sizeof(tailq_report));
	if (report == NULL) {
		perror("malloc failed");
		fclose(file);
		return NULL;
	}
	enum test_format format;
	format = detect_format(path);
	switch (format) {
	case FORMAT_JUNIT:
		report->format = FORMAT_JUNIT;
		report->suites = parse_junit(file);
		break;
	case FORMAT_TAP13:
		report->format = FORMAT_TAP13;
		report->suites = parse_testanything(file);
		break;
	case FORMAT_SUBUNIT_V1:
		report->format = FORMAT_SUBUNIT_V1;
		report->suites = parse_subunit_v1(file);
		break;
	case FORMAT_SUBUNIT_V2:
		report->format = FORMAT_SUBUNIT_V2;
		report->suites = parse_subunit_v2(file);
		break;
	case FORMAT_UNKNOWN:
		report->format = FORMAT_UNKNOWN;
		return report;
	}
	fclose(file);

	report->path = (unsigned char*)strdup(path);

	int length = 20;
	unsigned char digest[length];
	SHA1_CTX ctx;
	SHA1Init(&ctx);
	SHA1Update(&ctx, report->path, strlen(path));
	SHA1Final(digest, &ctx);

	report->id = calloc(length, sizeof(unsigned char*));
	digest_to_str(report->id, digest, length);

	struct stat sb;
	if (stat(path, &sb) == -1) {
		perror("cannot open specified path");
		return NULL;
	}
	report->time = sb.st_mtime;

	return report;
}

struct tailq_report *is_report_exists(struct reportq *reports, const char* report_id) {

	tailq_report *report_item = NULL;
	TAILQ_FOREACH(report_item, reports, entries) {
	    if (strcmp(report_id, (char*)report_item->id) == 0) {
		break;
	    }
	}

	return report_item;
}

/*
static int cmp_date(const void *p1, const void *p2) {
   return strcmp(* (char * const *) p1, * (char * const *) p2);
}

struct reportq *sort_reports(struct reportq *reports) {
   return reports;
}
*/

int num_by_status_class(struct tailq_report *report, enum test_status_class c) {

   int number = 0;
   if (report->suites != NULL) {
      tailq_suite *suite_item = NULL;
      TAILQ_FOREACH(suite_item, report->suites, entries) {
         if (!TAILQ_EMPTY(suite_item->tests)) {
            tailq_test *test_item = NULL;
            TAILQ_FOREACH(test_item, suite_item->tests, entries) {
               if (class_by_status(test_item->status) == c) number++;
            }
         }
      }
   }

   return number;
}

enum test_status_class class_by_status(enum test_status status) {

   switch (status) {
   case STATUS_OK:
     return STATUS_CLASS_PASS;
   case STATUS_PASS:
     return STATUS_CLASS_PASS;
   case STATUS_SUCCESS:
     return STATUS_CLASS_PASS;
   case STATUS_NOTOK:
     return STATUS_CLASS_FAIL;
   case STATUS_ERROR:
     return STATUS_CLASS_FAIL;
   case STATUS_FAILURE:
     return STATUS_CLASS_FAIL;
   case STATUS_FAILED:
     return STATUS_CLASS_FAIL;
   case STATUS_XFAILURE:
     return STATUS_CLASS_FAIL;
   case STATUS_UXSUCCESS:
     return STATUS_CLASS_FAIL;
   case STATUS_MISSING:
     return STATUS_CLASS_SKIP;
   case STATUS_TODO:
     return STATUS_CLASS_SKIP;
   case STATUS_SKIP:
     return STATUS_CLASS_SKIP;
   case STATUS_SKIPPED:
     return STATUS_CLASS_SKIP;
   case STATUS_UNDEFINED:
     return STATUS_CLASS_SKIP;
   case STATUS_ENUMERATION:
     return STATUS_CLASS_SKIP;
   case STATUS_INPROGRESS:
     return STATUS_CLASS_SKIP;
   default:
     break;
   }
}

/*
 * passed, failed and skipped calculated twice.
 * it makes sense to keep values in tailq_report struct
 */
double calc_success_perc(struct tailq_report *report) {
    double num = 0;
    if (report->suites != NULL) {
       int passed = num_by_status_class(report, STATUS_CLASS_PASS);
       int failed = num_by_status_class(report, STATUS_CLASS_FAIL);
       int skipped = num_by_status_class(report, STATUS_CLASS_SKIP);
       num = (double)passed / (double)(passed + failed + skipped) * 100;
    } else {
       num = 0;
    }

    return round(num);
}

struct reportq*
process_dir(char *path) {

	DIR *d;
	int fd;

	fd = open(path, O_RDONLY);
	if ((d = fdopendir(fd)) == NULL) {
		printf("failed to open dir %s\n", path);
		close(fd);
		return NULL;
	}

	struct reportq *reports;
	reports = calloc(1, sizeof(struct reportq));
	if (reports == NULL) {
	   return NULL;
	}
	TAILQ_INIT(reports);

	struct dirent *dir;
	char *path_file = (char *) NULL;
	tailq_report *report_item;
	while ((dir = readdir(d)) != NULL) {
		char *basename;
		basename = dir->d_name;
		if ((strcmp("..", basename) == 0) || (strcmp(".", basename) == 0)) {
		   continue;
		}
		/* TODO: recursive search in directories */
		int path_len = strlen(path) + strlen(basename) + 2;
		path_file = calloc(path_len, sizeof(char));
		if (path_file == NULL) {
			return NULL;
		}
		snprintf(path_file, path_len, "%s/%s", path, basename);

		struct stat path_st;
		if (stat(path, &path_st) == -1) {
		   perror("cannot open specified path");
		   return NULL;
		}
		if (S_ISREG(path_st.st_mode)) {
		   continue;
		}
		report_item = process_file(path_file);
		if (report_item->format != FORMAT_UNKNOWN) {
		   TAILQ_INSERT_TAIL(reports, report_item, entries);
        }
		free(path_file);
	}
	close(fd);
	closedir(d);

	return reports;
}

struct reportq*
process_db(char *path)
{
	/* not implemented */
	return NULL;
}

int
check_sqlite(char *path)
{
	/* not implemented */
	/* see https://www.sqlite.org/fileformat.html */
	return 1;
}

struct test_metrics *calc_test_metrics(struct reportq *reports, char *name) {
   int num = 0;			/* total number of executions */
   int fail_num = 0;	/* number of failed executions */
   double time = 0;		/* duration of all executions */
   struct test_metrics *metrics;
   metrics = malloc(sizeof(struct test_metrics));
   if (metrics == NULL) {
      fprintf(stderr, "malloc()");
      return 0;
   }
   metrics->avg_time = 0.0;
   metrics->avg_faults = 0;

   tailq_report *report_item = NULL;
   TAILQ_FOREACH(report_item, reports, entries) {
      if (report_item->suites != NULL) {
         tailq_suite *suite_item = NULL;
         TAILQ_FOREACH(suite_item, report_item->suites, entries) {
            if (!TAILQ_EMPTY(suite_item->tests)) {
               tailq_test *test_item = NULL;
               TAILQ_FOREACH(test_item, suite_item->tests, entries) {
                  if (strcmp(test_item->name, name) == 0) {
					 time += atof(test_item->time);
					 num++;
					 if (class_by_status(test_item->status) == STATUS_CLASS_FAIL) {
						fail_num++;
					 }
			      }
               }
            }
         }
      }
   }
   if (num != 0) {
      metrics->avg_time = time / num;
      metrics->avg_faults = (int)(100 * fail_num / num);
   }

   return metrics;
}

struct reportq *filter_reports(struct reportq *reports, const char *qsearch) {

	if (qsearch == NULL || reports == NULL) {
		return NULL;
	}

	struct reportq *filtered;
	filtered = calloc(1, sizeof(struct reportq));
	if (filtered == NULL) {
		return NULL;
	}
	TAILQ_INIT(filtered);

	tailq_report *report_item = NULL;
	TAILQ_FOREACH(report_item, reports, entries) {
		if (report_item->suites != NULL) {
			tailq_suite *suite_item = NULL;
			int matched = 0;
			TAILQ_FOREACH(suite_item, report_item->suites, entries) {
				if (matched == 1) {
					break;
				}
				if (!TAILQ_EMPTY(suite_item->tests)) {
					tailq_test *test_item = NULL;
					TAILQ_FOREACH(test_item, suite_item->tests, entries) {
						if (strstr(test_item->name, qsearch) != NULL) {
							TAILQ_INSERT_TAIL(filtered, report_item, entries);
							matched = 1;
							break;
						}
					}
				}
			}
		}
	}

	return reports;
}

int cgi_parse(char *query_string, struct config *conf) {
	conf->cgi_action = strtok(query_string, "=");
	conf->cgi_args = strtok(NULL, "=");
	if (conf->cgi_action == NULL) {
		return 1;
	}

	return 0;
}

char *slowest_testcase(struct tailq_report *report) {

	char *slowest = NULL;
	double time = 0;
	tailq_suite *suite_item = NULL;
	TAILQ_FOREACH(suite_item, report->suites, entries) {
		if (!TAILQ_EMPTY(suite_item->tests)) {
			tailq_test *test_item = NULL;
			TAILQ_FOREACH(test_item, suite_item->tests, entries) {
				if (class_by_status(test_item->status) == STATUS_CLASS_FAIL) {
					double t = atof(test_item->time);
					if ((t > time) && (t > SLOWEST_THRESHOLD)) {
						time = t;
						slowest = (char*)test_item->name;
					}
				}
			}
		}
	}

	return slowest;
}

double report_total_time(struct tailq_report *report) {

	double time = 0;
	tailq_suite *suite_item = NULL;
	TAILQ_FOREACH(suite_item, report->suites, entries) {
		if (!TAILQ_EMPTY(suite_item->tests)) {
			tailq_test *test_item = NULL;
			TAILQ_FOREACH(test_item, suite_item->tests, entries) {
				time += atof(test_item->time);
			}
		}
	}

	return time;
}
