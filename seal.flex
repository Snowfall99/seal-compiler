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
bool flag = true;
int str;

char* hex2Dec (char* hex) {
  int number;
  char* res = new char[MAX_STR_CONST];
  number = std::stoi(hex, nullptr, 16);
  sprintf(res, "%d", number);
  return res;
}

char* getstr (const char* str) {
  char *result;
  int i = 1, j = 0;
  int len = strlen(str);
  result = new char[MAX_STR_CONST];

  while (i < len - 1) {
    if (i < len-2 && str[i] == '\\') {
      if (str[i+1] == '\n') {
        result[j] = '\n';
        i += 2; 
      } else if (str[i+1] == '\\') {
        result[j] = '\\';
        i += 2;
      } else if (str[i+1] == '0') {
        result[j] = '\\';
        i += 1;
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

%}

 /*
  * Define names for regular expressions here.
  */
HEX         0x[A-Fa-f0-9]+
NUMBER      (0|[1-9][0-9]*)
FLOAT       (0|[1-9][0-9]*).[0-9]+
TYPE_IDENTIFIER   (Float|Int|Bool|String)
OBJ_IDENTIFIER    [a-z_][a-zA-Z0-9_]*

%Start COMMENT 
%Start STRING

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
<INITIAL>"+"        { return ('+'); }
<INITIAL>"-"        { return ('-'); }
<INITIAL>"*"        { return ('*'); }
<INITIAL>"/"        { return ('/'); }
<INITIAL>"<"        { return ('<'); }
<INITIAL>">"        { return ('>'); }
<INITIAL>"|"        { return ('|'); }
<INITIAL>"%"        { return ('%'); }
<INITIAL>"="        { return ('='); }
<INITIAL>"."        { return ('.'); }
<INITIAL>";"        { return (';'); }
<INITIAL>"~"        { return ('~'); }
<INITIAL>\{         { return ('{'); }
<INITIAL>\}         { return ('}'); }
<INITIAL>\(         { return ('('); }
<INITIAL>\)         { return (')'); }
<INITIAL>":"        { return (':'); }
<INITIAL>","        { return (','); }


  /* int */
<INITIAL>{NUMBER} {
                    seal_yylval.symbol = inttable.add_string(yytext);
                    return CONST_INT;
                  }
<INITIAL>{HEX}    {
                    char* number = hex2Dec(yytext);
                    seal_yylval.symbol = inttable.add_string(number);
                    return CONST_INT;
                  }
  /* float */
<INITIAL>{FLOAT} {
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
<INITIAL>{TYPE_IDENTIFIER}  {
                              seal_yylval.symbol = idtable.add_string(yytext);
                              return TYPEID;
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
<INITIAL>\" { BEGIN STRING; str = 0; yymore(); }
<INITIAL>` { BEGIN STRING; str = 1; yymore(); }
<STRING>\\0 { 
              if (str == 0) {
                yylval.error_msg = "String contains null character '\\0'"; 
                flag = false;
              } else {
                yymore();
              } 
            }
<STRING><<EOF>>   {
                    yylval.error_msg = "EOF in string constant";
                    BEGIN INITIAL;
                    yyrestart(yyin);
                    return ERROR;
                  }
<STRING>[ ] { yymore(); }
<STRING>[^\\\"\`\n]* { yymore(); }
<STRING>\\[^\n] { yymore(); }
<STRING>\\\n { curr_lineno++; yymore(); }
<STRING>\n {
  if (str == 0) {
    seal_yylval.error_msg = "newline in quotation must use a '\\'";
    curr_lineno += 1;
    BEGIN INITIAL;
    return ERROR;
  } else {
    curr_lineno += 1;
    yymore();
  }
}
<STRING>\"  {
              if (flag == false) {
                flag = true;
                BEGIN INITIAL;
                return ERROR;
              } else {
                if (yyleng >= MAX_STR_CONST) {
                  seal_yylval.error_msg = "String constant too long";
                  BEGIN INITIAL;
                  return ERROR;
                } else {
                  char* result = getstr(yytext);
                  seal_yylval.symbol = stringtable.add_string(result);
                  BEGIN INITIAL;
                  return CONST_STRING;
                }
              }                 
            }
<STRING>`  {
              if (flag == false) {
                flag = true;
                BEGIN INITIAL;
              } else {
                if (yyleng >= MAX_STR_CONST) {
                  seal_yylval.error_msg = "String is too long.";
                  BEGIN INITIAL;
                  return ERROR;
                } else {
                  char* result = getstr(yytext);
                  seal_yylval.symbol = stringtable.add_string(result);
                  BEGIN INITIAL;
                  return CONST_STRING;
                }
              }                 
            }

  /* space */
[ \t] {}
\n    { curr_lineno++; }

  /* error */
<INITIAL>.	{
  yylval.error_msg = yytext;
  return ERROR;
}

%%
