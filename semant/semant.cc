#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "semant.h"
#include "utilities.h"
#include <map>
#include <vector>

extern int semant_debug;
extern char *curr_filename;

static ostream& error_stream = cerr;
static int semant_errors = 0;
static Decl curr_decl = 0;

typedef SymbolTable<Symbol, Symbol> ObjectEnvironment; // name, type
ObjectEnvironment objectEnv;

typedef std::map<Symbol, Symbol> CallTable;
CallTable callTable;

// globalVars stores global variables' name and type
typedef std::map<Symbol, Symbol> GlobalVariables;
GlobalVariables globalVars;

// localVars stores local variables' name and type
typedef std::map<Symbol, Symbol> LocalVariables;
LocalVariables localVars;

// MethodClass stores para type
// MethodTable stores name and related paras
typedef std::vector<Symbol> MethodClass;
typedef std::map<Symbol, MethodClass> MethodTable;
MethodTable methodTable;

// InstallTable marks whether function is installed or not
typedef std::map<Symbol, bool> InstallTable;
InstallTable installTable;

///////////////////////////////////////////////
// helper func
///////////////////////////////////////////////


static ostream& semant_error() {
    semant_errors++;
    return error_stream;
}

static ostream& semant_error(tree_node *t) {
    error_stream << t->get_line_number() + 1 << ": ";
    return semant_error();
}

static ostream& internal_error(int lineno) {
    error_stream << "FATAL:" << lineno << ": ";
    return error_stream;
}

//////////////////////////////////////////////////////////////////////
//
// Symbols
//
// For convenience, a large number of symbols are predefined here.
// These symbols include the primitive type and method names, as well
// as fixed names used by the runtime system.
//
//////////////////////////////////////////////////////////////////////

static Symbol 
    Int,
    Float,
    String,
    Bool,
    Void,
    Main,
    print
    ;

bool isValidCallName(Symbol type) {
    return type != (Symbol)print;
}

bool isValidTypeName(Symbol type) {
    return type != Void;
}

//
// Initializing the predefined symbols.
//

static void initialize_constants(void) {
    // 4 basic types and Void type
    Bool        = idtable.add_string("Bool");
    Int         = idtable.add_string("Int");
    String      = idtable.add_string("String");
    Float       = idtable.add_string("Float");
    Void        = idtable.add_string("Void");  
    // main function
    Main        = idtable.add_string("main");

    // classical function to print things, so defined here for call.
    print        = idtable.add_string("printf");
}

/*
    TODO :
    you should fill the following function defines, so that semant() can realize a semantic 
    analysis in a recursive way. 
    Of course, you can add any other functions to help.
*/

static bool sameType(Symbol name1, Symbol name2) {
    return strcmp(name1->get_string(), name2->get_string()) == 0;
}

static void install_calls(Decls decls) {
    for (int i=decls->first(); decls->more(i); i=decls->next(i)) {
        Symbol name = decls->nth(i)->getName();
        Symbol type = decls->nth(i)->getType();
        if (decls->nth(i)->isCallDecl()) {
            if (callTable[name] != NULL) {
                semant_error(decls->nth(i))<<"Function "<<name<<" was previously defined."<<endl;
            } else if (type != Int && type != Void && type != String && type != Float && type != Bool) {
                semant_error(decls->nth(i))<<"Function returnType error."<<endl;
            } else if (!isValidCallName(name)) {
                semant_error(decls->nth(i))<<"Function printf cannot have a name as printf"<<endl;
            }
            callTable[name] = type;
            installTable[name] = false;
            decls->nth(i)->check();
        }
    }
}

static void install_globalVars(Decls decls) {
    for (int i=decls->first(); decls->more(i); i=decls->next(i)) {
        Symbol name = decls->nth(i)->getName();
        Symbol type = decls->nth(i)->getType();
        if (!decls->nth(i)->isCallDecl()) {
            if (globalVars[name] != NULL) {
                semant_error(decls->nth(i))<<"Global variable redefined."<<endl;
            } else if (type == Void) {
                semant_error(decls->nth(i))<<"var "<<name<<" cannot be of type Void. Void can just be used as return type."<<endl;
            } else if (name == print) {
                semant_error(decls->nth(i))<<"Variable printf cannot have a name as printf"<<endl;
            }
            globalVars[name] = type;
        }
    }
}

