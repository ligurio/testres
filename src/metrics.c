/*
 * Copyright © 2019 Sergey Bronnikov
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

#include <math.h>

#include "metrics.h"

/* FIXME: passed, failed and skipped calculated twice */
double metric_pass_rate(struct tailq_report *report) {

    if (!report)
      return 0;

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

double metric_tc_avg_time(struct reportq *reports, char *tc_name) {
   int total_num = 0;
   double total_time = 0;

   tailq_report *report_item = NULL;
   TAILQ_FOREACH(report_item, reports, entries) {
      if (report_item->suites != NULL) {
         tailq_suite *suite_item = NULL;
         TAILQ_FOREACH(suite_item, report_item->suites, entries) {
            if (!TAILQ_EMPTY(suite_item->tests)) {
               tailq_test *test_item = NULL;
               TAILQ_FOREACH(test_item, suite_item->tests, entries) {
                  if (strcmp(test_item->name, tc_name) == 0) {
			total_time += atof(test_item->time);
			total_num++;
		  }
               }
            }
         }
      }
   }

   if (total_num != 0) {
      return total_time / total_num;
   }

   return 0;
}

/*  rate of fault detection per percentage of test suite execution */
int metric_apfd(struct reportq *reports, char *tc_name) {

   int total_num = 0;
   int failed_num = 0;
   double total_time = 0;

   tailq_report *report_item = NULL;
   TAILQ_FOREACH(report_item, reports, entries) {
      if (report_item->suites != NULL) {
         tailq_suite *suite_item = NULL;
         TAILQ_FOREACH(suite_item, report_item->suites, entries) {
            if (!TAILQ_EMPTY(suite_item->tests)) {
               tailq_test *test_item = NULL;
               TAILQ_FOREACH(test_item, suite_item->tests, entries) {
                  if (strcmp(test_item->name, tc_name) == 0) {
					 total_time += atof(test_item->time);
					 total_num++;
					 if (class_by_status(test_item->status) == STATUS_CLASS_FAIL) {
						failed_num++;
					 }
			      }
               }
            }
         }
      }
   }
   if (total_num != 0) {
      return (int)(100 * failed_num / total_num);
   }

   return 0;
}

char *metric_slowest_testcase(struct tailq_report *report) {

	if (!report)
	  return NULL;

	char *slowest = NULL;
	double time = 0;
	tailq_suite *suite_item = NULL;
	TAILQ_FOREACH(suite_item, report->suites, entries) {
		if (!TAILQ_EMPTY(suite_item->tests)) {
			tailq_test *test_item = NULL;
			TAILQ_FOREACH(test_item, suite_item->tests, entries) {
				if ((class_by_status(test_item->status) == STATUS_CLASS_FAIL) &&
					(test_item->time)) {
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

double metric_total_time(struct tailq_report *report) {

	if (!report)
	  return 0;

	double time = 0;
	tailq_suite *suite_item = NULL;
	TAILQ_FOREACH(suite_item, report->suites, entries) {
		if (!TAILQ_EMPTY(suite_item->tests)) {
			tailq_test *test_item = NULL;
			TAILQ_FOREACH(test_item, suite_item->tests, entries) {
				if (test_item->time)
				  time += atof(test_item->time);
			}
		}
	}

	return time;
}
