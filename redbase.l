%{
	#include "y.tab.h"
	extern void yyerror (const char *s);
	extern FlexLexer *lexer;
        extern int yywrap(void);
%}

D	[0-9]
L	[a-zA-Z_]
ID	{L}({D}|{L}){0,39}

%%

\"([^\"\n]){1,255}\"	{ return STRINGVAL; }
{D}+"."{D}+		{ return FLOATVAL; }
{D}+			{ return INTVAL; }

("i"|"f"|"c"){D}+	{ return ATTRTYPE; }

";"		{ return SEMICOLON; }
"="		{ return EQUALS; }
","		{ return COMMA; }
"("		{ return LPAREN; }
")"		{ return RPAREN; } 
"\""		{ return DBLQUOTE; } 

"reset"		{ return RESET; }
"buffer"	{ return BUFFER; }
"io"		{ return IO; }

"create"	{ return CREATE; }
"drop"		{ return DROP; 	}
"delete"	{ return DELETE; }
"from"		{ return FROM; }
"table"		{ return TABLE; }
"load"		{ return LOAD; 	}
"help"		{ return HELP; }
"print"		{ return PRINT; }
"equijoin"	{ return EQUIJOIN; }
"equijoin_opt"	{ return EQUIJOIN_OPT; }
"cross_product"	{ return CROSS_PRODUCT; }
"on"		{ return ON; }
"exit"		{ return EXIT; 	}
"\\q"            { return EXIT;  }
"select"        { return SELECT;}
("+"|"-"|"*"|"/")         { return CALCULATE;}
"!"[^\n]*	{ return SHELL; }

[ \t\n]+	{ /*eat white space*/ } 
{ID}		{ return ID; }
. 		{ return SEMICOLON; }

%%
int yyFlexLexer::yywrap()
{
    return 1;
}
/*
int main () {
	FlexLexer *lexer = new yyFlexLexer;
	while (lexer->yylex() != 0)
		;
	return 0;
}
*/
