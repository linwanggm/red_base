%token LPAREN RPAREN SEMICOLON COMMA DBLQUOTE SHELL USE
%token CREATE DROP DATABASE TABLE DELETE FROM 
%token CROSS_PRODUCT EQUIJOIN EQUIJOIN_OPT ON EQUALS
%token LOAD HELP PRINT EXIT
%token INTVAL FLOATVAL STRINGVAL ID ATTRTYPE
%token RESET IO BUFFER
%token SELECT CALCULATE

%{
	/* All kinds of ugly hacks follow :-). These are necessary because
	 * we use flex to generate C++ output, not C output. The C++
	 * output is re-entrant and therefore, has no memory leaks. The C 
	 * output is not. */
	#include <FlexLexer.h>
	#include <string.h>
	#include <iostream>
	using namespace std;
	class yyFlexLexer;
	FlexLexer *lexer = NULL;
	#define YYSTYPE int
	#define yylex lexer->yylex
	YYSTYPE yyparse ();
	void    yyerror (const char *s);
	istream *yyin;
	/* end of uh */


	/* Include all necessary header files */
	#include "PF_Manager.h"
	#include "RM_Manager.h"
	#include "SM_Manager.h"

	PF_Manager pfm;
	RM_Manager rmm (pfm);
	SM_Manager smm (rmm);

	/* These 2 variables are used when a CREATE TABLE command is invoked. */
	AttrInfo attrInfo[MAXATTRS]; /* Stores information about attributes
					of the relation being created. */
	int nAttrs = 0;	/* Number of attributes in relation. */

	/* Prototypes for functions defined below. These fuctions in turn
	 * call functions in SM_Manager.*/
	RC AddToAttrInfo (const char *attrName, const char *cAttrType);
	void CreateTable (const char *tableName, AttrInfo *attrInfo, int nAttrs);
	void DropTable (const char *tableName);
	void DeleteRecords (const char *tableName);
	void Load (const char *relName, const char *fileName);
	void Print (const char *relName);
	void EquiJoin (const char *rel1, const char *rel2);
	void EquiJoin_Opt (const char *rel1, const char *rel2);

	/* Auxillary functions */
	void PrintAttrInfo ();

	/* Auxillary variables */
	char id0[MAXNAME+1];
	char id1[MAXNAME+1];
	char id2[MAXNAME+1];
	char shell_cmd[1024];
	/* Auxillary variables for storing a condition list in an equijoin
	 * query */
	int  intVal1, intVal2;
	int  attrList1[MAXATTRS],
	     attrList2[MAXATTRS];
	int  condCount = 0;

	/* As long as this variable is false, the parser accepts commands
	 * from the command line. */
	bool stopParser = false;

	/* Defines the colorful redbase prompt */
	#define PROMPT "\n[31mredbase>[39;49m"
%}

%%

input: 		command 
		{
			YYACCEPT;
		}

