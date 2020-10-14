 /*
  *  The scanner definition for seal.
  */
%option noyywrap
 /*
  *  Stuff enclosed in %{ %} in the first section is copied verbatim to the
  *  output, so headers and global definitions are placed here to be visible
  * to the code in the file.  Don't remove anything that was here initially
  */
%{

#include <seal-parse.h>
#include <stringtab.h>
#include <utilities.h>
#include <stdint.h>
#include <string>
#include <stdio.h>

/* The compiler assumes these identifiers. */
#define yylval seal_yylval
#define yylex  seal_yylex

/* Max size of string constants */
#define MAX_STR_CONST 256
#define YY_NO_UNPUT   /* keep g++ happy */

extern FILE *fin; /* we read from this file */

/* define YY_INPUT so we read from the FILE fin:
 * This change makes it possible to use this scanner in
 * the seal compiler.
 */
#undef YY_INPUT
#define YY_INPUT(buf,result,max_size) \
	if ( (result = fread( (char*)buf, sizeof(char), max_size, fin)) < 0) \
		YY_FATAL_ERROR( "read() in flex scanner failed");

char string_buf[MAX_STR_CONST]; /* to assemble string constants */
char *string_buf_ptr;

extern int curr_lineno;
extern int verbose_flag;

extern YYSTYPE seal_yylval;

/*
 *  Add Your own definitions here
 */
int commentLevel = 0;

char* hex2Dec (char* hex) {
  int number;
  char* res = new char[MAX_STR_CONST];
  number = std::stoi(hex, nullptr, 16);
  sprintf(res, "%d", number);
  return res;
}

char* getstr1 (const char* str) {
  char *result;
  int i, j;
  int len = strlen(str);
  if (len == 2) {
    result = "";
    return result;
  } else if (len > MAX_STR_CONST) {
    return "ERROR";
  }
  result = new char[MAX_STR_CONST];
  i = 1;
  j = 0;
  while (i < len - 1) {
    if (i < len-2 && str[i] == '\\') {
      if (str[i+1] == '\n') {
        curr_lineno += 1;
        result[j] = '\n';
        i += 2; 
      } else if (str[i+1] == 'n') {
        result[j] = '\n';
        i += 2;
      } else if (str[i+1] == '0') {
        return "null";
      } else if (str[i+1] == '\\') {
        result[j] = '\\';
        i += 2;
      } else {
        i++;
        while (i<len && str[i] != '\\') i++;
        i ++;
        j --;
      }
    } else {
      result[j] = str[i];
      i ++;
    }
    j ++;
  }
  result[j] = '\0';

  return result;
}

char *getstr2(const char* str) {
  char *result;
  int i, j;
  int len = strlen(str);
  if (len == 2) {
    result = "";
    return result;
  } else if (len > MAX_STR_CONST) {
    return "ERROR";
  }
  result = new char[MAX_STR_CONST];
  i = 1;
  j = 0;
  while (i < len - 1) {
    if (i < len-2 && str[i] == '\\') {
      if (str[i+1] == '\n') {
        curr_lineno += 1;
        result[j] = '\n';
        i += 2; 
      } else if (str[i+1] == 'n') {
        result[j] = '\n';
        i += 2;
      } else if (str[i+1] == '\\') {
        result[j] = '\\';
        i += 2;
      } else {
        result[j] = '\\';
        i++;
  
      }
    } else {
      result[j] = str[i];
      i ++;
    }
    j ++;
  }
  result[j] = '\0';

  return result;
}

%}

 /*
  * Define names for regular expressions here.
  */
HEX         0x[A-Fa-f0-9]+
DIGIT       [0-9]
NUMBER      [1-9][0-9]*
FLOAT1      [0-9].[0-9]+
FLOAT2      [1-9][0-9]+.[0-9]+
ERROR_IDENTIFIER   [A-Z_][a-zA-Z0-9_]*
OBJ_IDENTIFIER    [a-z_][a-zA-Z0-9_]*

%Start COMMENT

%%

  /*	
  *	Add Rules here. Error function has been given.
  */

  /* Operators */
