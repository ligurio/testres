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

%{
    #include <stdio.h>
    void yyerror(char *);
    int yylex(void);
%}

%token GT LT LE GE EQ COLON NL
%token FMT SUITE TEST HOSTNAME HOST CREATED PASSRATE
%token FORMAT DATE NAME NUMBER

/*
 * https://github.com/jgarzik/sqlfun/blob/master/sql.y
 * https://github.com/itechbear/SimpleQueryParser/blob/master/parser.y
 * https://github.com/wclever/NdYaccLexTool/tree/master/progs
 *
 */

%%

program		: program expression NL
		| error NL { yyerrok; }
		|
		;

expression	: TEST COLON NAME {
			printf("TEST\n");
		}
		| SUITE COLON NAME {
			printf("SUITE\n");
		}
		| FMT COLON FORMAT {
			printf("FMT\n");
		}
		| HOSTNAME COLON HOST {
			printf("HOSTNAME\n");
		}
		| CREATED compare_op DATE {
			printf("CREATED\n");
		}
		| PASSRATE compare_op NUMBER {
			printf("PASSRATE\n");
		}
		;

compare_op	: EQ
		| GT
		| LT
		| LE
		| GE
		;
%%

#include <ctype.h>

char *progname;
extern int yylex();
extern int yyparse();
extern int yylineno;
extern FILE *yyin;

void yyerror(char *s)
{
	fprintf(stderr, "Warning: %s, line %d\n", s, yylineno);
}

int main( int argc, char **argv ) {

  progname = argv[0];

  if (argc > 1)
  {
	yyin = fopen(argv[1], "r");
	yylineno = 0;
	if (!yyin) {
		printf("Can't open file %s\n", argv[1]);
		return -1;
	}
  }

  yyparse();

  /* close(yyin); */
  return 0;
}