static void check_calls(Decls decls) {
    objectEnv.enterscope();
    for (int i=decls->first(); decls->more(i); i=decls->next(i)) {
        if (decls->nth(i)->isCallDecl()) {
            localVars.clear();
            decls->nth(i)->check();
        }
    }
    objectEnv.exitscope();
}

static void check_main() {
    if (callTable[Main] == NULL) {
        semant_error()<<"main function is not defined."<<endl;
    }
}

void VariableDecl_class::check() {
    Symbol name = this->getName();
    Symbol type = this->getType();
    if (type == Void) {
        semant_error()<<"var "<<name<<" cannot be of type Void. Void can just be used as return type."<<endl;
    } else {
        objectEnv.addid(name, new Symbol(type));
        localVars[name] = type;
    }
}

void CallDecl_class::check() {
    Variables vars = this->getVariables();
    Symbol funcName = this->getName(); 
    Symbol returnType = this->getType();
    StmtBlock stmtblock = this->getBody();
    
    objectEnv.enterscope();
    // install paras
    if (installTable[name] == false) {
        // methodclass stores paras type
        MethodClass mclass;
        for (int j=vars->first(); vars->more(j); j=vars->next(j)) {
            Symbol name = vars->nth(j)->getName();
            Symbol type = vars->nth(j)->getType();
            
            /* No need to check paras' type because of syntax rules */

            // check if there are duplicated paras
            if (objectEnv.lookup(name) != NULL) {
                semant_error(this)<<"Function "<<funcName<< "'s parameter has a duplicate name "<<name<<endl;
            }
            objectEnv.addid(name, &type);
            localVars[name] = type;
            mclass.push_back(type);
        }

        // methodTable map paras to funcname
        methodTable[funcName] = mclass;
        installTable[name] = true;
    } else {
        for (int j=vars->first(); vars->more(j); j=vars->next(j)) {
            Symbol name = vars->nth(j)->getName();
            Symbol type = vars->nth(j)->getType();
            
            /* No need to check paras' type because of syntax rules */

            // check if there are duplicated paras
            if (objectEnv.lookup(name) != NULL) {
                semant_error(this)<<"Function "<<funcName<< "'s parameter has a duplicate name "<<name<<endl;
            }
            objectEnv.addid(name, new Symbol (type));
            localVars[name] = type;
        }
        // main function should not have any paras
        if (funcName == Main && vars->len() != 0) {
            semant_error(this)<<"Main function should not have paras"<<endl;
        } else if (callTable[Main] != Void) {
            semant_error(this)<<"main function should have return type Void."<<endl;
        }

        // check stmtBlock
        // 1. check variableDecls
        VariableDecls varDecls = stmtblock->getVariableDecls();
        for (int j=varDecls->first(); varDecls->more(j); j=varDecls->next(j)) {
            varDecls->nth(j)->check();
        }

        // 2. check stmts
        stmtblock->check(returnType);
        if (!stmtblock->isReturn()) {
            semant_error(this)<<"Function "<<name<<" must have an overall return statement."<<endl;
        }
        if (stmtblock->isBreak()) {
            Stmts stmts = stmtblock->getStmts();
            for (int i=stmts->first(); stmts->more(i); i=stmts->next(i)) {
                if (stmts->nth(i)->isBreak()) {
                    semant_error(stmts->nth(i))<<"break must be used in a loop sentence"<<endl;
                }
            }
        }
        if (stmtblock->isContinue()) {
            Stmts stmts = stmtblock->getStmts();
            for (int i=stmts->first(); stmts->more(i); i=stmts->next(i)) {
                if (stmts->nth(i)->isContinue()) {
                    semant_error(stmts->nth(i))<<"continue must be used in a loop sentence."<<endl;
                }
            }
        }
    }

    objectEnv.exitscope();
}

void StmtBlock_class::check(Symbol type) {
    Stmts stmts = this->getStmts(); 
    for (int j=stmts->first(); stmts->more(j); j=stmts->next(j)) {
        stmts->nth(j)->check(type);
    }
}