<INITIAL>if         { return IF; }
<INITIAL>else       { return ELSE; }
<INITIAL>while      { return WHILE; }
<INITIAL>for        { return FOR; }
<INITIAL>break      { return BREAK; }
<INITIAL>continue   { return CONTINUE; }
<INITIAL>func       { return FUNC; }
<INITIAL>return     { return RETURN; }
<INITIAL>var        { return VAR; }
<INITIAL>struct     { return STRUCT; }
<INITIAL>"=="       { return EQUAL; }
<INITIAL>"!="       { return NE; }
<INITIAL>">="       { return GE; }
<INITIAL>"<="       { return LE; }
<INITIAL>"&&"       { return AND; }
<INITIAL>"||"       { return OR; }


  /* int */
<INITIAL>{NUMBER} {
                    seal_yylval.symbol = inttable.add_string(yytext);
                    return CONST_INT;
                  }
<INITIAL>{DIGIT}  {
                    seal_yylval.symbol = inttable.add_string(yytext);
                    return CONST_INT;
                  }
<INITIAL>{HEX}    {
                    char* number = hex2Dec(yytext);
                    seal_yylval.symbol = inttable.add_string(number);
                    return CONST_INT;
                  }
  /* float */
<INITIAL>{FLOAT1} {
                    seal_yylval.symbol = floattable.add_string(yytext);
                    return CONST_FLOAT;
                  }
<INITIAL>{FLOAT2} {
                    seal_yylval.symbol = floattable.add_string(yytext);
                    return CONST_FLOAT;
                  }
  /* boolean */
<INITIAL>true     { 
                    seal_yylval.boolean = 1;
                    return CONST_BOOL;
                  }
<INITIAL>false    {
                    seal_yylval.boolean = 0;
                    return CONST_BOOL;
                  }


  /* identifiers */
<INITIAL>{OBJ_IDENTIFIER}   {
                              seal_yylval.symbol = idtable.add_string(yytext);
                              return OBJECTID;
                            }
<INITIAL>Float              {
                              seal_yylval.symbol = idtable.add_string(yytext);
                              return TYPEID;
                            }
<INITIAL>Int                {
                              seal_yylval.symbol = idtable.add_string(yytext);
                              return TYPEID;
                            }
<INITIAL>String             {
                              seal_yylval.symbol = idtable.add_string(yytext);
                              return TYPEID;
                            }
<INITIAL>Bool               {
                              seal_yylval.symbol = idtable.add_string(yytext);
                              return TYPEID;
                            }
<INITIAL>{ERROR_IDENTIFIER}   {
                                seal_yylval.error_msg = yytext;
                                return ERROR;
                              }
  /* comment */
<INITIAL>"/*" { 
                commentLevel += 1;
                BEGIN COMMENT;
              }
<COMMENT>"/*" {
                commentLevel += 1;
              }
<COMMENT>"*/" { 
                if (--commentLevel == 0)
                  BEGIN INITIAL; 
              }
<COMMENT>. {}

  /* string */
<INITIAL>\"[a-zA-Z0-9_ \\\\\n]*\" { 
                                    char* result = getstr1(yytext);
                                    if (result == "ERROR") {
                                      seal_yylval.error_msg = yytext;
                                      return ERROR;
                                    } else if (result == "null") {
                                      seal_yylval.error_msg = "String contains null character '\\0'";
                                      return ERROR;
                                    }
                                    seal_yylval.symbol = stringtable.add_string(result);
                                    return CONST_STRING;
                                  }
<INITIAL>\`([a-z0-9A-Z_ \\\n])*\` { 
                                    char* result = getstr2(yytext);
                                    if (result == "ERROR") {
                                      seal_yylval.error_msg = yytext;
                                      return ERROR;
                                    } else if (result == "null") {
                                      seal_yylval.error_msg = "String contains null character '\\0'";
                                      return ERROR;
                                    }
                                    seal_yylval.symbol = stringtable.add_string(result);
                                    return CONST_STRING;
                                  }
                          
  /* space */
[ \t] {}
\n    { curr_lineno++; }

  /* error */
<INITIAL>@  { seal_yylval.error_msg = yytext; return ERROR; }

<INITIAL>.	{
  return yytext[0];
}



%%
