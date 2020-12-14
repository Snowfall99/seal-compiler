#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "semant.h"
#include "utilities.h"

extern int semant_debug;
extern char *curr_filename;

static ostream& error_stream = cerr;
static int semant_errors = 0;
static Decl curr_decl = 0;

typedef SymbolTable<Symbol, Symbol> ObjectEnvironment; // name, type
ObjectEnvironment objectEnv; //globalvars -> paras <-  -> localvars <-

typedef SymbolTable<Symbol, Symbol> Calls_rtType; // name, returnType
Calls_rtType calls_rtType;  //calls

typedef SymbolTable<Symbol, Variables> Calls_paras; // name, paras
Calls_paras calls_paras;  //calls

typedef SymbolTable<int, int> LoopBody; // every type is ok.Just like a stack
LoopBody loopBody;

bool return_InTheEnd; 


///////////////////////////////////////////////
// helper func
///////////////////////////////////////////////


static ostream& semant_error() {
    semant_errors++;
    return error_stream;
}

static ostream& semant_error(tree_node *t) {
    error_stream << t->get_line_number() << ": ";
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
    // Main function
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

    calls_rtType.enterscope();

    for(int n=decls->first(); decls->more(n); n=decls->next(n)){
        if(decls->nth(n)->isCallDecl() == true){
                
            Symbol name = decls->nth(n)->getName();
            Symbol returnType = decls->nth(n)->getType();

            if(calls_rtType.probe(name) != NULL){
                semant_error(decls->nth(n))<<"Function "<<name<<" was previously defined."<<endl;
            } 
            if(isValidCallName(name) == false){
                semant_error(decls->nth(n))<<"Function printf cannot be redefination."<<endl;
            } 
            if(returnType!=Int && returnType!=Void && returnType!=String && returnType!=Float && returnType!=Bool){
                semant_error(decls->nth(n))<<"Function's returnType is out of scope."<<endl;
            } 

            calls_rtType.addid(name, new Symbol(returnType));
        }
    }
}

static void install_globalVars(Decls decls) {

    objectEnv.enterscope();

    for(int n=decls->first(); decls->more(n); n=decls->next(n)){
        if(decls->nth(n)->isCallDecl() == false){

            Symbol name = decls->nth(n)->getName();
            Symbol type = decls->nth(n)->getType();

            if(objectEnv.probe(name) != NULL){
                semant_error(decls->nth(n))<<"Global variable redefined."<<endl;
            }

            decls->nth(n)->check();
        }
    }
}

static void check_calls(Decls decls) {

    for(int n=decls->first(); decls->more(n); n=decls->next(n)){
        if(decls->nth(n)->isCallDecl() == true){
            return_InTheEnd = false;
            decls->nth(n)->check();
        }
    }
}

static void check_main() {
    if(calls_rtType.probe(Main) == NULL){
        semant_error()<<"Main function is not defined."<<endl;
    }
    if(*(calls_rtType.probe(Main)) != Void){
        semant_error()<<"Main function should have return type Void."<<endl;
    }
}

void VariableDecl_class::check() {

    Symbol name = this->getName();
    Symbol type = this->getType();

    if(type == Void){
        semant_error(this)<<"var "<<name<<" cannot be of type Void. Void can just be used as return type."<<endl;
    } 
    else{
        objectEnv.addid(name, new Symbol(type));
    }
}

void CallDecl_class::check() {

    Symbol Func_name = this->getName(); 
    Variables paras = this->getVariables();
    Symbol Func_returnType = this->getType();
    StmtBlock Func_stmtbody = this->getBody();

    objectEnv.enterscope();
    calls_paras.enterscope();

    if(paras->len() > 6){
        semant_error(this)<<"printf() can't have more than six called parameters ."<<endl;
    }

    calls_paras.addid(Func_name, new Variables(paras));

    if(Func_name == Main && paras->len() != 0){
        semant_error(this)<<"Main function should not have any parameters."<<endl;
    }

    for (int n=paras->first(); paras->more(n); n=paras->next(n)){

        Symbol paras_name = paras->nth(n)->getName();
        Symbol paras_type = paras->nth(n)->getType();

        if(paras_type == Void){
            semant_error(this)<<"Function "<<Func_name<<" 's parameter has an invalid type Void."<<endl;
        }
        else {
            if(objectEnv.probe(paras_name) != NULL){
                semant_error(this)<<"Function "<<Func_name<<" 's parameter has a duplicate name "<<paras_name<<"."<<endl;      
            }
            else objectEnv.addid(paras_name, new Symbol(paras_type));
        }
    }   

    Func_stmtbody->check(Func_returnType);
    
    if(return_InTheEnd == false){
        semant_error(this)<<"Function "<<Func_name<<" must have an overall return statement."<<endl;
    }

    calls_paras.exitscope();
    objectEnv.exitscope();
}

