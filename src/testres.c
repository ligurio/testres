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

#include <err.h>
#include <fcntl.h>
#include <libgen.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include "metrics.h"
#include "parse_common.h"
#include "testres.h"
#include "ui_console.h"
#include "ui_http.h"

void
usage(char *path)
{
	char *name = basename(path);
	fprintf(stderr, "Usage: %s [-s file | directory] [-t name] [-h|-v]\n", name);
}

int
main(int argc, char *argv[])
{
	char *path = (char *) NULL;
	char *name = (char *) NULL;
	int opt = 0;

#ifdef __OpenBSD__
	if (pledge("stdio rpath", NULL) == -1) {
	    warn("pledge");
	    return EXIT_FAILURE;
	}
#endif /* __OpenBSD__ */

	while ((opt = getopt(argc, argv, "vhs:t:")) != -1) {
		switch (opt) {
		case 'h':
			usage(argv[0]);
			return 0;
		case 'v':
			printf("%s\n", VERSION);
			return 0;
		case 's':
			path = realpath(optarg, path);
			break;
		case 't':
			name = optarg;
			break;
		default:	/* '?' */
			usage(argv[0]);
			return 1;
		}
	}

	if (argc == 1) {
		usage(argv[0]);
		return 1;
	}

	struct stat path_st;
	if (stat(path, &path_st) == -1) {
	   fprintf(stderr, "cannot open specified path");
	   perror("stat:");
	   return 1;
	}

	config *conf;
	conf = calloc(1, sizeof(config));
	if (conf == NULL) {
		perror("calloc:");
		return 1;
	}

	if (isatty(fileno(stdin)) == 0) {
		conf->mode = HTTP_MODE;
	} else {
		conf->mode = TEXT_MODE;
	}

	char *query_string = getenv("QUERY_STRING");
	if (conf->mode == HTTP_MODE) {
		if (cgi_parse(query_string, conf) == 1) {
			print_html_headers();
			printf("wrong a http request: %s\n", query_string);
			print_html_footer();
			return 1;
		}
	}

	struct reportq *reports;
	struct tailq_report *report;
	if (S_ISREG(path_st.st_mode)) {
		if (name != NULL) {
			fprintf(stderr, "testcase metrics not supported for single report\n");
			return 1;
		}
		if (check_sqlite(path) == 0) {
			conf->source = SOURCE_SQLITE;
			reports = process_db(path);
		} else {
			conf->source = SOURCE_FILE;
			report = process_file(path);
		}
	} else if (S_ISDIR(path_st.st_mode)) {
		conf->source = SOURCE_DIR;
		reports = process_dir(path);
	} else {
		fprintf(stderr, "unsupported source type");
		return 1;
	}

	if (conf->mode == HTTP_MODE) {
		if (strcmp(conf->cgi_action, "/") == 0) {
			print_html_headers();
			print_html_reports(reports);
			print_html_footer();
			free_reports(reports);
			return 0;
		} else if (strcmp(conf->cgi_action, "show") == 0) {
			if ((report = is_report_exists(reports, conf->cgi_args)) != NULL) {
				print_html_headers();
				print_html_report(report);
				print_html_footer();
			} else {
				print_html_headers();
				printf("report not found\n");
				print_html_footer();
			}
			free_report(report);
			return 0;
		} else if (strcmp(conf->cgi_action, "q") == 0) {
			struct reportq *filtered = NULL;
			filtered = filter_reports(reports, conf->cgi_args);
			print_html_headers();
			print_html_reports(filtered);
			print_html_footer();
			free_reports(reports);
			free_reports(filtered);
			return 0;
		} else {
			print_html_headers();
			printf("unknown action\n");
			print_html_footer();
		}
	}

	if (conf->mode == TEXT_MODE) {
		if (conf->source == SOURCE_DIR) {
			if (name != NULL) {
				struct test_metrics *metrics;
				metrics = calc_test_metrics(reports, name);
				printf("Tescase Name: %s\n", name);
				printf("Average Percentage of Fault Detected (APFD): %d%%\n",
						metrics->avg_faults);
				printf("Average Execution Time: %f\n", metrics->avg_time);
				free(metrics);
				free_reports(reports);
				return 0;
			}
			print_reports(reports);
			free_reports(reports);
			return 0;
		} else if (conf->source == SOURCE_FILE) {
			print_report(report);
			free_report(report);
			return 0;
		} else {
			fprintf(stderr, "unknown source");
			return 1;
		}
	}

	return 0;
}
