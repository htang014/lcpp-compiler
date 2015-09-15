%{
#include "scanAST.h"

Block *programBlock;
extern int yylex();
void yyerror(const char *s);
%}

%union{
        Node *node;
        Block *block;
        Expression *expr;
        Integer *numeric;
        Statement *stmt;
        Identifier *ident;
        VariableDeclaration *var_decl;
        std::vector<VariableDeclaration*> *varvec;
        std::vector<Expression*> *exprvec;
        std::string *strVal;
        int token;
}

%token<strVal> INT ID
%token<token> ADD SUB EQL ISGT ISLT ISGTE ISLTE ISEQ
%token<token> OP CP OB CB COMMA
%token<token> RETURN DOWHILE IF

%type<ident> ident
%type<expr> expr condition
%type<numeric> numeric
%type<varvec> func_decl_args
%type<exprvec> call_args
%type<block> program stmts block
%type<stmt> stmt var_decl func_decl return_statement do_while_statement if_statement

%left ADD SUB

%start program

%%

program : stmts { programBlock = $1; }
        ;

stmts : stmt { $$ = new Block(); $$->statements.push_back($<stmt>1); }
      | stmts stmt { $1->statements.push_back($<stmt>2); }
      ;

stmt : var_decl | func_decl | if_statement | do_while_statement | return_statement
     | expr { $$ = new ExpressionStatement(*$1); }
     ;

block : OB stmts CB { $$ = $2; }
      | OB CB { $$ = new Block(); }
      ;

var_decl : ident ident { $$ = new VariableDeclaration(*$1, *$2);}
         | ident ident EQL numeric { $$ = new VariableDeclaration(*$1, *$2, $4);}
         ;

func_decl : ident ident OP func_decl_args CP block
           { $$ = new FunctionDeclaration(*$1, *$2, *$4, *$6);
           hashPush($2->get_name(),functionDeclTable);
           delete $4;}
          ;

func_decl_args : /*nothing*/ { $$ = new VariableList(); }
               | var_decl { $$ = new VariableList(); $$->push_back($<var_decl>1); }
               | func_decl_args COMMA var_decl { $1->push_back($<var_decl>3); }
               ;

return_statement : RETURN expr { $$ = new ReturnStatement(*$2);}
                 ;

do_while_statement : DOWHILE OP condition CP block
                        { $$ = new DoWhileLoop(*$3, *$5); }
                   ;

if_statement : IF OP condition CP block { $$ = new IfStatement(*$3, *$5); }
             ;

ident : ID { $$ = new Identifier(*$1); delete $1; }
      ;

numeric : INT { $$ = new Integer(atoi($1->c_str())); delete $1; }
        ;

expr : ident EQL expr { $$ = new Assignment(*$<ident>1, *$3);}
     | ident OP call_args CP { $$ = new MethodCall(*$1, *$3); delete $3; }
     | ident { $<ident>$ = $1; }
     | numeric
     | condition
     | expr ADD expr { $$ = new BinaryOperator(*$1, $2, *$3); }
     | expr SUB expr { $$ = new BinaryOperator(*$1, $2, *$3); }
     | OP expr CP { $$ = $2; }
     ;

call_args : /*nothing*/ { $$ = new ExpressionList(); }
          | expr { $$ = new ExpressionList(); $$->push_back($1); }
          | call_args COMMA expr { $1->push_back($3); }
          ;

condition : expr ISGT expr { $$ = new Comparison( *$1, $2, *$3); }
          | expr ISGTE expr { $$ = new Comparison( *$1, $2, *$3); }
          | expr ISLT expr { $$ = new Comparison( *$1, $2, *$3); }
          | expr ISLTE expr { $$ = new Comparison( *$1, $2, *$3); }
          | expr ISEQ expr { $$ = new Comparison( *$1, $2, *$3); }
          ;
%%

int main(int argc, char **argv) {
        if (argc > 1){
                int fd = 0;
                if (-1 == (fd = open(argv[1], O_RDONLY, 0666))){
                        perror("File does not exist.");
                        return -1;
                }
                if (-1 == dup2(fd, 0))
                        perror("There was an error with dup2()");
        }


        yyparse();
        Expression_interp(programBlock);
        std::cout << ";-------------------------------\n"
                  << ";Remote Data\n"
                  << ";-------------------------------\n"
                  << ".ORIG x6000\n";

        std::cout << "FuncCallBackup" << "\t.BLKW\t#1\n";
        std::cout << "FuncCallParameters" << "\t.BLKW\t#7\n";
        std::cout << "ComparisonStorage" << "\t.BLKW\t#30\n";

        return 0;
}

void yyerror(const char* s){
        std::cerr << "error: " << s << std::endl;
}