void StmtBlock_class::check(Symbol type) {

    VariableDecls vars = this->getVariableDecls();
	Stmts stmts = this->getStmts();

    objectEnv.enterscope();

    for(int n=vars->first(); vars->more(n); n=vars->next(n)){

        Symbol vars_name = vars->nth(n)->getName();
        Symbol vars_type = vars->nth(n)->getType();

        if(objectEnv.probe(vars_name) != NULL){
            semant_error(vars->nth(n))<<"var "<<vars_name<< "was previously defined."<<endl;
        }
        else vars->nth(n)->check();
    }

    for(int n=stmts->first(); stmts->more(n); n=stmts->next(n)){
        stmts->nth(n)->check(type);
    }

    objectEnv.exitscope();

}

void IfStmt_class::check(Symbol type) {

    Expr condition = this->getCondition();
    StmtBlock thenExpr = this->getThen();
    StmtBlock elseExpr = this->getElse();

    if(condition->checkType() != Bool){
        semant_error(this)<<"Condition must be a Bool, got "<<condition->checkType()<<endl;
    }

    thenexpr->check(type);
    elseexpr->check(type);
}

void WhileStmt_class::check(Symbol type) {

    Expr condition = this->getCondition();
	StmtBlock body = this->getBody();

    if(condition->checkType() != Bool){
        semant_error(this)<<"Condition must be a Bool, got "<<condition->checkType()<<endl;
    }
    
    
    loopBody.enterscope();

    loopBody.addid(1, new int(1));
    body->check(type);

    loopBody.exitscope();
}

void ForStmt_class::check(Symbol type) {

    Expr initExpr = this->getInit();
    Expr condition = this->getCondition();
    Expr loopact = this->getLoop();
    StmtBlock body = this->getBody();

    initExpr->check(type);

    if(condition->is_empty_Expr() == false){
        if(condition->checkType() != Bool){
        semant_error(this)<<"Condition must be a Bool, got "<<condition->checkType()<<endl;
        }
    }

    loopact->check(type);


    loopBody.enterscope();

    loopBody.addid(1, new int(1));
    body->check(type);

    loopBody.exitscope();
    
}

void ReturnStmt_class::check(Symbol type) {

    Expr value = this->getValue();

    if(value->checkType() != type)
        semant_error(this)<<"Returns "<<value->checkType()<<" , but need "<<type<<endl;
    
    if(return_InTheEnd == false){
        return_InTheEnd = true;
    }
}

void ContinueStmt_class::check(Symbol type) {

    if(loopBody.lookup(1) == NULL){
        semant_error(this)<<"continue must be used in a loop sentence."<<endl;
    }
}

void BreakStmt_class::check(Symbol type) {

    if(loopBody.lookup(1) == NULL){
        semant_error(this)<<"break must be used in a loop sentence."<<endl;
    }
}

