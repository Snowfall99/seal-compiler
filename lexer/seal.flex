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
bool flag = false;
std::string cur_string;

char* hex2Dec (char* hex) {
  int number;
  char* res = new char[MAX_STR_CONST];
  number = std::stoi(hex, nullptr, 16);
  sprintf(res, "%d", number);
  return res;
}

%}

 /*
  * Define names for regular expressions here.
  */
HEX         0[xX][A-Fa-f0-9]+
NUMBER      (0|[1-9][0-9]*)
FLOAT       (0|[1-9][0-9]*).[0-9]+
TYPE_IDENTIFIER   (Float|Int|Bool|String|Void)
OBJ_IDENTIFIER    [a-z][a-zA-Z0-9_]*
WRONG_IDENTIFIER  [A-Z_][a-zA-Z0-9_]*

%Start COMMENT1 COMMENT2 
%Start STRING1 STRING2

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
<INITIAL>"&"        { return ('&'); }
<INITIAL>"^"        { return ('^'); }
<INITIAL>"%"        { return ('%'); }
<INITIAL>"="        { return ('='); }
<INITIAL>";"        { return (';'); }
<INITIAL>"~"        { return ('~'); }
<INITIAL>"!"        { return ('!'); }
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
<INITIAL>{WRONG_IDENTIFIER} {
                              strcpy(string_buf, "illegal TYPEID ");
                              seal_yylval.error_msg = strcat(string_buf, yytext);
                              return ERROR;
                            }
  /* comment */
<INITIAL>"//" { BEGIN COMMENT1; }
<COMMENT1>.   {}
<COMMENT1>\n  { curr_lineno++; BEGIN INITIAL; }
<INITIAL>"/*" { 
                commentLevel += 1;
                BEGIN COMMENT2;
              }
<INITIAL>"*/" { seal_yylval.error_msg = "Unmatched */"; return ERROR; }
<COMMENT2>"/*" {
                commentLevel += 1;
              }
<COMMENT2><<EOF>>  {
                    seal_yylval.error_msg = "EOF in comment constant";
                    BEGIN INITIAL;
                    return ERROR;
                  }
<COMMENT2>"*/" { 
                if (--commentLevel == 0)
                  BEGIN INITIAL; 
              }
<COMMENT2>. {}

  /* string start with " */
<INITIAL>\" { BEGIN STRING1; cur_string = ""; flag = false; }
<STRING1>\\ { char c = yyinput();
              switch(c) {
                case 't': cur_string += '\t'; break;
                case 'b': cur_string += '\b'; break;
                case 'f': cur_string += '\f'; break;
                case '0': seal_yylval.error_msg = "String contains null character '\\0'"; flag = true; return ERROR;
                case 'n': cur_string += '\n'; break;
                default: cur_string += c;
              }
            }
<STRING1>\" {
              if (cur_string.size() > MAX_STR_CONST) {
                seal_yylval.error_msg = "String is too long";
                BEGIN INITIAL;
                return ERROR;
              }
              BEGIN INITIAL;
              if (!flag) {
                seal_yylval.symbol = stringtable.add_string((char*)cur_string.c_str());
                flag = false;
                return CONST_STRING;
              }
            }
<STRING1>.  { cur_string += yytext; }
<STRING1>\\\n { ++curr_lineno; cur_string += "\n"; }
<STRING1>\n   { 
                seal_yylval.error_msg = "newline in quotation must use a '\\'";
                BEGIN INITIAL;
                ++curr_lineno;
                return ERROR;
              }
<STRING1><<EOF>>  {
                    seal_yylval.error_msg = "EOF in string constant";
                    BEGIN INITIAL;
                    yyrestart(yyin);
                    return ERROR;
                  }

  /* String start with ` */
<INITIAL>`  {
              cur_string = "";
              BEGIN STRING2;
            } 
<STRING2>\n {
              ++curr_lineno;
              cur_string += "\n";
            }
<STRING2>`  {
              if (cur_string.size() > MAX_STR_CONST) {
                seal_yylval.error_msg = "String is too long";
                BEGIN INITIAL;
                return ERROR;
              }
              BEGIN INITIAL;
              if (!flag) {
                seal_yylval.symbol = stringtable.add_string((char*)cur_string.c_str());
                flag = false;
                return CONST_STRING;
              }
            }
<STRING2>.  { cur_string += yytext; }
<STRING2><<EOF>>  {
                    seal_yylval.error_msg = "EOF in string constant";
                    BEGIN INITIAL;
                    yyrestart(yyin);
                    return ERROR;
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