void IfStmt_class::check(Symbol type) {
    Expr condition = this->getCondition();
    StmtBlock thenExpr = this->getThen();
    StmtBlock elseExpr = this->getElse();
    
    // If condition should be Bool
    Symbol conditionType = condition->checkType();
    if (conditionType != Bool) {
        semant_error(this)<<"condition type should be Bool, but found"<<conditionType<<endl;
    }

    // check thenExpr and elseExpr
    thenExpr->check(type);
    elseExpr->check(type);
}

void WhileStmt_class::check(Symbol type) {
    Expr condition = this->getCondition();
    StmtBlock body = this->getBody();

    // While condition should be Bool
    Symbol conditionType = condition->checkType();
    if (conditionType != Bool) {
        semant_error(this)<<"condition type should be Bool, but found"<<conditionType<<endl;
    }

    // check while body
    body->check(type);
}

void ForStmt_class::check(Symbol type) {
    Expr init = this->getInit();
    Expr condition = this->getCondition();
    Expr loop = this->getLoop();
    StmtBlock body = this->getBody();

    init->checkType();
    loop->checkType();
    // For condition should be Bool
    Symbol conditionType = condition->checkType();
    if (conditionType != Bool) {
        semant_error(this)<<"condition type should be Bool, but found"<<conditionType<<endl;
    }

    // check For body
    body->check(type);
}

void ReturnStmt_class::check(Symbol type) {
    Expr expr = this->getValue();
    
    // check if return Type match
    Symbol returnType = expr->checkType();
    if (returnType != Void) {
        if (expr->is_empty_Expr() && type != Void) {
            semant_error(this)<<"Returns Void, but need "<<type<<endl;
        } else if (!expr->is_empty_Expr() && type != returnType) {
            semant_error(this)<<"Returns "<<returnType<<" but need "<<type<<endl;
        }
    }
}

void ContinueStmt_class::check(Symbol type) {
    
}

void BreakStmt_class::check(Symbol type) {

}

Symbol Call_class::checkType(){
    Symbol name = this->getName();
    Actuals actuals = this->getActuals();
    unsigned int j = 0;
    
    if (name == print) {
        if (actuals->len() == 0) {
            semant_error(this)<<"printf() must has at last one parameter of type String."<<endl;
            this->setType(Void);
            return type;
        }
        Symbol sym = actuals->nth(actuals->first())->checkType();
        if (sym != String) {
            semant_error(this)<<"printf()'s first parameter must be of type String."<<endl;
            this->setType(Void);
            return type;
        }
        this->setType(Void);
        return type;
    }

    if (actuals->len() > 0){
        if (actuals->len() != int(methodTable[name].size())) {
            semant_error(this)<<"Wrong number of paras"<<endl;
        }
        for (int i=actuals->first(); actuals->more(i) && j<methodTable[name].size(); i=actuals->next(i)) {
            Expr expr = actuals->nth(i)->copy_Expr();
            Symbol sym = expr->checkType();
            // check function call's paras fit funcdecl's paras
            if (sym != methodTable[name][j]) {
                semant_error(this)<<"Function "<<name<<", type "<<sym<<" does not conform to declared type "<<methodTable[name][j]<<endl;
            }
            j ++;
            actuals->nth(i)->checkType();
        }
    }
    
    if (callTable[name] == NULL) {
        semant_error(this)<<"Object "<<name<<" has not been defined"<<endl;
        this->setType(Void);
        return type;
    } 
    this->setType(callTable[name]);
    return type;
}

Symbol Actual_class::checkType(){
    Symbol sym = expr->checkType();
    this->setType(sym);
    return type;
}

Symbol Assign_class::checkType(){
    if (objectEnv.lookup(lvalue) == NULL && globalVars[lvalue] == NULL) {
        semant_error(this)<<"Undefined value"<<endl;
    } 
    Symbol ls = localVars[lvalue];
    Symbol rs = value->checkType();
    if (ls != rs) {
        semant_error(this)<<"assign value mismatch"<<endl;
    }    
    this->setType(rs);
    return type;
}

Symbol Add_class::checkType(){
    Expr lvalue = e1;
    Expr rvalue = e2;
    Symbol ls = lvalue->checkType();
    Symbol rs = rvalue->checkType();

    if (ls != rs && !(ls == Int && rs == Float) && !(ls == Float && rs == Int)) {
        semant_error(this)<<"lvalue and rvalue should have same type."<<endl;
    }
    if ((ls == Float && rs == Int)||(ls == Int && rs == Float)) {
        this->setType(Float);
        return type;
    }

    this->setType(ls);
    return type;
}

