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

#include <err.h>
#include <fcntl.h>
#include <libgen.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include "parse_common.h"
#include "ui_console.h"
#include "ui_http.h"

char version[1024];

void
usage(char *path)
{
	char *name = basename(path);
	fprintf(stderr, "Usage: %s [-s file | directory] [-h|-v]\n", name);
}

int
main(int argc, char *argv[])
{
	char *path = (char *) NULL;
	const char *stylesheet = "/testres.css";
	int opt = 0;

#ifdef __OpenBSD__
	if (pledge("stdio rpath", NULL) == -1) {
	    warn("pledge");
	    return EXIT_FAILURE;
	}
#endif /* __OpenBSD__ */

	snprintf(version, sizeof(version), "%s %s", __DATE__, __TIME__);
	while ((opt = getopt(argc, argv, "vhs:")) != -1) {
		switch (opt) {
		case 'h':
			usage(argv[0]);
			return 0;
		case 'v':
			printf("Build-date: %s\n", version);
			return 0;
		case 's':
			path = realpath(optarg, path);
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

	if (path == (char*)NULL) {
		fprintf(stderr, "specified path is empty");
		return 1;
	}

	struct stat path_st;
	if (stat(path, &path_st) == -1) {
	   fprintf(stderr, "cannot open specified path");
	   return 1;
	}

	char *query_string = getenv("QUERY_STRING");
	tailq_report *report_item;
	if (S_ISREG(path_st.st_mode)) {
	   report_item = process_file(path);
	   if (query_string != NULL) {
	      print_html_headers(version, stylesheet);
	      print_html_report(report_item);
	      print_html_footer(version);
	   } else {
	      /* print_report_prof(report_item); */
	      print_report(report_item);
 	   }
	   free_report(report_item);
	   return 0;
	}

	struct reportq *reports;
	reports = process_dir(path);
	if (query_string == NULL) {
	   print_reports(reports);
	   free_reports(reports);
	   return 0;
	}

	if (strcmp(query_string, "index") == 0) {
	   print_html_headers(version, stylesheet);
	   print_html_reports(reports);
	   print_html_footer(version);
	   free_reports(reports);
	   return 0;
	}

	if (strcmp(strtok(query_string, "="), "show") == 0) {
	   const char *report_id = strtok(NULL, "=");
	   struct tailq_report *report;
	   if ((report = is_report_exists(reports, report_id)) != NULL) {
	      print_html_headers(version, stylesheet);
	      print_html_report(report);
	      print_html_footer(version);
	      free_report(report);
	      free_reports(reports);
	   } else {
	      print_html_headers(version, stylesheet);
	      printf("not found\n");
	      print_html_footer(version);
	   }
	}
	return 0;
}
