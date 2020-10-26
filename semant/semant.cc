#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "semant.h"
#include "utilities.h"
#include <map>

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

// typedef std::map<CallDecl_class, Stmt_class> StmtTable;
// StmtTable stmtTable;

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
                semant_error(decls->nth(i))<<"Redefined function."<<endl;
            } else if (type != Int && type != Void && type != String && type != Float && type != Bool) {
                semant_error(decls->nth(i))<<"Function returnType error."<<endl;
            }
            callTable[name] = type;
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
            }
            globalVars[name] = type;
        }
    }
}

static void check_calls(Decls decls) {
    objectEnv.enterscope();
    for (int i=decls->first(); decls->more(i); i=decls->next(i)) {
        if (decls->nth(i)->isCallDecl()) {
            decls->nth(i)->check();

            // check paras
            // Variables vars = decls->nth(i)->getVariables();
            // for (int j=vars->first(); vars->more(j); j=vars->next(j)) {
            //     // main function should not have any paras
            //     if (objectEnv.probe(Main) != NULL) {
            //         semant_error(decls->nth(i))<<"Main function should not have paras"<<endl;
            //     }
            //     Symbol name = vars->nth(j)->getName();
            //     Symbol type = vars->nth(j)->getType();
                
            //     /* No need to check paras' type because of syntax rules */

            //     // check if there are duplicated paras
            //     if (objectEnv.lookup(name) != NULL) {
            //         semant_error(decls->nth(i))<<"Function "<<FuncName<< "'s parameter has a duplicate name "<<name<<endl;
            //     }

            //     objectEnv.addid(name, &type);

            // }

            // // check stmtBlock
            // StmtBlock stmtblock = decls->nth(i)->getBody();
            // Stmts stmts = stmtblock->getStmts();
            // for (int i=stmts->first(); stmts->more(i); i=stmts->next(i)) {
            //     Stmt stmt = stmts->nth(i);
            //     stmt->check(FuncType);
                
            // }
        }
    }
    objectEnv.exitscope();
}

static void check_main() {
    if (callTable[Main] == NULL) {
        semant_error()<<"main function is not defined."<<endl;
    } else if (callTable[Main] != Void) {
        semant_error()<<"main function should have return type Void."<<endl;
    }
}

void VariableDecl_class::check() {
    cout<<"Helllo"<<endl;
    Symbol name = this->getName();
    Symbol type = this->getType();
    if (type == Void) {
        semant_error()<<"var "<<name<<" cannot be of type Void. Void can just be used as return type."<<endl;
    } else {
        objectEnv.addid(name, &type);
    }
}

void CallDecl_class::check() {
    objectEnv.enterscope();
    Variables vars = this->getVariables();
    Symbol funcName = this->getName(); 
    Symbol returnType = this->getType();
    StmtBlock stmtblock = this->getBody();
    
    // main function should not have any paras
    if (funcName == Main && vars->len() != 0) {
        semant_error(this)<<"Main function should not have paras"<<endl;
    }

    for (int j=vars->first(); vars->more(j); j=vars->next(j)) {
        Symbol name = vars->nth(j)->getName();
        Symbol type = vars->nth(j)->getType();
        
        /* No need to check paras' type because of syntax rules */

        // check if there are duplicated paras
        if (objectEnv.lookup(name) != NULL) {
            semant_error(this)<<"Function "<<funcName<< "'s parameter has a duplicate name "<<name<<endl;
        }
        objectEnv.addid(name, &type);
    }

    // check stmtBlock
    VariableDecls varDecls = stmtblock->getVariableDecls();
    for (int j=varDecls->first(); varDecls->more(j); j=varDecls->next(j)) {
        varDecls->nth(j)->check();
    }
    Stmts stmts = stmtblock->getStmts();
    objectEnv.exitscope();
}

void StmtBlock_class::check(Symbol type) {
    // VariableDecls vars = this->getVariableDecls();
    // Stmts stmts = this->getStmts();
    // // check variables declarations
    // for (int i=vars->first(); vars->more(i); i=vars->next(i)) {
    //     VariableDecl var = vars->nth(i);
    //     var->check();
    // }

    // // check stmt in stmtblock
    // for (int i=stmts->first(); stmts->more(i); i=stmts->next(i)) {
    //     Stmt stmt = stmts->nth(i);
    //     // stmt->check(type);
    // }
    

}

void IfStmt_class::check(Symbol type) {
    
}

void WhileStmt_class::check(Symbol type) {
    
}

void ForStmt_class::check(Symbol type) {
    
}

void ReturnStmt_class::check(Symbol type) {

}

void ContinueStmt_class::check(Symbol type) {
    
}

void BreakStmt_class::check(Symbol type) {

}

Symbol Call_class::checkType(){

}

Symbol Actual_class::checkType(){

}

Symbol Assign_class::checkType(){
    
}

Symbol Add_class::checkType(){
 
}

Symbol Minus_class::checkType(){
 
}

Symbol Multi_class::checkType(){
 
}

Symbol Divide_class::checkType(){

}

Symbol Mod_class::checkType(){

}

Symbol Neg_class::checkType(){

}

Symbol Lt_class::checkType(){

}

Symbol Le_class::checkType(){

}

Symbol Equ_class::checkType(){

}

Symbol Neq_class::checkType(){

}

Symbol Ge_class::checkType(){

}

Symbol Gt_class::checkType(){

}

Symbol And_class::checkType(){

}

Symbol Or_class::checkType(){

}

Symbol Xor_class::checkType(){

}

Symbol Not_class::checkType(){

}

Symbol Bitand_class::checkType(){

}

Symbol Bitor_class::checkType(){

}

Symbol Bitnot_class::checkType(){

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



