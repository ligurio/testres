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
#include <stdio.h>
#include <limits.h>	/* strtol() */
#include <stdlib.h> /* strtol() */

#include "parse_testanything.tab.h"
//#include "parse_common.h"

extern YYSTYPE yylval;
%}

%option nounput
%option noinput
%option yylineno
%option noyywrap

DIGIT			[0-9]
YAML_START		"---"
YAML_END		"..."|"---"
DOTS			".."
SKIP			[Ss][Kk][Ii][Pp]([Pp][Ee][Dd])?
TODO			"TODO"
OK				"ok"
NOT				"not"
BAILOUT			"Bail out!"
SYMBOL			[[:alpha:][:punct:]]
TAP_VERSION		^"TAP version"

%%
{OK}{1}			return OK;
{NOT}{1}		return NOT;
{BAILOUT}{1}		return BAILOUT;
{TODO}{1}		return TODO;
{SKIP}{1}		return SKIP;
{TAP_VERSION}{1}	return TAP_VERSION;
-			return DASH;
#			return HASH;
{DIGIT}{1}{DOTS}{1}{DIGIT}+	{ yylval.string = strdup(yytext); return PLAN; }
{YAML_START}{1}		return YAML_START;
{YAML_END}{1}		return YAML_END;
{DIGIT}+		{ yylval.long_val = strtol(yytext, 0, 10); return NUMBER; }
{SYMBOL}+		{ yylval.string = strdup(yytext); return WORD; };
\n			return NL;
[ \t]+			/* skip whitespace */
%%