Symbol Minus_class::checkType(){
    Expr lvalue = e1;
    Expr rvalue = e2;
    Symbol ls = lvalue->checkType();
    Symbol rs = rvalue->checkType();

    if (ls != rs && !(ls == Int && rs == Float) && !(ls == Float && rs == Int)) {
        semant_error(this)<<"Both lvalue and rvalue should be type Int"<<endl;
    }
    if ((ls == Float && rs == Int)||(ls == Int && rs == Float)) {
        this->setType(Float);
        return type;
    }

    this->setType(ls);
    return type;
}

Symbol Multi_class::checkType(){
    Expr lvalue = e1;
    Expr rvalue = e2;
    Symbol ls = lvalue->checkType();
    Symbol rs = rvalue->checkType();

    if (ls != rs && !(ls == Int && rs == Float) && !(ls == Float && rs == Int)) {
        semant_error(this)<<"Both lvalue and rvalue should be type Int"<<endl;
    }
    if ((ls == Float && rs == Int)||(ls == Int && rs == Float)) {
        this->setType(Float);
        return type;
    }

    this->setType(ls);
    return type;
}

Symbol Divide_class::checkType(){
    Expr lvalue = e1;
    Expr rvalue = e2;
    Symbol ls = lvalue->checkType();
    Symbol rs = rvalue->checkType();

    if (ls != rs && !(ls == Int && rs == Float) && !(ls == Float && rs == Int)) {
        semant_error(this)<<"Both lvalue and rvalue should be type Int"<<endl;
    }
    if ((ls == Float && rs == Int)||(ls == Int && rs == Float)) {
        this->setType(Float);
        return type;
    }

    this->setType(ls);
    return type;
}

Symbol Mod_class::checkType(){
    Expr lvalue = e1;
    Expr rvalue = e2;
    Symbol ls = lvalue->checkType();
    Symbol rs = rvalue->checkType();

    if (ls != rs && !(ls == Int && rs == Float) && !(ls == Float && rs == Int)) {
        semant_error(this)<<"Both lvalue and rvalue should be type Int"<<endl;
    }
    if ((ls == Float && rs == Int)||(ls == Int && rs == Float)) {
        this->setType(Float);
        return type;
    }

    this->setType(ls);
    return type;
}

Symbol Neg_class::checkType(){
    Expr value = e1;
    Symbol sym = e1->checkType();
    if (sym != Int && sym != Float) {
        semant_error(this)<<"Neg_class should have Int type"<<endl;
    }
    this->setType(Int);
    return type;
}

Symbol Lt_class::checkType(){
    Expr lvalue = e1;
    Expr rvalue = e2;
    Symbol ls = e1->checkType();
    Symbol rs = e2->checkType();
    if ((ls != Int && ls !=Float) || (rs != Int && rs != Float)) {
        semant_error(this)<<"ltype mismatch rvalue"<<endl;
    }
    this->setType(Bool); 
    return type;
}

Symbol Le_class::checkType(){
    Expr lvalue = e1;
    Expr rvalue = e2;
    Symbol ls = e1->checkType();
    Symbol rs = e2->checkType();
    if ((ls != Int && ls !=Float) || (rs != Int && rs != Float)) {
        semant_error(this)<<"ltype mismatch rtype"<<endl;
    }
    this->setType(Bool); 
    return type;
}

Symbol Equ_class::checkType(){
    Expr lvalue = e1;
    Expr rvalue = e2;
    Symbol ls = e1->checkType();
    Symbol rs = e2->checkType();
    if (((ls != Int && ls !=Float) || (rs != Int && rs != Float))&&(ls != Bool || rs != Bool)) {
        semant_error(this)<<"ltype mismatch rtype"<<endl;
    }    
    this->setType(Bool); 
    return type;
}

Symbol Neq_class::checkType(){
    Expr lvalue = e1;
    Expr rvalue = e2;
    Symbol ls = e1->checkType();
    Symbol rs = e2->checkType();
    if (((ls != Int && ls !=Float) || (rs != Int && rs != Float))&&(ls != Bool || rs != Bool)) {
        semant_error(this)<<"ltype mismatch rtype"<<endl;
    }    
    this->setType(Bool); 
    return type;
}