Symbol Call_class::checkType(){

    Symbol name = this->getName();
    Actuals actuals = this->getActuals();

    Variables called_paras = NULL;
    Symbol called_rtType = NULL;

    if(name == print) {
            if(actuals->len() == 0){
                semant_error(this)<<"printf() must has at last one parameter of type String."<<endl;
            }
            else if(actuals->len() > 6){
                semant_error(this)<<"printf() can't have more than six actual parameters ."<<endl;
            }
            else if(actuals->nth(0)->getType() != String) {
                semant_error(this)<<"printf()'s first parameter must be of type String."<<endl;
            }
            this->setType(Void);
            return type;
    }


    if(calls_rtType.lookup(name) == NULL){
        semant_error(this)<<"Function "<<name<<" has not been defined."<<endl;
    }
    else{
        Variables called_paras = *calls_paras.lookup(name);
        Symbol called_rtType = *calls_rtType.lookup(name);
        this->setType(called_rtType);
    }  
    

    if(actuals->len() != called_paras->len()){
        semant_error(this)<<"Function "<<name<<" called with wrong number of arguments."<<endl;
    }

    else{
        for(int n=actuals->first(); actuals->more(n); n=actuals->next(n)){
            if(actuals->nth(n)->checkType() != called_paras->nth(n)->getType()){
                semant_error(this)<<"Function "<<name<<"'s actual parameter is different from called parameter with type."<<endl;
            }
        }
    }
    
    return type;
}

Symbol Actual_class::checkType(){

    this->setType(expr->checkType());
    return type;
}

Symbol Assign_class::checkType(){

    if(objectEnv.lookup(lvalue) == NULL){
        semant_error(this)<<"Left value "<<lvalue<<" has not been defined."<<endl;
    }

    Symbol lvalue_type = *(objectEnv.lookup(lvalue));
    Symbol value_type = value->checkType();

    if(lvalue_type != value_type){
        semant_error(this)<<"The type of right value is different from the type of left value."<<endl;
    }
    
    this->setType(lvalue_type);
    return(type);
}

Symbol Add_class::checkType(){

    Symbol a = e1->checkType();
    Symbol b = e2->checkType();

    if((a != Int && a != Float)  ||  (b != Int && b != Float)){
        semant_error(this)<<"Cannot add a "<<a<<" and a "<<b<<"."<<endl;
    }
    
    this->setType(Int);
    if(a == Float) this->setType(Float);
    if(b == Float) this->setType(Float);
    
    return type;
}

Symbol Minus_class::checkType(){
    
    Symbol a = e1->checkType();
    Symbol b = e2->checkType();

    if((a != Int && a != Float)  ||  (b != Int && b != Float)){
        semant_error(this)<<"Cannot minus a "<<a<<" and a "<<b<<"."<<endl;
    }
    
    this->setType(Int);
    if(a == Float) this->setType(Float);
    if(b == Float) this->setType(Float);
    
    return type;
}

Symbol Multi_class::checkType(){
    
    Symbol a = e1->checkType();
    Symbol b = e2->checkType();

    if((a != Int && a != Float)  ||  (b != Int && b != Float)){
        semant_error(this)<<"Cannot multiply a "<<a<<" and a "<<b<<"."<<endl;
    }

    this->setType(Int);
    if(a == Float) this->setType(Float);
    if(b == Float) this->setType(Float);
    
    return type;
}

Symbol Divide_class::checkType(){

    Symbol a = e1->checkType();
    Symbol b = e2->checkType();

    if((a != Int && a != Float)  ||  (b != Int && b != Float)){
        semant_error(this)<<"Cannot divide a "<<a<<" and a "<<b<<"."<<endl;
    }

    this->setType(Int);
    if(a == Float) this->setType(Float);
    if(b == Float) this->setType(Float);
    
    return type;
}

Symbol Mod_class::checkType(){

    Symbol a = e1->checkType();
    Symbol b = e2->checkType();

    if(a != Int || b != Int){
        semant_error(this)<<"Cannot mod a "<<a<<" and a "<<b<<"."<<endl;
    }

    this->setType(Int);
    return type;
}

Symbol Neg_class::checkType(){

    Symbol a = e1->checkType();

    if(a != Int || a != Float){
        semant_error(this)<<"A "<<a<<" doesn't have a negative."<<endl;
    }

    this->setType(a);
    return type;
}

Symbol Lt_class::checkType(){

    Symbol a = e1->checkType();
    Symbol b = e2->checkType();

    if((a != Int && a != Float)  ||  (b != Int && b != Float)){
        semant_error(this)<<"Cannot compare a "<<a<<" and a "<<b<<"(less)."<<endl;
    }

    this->setType(Bool);
    return type;
}