command:	CREATE TABLE ID {strcpy(id0,lexer->YYText());} LPAREN attr_type_list RPAREN SEMICOLON 
		{
			CreateTable (id0);
			/* Reset attrInfo stack */
			nAttrs = 0;
		}
        |
                SELECT INTVAL {strcpy(id0,lexer->YYText());} CALCULATE { strcpy(id1,lexer->YYText());} INTVAL {strcpy(id2,lexer->YYText());} SEMICOLON
                {
                      int n1 = atoi(id0);
                      char cal=id1[0];
                      int n3 = atof(id2);
                      switch(cal)
                      {
                          case '+':
                                   printf("%d", n1+n3);break;
                          case '-':
                                   printf("%d", n1-n3);break;
                          case '*':
                                   printf("%d", n1*n3);break;
                          case '/':
                                   printf("%f", n1/float(n3));break;
                          defalut: printf("error.\n");
                      }
                }
        |
		DROP TABLE ID {strcpy(id0,lexer->YYText());} SEMICOLON
			{ DropTable (id0); }
		|
		DELETE FROM ID {strcpy(id0,lexer->YYText());} SEMICOLON
			{ DeleteRecords (id0); }
		|
		LOAD ID {strcpy(id0,lexer->YYText());} 
		LPAREN STRINGVAL {strcpy(id1,lexer->YYText());} RPAREN SEMICOLON
			{ Load (id0, id1); }
		|
		HELP SEMICOLON
			{ Help (NULL); }
		|
		HELP ID {strcpy(id0,lexer->YYText());} SEMICOLON
			{ Help (id0); }
		|
		PRINT ID {strcpy(id0,lexer->YYText());} SEMICOLON
			{ Print (id0); }
		| 
		CROSS_PRODUCT ID {strcpy(id0,lexer->YYText());} COMMA ID {strcpy(id1,lexer->YYText());}
		{ 
			CrossProduct (id0,id1); 
		}
		|
		EQUIJOIN ID {strcpy(id0,lexer->YYText());} COMMA ID {strcpy(id1,lexer->YYText());}
			ON cond_list SEMICOLON
		{ 
			EquiJoin (id0,id1); 
			condCount = 0;
		}
		|
		EQUIJOIN_OPT ID {strcpy(id0,lexer->YYText());} COMMA ID {strcpy(id1,lexer->YYText());}
			ON cond_list SEMICOLON
		{
			EquiJoin_Opt (id0,id1); 
			condCount = 0;
		}
		|
		SHELL {strcpy(shell_cmd,lexer->YYText());} 
		{ 
			int ret = system (&shell_cmd[1]);
			printf ("Command returned %d\n", ret);
		}
		|
		EXIT  
			{ stopParser = true; }
		|
		PRINT IO SEMICOLON { bfm.PrintIOStat(); }
		|
		RESET IO SEMICOLON { bfm.ResetIOStat(); }
		|
		PRINT BUFFER SEMICOLON { bfm.PrintBuffer(); }
		|
		RESET BUFFER SEMICOLON { bfm.ResetBuffer(); }

attr_type_list:	attr_type COMMA attr_type_list
		| attr_type ;

attr_type:	ID {strcpy(id1,lexer->YYText());} ATTRTYPE
		{
			strcpy(id2,lexer->YYText()); /*Stores attr type */

			RC rc;
			/* Add attr info to list */
			rc = AddToAttrInfo (id1, id2);
			/* Check if an error has occured */
			if (rc != SUCCESS) {
				PrintError (rc);
				YYREJECT;
			}
		}
		;

cond_list:	cond COMMA cond_list
		| cond;

cond:		INTVAL { intVal1 = atoi(lexer->YYText()); }
		EQUALS INTVAL 
		{ 
			intVal2 = atoi(lexer->YYText()); 
			/* Check for errors */
			if (condCount+1 == MAXATTRS) {
				fprintf (stderr, "Too many conditions in query\n");
				YYREJECT;
			}
			/* Add condition to list of conditions */
			attrList1[condCount] = intVal1;
			attrList2[condCount] = intVal2;
			condCount++;
		}
		;
%%




/////////////////////////////////////////////////////////////////////////////

RC AddToAttrInfo (const char *attrName, const char *cAttrType) {
	/* Check if number of attributes in table has reached its limit */
	if (nAttrs == MAXATTRS) {
		return PR_TOOMANYATTRS;
	}


	AttrType attrType;
	int attrLength;
	/* Determine attr type and length */
	switch (cAttrType[0]) {
		case 'i':
			attrLength = 4;
			attrType = TYPE_INT;
			break;
		case 'f':
			attrLength = 4;
			attrType = TYPE_FLOAT;
			break;
		case 'c':
			attrLength = strtol(&cAttrType[1], (char **)NULL, 10);
			if (attrLength < 1 || attrLength > MAXSTRINGLEN) {
				return PR_INVALIDATTRLEN;
			}
			attrType = TYPE_STRING;
			break;
	}

	/* Store new attribute's info */
	strcpy (attrInfo[nAttrs].attrName, attrName);
	attrInfo[nAttrs].attrType = attrType;
	attrInfo[nAttrs].attrLength = attrLength;
	nAttrs++;

	return SUCCESS;
}

