%{
#include <stdio.h>
#include <stdlib.h>

#define YYDEBUG 1

void yyerror(char *);
int yylex(void);
%}

%token TEST SUCCESS FAILURE ERROR SKIP XFAIL UXSUCCESS
%token PROGRESS TAGS TIME ACTION CONTENT_TYPE MULTIPART NL
%token ZERO OPEN_BRACKET CLOSE_BRACKET CONTENT
%token WORD NUMBER

%%
program			: program testline NL
			| error NL { yyerrok; }
			|
			;

testline		: TEST WORD {
				printf("TEST\n");
			}
			| status WORD details {
				printf("STATUS\n");
			}
			| PROGRESS ACTION {
				printf("PROGRESS\n");
			}
			| PROGRESS NUMBER {
				printf("PROGRESS %d\n", $2);
			}
			| TAGS tags {
				printf("TAGS\n");
			}
			| TIME WORD WORD {
				printf("TIME\n");
			}
			;

details			: OPEN_BRACKET MULTIPART NL CONTENT CLOSE_BRACKET
			| OPEN_BRACKET NL string NL CLOSE_BRACKET
			|
			;

string			: WORD
			| string WORD
			|
			;

tags			: WORD
			| tags WORD
			;

status			: SUCCESS
			| FAILURE
			| ERROR
			| SKIP
			| XFAIL
			| UXSUCCESS
			;
%%

#include <ctype.h>
#include <sys/queue.h>
#include <unistd.h>

char *progname;
extern int yylex();
extern int yyparse();
extern int yylineno;
extern FILE *yyin;

struct tailq_entry {
	int tc_number;
	char tc_desc;
	char tc_comment;
	TAILQ_ENTRY(tailq_entry) entries;
};

TAILQ_HEAD(, tailq_entry) report_head;

void yyerror(char *s)
{
    fprintf( stderr, "Warning: %s, line %d\n", s, yylineno);
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

/*
  struct tailq_entry *item;
  struct tailq_entry *tmp_item;
  int i;

  TAILQ_INIT(&report_head);

  for (i = 0; i < 10; i++) {
    item = malloc(sizeof(*item));
	if (item == NULL) {
	  perror("malloc failed");
	  exit(EXIT_FAILURE);
	}

    item->tc_number = i;
    TAILQ_INSERT_TAIL(&report_head, item, entries);
  }

  TAILQ_FOREACH(item, &report_head, entries) {
	printf("%d ", item->tc_number);
  }
*/

  /* close(yyin); */
  return 0;
}