Symbol Le_class::checkType(){

    Symbol a = e1->checkType();
    Symbol b = e2->checkType();

    if((a != Int && a != Float)  ||  (b != Int && b != Float)){
        semant_error(this)<<"Cannot compare a "<<a<<" and a "<<b<<"(less than or equal)."<<endl;
    }

    this->setType(Bool);
    return type;
}

Symbol Equ_class::checkType(){

    Symbol a = e1->checkType();
    Symbol b = e2->checkType();

    if(((a != Int && a != Float)||(b != Int && b != Float)) && (a != Bool || b != Bool)){
        semant_error(this)<<"Cannot compare a "<<a<<" and a "<<b<<"(equal)."<<endl;
    }

    this->setType(Bool);
    return type;
}

Symbol Neq_class::checkType(){

    Symbol a = e1->checkType();
    Symbol b = e2->checkType();

    if(((a != Int && a != Float)||(b != Int && b != Float)) && (a != Bool || b != Bool)){
        semant_error(this)<<"Cannot compare a "<<a<<" and a "<<b<<"(not equal)."<<endl;
    }

    this->setType(Bool);
    return type;
}

Symbol Ge_class::checkType(){

    Symbol a = e1->checkType();
    Symbol b = e2->checkType();

    if((a != Int && a != Float)  ||  (b != Int && b != Float)){
        semant_error(this)<<"Cannot compare a "<<a<<" and a "<<b<<"(greater than or equal)."<<endl;
    }

    this->setType(Bool);
    return type;

}

Symbol Gt_class::checkType(){

    Symbol a = e1->checkType();
    Symbol b = e2->checkType();

    if((a != Int && a != Float)  ||  (b != Int && b != Float)){
        semant_error(this)<<"Cannot compare a "<<a<<" and a "<<b<<"(greater)."<<endl;
    }

    this->setType(Bool);
    return type;
}

Symbol And_class::checkType(){

    Symbol a = e1->checkType();
    Symbol b = e2->checkType();

    if((a != Bool || b != Bool)){
        semant_error(this)<<"Cannot use and(&&) between "<<a<<" and "<<b<<"."<<endl;
    }

    this->setType(Bool);
    return type;
}

Symbol Or_class::checkType(){

    Symbol a = e1->checkType();
    Symbol b = e2->checkType();

    if((a != Bool || b != Bool)){
        semant_error(this)<<"Cannot use or(||) between "<<a<<" and "<<b<<"."<<endl;
    }

    this->setType(Bool);
    return type;
}

Symbol Xor_class::checkType(){

    Symbol a = e1->checkType();
    Symbol b = e2->checkType();

    if(((a != Bool)||(b != Bool)) && ((a != Int)||(b != Int))){
        semant_error(this)<<"Cannot use xor(^) between "<<a<<" and "<<b<<"."<<endl;
    }

    this->setType(a);
    return type;
}

Symbol Not_class::checkType(){

    Symbol a = e1->checkType();

    if(a != Bool){
        semant_error(this)<<"Cannot use not(!) upon "<<a<<"."<<endl;
    }

    this->setType(Bool);
    return type;
}

Symbol Bitand_class::checkType(){

    Symbol a = e1->checkType();
    Symbol b = e2->checkType();

    if((a != Int || b != Int)){
        semant_error(this)<<"Cannot use bit-and(&) between "<<a<<" and "<<b<<"."<<endl;
    }

    this->setType(Int);
    return type;
}

Symbol Bitor_class::checkType(){

    Symbol a = e1->checkType();
    Symbol b = e2->checkType();

    if((a != Int || b != Int)){
        semant_error(this)<<"Cannot use bit-or(|) between "<<a<<" and "<<b<<"."<<endl;
    }

    this->setType(Int);
    return type;
}

Symbol Bitnot_class::checkType(){

    Symbol a = e1->checkType();

    if(a != Int){
        semant_error(this)<<"Cannot use unary not(~) upon "<<a<<"."<<endl;
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
    
    if(objectEnv.lookup(var) == NULL){
        semant_error(this)<<"object "<<var<<" has not been defined."<<endl;
    }
    
    this->setType(*(objectEnv.lookup(var)));
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