Symbol Ge_class::checkType(){
    Expr lvalue = e1;
    Expr rvalue = e2;
    Symbol ls = e1->checkType();
    Symbol rs = e2->checkType();
    if ((ls != Int && ls !=Float) || (rs != Int && rs != Float)) {
        semant_error(this)<<"ltype mismatch rtype"<<endl;
    }    
    this->setType(Bool); 
    return type;
}

Symbol Gt_class::checkType(){
    Expr lvalue = e1;
    Expr rvalue = e2;
    Symbol ls = e1->checkType();
    Symbol rs = e2->checkType();
    if ((ls != Int && ls !=Float) || (rs != Int && rs != Float)) {
        semant_error(this)<<"ltype mismatch rtype"<<endl;
    }    
    this->setType(Bool); 
    return type;
}

Symbol And_class::checkType(){
    Expr lvalue = e1;
    Expr rvalue = e2;
    Symbol ls = lvalue->checkType();
    Symbol rs = rvalue->checkType();

    if (ls != Bool || rs != Bool) {
        semant_error(this)<<"Both lvalue and rvalue should be type Bool"<<endl;
    }
    this->setType(Bool);
    return type;
}

Symbol Or_class::checkType(){
    Expr lvalue = e1;
    Expr rvalue = e2;
    Symbol ls = lvalue->checkType();
    Symbol rs = rvalue->checkType();

    if (ls != Bool || rs != Bool) {
        semant_error(this)<<"Both lvalue and rvalue should be type Bool"<<endl;
    }
    this->setType(Bool);
    return type;
}

Symbol Xor_class::checkType(){
    Expr lvalue = e1;
    Expr rvalue = e2;
    Symbol ls = lvalue->checkType();
    Symbol rs = rvalue->checkType();

    if (!(ls == Bool && rs == Bool) && !(ls == Int && rs ==Int)) {
        semant_error(this)<<"Both lvalue and rvalue should be type Bool"<<endl;
    }
    this->setType(Bool);
    return type;
}

Symbol Not_class::checkType(){
    Symbol sym = e1->checkType();
    if (sym != Bool) {
        semant_error(this)<<"Not class should have Bool type"<<endl;
    }

    this->setType(sym);
    return type;
}

Symbol Bitand_class::checkType(){
    Symbol ls = e1->checkType();
    Symbol rs = e2->checkType();

    if (ls != Int || rs != Int) {
        semant_error(this)<<"Bitand class should have Int type"<<endl;
    }

    this->setType(Int);
    return type;
}

Symbol Bitor_class::checkType(){
    Symbol ls = e1->checkType();
    Symbol rs = e2->checkType();

    if (ls != Int || rs != Int) {
        semant_error(this)<<"Bitor class should have Int type"<<endl;
    }

    this->setType(Int);
    return type;
}

Symbol Bitnot_class::checkType(){
    Symbol sym = e1->checkType();

    if (sym != Int) {
        semant_error(this)<<"Bitnot class should have Int type"<<endl;
    }

    this->setType(Int);
    return type;
}

Symbol Const_int_class::checkType(){
    setType(Int);
    return type;
}

Symbol Const_string_class::checkType(){
    setType(String);
    return type;
}

Symbol Const_float_class::checkType(){
    setType(Float);
    return type;
}

Symbol Const_bool_class::checkType(){
    setType(Bool);
    return type;
}

Symbol Object_class::checkType(){
    if (objectEnv.lookup(var) == NULL) {
        semant_error(this)<<"object "<<var<<" has not been defined."<<endl;
        this->setType(Void);
        return type;
    }
    Symbol ty = localVars[var];
    this->setType(ty);
    return type;
}

Symbol No_expr_class::checkType(){
    setType(Void);
    return getType();
}

void Program_class::semant() {
    initialize_constants();
    install_calls(decls);
    check_main();
    install_globalVars(decls);
    check_calls(decls);
    
    if (semant_errors > 0) {
        cerr << "Compilation halted due to static semantic errors." << endl;
        exit(1);
    }
}



