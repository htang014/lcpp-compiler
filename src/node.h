#include <iostream>
#include <vector>

class Statement;
class Expression;
class VariableDeclaration;

typedef std::vector<Statement*> StatementList;
typedef std::vector<Expression*> ExpressionList;
typedef std::vector<VariableDeclaration*> VariableList;

class Node {
public:
	virtual std::string type() {return "Node";};
};

class Expression : public Node {
public:
        virtual std::string type() {return "Expression";};
};

class Statement : public Node {
public:
        virtual std::string type() {return "Statement";};
};

class Integer : public Expression {
public:
	long long value;
	Integer(long long value) : value(value) { }
	std::string type() {return "Integer";};
};

class Double : public Expression {
public:
	double value;
	Double(double value) : value(value) { }
	std::string type() {return "Double";};
};

class Identifier : public Expression {
public:
	std::string name;
	Identifier(const std::string& name) : name(name) { }
	std::string type() {return "Identifier";};
};

class MethodCall : public Expression {
public:
	const Identifier& id;
	ExpressionList arguments;
	MethodCall(const Identifier& id, ExpressionList& arguments) :
		id(id), arguments(arguments) { }
	MethodCall(const Identifier& id) : id(id) { }
	std::string type() {return "MethodCall";};
};

class BinaryOperator : public Expression {
public:
	int op;
	Expression& lhs;
	Expression& rhs;
	BinaryOperator(Expression& lhs, int op, Expression& rhs) :
		lhs(lhs), rhs(rhs), op(op) { }
	std::string type() {return "BinaryOperator";};
};

class Comparison : public Expression {
public:
        int op;
	Expression& lhs;
	Expression& rhs;
        Comparison(Expression& lhs, int op, Expression& rhs) :
                lhs(lhs), op(op), rhs(rhs) { }
        std::string type() {return "Comparison";};
};

class Assignment : public Expression {
public:
	Identifier& lhs;
	Expression& rhs;
	Assignment(Identifier& lhs, Expression& rhs) : 
		lhs(lhs), rhs(rhs) { }
	std::string type() {return "Assignment";};
};

class Block : public Expression {
public:
	StatementList statements;
	Block() { }
	std::string type() {return "Block";};
};

class ExpressionStatement : public Statement {
public:
	Expression& expression;
	ExpressionStatement(Expression& expression) : 
		expression(expression) { }
	std::string type() {return "ExpressionStatement";};
};

class ReturnStatement : public Statement {
public:
	Expression& expression;
	ReturnStatement(Expression& expression) : 
		expression(expression) { }
	std::string type() {return "ReturnStatement";};
};

class VariableDeclaration : public Statement {
public:
	const Identifier& var_type;
	Identifier& id;
	Expression *assignmentExpr;
	VariableDeclaration(const Identifier& var_type, Identifier& id) :
		var_type(var_type), id(id) { }
	VariableDeclaration(const Identifier& var_type, Identifier& id, Expression *assignmentExpr) :
		var_type(var_type), id(id), assignmentExpr(assignmentExpr) { }
	std::string type() {return "VariableDeclaration";};
};

class FunctionDeclaration : public Statement {
public:
	const Identifier& func_type;
	const Identifier& id;
	VariableList arguments;
	Block& block;
	FunctionDeclaration(const Identifier& func_type, const Identifier& id, 
			const VariableList& arguments, Block& block) :
		func_type(func_type), id(id), arguments(arguments), block(block) { }
	std::string type() {return "FunctionDeclaration";};
};

class DoWhileLoop : public Statement {
public:
        Block& action;
        Expression& condition;
        DoWhileLoop(Block& action, Expression& condition) :
                action(action), condition(condition) { }
        std::string type() {return "DoWhileLoop";};
};

class IfStatement : public Statement {
public:
        Expression& condition;
        Block& action;
        IfStatement(Expression& condition, Block& action) :
                condition(condition), action(action) { }
        std::string type() {return "IfStatement";};
};
