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
#include "parse_query.tab.h"
extern int yylval;
extern void yyerror(char *s);
%}

%option yylineno
%option noyywrap

FORMAT			"junit"|"testanything"|"subunit_v1"|"subunit_v2"
FMT_KEYWORD		"fmt"
SUITE_KEYWORD		"suite"
TEST_KEYWORD		"test"
PASSRATE_KEYWORD	"passrate"
CREATED_KEYWORD		"created"
HOSTNAME_KEYWORD	"hostname"
HOST			[A-Za-z][A-Za-z0-9\.\-]+
DIGIT			[0-9]
SYMBOLS			[A-Za-z0-9_\-]

%%
"="				return EQ;
">"				return GT;
">="				return GE;
"<"				return LT;
"<="				return LE;
":"				return COLON;
{FMT_KEYWORD}{1}		return FMT_KEYWORD;
{SUITE_KEYWORD}{1}		return SUITE_KEYWORD;
{TEST_KEYWORD}{1}		return TEST_KEYWORD;
{HOSTNAME_KEYWORD}{1}		return HOSTNAME_KEYWORD;
{CREATED_KEYWORD}{1}		return CREATED_KEYWORD;
{PASSRATE_KEYWORD}{1}		return PASSRATE_KEYWORD;
{FORMAT}{1}			return FORMAT;
{DIGIT}{8}			return DATE;
{DIGIT}+			return NUMBER;
{HOST}{1}			return HOST;
{SYMBOLS}+			return NAME;
\n				return NL;
[ \t]+				/* skip whitespace */;
.				{ yyerror("Unrecognized character"); }

%%
