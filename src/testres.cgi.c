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

void  cgi_parse(char *query_string, struct config *conf);

void cgi_parse(char *query_string, struct config *conf) {
	conf->cgi_action = NULL;
	conf->cgi_args = NULL;
	conf->cgi_action = strtok(query_string, "=");
	if (conf->cgi_action)
		conf->cgi_args = strtok(NULL, "=");
}

int main(void) {
	config *conf = calloc(1, sizeof(config));
	if (!conf) {
		print_html_headers();
		printf("calloc\n");
		print_html_footer();
		return 1;
	}

	struct reportq *reports = process_dir(REPORTS_DIR);
	if (!reports) {
		print_html_headers();
		printf("no reports found\n");
		print_html_footer();
		return 1;
	}

	char *query_string = getenv("QUERY_STRING");
	cgi_parse(query_string, conf);

	print_html_headers();
	if (!(conf->cgi_action && conf->cgi_args)) {
		print_html_reports(reports);
	} else {
		if (!strcmp(conf->cgi_action, "show")) {
			tailq_report *report;
			if ((report = is_report_exists(reports, conf->cgi_args))) {
				print_html_report(report);
				free_report(report);
			}
		} else if (!strcmp(conf->cgi_action, "q")) {
			struct reportq *filtered = NULL;
			filtered = filter_reports(reports, conf->cgi_args);
			print_html_reports(filtered);
			free_reports(filtered);
		} else {
			print_html_reports(reports);
		}
	}
	print_html_footer();
	free(conf);
	free_reports(reports);
	return 0;
}
