#include <vector>
#include <iostream>
#include <string>
#include <algorithm>
#include <stack>

//--------------------
struct Status {
        unsigned reg;
        unsigned address;
        unsigned ifStatement;
        std::string function;

        Status (unsigned reg, unsigned address, unsigned ifStatement, std::string function) :
                reg(reg), address(address), ifStatement(ifStatement), function(function) { }
};

class Object {
        public:
                virtual std::string type() {return "Object";}
                virtual std::string get_name() {return NULL;}
                virtual int get_value() {return 9999;}
                virtual void set_value(int n) { }
                virtual unsigned get_address() {return 0;}
                virtual unsigned get_reg() {return 9999;}
                virtual unsigned get_times_invoked() {return 9999;}
                virtual void increment_invocation() { }
};

class AssignedVariable : public Object {
        private:
                std::string name;
                int value;
                unsigned reg;
                unsigned timesInvoked;
        public:
                AssignedVariable(std::string name, unsigned reg, int value, unsigned timesInvoked) :
                        name(name), reg(reg), value(value), timesInvoked(timesInvoked) { }

                std::string type() {return "AssignedVariable";}
                std::string get_name() {return name;}
                int get_value() {return value;}
                void set_value(int n) {value = n;}
                unsigned get_reg() {return reg;}
                unsigned get_times_invoked() {return timesInvoked;}
                void increment_invocation() {timesInvoked++; }
};

class DeclaredFunction : public Object {
        private:
                std::string name;
                unsigned address;
                unsigned timesInvoked;
        public:
                DeclaredFunction(std::string name, unsigned address, unsigned timesInvoked) :
                        name(name), address(address), timesInvoked(timesInvoked) { }

                std::string type() {return "DeclaredFunction";}
                std::string get_name() {return name;}
                unsigned get_address() {return address;}
                unsigned get_times_invoked() {return timesInvoked;}
                void increment_invocation() {timesInvoked++; }
};

const unsigned NHASH = 9997;  //size of hash table
std::vector<int> variablePositions;  //Positions filled in hash tables
std::vector<int> functionPositions;
Status currentStatus (0,3000,0,"main"); //Starting status

//the hash tables
std::stack<std::vector<Object*>> variableTableBackup;
std::stack<std::vector<int>> variablePositionsBackup;
std::vector<Object*> variableAssignTable(NHASH);
std::vector<Object*> functionDeclTable(NHASH);

unsigned hash(const std::string& s){ //hash function
        unsigned n = 0;

        for (unsigned i = 0; i < s.size(); i++){
                for (unsigned j = 0; j < 3; j++)
                        n = n*9 ^ s.at(i);
        }

        return n;
}

int hashLookup(const std::string& s, std::vector<Object*>& table){ //search for position in hash table
        const unsigned hashNum = hash(s)%NHASH;

        if (table.at(hashNum) == NULL){
                return -1;
        }
        else if (table.at(hashNum)->get_name() == s){
                return hashNum;
        }
        else {
                for (unsigned i = hashNum; true; i++){
                        if (i == NHASH) i=0;
                        if (table.at(i)->get_name() == s)
                                return i;
                        else if (table.at(i) == NULL){
                                return -1;
                        }
                }
        }
}

int hashPush(const std::string& s, std::vector<Object*>& table, int value){ //add value to hash table
        unsigned hashNum = hash(s)%NHASH;

        if (table.at(hashNum) == NULL){
                variablePositions.push_back(hashNum);
                table.at(hashNum) = new AssignedVariable(s,currentStatus.reg,value,0);
                currentStatus.reg++;
                return 0;
        }
        else if (table.at(hashNum)->get_name() == s){
                return -1;
        }
        else {
                for (unsigned i = hashNum; true; i++){
                        if (i == NHASH) i=0;
                        if (table.at(i) == NULL){
                                variablePositions.push_back(i);
                                table.at(i) = new AssignedVariable(s,currentStatus.reg,value,0);
                                currentStatus.reg++;
                                return 0;
                        }
                        else if (table.at(i)->get_name() == s)
                                return -1;
                }
        }
}

int hashPush(const std::string& s, std::vector<Object*>& table){
        unsigned hashNum = hash(s)%NHASH;

        if (table.at(hashNum) == NULL){
                functionPositions.push_back(hashNum);
                table.at(hashNum) = new DeclaredFunction(s,currentStatus.address,0);
                currentStatus.address+=200;
                return 0;
        }
        else if (table.at(hashNum)->get_name() == s)
                return -1;
        else {
                for (unsigned i = hashNum; true; i++){
                        if (i == NHASH) i=0;
                        if (table.at(i) == NULL){
                                functionPositions.push_back(i);
                                table.at(i) = new DeclaredFunction(s,currentStatus.address,0);
                                currentStatus.address+=200;
                                return 0;
                        }
                        else if (table.at(i)->get_name() == s)
                                return -1;

                }
        }
}
