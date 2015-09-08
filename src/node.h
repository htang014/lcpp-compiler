#include <iostream>
#include <vector>

//node types
const unsigned NODE = 0,                EXPRESSION = 1,                 STATEMENT = 2,
               INTEGER = 3,             DOUBLE = 4,                     IDENTIFIER = 5,
               METHODCALL = 6,          BINARYOPERATOR = 7,             COMPARISON = 8,
               ASSIGNMENT = 9,          BLOCK = 10,                     EXPRESSIONSTATEMENT = 11,
               RETURNSTATEMENT = 12,    VARIABLEDECLARATION = 13,       FUNCTIONDECLARATION = 14,
               DOWHILELOOP = 15,        IFSTATEMENT = 16;

class Statement;
class Expression;
class VariableDeclaration;

typedef std::vector<Statement*> StatementList;
typedef std::vector<Expression*> ExpressionList;
typedef std::vector<VariableDeclaration*> VariableList;

class Node {
public:
	virtual int type() {return NODE;};
};

class Expression : public Node {
public:
        virtual int type() {return EXPRESSION;};
};

class Statement : public Node {
public:
        virtual int type() {return STATEMENT;};
};

class Integer : public Expression {
public:
	long long value;
	Integer(long long value) : value(value) { }
	int type() {return INTEGER;};
};

class Double : public Expression {
public:
	double value;
	Double(double value) : value(value) { }
	int type() {return DOUBLE;};
};

class Identifier : public Expression {
public:
	std::string name;
	Identifier(const std::string& name) : name(name) { }
	int type() {return IDENTIFIER;};
};

class MethodCall : public Expression {
public:
	const Identifier& id;
	ExpressionList arguments;
	MethodCall(const Identifier& id, ExpressionList& arguments) :
		id(id), arguments(arguments) { }
	MethodCall(const Identifier& id) : id(id) { }
	int type() {return METHODCALL;};
};

class BinaryOperator : public Expression {
public:
	int op;
	Expression& lhs;
	Expression& rhs;
	BinaryOperator(Expression& lhs, int op, Expression& rhs) :
		lhs(lhs), rhs(rhs), op(op) { }
	int type() {return BINARYOPERATOR;};
};

class Comparison : public Expression {
public:
        int op;
	Expression& lhs;
	Expression& rhs;
        Comparison(Expression& lhs, int op, Expression& rhs) :
                lhs(lhs), op(op), rhs(rhs) { }
        int type() {return COMPARISON;};
};

class Assignment : public Expression {
public:
	Identifier& lhs;
	Expression& rhs;
	Assignment(Identifier& lhs, Expression& rhs) : 
		lhs(lhs), rhs(rhs) { }
	int type() {return ASSIGNMENT;};
};

class Block : public Expression {
public:
	StatementList statements;
	Block() { }
	int type() {return BLOCK;};
};

class ExpressionStatement : public Statement {
public:
	Expression& expression;
	ExpressionStatement(Expression& expression) : 
		expression(expression) { }
	int type() {return EXPRESSIONSTATEMENT;};
};

class ReturnStatement : public Statement {
public:
	Expression& expression;
	ReturnStatement(Expression& expression) : 
		expression(expression) { }
	int type() {return RETURNSTATEMENT;};
};

class VariableDeclaration : public Statement {
public:
	const Identifier& variableType;
	Identifier& id;
	Expression *assignmentExpr;
	VariableDeclaration(const Identifier& variableType, Identifier& id) :
		variableType(variableType), id(id) { }
	VariableDeclaration(const Identifier& variableType, Identifier& id, Expression *assignmentExpr) :
		variableType(variableType), id(id), assignmentExpr(assignmentExpr) { }
	int type() {return VARIABLEDECLARATION;};
};

class FunctionDeclaration : public Statement {
public:
	const Identifier& functionType;
	const Identifier& id;
	VariableList arguments;
	Block& block;
	FunctionDeclaration(const Identifier& functionType, const Identifier& id, 
			const VariableList& arguments, Block& block) :
		functionType(functionType), id(id), arguments(arguments), block(block) { }
	int type() {return FUNCTIONDECLARATION;};
};

class DoWhileLoop : public Statement {
public:
        Block& action;
        Expression& condition;
        DoWhileLoop( Expression& condition, Block& action) :
                action(action), condition(condition) { }
        int type() {return DOWHILELOOP;};
};

class IfStatement : public Statement {
public:
        Expression& condition;
        Block& action;
        IfStatement(Expression& condition, Block& action) :
                condition(condition), action(action) { }
        int type() {return IFSTATEMENT;};
};
