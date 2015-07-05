%{
#include <iostream>
#include <string>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <list>
#include <algorithm>
#include <utility>
#include "node.h"

std::vector<std::pair<std::string, std::string> > varAssignList;
int val_find(unsigned fos, std::string match){
        for (unsigned i = varAssignList.size() - 1; i >= 0; i--){
                if (fos == 0 && varAssignList.at(i).first == match)
                        return i;
                else if (fos == 1 && varAssignList.at(i).second == match)
                        return i;
        }
        return -1;
}
int val_count(std::string match){
        int count = 0;
        for (unsigned i = 0; i < varAssignList.size(); i++){
                if (varAssignList.at(i).first == match)
                        count++;
        }
        return count;
}
std::vector<std::pair<std::string, std::string> > funcDeclList;
int fdl_find(unsigned fos, std::string match){
        for (unsigned i = 0; i < funcDeclList.size(); i++){
                if (fos == 0 && funcDeclList.at(i).first == match)
                        return i;
                else if (fos == 1 && funcDeclList.at(i).second == match)
                        return i;
        }
        return -1;
}
std::vector<std::pair<std::string, std::string> > funcCallList;
std::vector<int> variable_regs;
Block *programBlock;
unsigned reg = 0;
unsigned address = 3000;

extern int yylex();
void yyerror(const char *s);

void Statement_interp(Node* node);
std::string Expression_interp(Node* node);

std::string Expression_interp(Node* node){
        //std::cout << "Expression\n";

        if (node->type() == "Integer"){
                //std::cout << "Integer: " << static_cast<Integer*>(node)->value << std::endl;
                long long value = static_cast<Integer*>(node)->value;
                return std::to_string(value);
        }
        if (node->type() == "Identifier"){
                //std::cout << "Identifier: " << static_cast<Identifier*>(node)->name << std::endl;
                return static_cast<Identifier*>(node)->name;
        }
        if (node->type() == "MethodCall"){
                //std::cout << "MethodCall\n";

                const Identifier& id = static_cast<MethodCall*>(node)->id;
                std::string func_name = Expression_interp(const_cast<Identifier*>(&id));

                ExpressionList arguments = static_cast<MethodCall*>(node)->arguments;
                for (unsigned i = 0; i < arguments.size(); i++)
                        Expression_interp(arguments.at(i));

                int a = fdl_find(0, func_name);
                funcCallList.push_back(make_pair(func_name, funcDeclList.at(a).second));
                std::cout << "LD R6, " << func_name << std::endl;
                std::cout << "JSRR R6\n";

                return "MethodCall";
        }

        if (node->type() == "BinaryOperator"){
                //std::cout << "BinaryOperator: " << static_cast<BinaryOperator*>(node)->op << std::endl;

                int op = static_cast<BinaryOperator*>(node)->op;

                Expression& lhs = static_cast<BinaryOperator*>(node)->lhs;
                std::string left_value = Expression_interp(&lhs);

                Expression& rhs = static_cast<BinaryOperator*>(node)->rhs;
                std::string right_value = Expression_interp(&rhs);

                int left_index = val_find(0, left_value);
                int right_index = val_find(0, right_value);

                long long left_reg = -1;
                long long right_reg = -1;

                if (left_index != -1)
                        left_reg = variable_regs.at(left_index);
                if (right_index != -1)                        
                        right_reg = variable_regs.at(right_index);

                if (op == 0){
                        std::string output;
                        if (rhs.type() == "Integer"){
                                long long value = static_cast<Integer&>(rhs).value;
                                output = "R" + std::to_string(left_reg) + ", #" + std::to_string(value) + "\n";
                        }
                        else if (rhs.type() == "Identifier"){
                                output =  "R" + std::to_string(left_reg) + ", R" + std::to_string(right_reg) + "\n";
                        }
                        return output;
                }
                std::cerr << "Error: bad binary operator.\n";
                exit(1);
                return "";
        }

        if (node->type() == "Assignment"){
                //std::cout << "Assignment\n";

                Identifier& lhs = static_cast<Assignment*>(node)->lhs;
                std::string lhs_value = Expression_interp(&lhs);

                Expression& rhs = static_cast<Assignment*>(node)->rhs;
                std::string rhs_value = Expression_interp(&rhs);

                if (rhs.type() == "BinaryOperator"){
                        std::cout << "ADD " << rhs_value << std::endl;
                }
                else if (rhs.type() == "Integer"){
                        int left_index = val_find(0, lhs_value);
                        long long left_reg = -1;
                        if (left_index != -1)
                                left_reg = variable_regs.at(left_index);

                        long long numRepititions = val_count(lhs_value);
                        varAssignList.push_back(make_pair(lhs_value, rhs_value));
                        variable_regs.push_back(left_reg);

                        std::string new_label = lhs_value + "_" + std::to_string(numRepititions);

                        std::cout << "LD R" << left_reg << ", " << new_label << std::endl;
                        return rhs_value;
                }
                return "Assignment";
        }

        if (node->type() == "Block"){
                //std::cout << "Block\n";

                StatementList statements = static_cast<Block*>(node)->statements;
                for (unsigned i = 0; i < statements.size(); i++){
                        Statement_interp(statements.at(i));
                }
                return "Block";
        }
        return "Expression";

}

void Statement_interp(Node* node){
        //std::cout << "Statement\n";

        if (node->type() == "ExpressionStatement"){
                //std::cout << "ExpressionStatement\n";

                Expression& expression = static_cast<ExpressionStatement*>(node)->expression;
                Expression_interp(&expression);
        }
        if (node->type() == "VariableDeclaration"){
                //std::cout << "VariableDeclaration\n";

                const Identifier& var_type = static_cast<VariableDeclaration*>(node)->var_type;
                std::string type_name = Expression_interp(&(const_cast<Identifier&>(var_type)));

                Identifier& id = static_cast<VariableDeclaration*>(node)->id;
                std::string var_name = Expression_interp(&id);

                Expression *assignmentExpr = static_cast<VariableDeclaration*>(node)->assignmentExpr;
                std::string assignment_value = Expression_interp(assignmentExpr);

                std::cout << "LD R" << reg << ", " << var_name << std::endl;
                variable_regs.push_back(reg);
                reg++;
                varAssignList.push_back(make_pair(var_name, assignment_value));
        }
        if (node->type() == "FunctionDeclaration"){
                //std::cout << "FunctionDeclaration\n";
                reg = 0;
                varAssignList.clear();

                const Identifier& func_type = static_cast<FunctionDeclaration*>(node)->func_type;
                std::string type_name = Expression_interp(&(const_cast<Identifier&>(func_type)));

                const Identifier& id = static_cast<FunctionDeclaration*>(node)->id;
                std::string func_name = Expression_interp(&(const_cast<Identifier&>(id)));

                VariableList arguments = static_cast<FunctionDeclaration*>(node)->arguments;
                for (unsigned i = 0; i < arguments.size(); i++)
                        Expression_interp(arguments.at(i));

                std::cout << ";--------------------------------------\n";
                std::cout << ";Routine: " << func_name << std::endl;
                std::cout << ";--------------------------------------\n";

                std::cout << ".ORIG x" << address << std::endl;
                address += 200;

                Block& block = static_cast<FunctionDeclaration*>(node)->block;
                Expression_interp(&block);

                std::cout << ";------------\n";
                std::cout << ";Routine Data\n";
                std::cout << ";------------\n";

                for (unsigned i = 0; i < varAssignList.size(); i++){
                        long long count = 0;
                        std::string new_label = varAssignList.at(i).first;
                        std::vector<std::string> existing_labels;

                        while (std::find(existing_labels.begin(), existing_labels.end(), new_label) != existing_labels.end()){
                                new_label = varAssignList.at(i).first + "_" + std::to_string(count);
                        }
                        existing_labels.push_back(new_label);

                        std::cout << new_label << "\t.FILL\t"
                                  << varAssignList.at(i).second << std::endl;
                }

                std::cout << ";--------------------------------------\n";
                std::cout << ";End of routine\n";
                std::cout << ";--------------------------------------\n";

        }
}
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
%token<token> ADD SUB EQL
%token<token> OP CP OB CB COMMA

%type<ident> ident
%type<expr> expr
%type<numeric> numeric
%type<varvec> func_decl_args
%type<exprvec> call_args
%type<block> program stmts block
%type<stmt> stmt var_decl func_decl

%left ADD SUB

%start program

%%

program : stmts { programBlock = $1; }
        ;

stmts : stmt { $$ = new Block(); $$->statements.push_back($<stmt>1); }
      | stmts stmt { $1->statements.push_back($<stmt>2); }
      ;

stmt : var_decl | func_decl
     | expr { $$ = new ExpressionStatement(*$1); }
     ;

block : OB stmts CB { $$ = $2; }
      | OB CB { $$ = new Block(); }
      ;

var_decl : ident ident { $$ = new VariableDeclaration(*$1, *$2);}
         | ident ident EQL numeric { $$ = new VariableDeclaration(*$1, *$2, $4);}
         ;

func_decl : ident ident OP func_decl_args CP block
           { $$ = new FunctionDeclaration(*$1, *$2, *$4, *$6); delete $4;}
          ;

func_decl_args : /*nothing*/ { $$ = new VariableList(); }
               | var_decl { $$ = new VariableList(); $$->push_back($<var_decl>1); }
               | func_decl_args COMMA var_decl { $1->push_back($<var_decl>3); }
               ;

ident : ID { $$ = new Identifier(*$1); delete $1; }
      ;

numeric : INT { $$ = new Integer(atoi($1->c_str())); delete $1; }
        ;

expr : ident EQL numeric { $$ = new Assignment(*$<ident>1, *$3);}
     | ident OP call_args CP { $$ = new MethodCall(*$1, *$3); delete $3; }
     | ident { $<ident>$ = $1; }
     | numeric
     | expr ADD expr { $$ = new BinaryOperator(*$1, $2, *$3); }
     | expr SUB expr { $$ = new BinaryOperator(*$1, $2, *$3); }
     | OP expr CP { $$ = $2; }
     ;

call_args : /*nothing*/ { $$ = new ExpressionList(); }
          | expr { $$ = new ExpressionList(); $$->push_back($1); }
          | call_args COMMA expr { $1->push_back($3); }
          ;

%%

int main() {
        yyparse();

        Expression_interp(programBlock);

        return 0;
}

void yyerror(const char* s){
        std::cerr << "error: " << s << std::endl;
}
