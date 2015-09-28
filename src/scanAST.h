#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <cmath>
#include <utility>
#include <unistd.h>
#include <fcntl.h>
#include "node.h"
#include "hash.h"

//macros for hash tables
int CURRENTRETURNVALUE;
#define currentFunctionPosInHash hashLookup(currentStatus.function,functionDeclTable)
#define calledFunctionPosInHash hashLookup(calledFunctionName, functionDeclTable)
#define variablePosInHash(x) hashLookup(x, variableAssignTable)

void Statement_interp(Node* node);
std::string Expression_interp(Node* node);

std::string Expression_interp(Node* node){
        if (node->type() == INTEGER){
                return std::to_string(static_cast<Integer*>(node)->value);
        }
        if (node->type() == IDENTIFIER){
                return node->get_name();
        }
        if (node->type() == LCSTRING){
                std::string str = static_cast<LCString*>(node)->str;
                if (!str.empty()){
                        str.pop_back();
                        str.erase(str.begin());
                }
                return str;
        }
        if (node->type() == METHODCALL){
                const Identifier& id = static_cast<MethodCall*>(node)->id;
                std::string calledFunctionName = Expression_interp(const_cast<Identifier*>(&id));

                std::cout << "LD R7, FuncCallParameters_" << functionDeclTable.at(currentFunctionPosInHash)->get_address() << std::endl;

                ExpressionList& arguments = static_cast<MethodCall*>(node)->arguments;
                for (unsigned i = 0; i < arguments.size(); i++){
                        Expression_interp(arguments.at(i));

                        //The register # holding the value of the specified variable
                        int variableRegister = variableAssignTable.at(variablePosInHash(arguments.at(i)->get_name()))->get_reg();

                        std::cout << "STR R" << variableRegister << ", R7, #" << i << std::endl;
                }

                functionDeclTable.at(calledFunctionPosInHash)->increment_invocation();

                if (-1 == calledFunctionPosInHash){
                        std::cerr << "Function does not exist\n";
                        exit(1);
                }
 
                //outputs
                std::cout << "STI R7, FuncCallBackupAddr_" << functionDeclTable.at(currentFunctionPosInHash)->get_address() << std::endl;
                std::cout << "LD R7, " << calledFunctionName << '_' << functionDeclTable.at(currentFunctionPosInHash)->get_address()  << std::endl;
                std::cout << "JSRR R7\n";

                return "MethodCall";
        }
        if (node->type() == BINARYOPERATOR){
                int op = static_cast<BinaryOperator*>(node)->op;

                Expression& lhs = static_cast<BinaryOperator*>(node)->lhs;
                int left_index = hashLookup(Expression_interp(&lhs), variableAssignTable);

                Expression& rhs = static_cast<BinaryOperator*>(node)->rhs;
                int right_index = hashLookup(Expression_interp(&rhs), variableAssignTable);

                int left_reg = -1;
                int right_reg = -1;

                if (left_index == -1){
                        std::cerr << "Variable does not exist\n";
                        exit(1);
                }

                left_reg = variableAssignTable.at(left_index)->get_reg();
                if (right_index != -1)
                        right_reg = variableAssignTable.at(right_index)->get_reg();

                if (op == 0){
                        std::string output;
                        if (rhs.type() == INTEGER && std::abs(atoi(Expression_interp(&rhs).c_str())) <= 15){
                                int value = static_cast<Integer&>(rhs).value;
                                output = "R" + std::to_string(left_reg) + ", #" + std::to_string(value) + "\n";
                        }
                        else if (rhs.type() == INTEGER){
                                int value = static_cast<Integer&>(rhs).value;
                                variableAssignTable.at(variablePositions.at(0))->increment_invocation();
                                variableAssignTable.at(variablePositions.at(0))->set_value(atoi(Expression_interp(&rhs).c_str()));

                                std::cout << "LD R7, " << variableAssignTable.at(variablePositions.at(0))->get_name( ) << '_'
                                          << variableAssignTable.at(variablePositions.at(0))->get_times_invoked() << '_' 
                                          << functionDeclTable.at(hashLookup(currentStatus.function,functionDeclTable))->get_address()
                                          << std::endl;
                                output = "R" + std::to_string(left_reg) + ", R7" + "\n";
                        }
                        else if (rhs.type() == IDENTIFIER){
                                if (right_index == -1){
                                        std::cerr << "Variable does not exist\n";
                                        exit(1);
                                }
                                output =  "R" + std::to_string(left_reg) + ", R" + std::to_string(right_reg) + "\n";
                        }
                        else {
                                std::cerr << "Right-hand side must be integer or variable\n";
                                exit(1);
                        }
                        return output;
                }
                std::cerr << "Error: bad binary operator.\n";
                exit(1);
                return "";
        }

        if (node->type() == ASSIGNMENT){
                /*std::cout << "ST R7, General_R7_Backup_"
                          << functionDeclTable.at(hashLookup(currentStatus.function,functionDeclTable))->get_address() << "\n";*/

                Identifier& lhs = static_cast<Assignment*>(node)->lhs;
                std::string lhs_value = Expression_interp(&lhs);

                Expression& rhs = static_cast<Assignment*>(node)->rhs;
                std::string rhs_value = Expression_interp(&rhs);

                if (lhs_value == "int" || rhs_value == "int"){
                        std::cerr << "Error: Type \"int\" cannot be argument.\n";
                        exit(1);
                }

                if (rhs.type() == IDENTIFIER){
                        int leftReg;
                        int rightReg;
                        if (hashLookup(lhs_value,variableAssignTable) != -1)
                                leftReg = variableAssignTable.at(hashLookup(lhs_value,variableAssignTable))->get_reg();
                        else {
                                std::cerr << "Variable defined but never declared.\n";
                                exit(1);
                        }

                        if (hashLookup(rhs_value,variableAssignTable) != -1)
                                rightReg = variableAssignTable.at(hashLookup(rhs_value,variableAssignTable))->get_reg();
                        else {
                                std::cerr << "Variable defined but never declared.\n";
                                exit(1);
                        }
                        std::cout << "ADD R" << leftReg << ", R" << rightReg << ", #0" << std::endl;
                }
                else if (rhs.type() == BINARYOPERATOR){
                        int baseReg;
                        if (hashLookup(lhs_value,variableAssignTable) != -1)
                                baseReg = variableAssignTable.at(hashLookup(lhs_value,variableAssignTable))->get_reg();
                        else {
                                std::cerr << "Variable defined but never declared.\n";
                                exit(1);
                        }
                        std::cout << "ADD R" << baseReg << ", " << rhs_value << std::endl;
                        /*std::cout << "LD R7, General_R7_Backup_"
                                  << functionDeclTable.at(hashLookup(currentStatus.function,functionDeclTable))->get_address() << "\n";*/
                }
                else if (rhs.type() == INTEGER){
                        int left_index = hashLookup(lhs_value,variableAssignTable);
                        int left_reg = -1;
                        if (left_index != -1)
                                left_reg = variableAssignTable.at(left_index)->get_reg();
                        else {
                                std::cerr << "Variable defined but never declared.\n";
                                exit(1);
                        }
                        variableAssignTable.at(left_index)->increment_invocation();
                        int numRepititions = variableAssignTable.at(left_index)->get_times_invoked();
                        variableAssignTable.at(left_index)->set_value(atoi(rhs_value.c_str()));

                        std::string new_label = lhs_value + "_" + std::to_string(numRepititions);

                        std::cout << "LD R" << left_reg << ", " << lhs_value << '_'
                                  << std::to_string(numRepititions) << '_'
                                  << functionDeclTable.at(hashLookup(currentStatus.function,functionDeclTable))->get_address()  << std::endl;
                        return rhs_value;
                }
                else if (rhs.type() == METHODCALL){
                        int left_index = hashLookup(lhs_value,variableAssignTable);
                        int left_reg = -1;
                        if (left_index != -1)
                                left_reg = variableAssignTable.at(left_index)->get_reg();
                        else {
                                std::cerr << "Variable defined but never declared.\n";
                                exit(1);
                        }
                        variableAssignTable.at(left_index)->increment_invocation();
                        int numRepititions = variableAssignTable.at(left_index)->get_times_invoked();

                        variableAssignTable.at(left_index)->set_value(CURRENTRETURNVALUE);
                        std::string new_label = lhs_value + "_" + std::to_string(numRepititions);

                        std::cout << "LDI R" << left_reg << ", FunctionReturn_" << functionDeclTable.at(currentFunctionPosInHash)->get_address() << std::endl;
                }
                return "Assignment";
        }
        if (node->type() == COMPARISON){
                //std::cout << "Condition\n";
                Expression& lhs = static_cast<Comparison*>(node)->lhs;
                std::string lhs_value = Expression_interp(&lhs);

                Expression& rhs = static_cast<Comparison*>(node)->rhs;
                std::string rhs_value = Expression_interp(&rhs);

                std::cout << "LD R7, Compare_" << currentStatus.comparisons.size() << "_" << functionDeclTable.at(currentFunctionPosInHash)->get_address() << std::endl;

                if (rhs.type() == INTEGER){
                        currentStatus.comparisons.push_back(atoi(rhs_value.c_str()));
                }
                else if (rhs.type() == IDENTIFIER){
                        std::string name = static_cast<Identifier&>(rhs).get_name();
                        currentStatus.comparisons.push_back(variableAssignTable.at(variablePosInHash(name))->get_value());
                }

                return (lhs_value + "|" + rhs_value);

        }
        if (node->type() == BLOCK){
                //std::cout << "Block\n";
                variableTableBackup.push(variableAssignTable);
                variablePositionsBackup.push(variablePositions);
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

        if (node->type() == EXPRESSIONSTATEMENT){
                //std::cout << "ExpressionStatement\n";

                Expression& expression = static_cast<ExpressionStatement*>(node)->expression;
                Expression_interp(&expression);
        }
        if (node->type() == VARIABLEDECLARATION){
                //std::cout << "VariableDeclaration\n";

                const Identifier& variableType = static_cast<VariableDeclaration*>(node)->variableType;
                std::string type_name = Expression_interp(&(const_cast<Identifier&>(variableType)));

                if (type_name != "int"){
                        std::cerr << "Error: invalid type identifier\n";
                        exit(1);
                }

                Identifier& id = static_cast<VariableDeclaration*>(node)->id;
                std::string var_name = Expression_interp(&id);

                if (var_name == "int"){
                        std::cerr << "Error: type \"int\" cannot be variable name.\n";
                        exit(1);
                }

                if (static_cast<VariableDeclaration*>(node)->assignmentExpr == NULL){
                        static_cast<VariableDeclaration*>(node)->assignmentExpr = new Integer(0);

                }

                Expression *assignmentExpr = static_cast<VariableDeclaration*>(node)->assignmentExpr;
                std::string assignment_value = Expression_interp(assignmentExpr);

                if (!currentStatus.variableIsArgument){
                        std::cout << "LD R" << currentStatus.reg << ", " << var_name << '_'
                                            << functionDeclTable.at(hashLookup(currentStatus.function,functionDeclTable))->get_address()
                                            << std::endl;
                }
                else {
                        currentStatus.variableIsArgument = 0;
                }

                hashPush(var_name, variableAssignTable, atoi(assignment_value.c_str()));
                if (currentStatus.reg > 7){
                        std::cerr << "Error: Too many variable declarations.\n";
                        exit(1);
                }
        }
        if (node->type() == OUTSTATEMENT){
                LCString& message = static_cast<OutStatement*>(node)->message;
                std::string message_str = Expression_interp(&message);

                Identifier& variable = static_cast<OutStatement*>(node)->variable;
                std::string variableName = Expression_interp(&variable);

                if (message_str == "\n"){
                        std::cout << "ST R0, General_Backup_" << functionDeclTable.at(currentFunctionPosInHash)->get_address() << std::endl
                                  << "LD R0, newline_" << functionDeclTable.at(currentFunctionPosInHash)->get_address() << "\nOUT\n"
                                  << "LD R0, General_Backup_" << functionDeclTable.at(currentFunctionPosInHash)->get_address() << std::endl;
                }
                else if (message_str != ""){
                        std::cout << "ST R0, General_Backup_" << functionDeclTable.at(currentFunctionPosInHash)->get_address() << std::endl
                                  << "LEA R0, String_" << currentStatus.strings.size() << "_" << functionDeclTable.at(currentFunctionPosInHash)->get_address() << std::endl
                                  << "PUTS\n"
                                  << "LD R0, General_Backup_" << functionDeclTable.at(currentFunctionPosInHash)->get_address() << std::endl;

                        currentStatus.strings.push_back(message_str);
                }
                else if (variableName != ""){
                        std::cout << "ST R0, General_Backup_" << functionDeclTable.at(currentFunctionPosInHash)->get_address() << std::endl
                                  << "ADD R0, R" << variableAssignTable.at(variablePosInHash(variableName))->get_reg() << ", #0\n"
                                  << "OUT\n"
                                  << "LD R0, General_Backup_" << functionDeclTable.at(currentFunctionPosInHash)->get_address() << std::endl;

                        currentStatus.ascii.push_back(variableAssignTable.at(variablePosInHash(variableName))->get_value());
                }
        }
        if (node->type() == DOWHILELOOP){
                Expression& condition = static_cast<DoWhileLoop*>(node)->condition;
                std::string condition_str = Expression_interp(&condition);

                unsigned tempNum = currentStatus.doWhileLoop;
                currentStatus.doWhileLoop++;

                int dividerLoc = condition_str.find("|");
                int var_pos = hashLookup(condition_str.substr(0, dividerLoc),variableAssignTable);
                int var_reg;
                if (var_pos != -1)
                        var_reg = variableAssignTable.at(var_pos)->get_reg();
                else {
                        std::cerr << "Error: variable does not exist\n";
                        exit(1);
                }

                std::cout << "NOT R7, R7\n" << "ADD R7, R7, #1\n";
                std::cout << "ADD R" << var_reg << ", R" << var_reg << ", R7" << std::endl;
                std::cout << "LOOP_STATEMENT_" << tempNum << "\n\n";

                std::cout << "NOT R7, R7\n" << "ADD R7, R7, #1\n";
                std::cout << "ADD R" << var_reg << ", R" << var_reg << ", R7" << std::endl;
                Block& action = static_cast<DoWhileLoop*>(node)->action;
                Expression_interp(&action);

                std::cout << "NOT R7, R7\n" << "ADD R7, R7, #1\n";
                std::cout << "ADD R" << var_reg << ", R" << var_reg << ", R7" << std::endl;
                if (static_cast<Comparison&>(condition).op == 0) {
                        std::cout << "BRp LOOP_STATEMENT_" << tempNum << std::endl;
                }
                else if (static_cast<Comparison&>(condition).op == 1) {
                        std::cout << "BRn LOOP_STATEMENT_" << tempNum << std::endl;
                }
                else if (static_cast<Comparison&>(condition).op == 2) {
                        std::cout << "BRzp LOOP_STATEMENT_" << tempNum << std::endl;
                }
                else if (static_cast<Comparison&>(condition).op == 3) {
                        std::cout << "BRnz LOOP_STATEMENT_" << tempNum << std::endl;
                }
                else if (static_cast<Comparison&>(condition).op == 4) {
                        std::cout << "BRz LOOP_STATEMENT_" << tempNum << std::endl;
                }
                std::cout << "NOT R7, R7\n" << "ADD R7, R7, #1\n";
                std::cout << "ADD R" << var_reg << ", R" << var_reg << ", R7" << std::endl;

        }
        if (node->type() == IFSTATEMENT){
                Expression& condition = static_cast<IfStatement*>(node)->condition;
                std::string condition_str = Expression_interp(&condition);

                unsigned tempNum = currentStatus.ifStatement;
                currentStatus.ifStatement++;

                int dividerLoc = condition_str.find("|");
                int num = atoi((condition_str.substr(dividerLoc + 1, condition_str.size() - dividerLoc - 1)).c_str());
                int var_pos = hashLookup(condition_str.substr(0, dividerLoc),variableAssignTable);
                int var_reg;
                if (var_pos != -1)
                        var_reg = variableAssignTable.at(var_pos)->get_reg();
                else {
                        std::cerr << "Error: variable does not exist\n";
                        exit(1);
                }

                std::cout << "NOT R7, R7\n" << "ADD R7, R7, #1\n";
                std::cout << "ADD R" << var_reg << ", R" << var_reg << ", R7" << std::endl;
                if(static_cast<Comparison&>(condition).op == 0) {
                        std::cout << "BRnz IF_STATEMENT_SKIP_" << tempNum << std::endl;
                }
                else if (static_cast<Comparison&>(condition).op == 1) {
                        std::cout << "BRzp IF_STATEMENT_SKIP_" << tempNum << std::endl;
                }
                else if (static_cast<Comparison&>(condition).op == 2) {
                        std::cout << "BRn IF_STATEMENT_SKIP_" << tempNum << std::endl;
                }
                else if (static_cast<Comparison&>(condition).op == 3) {
                        std::cout << "BRp IF_STATEMENT_SKIP_" << tempNum << std::endl;
                }
                else if (static_cast<Comparison&>(condition).op == 4) {
                        std::cout << "BRnp IF_STATEMENT_SKIP_" << tempNum << std::endl;
                }
                std::cout << "NOT R7, R7\n" << "ADD R7, R7, #1\n";
                std::cout << "ADD R" << var_reg << ", R" << var_reg << ", R7" << std::endl;

                Block& action = static_cast<IfStatement*>(node)->action;
                Expression_interp(&action);
                std::cout << "BR IF_STATEMENT_ALT_" << tempNum << "\n\n";

                std::cout << "IF_STATEMENT_SKIP_" << tempNum << "\n\n";
                std::cout << "NOT R7, R7\n" << "ADD R7, R7, #1\n";
                std::cout << "ADD R" << var_reg << ", R" << var_reg << ", R7" << std::endl;
                std::cout << "IF_STATEMENT_ALT_" << tempNum << "\n\n";
        }
        if (node->type() == RETURNSTATEMENT){
                int a = hashLookup(currentStatus.function, functionDeclTable);
                int value;
                Expression& expression = static_cast<ReturnStatement*>(node)->expression;
                if (expression.type() == IDENTIFIER){
                        value = variableAssignTable.at(variablePosInHash(expression.get_name()))->get_value();
                        std::cout << "STI R" << variableAssignTable.at(variablePosInHash(expression.get_name()))->get_reg()
                                  << ", FunctionReturn_" << functionDeclTable.at(currentFunctionPosInHash)->get_address() << std::endl;
                }
                else if (expression.type() == INTEGER){
                        value = static_cast<Integer&>(expression).value;
                        std::cout << "LD R7, ReturnInteger_" << currentStatus.integerReturns.size() << "_" << functionDeclTable.at(currentFunctionPosInHash)->get_address() << std::endl;
                        std::cout << "STI R7, FunctionReturn_" << functionDeclTable.at(currentFunctionPosInHash)->get_address() << std::endl;
                        currentStatus.integerReturns.push_back(value);
                }

                std::cout << ";-----------------load backups---------------\n";
                std::cout << "LD R0, R0_BACKUP_" << functionDeclTable.at(a)->get_address() << std::endl;
                std::cout << "LD R1, R1_BACKUP_" << functionDeclTable.at(a)->get_address() << std::endl;
                std::cout << "LD R2, R2_BACKUP_" << functionDeclTable.at(a)->get_address() << std::endl;
                std::cout << "LD R3, R3_BACKUP_" << functionDeclTable.at(a)->get_address() << std::endl;
                std::cout << "LD R4, R4_BACKUP_" << functionDeclTable.at(a)->get_address() << std::endl;
                std::cout << "LD R5, R5_BACKUP_" << functionDeclTable.at(a)->get_address() << std::endl;
                std::cout << "LD R6, R6_BACKUP_" << functionDeclTable.at(a)->get_address() << std::endl;
                std::cout << "LD R7, R7_BACKUP_" << functionDeclTable.at(a)->get_address() << std::endl;
                std::cout << ";--------------------------------------------\n";

                if (functionDeclTable.at(a)->get_address() == 3000)
                        std::cout << "HALT\n";
                else
                        std::cout << "RET\n";
                CURRENTRETURNVALUE = value;
        }
        if (node->type() == FUNCTIONDECLARATION){
                currentStatus.reg = 0;

                const Identifier& functionType = static_cast<FunctionDeclaration*>(node)->functionType;
                std::string functionTypeString = Expression_interp(&(const_cast<Identifier&>(functionType)));

                if (functionTypeString != "int" && functionTypeString != "void"){
                        std::cerr << "Error: invalid type identifier\n";
                        exit(1);
                }

                const Identifier& id = static_cast<FunctionDeclaration*>(node)->id;
                std::string functionNameString = Expression_interp(&(const_cast<Identifier&>(id)));
                currentStatus.function = functionNameString;

                if (functionNameString == "int"){
                        std::cerr << "Error: type \"int\" cannot be function name.\n";
                }

                std::cout << ";%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n";
                std::cout << ";Routine: " << functionNameString << std::endl;
                std::cout << ";%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n";

                int a = hashLookup(functionNameString, functionDeclTable);
                std::cout << ".ORIG x" << functionDeclTable.at(a)->get_address() << std::endl;
                std::cout << "LDI R6, FuncCallBackupAddr_" << functionDeclTable.at(a)->get_address() << std::endl;

                std::cout << ";----------------store backups---------------\n";
                std::cout << "ST R0, R0_BACKUP_" << functionDeclTable.at(a)->get_address() << std::endl;
                std::cout << "ST R1, R1_BACKUP_" << functionDeclTable.at(a)->get_address() << std::endl;
                std::cout << "ST R2, R2_BACKUP_" << functionDeclTable.at(a)->get_address() << std::endl;
                std::cout << "ST R3, R3_BACKUP_" << functionDeclTable.at(a)->get_address() << std::endl;
                std::cout << "ST R4, R4_BACKUP_" << functionDeclTable.at(a)->get_address() << std::endl;
                std::cout << "ST R5, R5_BACKUP_" << functionDeclTable.at(a)->get_address() << std::endl;
                std::cout << "ST R6, R6_BACKUP_" << functionDeclTable.at(a)->get_address() << std::endl;
                std::cout << "ST R7, R7_BACKUP_" << functionDeclTable.at(a)->get_address() << std::endl;
                std::cout << ";--------------------------------------------\n";

                #define functionArguments static_cast<FunctionDeclaration*>(node)->arguments
                if (!functionArguments.empty()){
                        std::cout << "LD R6, FuncCallParameters_" << functionDeclTable.at(currentFunctionPosInHash)->get_address() << std::endl;
                        for (unsigned i = 0; i < functionArguments.size(); i++){
                                std::cout << "LDR R" << i << ", R6, #" << i << std::endl;
                                currentStatus.variableIsArgument = 1;
                                Statement_interp(functionArguments.at(i));
                        }
                }

                Block& block = static_cast<FunctionDeclaration*>(node)->block;
                Expression_interp(&block);

                std::cout << ";------------\n";
                std::cout << ";Routine Data\n";
                std::cout << ";------------\n";

                std::cout << ";-------------register backups---------------\n";
                std::cout << "R0_BACKUP_" << functionDeclTable.at(a)->get_address() << "\t.BLKW\t#1\n";
                std::cout << "R1_BACKUP_" << functionDeclTable.at(a)->get_address() << "\t.BLKW\t#1\n";
                std::cout << "R2_BACKUP_" << functionDeclTable.at(a)->get_address() << "\t.BLKW\t#1\n";
                std::cout << "R3_BACKUP_" << functionDeclTable.at(a)->get_address() << "\t.BLKW\t#1\n";
                std::cout << "R4_BACKUP_" << functionDeclTable.at(a)->get_address() << "\t.BLKW\t#1\n";
                std::cout << "R5_BACKUP_" << functionDeclTable.at(a)->get_address() << "\t.BLKW\t#1\n";
                std::cout << "R6_BACKUP_" << functionDeclTable.at(a)->get_address() << "\t.BLKW\t#1\n";
                std::cout << "R7_BACKUP_" << functionDeclTable.at(a)->get_address() << "\t.BLKW\t#1\n";
                std::cout << ";--------------------------------------------\n";

                for (unsigned i = 0; i < variablePositions.size(); i++){
                        unsigned pos = variablePositions.at(i);
                        for (unsigned j = 0; j <= variableAssignTable.at(pos)->get_times_invoked(); j++){
                                if (j == 0){
                                        std::cout << variableAssignTable.at(pos)->get_name() << '_' << functionDeclTable.at(a)->get_address()
                                                  << "\t.FILL\t#" << variableAssignTable.at(pos)->get_value(j) << std::endl;
                                        continue;
                                }
                                std::cout << variableAssignTable.at(pos)->get_name() << '_' << j << '_' << functionDeclTable.at(a)->get_address()
                                          << "\t.FILL\t#" << variableAssignTable.at(pos)->get_value(j) << std::endl;
                        }
                }

                for (unsigned i = 0; i < functionPositions.size(); i++){
                        unsigned pos = functionPositions.at(i);
                        if (functionDeclTable.at(pos)->get_times_invoked() > 0){
                                std::cout << functionDeclTable.at(pos)->get_name() << '_' << functionDeclTable.at(a)->get_address()
                                          << "\t.FILL\tx" << functionDeclTable.at(pos)->get_address() << std::endl;
                        }
                }

                std::cout << "FuncCallBackupAddr_" << functionDeclTable.at(currentFunctionPosInHash)->get_address() << "\t.FILL\tx6000\n";
                std::cout << "FuncCallParameters_" << functionDeclTable.at(currentFunctionPosInHash)->get_address() << "\t.FILL\tx6001\n";
                std::cout << "FunctionReturn_" << functionDeclTable.at(currentFunctionPosInHash)->get_address() << "\t.FILL\tx6008\n";
                std::cout << "newline_" << functionDeclTable.at(currentFunctionPosInHash)->get_address() << "\t.FILL\t#10\n";
                for (unsigned i = 0; i < currentStatus.integerReturns.size(); i++){
                        std::cout << "ReturnInteger_" << i << "_"  << functionDeclTable.at(currentFunctionPosInHash)->get_address() << "\t.FILL\t#" << currentStatus.integerReturns.at(i) << std::endl;
                }
                for (unsigned i = 0; i < currentStatus.comparisons.size(); i++){
                        std::cout << "Compare_" << i << "_" << functionDeclTable.at(currentFunctionPosInHash)->get_address()  << "\t.FILL\t#" << currentStatus.comparisons.at(i) << std::endl;
                }
                for (unsigned i = 0; i < currentStatus.strings.size(); i++){
                        std::cout << "String_" << i << "_" << functionDeclTable.at(currentFunctionPosInHash)->get_address() << "\t.STRINGZ\t\"" << currentStatus.strings.at(i) << "\"" << std::endl;
                }
                std::cout << "General_Backup_" << functionDeclTable.at(currentFunctionPosInHash)->get_address() << "\t.BLKW\t#1\n";

                std::cout << ";%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n";
                std::cout << ";End of routine\n";
                std::cout << ";%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n";

                variableAssignTable = variableTableBackup.top();
                variableTableBackup.pop();
                variablePositions = variablePositionsBackup.top();
                variablePositionsBackup.pop();

        }
}