void CreateTable (const char *tableName) {
	RC rc;
	if ((rc = smm.CreateTable(tableName, nAttrs, attrInfo)) != SUCCESS)
		{ PrintError(rc); }
	else
		{ printf ("Table created\n"); }
}

void DropTable (const char *tableName) {
	RC rc;
	if ((rc = smm.DropTable(tableName)) != SUCCESS) 
		{ PrintError(rc); }
	else 
		{ printf ("Table dropped\n"); }
}

void DeleteRecords (const char *tableName) {
	RC rc;
	if ((rc = smm.DeleteRecords(tableName)) != SUCCESS) 
		{ PrintError(rc); }
	else 
		{ printf ("All records deleted\n"); }
}

void Load (const char *relName, const char *fileName) {
	char fname[256];
	char *f = fname;
	strcpy (f, fileName);
	/* Remove leading and trailing quote */
	f[strlen(fname)-1] = '\0';
	f = &f[1];
	/* Load data */
	RC rc;
	if ((rc = smm.Load(relName, f)) != SUCCESS) 
		{ PrintError(rc); }
	else 
		{ printf ("Data loaded into table\n"); }
}

void Help (const char *relName) {
	RC rc;
	if (relName == NULL) 
		{ rc = smm.Help (); }
	else
		{ rc = smm.Help (relName); }
	if (rc != SUCCESS) 
		{ PrintError(rc); }
}

void Print (const char *relName) {
	RC rc;
	if ((rc = smm.Print(relName)) != SUCCESS) 
		{ PrintError(rc); }
}

void CrossProduct (const char *rel1, const char *rel2) {
	RC rc;
	rc = smm.CrossProduct (rel1, rel2);
	if (rc != SUCCESS) {
		PrintError (rc);
	}
}

void EquiJoin (const char *rel1, const char *rel2) {
	RC rc;
	if ((rc = smm.EquiJoin(rel1, rel2,
				attrList1, attrList2,
				condCount)) != SUCCESS) 
	{
		PrintError (rc);
	}
}

void EquiJoin_Opt (const char *rel1, const char *rel2) {
	RC rc;
	if ((rc = smm.EquiJoin_Opt(rel1, rel2,
				attrList1, attrList2,
				condCount)) != SUCCESS) 
	{
		PrintError (rc);
	}
}

void PrintAttrInfo () {
	printf ("Attribute information:\n");
	printf ("----------------------\n");
	for(int i = 0; i < nAttrs; i++) {
		printf ("(%d) %s,%d,%d\n", i, attrInfo[i].attrName,
				attrInfo[i].attrType, attrInfo[i].attrLength);
	}
}

/////////////////////////////////////////////////////////////////////////////

/* Prints error messages from lexer */
void yyerror (const char *s) {
	fprintf (stderr, "%s", s);
}

#define DBNAME "db.redbase"
int main (int argc, char *argv[]) {
	if (argc < 2) {
		printf ("Usage: %s <database-name>\n", argv[0]);
		return 0;
	}

	RC rc;
	/* Open the database */
	rc = smm.OpenDb (argv[1]);
	if (rc != SUCCESS) {
		PrintError (rc);
		return rc;
	}

	/* Parse and execute commands */
	while (stopParser == false) {
		lexer = new yyFlexLexer;
		printf (PROMPT);
		yyparse ();
		delete lexer;
	}

	/* Free up memory alloc'd by yacc. */
	{
		if (yyss) free(yyss);
		if (yyvs) free(yyvs);
	}

	/* Close the databse */
	rc = smm.CloseDb ();
	if (rc != SUCCESS) {
		PrintError (rc);
		return rc;
	}

	return 0;
}
