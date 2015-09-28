%{
#include <string>
#include "node.h"
#include "parser.hpp"
#define SAVE_TOKEN yylval.strVal = new std::string(yytext, yyleng)
#define TOKEN(t) (yylval.token = t)
extern "C" int yywrap() { }
%}

%option yylineno

%%
\".*\"                  { SAVE_TOKEN; return STRING; }
"+"                     { yylval.token = 0; return ADD; }
"-"                     { yylval.token = 1; return SUB; }
"="                     { return EQL; }
">"                     { yylval.token = 0; return ISGT; }
"<"                     { yylval.token = 1; return ISLT; }
">="                    { yylval.token = 2; return ISGTE; }
"<="                    { yylval.token = 3; return ISLTE; }
"=="                    { yylval.token = 4; return ISEQ; }
"return"                { return RETURN; }
"out"                   { return OUTPUT; }
"in"                    { return INPUT; }
"if"                    { return IF; }
"dowhile"               { return DOWHILE; }
-?[0-9]+                { SAVE_TOKEN; return INT; }
[a-zA-Z_][a-zA-Z0-9_]*  { SAVE_TOKEN; return ID; }
","                     { return COMMA; }
"("                     { return OP; }
")"                     { return CP; }
"{"                     { return OB; }
"}"                     { return CB; }
[ \t\n]                 { }
.                       { std::cout << "Unknown token: " << yytext << std::endl; yyterminate(); }
%%

