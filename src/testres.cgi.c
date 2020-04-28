/*
 * Copyright © 2020 Sergey Bronnikov
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

#include <string.h>
#include <parse_common.h>

#include "ui_http.h"

#define REPORTS_DIR "./reports"

struct config {
	char *cgi_action;
	char *cgi_args;
};

typedef struct config config;

int cgi_parse(char *query_string, struct config *conf);

int cgi_parse(char *query_string, struct config *conf) {
	if (query_string == NULL) {
		conf->cgi_action = (char*)"/";
		return 0;
	}
	conf->cgi_action = strtok(query_string, "=");
	conf->cgi_args = strtok(NULL, "=");
	if (conf->cgi_action == NULL) {
		return 1;
	}
	return 0;
}

int main(void) {
	print_html_headers();
	printf("Hello!\n");
	print_html_footer();

	config *conf = calloc(1, sizeof(config));
	if (conf == NULL) {
		perror("calloc");
		return 1;
	}

	char *query_string = getenv("QUERY_STRING");
	if (!cgi_parse(query_string, conf)) {
		print_html_headers();
		printf("wrong a http request: %s\n", query_string);
		print_html_footer();
		free(query_string);
		free(conf);
		return 1;
	}

	struct reportq *reports = process_dir(REPORTS_DIR);
	if (!reports) {
		print_html_headers();
		printf("no reports found\n");
		print_html_footer();
		return 0;
	}
	if (strcmp(conf->cgi_action, "/") == 0) {
		print_html_headers();
		print_html_reports(reports);
		print_html_footer();
		free(conf);
		free_reports(reports);
		return 0;
	} else if (strcmp(conf->cgi_action, "show")) {
		tailq_report *report;
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
	return 0;
}
