%{
#include <stdio.h>
#include "parse_subunit_v1.tab.h"
int yylval;
%}

%option yylineno
%option noyywrap

%%
^(test|testing):?	return TEST;
^(success|successful):?	return SUCCESS;
^failure:		return FAILURE;
^error:			return ERROR;
^skip:?			return SKIP;
^xfail:?		return XFAIL;
^uxsuccess:?		return UXSUCCESS;
^progress:		return PROGRESS;
^tags:			return TAGS;
^time:			return TIME;
pop|push		yylval = !strcmp(yytext, "pop"); return ACTION;
"["(.|\n)*"]"		return CONTENT;
[1-9][0-9]+		yylval = atoi(yytext); return NUMBER;
[[:alnum:][:punct:]]+	return WORD;
\n			return NL;
[ \t]+			/* skip whitespace */
%%
