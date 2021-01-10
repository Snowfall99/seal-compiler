
//**************************************************************
//
// Code generator SKELETON
//
//
//**************************************************************

#include "cgen.h"
#include "cgen_gc.h"
#include <vector>

using namespace std;

extern void emit_string_constant(ostream& str, char *s);
extern int cgen_debug;

static char *CALL_REGS[] = {RDI, RSI, RDX, RCX, R8, R9};
static char *CALL_XMM[] = {XMM0, XMM1, XMM2, XMM3};

void cgen_helper(Decls decls, ostream& s);
void code(Decls decls, ostream& s);

//////////////////////////////////////////////////////////////////
//
//
//    Helper Functions
//  
//
//////////////////////////////////////////////////////////////////
// variable name - memory
typedef SymbolTable<Symbol, int> variableTable;
variableTable variabletab;
// function - offset
typedef std::map<Symbol, int> functionTable;

int offset = 0;
int tempaddress = 0;
int labelNum = 0;
int continuePos = 0;
int breakPos = 0;
// you can add any helper functions here
static void emit_mrmovsd(const char *base_reg,int offset, const char *dest, ostream& s)
{
  s << MOVSD << offset << "(" << base_reg << ")" << COMMA << dest << endl;
}
static void emit_rmmovsd(const char *base_reg,int offset, const char *dest, ostream& s)
{
  s << MOVSD << base_reg << COMMA << offset << "(" << dest<< ")" << endl;
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
Symbol 
    Int,
    Float,
    String,
    Bool,
    Void,
    Main,
    print
    ;
//
// Initializing the predefined symbols.
//
static void initialize_constants(void)
{
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


//*********************************************************
//
// Define method for code generation
//
//
//*********************************************************

void Program_class::cgen(ostream &os) 
{
  // spim wants comments to start with '#'
  os << "# start of generated code\n";

  initialize_constants();
  cgen_helper(decls,os);

  os << "\n# end of generated code\n";
}


//////////////////////////////////////////////////////////////////////////////
//
//  emit_* procedures
//
//  emit_X  writes code for operation "X" to the output stream.
//  There is an emit_X for each opcode X, as well as emit_ functions
//  for generating names according to the naming conventions (see emit.h)
//  and calls to support functions defined in the trap handler.
//
//  Register names and addresses are passed as strings.  See `emit.h'
//  for symbolic names you can use to refer to the strings.
//
//////////////////////////////////////////////////////////////////////////////

static void emit_mov(const char *source, const char *dest, ostream& s)
{
  s << MOV << source << COMMA << dest << endl;
}

static void emit_rmmov(const char *source_reg, int offset, const char *base_reg, ostream& s)
{
  s << MOV << source_reg << COMMA << offset << "(" << base_reg << ")"
      << endl;
}

static void emit_mrmov(const char *base_reg, int offset, const char *dest_reg, ostream& s)
{
  s << MOV << offset << "(" << base_reg << ")" << COMMA << dest_reg  
      << endl;
}

static void emit_irmov(const char *immidiate, const char *dest_reg, ostream& s)
{
  s << MOV << "$" << immidiate << COMMA << dest_reg  
      << endl;
}

static void emit_irmovl(const char *immidiate, const char *dest_reg, ostream& s)
{
  s << MOVL << "$" << immidiate << COMMA << dest_reg  
      << endl;
}

static void emit_immov(const char *immidiate, int offset, const char *base_reg, ostream& s)
{
  s << MOV << "$" << immidiate << COMMA << "(" << offset << ")" << base_reg  
      << endl;
}

static void emit_add(const char *source_reg, const char *dest_reg, ostream& s)
{
  s << ADD << source_reg << COMMA << dest_reg << endl;
}

static void emit_sub(const char *source_reg, const char *dest_reg, ostream& s)
{
  s << SUB << source_reg << COMMA << dest_reg << endl;
}

static void emit_mul(const char *source_reg, const char *dest_reg, ostream& s)
{
  s << MUL << source_reg << COMMA << dest_reg << endl;
}

static void emit_div(const char *dest_reg, ostream& s)
{
  s << DIV << dest_reg << endl;
}

static void emit_cqto(ostream &s)
{
  s << CQTO << endl;
}

static void emit_neg(const char *dest_reg, ostream& s)
{
  s << NEG << dest_reg << endl;
}

static void emit_and(const char *source_reg, const char *dest_reg, ostream& s)
{
  s << AND << source_reg << COMMA << dest_reg << endl;
}

static void emit_or(const char *source_reg, const char *dest_reg, ostream& s)
{
  s << OR << source_reg << COMMA << dest_reg << endl;
}

static void emit_xor(const char *source_reg, const char *dest_reg, ostream& s)
{
  s << XOR << source_reg << COMMA << dest_reg << endl;
}

static void emit_not(const char *dest_reg, ostream& s)
{
  s << NOT << " " << dest_reg << endl;
}

static void emit_movsd(const char *source, const char *dest, ostream& s)
{
  s << MOVSD << source << COMMA << dest << endl;
}

static void emit_movaps(const char *source, const char *dest, ostream& s)
{
  s << MOVAPS << source << COMMA << dest << endl;
}

static void emit_addsd(const char *source_reg, const char *dest_reg, ostream& s)
{
  s << ADDSD << source_reg << COMMA << dest_reg << endl;
}

static void emit_subsd(const char *source_reg, const char *dest_reg, ostream& s)
{
  s << SUBSD << source_reg << COMMA << dest_reg << endl;
}

static void emit_mulsd(const char *source_reg, const char *dest_reg, ostream& s)
{
  s << MULSD << source_reg << COMMA << dest_reg << endl;
}

static void emit_divsd(const char *source_reg, const char *dest_reg, ostream& s)
{
  s << DIVSD << source_reg << COMMA << dest_reg << endl;
}

static void emit_cmp(const char *source_reg, const char *dest_reg, ostream& s)
{
  s << CMP << source_reg << COMMA << dest_reg << endl;
}

static void emit_test(const char *source_reg, const char *dest_reg, ostream& s)
{
  s << TEST << source_reg << COMMA << dest_reg << endl;
}

static void emit_ucompisd(const char *source_reg, const char *dest_reg, ostream& s)
{
  s << UCOMPISD << source_reg << COMMA << dest_reg << endl;
}

static void emit_xorpd(const char *source_reg, const char *dest_reg, ostream& s)
{
  s << XORPD << source_reg << COMMA << dest_reg << endl;
}
static void emit_jmp(const char *dest, ostream& s)
{
  s << JMP << " " << dest << endl;
}

static void emit_jl(const char *dest, ostream& s)
{
  s << JL << " " << dest << endl;
}

static void emit_jle(const char *dest, ostream& s)
{
  s << JLE << " " << dest << endl;
}

static void emit_je(const char *dest, ostream& s)
{
  s << JE << " " << dest << endl;
}

static void emit_jne(const char *dest, ostream& s)
{
  s << JNE << " " << dest << endl;
}

static void emit_jg(const char *dest, ostream& s)
{
  s << JG << " " << dest << endl;
}

static void emit_jge(const char *dest, ostream& s)
{
  s << JGE << " " << dest << endl;
}

static void emit_jb(const char *dest, ostream& s)
{
  s << JB << " " << dest << endl;
}

static void emit_jbe(const char *dest, ostream& s)
{
  s << JBE << " " << dest << endl;
}

static void emit_ja(const char *dest, ostream& s)
{
  s << JA << " " << dest << endl;
}

static void emit_jae(const char *dest, ostream& s)
{
  s << JAE << " " << dest << endl;
}

static void emit_jp(const char *dest, ostream& s)
{
  s << JP << " " << dest << endl;
}

static void emit_jz(const char *dest, ostream& s)
{
  s << JZ << " " << dest << endl;
}

static void emit_jnz(const char *dest, ostream& s)
{
  s << JNZ << " " << dest << endl;
}

static void emit_call(const char *dest, ostream& s)
{
  s << CALL << " " << dest << endl;
}

static void emit_ret(ostream& s)
{
  s << RET << endl;
}

static void emit_push(const char *reg, ostream& s)
{
  s << PUSH << " " << reg << endl;
}

static void emit_pop(const char *reg, ostream& s)
{
  s << POP << " " << reg << endl;
}

static void emit_leave(ostream& s)
{
  s << LEAVE << endl;
}

static void emit_position(const char *p, ostream& s)
{
  s << p << ":" << endl;
}

static void emit_float_to_int(const char *float_mmx, const char *int_reg, ostream& s)
{
  s << CVTTSD2SIQ << float_mmx << COMMA << int_reg << endl;
}

static void emit_int_to_float(const char *int_reg, const char *float_mmx, ostream& s)
{
  s << CVTSI2SDQ << int_reg << COMMA << float_mmx << endl;
}
///////////////////////////////////////////////////////////////////////////////
//
// coding strings, ints, and booleans
//
// Seal has four kinds of constants: strings, ints, and booleans.
// This section defines code generation for each type.
//
// If you like, you can add any ***Entry::code_def() and ***Entry::code_ref()
// functions to help.
//
///////////////////////////////////////////////////////////////////////////////

//
// Strings
//
void StringEntry::code_ref(ostream& s)
{
  s << "$" << STRINGCONST_PREFIX << index;
}

//
// Emit code for a constant String.
//

void StringEntry::code_def(ostream& s)
{
  s << STRINGCONST_PREFIX << index << ":" << endl;
  s  << STRINGTAG ; emit_string_constant(s,str);                                                // align to word
}

//
// StrTable::code_string
// Generate a string object definition for every string constant in the 
// stringtable.
//
void StrTable::code_string_table(ostream& s)
{  
  for (List<StringEntry> *l = tbl; l; l = l->tl())
    l->hd()->code_def(s);
}

// the following 2 functions are useless, please DO NOT care about them
void FloatEntry::code_ref(ostream &s)
{
  s << FLOATTAG << index;
}

void IntEntry::code_def(ostream &s)
{
  s << GLOBAL;
}

//***************************************************
//
//  Emit global var and functions.
//
//***************************************************

static void emit_global_int(Symbol name, ostream& s) {
  s << GLOBAL << name << endl << 
  ALIGN << 8 << endl << 
  SYMBOL_TYPE << name << COMMA << OBJECT << endl <<
  SIZE << name << COMMA << 8 << endl << 
  name << ":" << endl << 
  INTTAG << 0 << endl;
}

static void emit_global_float(Symbol name, ostream& s) {
  s << GLOBAL << name << endl << 
  ALIGN << 8 << endl << 
  SYMBOL_TYPE << name << COMMA << OBJECT << endl <<
  SIZE << name << COMMA << 8 << endl << 
  name << ":" << endl <<
  FLOATTAG << 0 << endl <<
  FLOATTAG << 0 << endl;
}

static void emit_global_bool(Symbol name, ostream& s) {
  s << GLOBAL << name << endl << 
  ALIGN << 8 << endl << 
  SYMBOL_TYPE << name << COMMA << OBJECT << endl <<
  SIZE << name << COMMA << 8 << endl << 
  name << ":" << endl << 
  BOOLTAG << 0 << endl;
}

void code_global_data(Decls decls, ostream &str) {
  int count = 0;
  for (int i=decls->first(); decls->more(i); i=decls->next(i)) {
    if (!decls->nth(i)->isCallDecl()) {
      count ++;
      if (count == 1) str<<DATA<<endl;
      Symbol name = decls->nth(i)->getName();
      Symbol type = decls->nth(i)->getType();
      if (type == Int) {
        emit_global_int(name, str);
      } else if (type == Bool) {
        emit_global_bool(name, str);
      } else if (type == Float) {
        emit_global_float(name, str);
      }
    }
  }
}

void code_calls(Decls decls, ostream &str) {
  str<<SECTION<<RODATA<<endl;
  stringtable.code_string_table(str);
  str<<TEXT<<endl;
  for (int i=decls->first(); decls->more(i); i=decls->next(i)) {
    if (decls->nth(i)->isCallDecl()) {
      if (decls->nth(i)->getName() == Main) {
        offset = tempaddress = 0;
      } else {
        offset = tempaddress = -56;
      }
      decls->nth(i)->code(str);
    }
  }
}

//***************************************************
//
//  Emit code to start the .text segment and to
//  declare the global names.
//
//***************************************************



//********************************************************
//
// Cgen helper helps to initialize and call code() function.
// You can do any initializing operations here
//
//********************************************************

void cgen_helper(Decls decls, ostream& s)
{

  code(decls, s);
}


void code(Decls decls, ostream& s)
{
  if (cgen_debug) cout << "Coding global data" << endl;
  code_global_data(decls, s);

  if (cgen_debug) cout << "Coding calls" << endl;
  code_calls(decls, s);
}

//******************************************************************
//
//   Fill in the following methods to produce code for the
//   appropriate expression.  You may add or remove parameters
//   as you wish, but if you do, remember to change the parameters
//   of the declarations in `seal-decl.h', `seal-expr.h' and `seal-stmt.h'
//   Sample code for constant integers, strings, and booleans are provided.
//   
//*****************************************************************

void CallDecl_class::code(ostream &s) {
  variabletab.enterscope();

  s<<GLOBAL<<name<<endl<<
  SYMBOL_TYPE<<name<<COMMA<<FUNCTION<<endl;

  s<<name<<":"<<endl;
  emit_push(RBP, s);
  emit_mov(RSP, RBP, s);
  emit_push(RBX, s);
  emit_push(R10, s);
  emit_push(R11, s);
  emit_push(R12, s);
  emit_push(R13, s);
  emit_push(R14, s);
  emit_push(R15, s);  

  // paras
  int int_num = 0;
  int float_num = 0;
  for (int i=paras->first(); paras->more(i); i=paras->next(i)) {
    Symbol name = paras->nth(i)->getName();
    Symbol type = paras->nth(i)->getType();
    if (type == Int || type == Bool) {
      emit_sub("$8", RSP, s);
      offset -= 8;

      variabletab.addid(name, new int(offset));
      s << MOV << CALL_REGS[int_num ++] << COMMA << offset << '(' << RBP << ')'<<endl;
    } else if (type == Float) {
      emit_sub("$8", RSP, s);
      offset -= 8;

      variabletab.addid(name, new int(offset));
      s << MOV << CALL_XMM[float_num ++] << COMMA << offset << '(' << RBP << ')' <<endl;
    }
  }

  // body
  body->code(s);

  s<<SIZE<<name<<", "<<".-"<<name<<endl;
  variabletab.exitscope();
}

void StmtBlock_class::code(ostream &s){
  // variable decls
  for (int i=vars->first(); vars->more(i); i=vars->next(i)) {
    Symbol name = vars->nth(i)->getName();

    offset -= 8;
    variabletab.addid(name, new int(offset));
    
    emit_sub("$8", RSP, s);
  }

  for (int i=stmts->first(); stmts->more(i); i=stmts->next(i)) {
    stmts->nth(i)->code(s);
  }
}

void IfStmt_class::code(ostream &s) {
  condition->code(s);
  emit_mrmov(RBP, tempaddress, RAX, s);
  emit_test(RAX, RAX, s);
  int else_pos = labelNum ++;
  int then_pos = labelNum ++;
  s<<JZ<<" "<<POSITION<<else_pos<<endl;
  thenexpr->code(s);
  s<<JMP<<" "<<POSITION<<then_pos<<endl;
  s<<POSITION<<else_pos<<":"<<endl;
  elseexpr->code(s);
  s<<POSITION<<then_pos<<":"<<endl;
}

void WhileStmt_class::code(ostream &s) {
  int condition_pos = labelNum ++;
  int end_pos = labelNum ++;
  continuePos = condition_pos;
  breakPos = end_pos;

  s<<POSITION<<condition_pos<<":"<<endl;
  condition->code(s);
  emit_mrmov(RBP, tempaddress, RAX, s);
  emit_test(RAX, RAX, s);
  s<<JZ<<' '<<POSITION<<end_pos<<endl;
  body->code(s);
  s<<JMP<<' '<<POSITION<<condition_pos<<endl;
  s<<POSITION<<end_pos<<":"<<endl;
}

void ForStmt_class::code(ostream &s) {
  int condition_pos = labelNum ++;
  int expr_pos = labelNum ++;
  int end_pos = labelNum ++;
  continuePos = expr_pos;
  breakPos = end_pos;

  initexpr->code(s);
  s<<POSITION<<condition_pos<<":"<<endl;
  condition->code(s);
  emit_mrmov(RBP, tempaddress, RAX, s);
  emit_test(RAX, RAX, s);
  s<<JZ<<" "<<POSITION<<end_pos<<endl;
  body->code(s);
  s<<POSITION<<expr_pos<<":"<<endl;
  loopact->code(s);
  s<<JMP<<" "<<POSITION<<condition_pos<<endl;
  s<<POSITION<<end_pos<<":"<<endl;
}

void ReturnStmt_class::code(ostream &s) {
  value->code(s);
  if (value->getType()->get_string() == Float->get_string()) {
    s<<MOVAPS<<tempaddress<<"("<<RBP<<"), "<<XMM0<<endl;
  } else if (value->getType()->get_string() != Void->get_string()) {
    emit_mrmov(RBP, tempaddress, RAX, s);
  }

  emit_pop(R15, s);
  emit_pop(R14, s);
  emit_pop(R13, s);
  emit_pop(R12, s);
  emit_pop(R11, s);
  emit_pop(R10, s);
  emit_pop(RBX, s);

  s<<LEAVE<<endl
  <<RET<<endl;
}

void ContinueStmt_class::code(ostream &s) {
  s<<JMP<<" "<<POSITION<<continuePos<<endl;
}

void BreakStmt_class::code(ostream &s) {
  s<<JMP<<" "<<POSITION<<breakPos<<endl;
}

void Call_class::code(ostream &s) {
  int int_num = 0;
  int float_num = 0;
  int addr[actuals->len()];
  int num = 0;

  for (int i=actuals->first(); actuals->more(i); i=actuals->next(i)) {
    if (actuals->nth(i)->getType()->get_string() == Int->get_string() || actuals->nth(i)->getType()->get_string() == Bool->get_string() || actuals->nth(i)->getType()->get_string() == String->get_string()) {
      actuals->nth(i)->code(s);
      addr[i] = tempaddress;
    }

    if (actuals->nth(i)->getType()->get_string() == Float->get_string()) {
      num ++;
      actuals->nth(i)->code(s);
      addr[i] = tempaddress;
    }
  }

  for (int i=actuals->first(); actuals->more(i); i=actuals->next(i)) {
    if (actuals->nth(i)->getType()->get_string() == Int->get_string() || actuals->nth(i)->getType()->get_string() == Bool->get_string() || actuals->nth(i)->getType()->get_string() == String->get_string()) {
      s<<MOV<<addr[i]<<"("<<RBP<<")"<<COMMA<<CALL_REGS[int_num ++]<<endl;
    } else if (actuals->nth(i)->getType()->get_string() == Float->get_string()) {
      s<<MOVSD<<addr[i]<<"("<<RBP<<")"<<COMMA<<CALL_XMM[float_num ++]<<endl;
    }
  }

  if (name == print) {
    emit_sub("$8", RSP, s);
    offset -= 8;
    s<<MOVL<<"$"<<num<<COMMA<<EAX<<endl;
    emit_call("printf", s);
  } else if(type->get_string() == Int->get_string() || type->get_string() == Bool->get_string() || type->get_string() == String->get_string()){
    emit_call(name->get_string(), s);
    emit_sub("$8", RSP, s);
    offset -= 8;
    tempaddress = offset;
    emit_rmmov(RAX, offset, RBP, s);
  } else if (type->get_string() == Float->get_string()) {
    emit_call(name->get_string(), s);
    emit_sub("$8", RSP, s);
    offset -= 8;
    tempaddress = offset;
    emit_rmmovsd(XMM0, offset, RBP, s);
  }
  //
  /*
   if function name is printf

    // please set %eax to the number of Float parameters, num.
    //  把%eax赋值为Float类型的参数个数, num
    emit_sub("$8", RSP, s);
    emit_irmovl(num, EAX, s);
    emit_call("printf", s);
      
    return;
  }
  */
  //
}

void Actual_class::code(ostream &s) {
  expr->code(s);
}

void Assign_class::code(ostream &s) {
  value->code(s);
  emit_mrmov(RBP, tempaddress, RAX, s);
  
  variabletab.enterscope();
  tempaddress = *variabletab.lookup(lvalue);
  variabletab.exitscope();

  emit_rmmov(RAX, tempaddress, RBP, s);
}

void Add_class::code(ostream &s) {
  e1->code(s);
  int addr1 = tempaddress;
  e2->code(s);
  int addr2 = tempaddress;
  emit_sub("$8", RSP, s);
  offset -= 8;
  tempaddress = offset;
  if (e1->getType()->get_string() == Int->get_string() && e2->getType()->get_string() == Int->get_string()) {
    emit_mrmov(RBP, addr1, RBX, s);
    emit_mrmov(RBP, addr2, R10, s);
    emit_add(R10, RBX, s);
    emit_rmmov(RBX, offset, RBP, s);
  } else if (e1->getType()->get_string() == Float->get_string() && e2->getType()->get_string() == Float->get_string()) {
    emit_mrmovsd(RBP, addr1, XMM4, s);
    emit_mrmovsd(RBP, addr2, XMM5, s);
    emit_addsd(XMM5, XMM4, s);
    emit_rmmovsd(XMM4, offset, RBP, s);
  } else if (e1->getType()->get_string() == Int->get_string() && e2->getType()->get_string() == Float->get_string()) {
    emit_mrmov(RBP, addr1, RBX, s);
    emit_mrmovsd(RBP, addr2, XMM5, s);
    emit_int_to_float(RBX, XMM4, s);
    emit_addsd(XMM5, XMM4, s);
    emit_rmmovsd(XMM4, offset, RBP, s);
  } else if (e1->getType()->get_string() == Float->get_string() && e2->getType()->get_string() == Int->get_string()) {
    emit_mrmovsd(RBP, addr1, XMM4, s);
    emit_mrmov(RBP, addr2, RBX, s);
    emit_int_to_float(RBX, XMM5, s);
    emit_addsd(XMM5, XMM4, s);
    emit_rmmovsd(XMM4, offset, RBP, s);
  }
}

void Minus_class::code(ostream &s) {
  e1->code(s);
  int addr1 = tempaddress;
  e2->code(s);
  int addr2 = tempaddress;
  emit_sub("$8", RSP, s);
  offset -= 8;
  tempaddress = offset;
  if (e1->getType()->get_string() == Int->get_string() && e2->getType()->get_string() == Int->get_string()) {
    emit_mrmov(RBP, addr1, RBX, s);
    emit_mrmov(RBP, addr2, R10, s);
    emit_sub(R10, RBX, s);
    emit_rmmov(RBX, offset, RBP, s);
  } else if (e1->getType()->get_string() == Float->get_string() && e2->getType()->get_string() == Float->get_string()) {
    emit_mrmovsd(RBP, addr1, XMM4, s);
    emit_mrmovsd(RBP, addr2, XMM5, s);
    emit_subsd(XMM5, XMM4, s);
    emit_rmmovsd(XMM4, offset, RBP, s);
  } else if (e1->getType()->get_string() == Int->get_string() && e2->getType()->get_string() == Float->get_string()) {
    emit_mrmov(RBP, addr1, RBX, s);
    emit_mrmovsd(RBP, addr2, XMM5, s);
    emit_int_to_float(RBX, XMM4, s);
    emit_subsd(XMM5, XMM4, s);
    emit_rmmovsd(XMM4, offset, RBP, s);
  } else if (e1->getType()->get_string() == Float->get_string() && e2->getType()->get_string() == Int->get_string()) {
    emit_mrmovsd(RBP, addr1, XMM4, s);
    emit_mrmov(RBP, addr2, RBX, s);
    emit_int_to_float(RBX, XMM5, s);
    emit_subsd(XMM5, XMM4, s);
    emit_rmmovsd(XMM4, offset, RBP, s);
  }
}

void Multi_class::code(ostream &s) {
  e1->code(s);
  int addr1 = tempaddress;
  e2->code(s);
  int addr2 = tempaddress;
  emit_sub("$8", RSP, s);
  offset -= 8;
  tempaddress = offset;
  if (e1->getType()->get_string() == Int->get_string() && e2->getType()->get_string() == Int->get_string()) {
    emit_mrmov(RBP, addr1, RBX, s);
    emit_mrmov(RBP, addr2, R10, s);
    emit_mul(R10, RBX, s);
    emit_rmmov(RBX, offset, RBP, s);
  } else if (e1->getType()->get_string() == Float->get_string() && e2->getType()->get_string() == Float->get_string()) {
    emit_mrmovsd(RBP, addr1, XMM4, s);
    emit_mrmovsd(RBP, addr2, XMM5, s);
    emit_mulsd(XMM5, XMM4, s);
    emit_rmmovsd(XMM4, offset, RBP, s);
  } else if (e1->getType()->get_string() == Int->get_string() && e2->getType()->get_string() == Float->get_string()) {
    emit_mrmov(RBP, addr1, RBX, s);
    emit_mrmovsd(RBP, addr2, XMM5, s);
    emit_int_to_float(RBX, XMM4, s);
    emit_mulsd(XMM5, XMM4, s);
    emit_rmmovsd(XMM4, offset, RBP, s);
  } else if (e1->getType()->get_string() == Float->get_string() && e2->getType()->get_string() == Int->get_string()) {
    emit_mrmovsd(RBP, addr1, XMM4, s);
    emit_mrmov(RBP, addr2, RBX, s);
    emit_int_to_float(RBX, XMM5, s);
    emit_mulsd(XMM5, XMM4, s);
    emit_rmmovsd(XMM4, offset, RBP, s);
  }
}

void Divide_class::code(ostream &s) {
  e1->code(s);
  int addr1 = tempaddress;
  e2->code(s);
  int addr2 = tempaddress;
  emit_sub("$8", RSP, s);
  offset -= 8;
  tempaddress = offset;
  if (e1->getType()->get_string() == Int->get_string() && e2->getType()->get_string() == Int->get_string()) {
    emit_mrmov(RBP, addr1, RAX, s);
    emit_cqto(s);
    emit_mrmov(RBP, addr2, RBX, s);
    emit_div(RBX, s);
    emit_rmmov(RAX, offset, RBP, s);
  } else if (e1->getType()->get_string() == Float->get_string() && e2->getType()->get_string() == Float->get_string()) {
    emit_mrmovsd(RBP, addr1, XMM4, s);
    emit_mrmovsd(RBP, addr2, XMM5, s);
    emit_divsd(XMM5, XMM4, s);
    emit_rmmovsd(XMM4, offset, RBP, s);
  } else if (e1->getType()->get_string() == Int->get_string() && e2->getType()->get_string() == Float->get_string()) {
    emit_mrmov(RBP, addr1, RBX, s);
    emit_mrmovsd(RBP, addr2, XMM5, s);
    emit_int_to_float(RBX, XMM4, s);
    emit_divsd(XMM5, XMM4, s);
    emit_rmmovsd(XMM4, offset, RBP, s);
  } else if (e1->getType()->get_string() == Float->get_string() && e2->getType()->get_string() == Int->get_string()) {
    emit_mrmovsd(RBP, addr1, XMM4, s);
    emit_mrmov(RBP, addr2, RBX, s);
    emit_int_to_float(RBX, XMM5, s);
    emit_divsd(XMM5, XMM4, s);
    emit_rmmovsd(XMM4, offset, RBP, s);
  }
}

void Mod_class::code(ostream &s) {
  e1->code(s);
  int addr1 = tempaddress;
  e2->code(s);
  int addr2 = tempaddress;
  emit_sub("$8", RSP, s); 
  offset -= 8;
  tempaddress = offset;

  emit_mrmov(RBP, addr1, RAX, s);
  emit_cqto(s);
  emit_mrmov(RBP, addr2, RBX, s);
  emit_div(RBX, s);
  emit_rmmov(RDX, offset, RBP, s);
}

void Neg_class::code(ostream &s) {
  e1->code(s);
  int addr1 = tempaddress;
  emit_sub("$8", RSP, s);
  offset -= 8;
  tempaddress = offset;

  if (e1->getType()->get_string() == Int->get_string()) {
    emit_mrmov(RBP, addr1, RAX, s);
    emit_neg(RAX, s);
    emit_rmmov(RAX, offset, RBP, s);
  } else {
    emit_sub("$8", RSP, s);
    emit_mov("$0x8000000000000000", RAX, s);
    emit_mrmov(RBP, offset, RDX, s);
    emit_xor(RAX, RDX, s);
    emit_rmmov(RDX, offset, RBP, s);
  }
}

void Lt_class::code(ostream &s) {
  e1->code(s);
  int addr1 = tempaddress;
  e2->code(s);
  int addr2 = tempaddress;

  emit_sub("$8", RSP, s);
  offset -= 8;
  tempaddress = offset;
  if (e1->getType()->get_string() == Int->get_string() && e2->getType()->get_string() == Int->get_string()) {
    emit_mrmov(RBP, addr1, RAX, s);
    emit_mrmov(RBP, addr2, RDX, s);
    emit_cmp(RDX, RAX, s);
    int pos1 = labelNum ++;
    int pos2 = labelNum ++;
    s<<JL<<" "<<POSITION<<pos1<<endl;
    emit_mov("$0", RAX, s);
    s<<JMP<<" "<<POSITION<<pos2<<endl;
    s<<POSITION<<pos1<<":"<<endl;
    emit_mov("$1", RAX, s);
    s<<POSITION<<pos2<<":"<<endl;
    emit_rmmov(RAX, offset, RBP, s);
  } else if (e1->getType()->get_string() == Float->get_string() && e2->getType()->get_string() == Int->get_string()) {
    emit_mrmovsd(RBP, addr1, XMM1, s);
    emit_mrmov(RBP, addr2, RAX, s);
    emit_int_to_float(RAX, XMM0, s);
    emit_ucompisd(XMM0, XMM1, s);
    int pos1 = labelNum ++;
    int pos2 = labelNum ++;
    s<<JB<<" "<<POSITION<<pos1<<endl;
    emit_mov("$0", RAX, s);
    s<<JMP<<" "<<POSITION<<pos2<< endl;
    s<<POSITION<<pos1<<":"<<endl;
    emit_mov("$1", RAX, s);
    s<<POSITION<<pos2<<":"<<endl;
    emit_rmmov(RAX, offset, RBP, s);
  } else if (e1->getType()->get_string() == Int->get_string() && e2->getType()->get_string() == Float->get_string()) {
    emit_mrmov(RBP, addr1, RAX, s);
    emit_int_to_float(RAX, XMM0, s);
    emit_mrmovsd(RBP, addr2, XMM1, s);
    emit_ucompisd(XMM0, XMM1, s);
    int pos1 = labelNum ++;
    int pos2 = labelNum ++;
    s<<JB<<" "<<POSITION<<pos1<<endl;
    emit_mov("$0",RAX,s);
    s<<JMP<<" "<<POSITION<<pos2<<endl;
    s<<POSITION<<pos1<<":"<<endl;
    emit_mov("$1",RAX,s);
    s<<POSITION<<pos2<<":"<<endl;
    emit_rmmov(RAX, 0, RBP, s);
  } else if (e1->getType()->get_string() == Float->get_string() && e2->getType()->get_string() == Float->get_string()) {
    emit_mrmovsd(RBP, addr1, XMM0, s);
    emit_mrmovsd(RBP, addr2, XMM1, s);
    emit_ucompisd(XMM0, XMM1, s);
    int pos1 = labelNum ++;
    int pos2 = labelNum ++;
    s<<JB<<" "<<POSITION<<pos1<<endl;
    emit_mov("$0",RAX,s);
    s<<JMP<<" "<<POSITION<<pos2<<endl;
    s<<POSITION<<pos1<<":"<<endl;
    emit_mov("$1",RAX,s);
    s<<POSITION<<pos2<<":"<<endl;
    emit_rmmov(RAX,offset,RBP,s);
  }
}

void Le_class::code(ostream &s) {
  e1->code(s);
  int addr1 = tempaddress;
  e2->code(s);
  int addr2 = tempaddress;

  emit_sub("$8", RSP, s);
  offset -= 8;
  tempaddress = offset;
  if (e1->getType()->get_string() == Int->get_string() && e2->getType()->get_string() == Int->get_string()) {
    emit_mrmov(RBP,addr1,RAX,s);
    emit_mrmov(RBP,addr2,RDX,s);
    emit_cmp(RDX,RAX,s);
    int pos1 = labelNum ++;
    int pos2 = labelNum ++;
    s<<JLE<<" "<<POSITION<<pos1<<endl;
    emit_mov("$0", RAX, s);
    s<<JMP<<" "<<POSITION<<pos2<< endl;
    s<<POSITION<<pos1<<":"<<endl;
    emit_mov("$1",RAX,s);
    s<<POSITION<<pos2<<":"<<endl;
    emit_rmmov(RAX,offset,RBP,s);
  } else if (e1->getType()->get_string() == Float->get_string() && e2->getType()->get_string() == Int->get_string()) {
    emit_mrmovsd(RBP, addr1, XMM1, s);
    emit_mrmov(RBP, addr2, RAX, s);
    emit_int_to_float(RAX, XMM0, s);
    emit_ucompisd(XMM0, XMM1, s);
    int pos1 = labelNum ++;
    int pos2 = labelNum ++;
    s<<JBE<<" "<<POSITION<<pos1<<endl;
    emit_mov("$0",RAX,s);
    s<<JMP<<" "<<POSITION<<pos2<< endl;
    s<<POSITION<<pos1<<":"<<endl;
    emit_mov("$1",RAX,s);
    s<<POSITION<<pos2<<":"<<endl;
    emit_rmmov(RAX,offset,RBP,s);
  } else if (e1->getType()->get_string() == Int->get_string() && e2->getType()->get_string() == Float->get_string()) {
    emit_mrmov(RBP,addr1,RAX,s);
    emit_int_to_float(RAX, XMM0, s);
    emit_mrmovsd(RBP, addr2, XMM1, s);
    emit_ucompisd(XMM0, XMM1, s);
    int pos1 = labelNum ++;
    int pos2 = labelNum ++;
    s<<JBE<<" "<<POSITION<<pos1<<endl;
    emit_mov("$0",RAX,s);
    s<<JMP<<" "<<POSITION<<pos2<<endl;
    s<<POSITION<<pos1<<":"<<endl;
    emit_mov("$1",RAX,s);
    s<<POSITION<<pos2<<":"<<endl;
    emit_rmmov(RAX,0,RBP,s);
  } else if (e1->getType()->get_string() == Float->get_string() && e2->getType()->get_string() == Float->get_string()) {
    emit_mrmovsd(RBP, addr1, XMM0, s);
    emit_mrmovsd(RBP, addr2, XMM1, s);
    emit_ucompisd(XMM0, XMM1,s);
    int pos1 = labelNum ++;
    int pos2 = labelNum ++;
    s<<JBE<<" "<<POSITION<<pos1<<endl;
    emit_mov("$0",RAX,s);
    s<<JMP<<" "<<POSITION<<pos2<<endl;
    s<<POSITION<<pos1<<":"<<endl;
    emit_mov("$1",RAX,s);
    s<<POSITION<<pos2<<":"<<endl;
    emit_rmmov(RAX,offset,RBP,s);
  }
}

void Equ_class::code(ostream &s) {
  e1->code(s);
  int addr1 = tempaddress;
  e2->code(s);
  int addr2 = tempaddress;

  emit_sub("$8", RSP, s);
  offset -= 8;
  tempaddress = offset;
  if (e1->getType()->get_string() == Int->get_string() && e2->getType()->get_string() == Int->get_string()) {
    emit_mrmov(RBP,addr1,RAX,s);
    emit_mrmov(RBP,addr2,RDX,s);
    emit_cmp(RDX,RAX,s);
    int pos1 = labelNum ++;
    int pos2 = labelNum ++;
    s<<JE<<" "<<POSITION<<pos1<<endl;
    emit_mov("$0", RAX, s);
    s<<JMP<<" "<<POSITION<<pos2<< endl;
    s<<POSITION<<pos1<<":"<<endl;
    emit_mov("$1",RAX,s);
    s<<POSITION<<pos2<<":"<<endl;
    emit_rmmov(RAX,offset,RBP,s);
  } else if (e1->getType()->get_string() == Float->get_string() && e2->getType()->get_string() == Int->get_string()) {
    emit_mrmovsd(RBP, addr1, XMM1, s);
    emit_mrmov(RBP, addr2, RAX, s);
    emit_int_to_float(RAX, XMM0, s);
    emit_ucompisd(XMM0, XMM1, s);
    int pos1 = labelNum ++;
    int pos2 = labelNum ++;
    s<<JE<<" "<<POSITION<<pos1<<endl;
    emit_mov("$0",RAX,s);
    s<<JMP<<" "<<POSITION<<pos2<< endl;
    s<<POSITION<<pos1<<":"<<endl;
    emit_mov("$1",RAX,s);
    s<<POSITION<<pos2<<":"<<endl;
    emit_rmmov(RAX,offset,RBP,s);
  } else if (e1->getType()->get_string() == Int->get_string() && e2->getType()->get_string() == Float->get_string()) {
    emit_mrmov(RBP,addr1,RAX,s);
    emit_int_to_float(RAX, XMM0, s);
    emit_mrmovsd(RBP, addr2, XMM1, s);
    emit_ucompisd(XMM0, XMM1, s);
    int pos1 = labelNum ++;
    int pos2 = labelNum ++;
    s<<JE<<" "<<POSITION<<pos1<<endl;
    emit_mov("$0",RAX,s);
    s<<JMP<<" "<<POSITION<<pos2<<endl;
    s<<POSITION<<pos1<<":"<<endl;
    emit_mov("$1",RAX,s);
    s<<POSITION<<pos2<<":"<<endl;
    emit_rmmov(RAX,0,RBP,s);
  } else if (e1->getType()->get_string() == Float->get_string() && e2->getType()->get_string() == Float->get_string()) {
    emit_mrmovsd(RBP, addr1, XMM0, s);
    emit_mrmovsd(RBP, addr2, XMM1, s);
    emit_ucompisd(XMM0, XMM1, s);
    int pos1 = labelNum ++;
    int pos2 = labelNum ++;
    s<<JE<<" "<<POSITION<<pos1<<endl;
    emit_mov("$0",RAX,s);
    s<<JMP<<" "<<POSITION<<pos2<<endl;
    s<<POSITION<<pos1<<":"<<endl;
    emit_mov("$1",RAX,s);
    s<<POSITION<<pos2<<":"<<endl;
    emit_rmmov(RAX,offset,RBP,s);
  }
}

void Neq_class::code(ostream &s) {
  e1->code(s);
  int addr1 = tempaddress;
  e2->code(s);
  int addr2 = tempaddress;

  emit_sub("$8", RSP, s);
  offset -= 8;
  tempaddress = offset;
  if (e1->getType()->get_string() == Int->get_string() && e2->getType()->get_string() == Int->get_string()) {
    emit_mrmov(RBP,addr1,RAX,s);
    emit_mrmov(RBP,addr2,RDX,s);
    emit_cmp(RDX,RAX,s);
    int pos1 = labelNum ++;
    int pos2 = labelNum ++;
    s<<JNE<<" "<<POSITION<<pos1<<endl;
    emit_mov("$0", RAX, s);
    s<<JMP<<" "<<POSITION<<pos2<< endl;
    s<<POSITION<<pos1<<":"<<endl;
    emit_mov("$1",RAX,s);
    s<<POSITION<<pos2<<":"<<endl;
    emit_rmmov(RAX,offset,RBP,s);
  } else if (e1->getType()->get_string() == Float->get_string() && e2->getType()->get_string() == Int->get_string()) {
    emit_mrmovsd(RBP, addr1, XMM1, s);
    emit_mrmov(RBP, addr2, RAX, s);
    emit_int_to_float(RAX, XMM0, s);
    emit_ucompisd(XMM0, XMM1, s);
    int pos1 = labelNum ++;
    int pos2 = labelNum ++;
    s<<JNE<<" "<<POSITION<<pos1<<endl;
    emit_mov("$0",RAX,s);
    s<<JMP<<" "<<POSITION<<pos2<< endl;
    s<<POSITION<<pos1<<":"<<endl;
    emit_mov("$1",RAX,s);
    s<<POSITION<<pos2<<":"<<endl;
    emit_rmmov(RAX,offset,RBP,s);
  } else if (e1->getType()->get_string() == Int->get_string() && e2->getType()->get_string() == Float->get_string()) {
    emit_mrmov(RBP, addr1, RAX, s);
    emit_int_to_float(RAX, XMM0, s);
    emit_mrmovsd(RBP, addr2, XMM1, s);
    emit_ucompisd(XMM0, XMM1, s);
    int pos1 = labelNum ++;
    int pos2 = labelNum ++;
    s<<JNE<<" "<<POSITION<<pos1<<endl;
    emit_mov("$0",RAX,s);
    s<<JMP<<" "<<POSITION<<pos2<<endl;
    s<<POSITION<<pos1<<":"<<endl;
    emit_mov("$1",RAX,s);
    s<<POSITION<<pos2<<":"<<endl;
    emit_rmmov(RAX,0,RBP,s);
  } else if (e1->getType()->get_string() == Float->get_string() && e2->getType()->get_string() == Float->get_string()) {
    emit_mrmovsd(RBP, addr1, XMM0, s);
    emit_mrmovsd(RBP, addr2, XMM1, s);
    emit_ucompisd(XMM0, XMM1, s);
    int pos1 = labelNum ++;
    int pos2 = labelNum ++;
    s<<JNE<<" "<<POSITION<<pos1<<endl;
    emit_mov("$0",RAX,s);
    s<<JMP<<" "<<POSITION<<pos2<<endl;
    s<<POSITION<<pos1<<":"<<endl;
    emit_mov("$1",RAX,s);
    s<<POSITION<<pos2<<":"<<endl;
    emit_rmmov(RAX,offset,RBP,s);
  }
}

void Ge_class::code(ostream &s) {
  e1->code(s);
  int addr1 = tempaddress;
  e2->code(s);
  int addr2 = tempaddress;

  emit_sub("$8", RSP, s);
  offset -= 8;
  tempaddress = offset;
  if (e1->getType()->get_string() == Int->get_string() && e2->getType()->get_string() == Int->get_string()) {
    emit_mrmov(RBP,addr1,RAX,s);
    emit_mrmov(RBP,addr2,RDX,s);
    emit_cmp(RDX,RAX,s);
    int pos1 = labelNum ++;
    int pos2 = labelNum ++;
    s<<JGE<<" "<<POSITION<<pos1<<endl;
    emit_mov("$0", RAX, s);
    s<<JMP<<" "<<POSITION<<pos2<< endl;
    s<<POSITION<<pos1<<":"<<endl;
    emit_mov("$1",RAX,s);
    s<<POSITION<<pos2<<":"<<endl;
    emit_rmmov(RAX,offset,RBP,s);
  } else if (e1->getType()->get_string() == Float->get_string() && e2->getType()->get_string() == Int->get_string()) {
    emit_mrmovsd(RBP, addr1, XMM1, s);
    emit_mrmov(RBP, addr2, RAX, s);
    emit_int_to_float(RAX, XMM0, s);
    emit_ucompisd(XMM0, XMM1, s);
    int pos1 = labelNum ++;
    int pos2 = labelNum ++;
    s<<JAE<<" "<<POSITION<<pos1<<endl;
    emit_mov("$0",RAX,s);
    s<<JMP<<" "<<POSITION<<pos2<< endl;
    s<<POSITION<<pos1<<":"<<endl;
    emit_mov("$1",RAX,s);
    s<<POSITION<<pos2<<":"<<endl;
    emit_rmmov(RAX,offset,RBP,s);
  } else if (e1->getType()->get_string() == Int->get_string() && e2->getType()->get_string() == Float->get_string()) {
    emit_mrmov(RBP,addr1,RAX,s);
    emit_int_to_float(RAX, XMM0, s);
    emit_mrmovsd(RBP, addr2, XMM1, s);
    emit_ucompisd(XMM0, XMM1, s);
    int pos1 = labelNum ++;
    int pos2 = labelNum ++;
    s<<JAE<<" "<<POSITION<<pos1<<endl;
    emit_mov("$0",RAX,s);
    s<<JMP<<" "<<POSITION<<pos2<<endl;
    s<<POSITION<<pos1<<":"<<endl;
    emit_mov("$1",RAX,s);
    s<<POSITION<<pos2<<":"<<endl;
    emit_rmmov(RAX,0,RBP,s);
  } else if (e1->getType()->get_string() == Float->get_string() && e2->getType()->get_string() == Float->get_string()) {
    emit_mrmovsd(RBP, addr1, XMM0, s);
    emit_mrmovsd(RBP, addr2, XMM1, s);
    emit_ucompisd(XMM0, XMM1, s);
    int pos1 = labelNum ++;
    int pos2 = labelNum ++;
    s<<JAE<<" "<<POSITION<<pos1<<endl;
    emit_mov("$0",RAX,s);
    s<<JMP<<" "<<POSITION<<pos2<<endl;
    s<<POSITION<<pos1<<":"<<endl;
    emit_mov("$1",RAX,s);
    s<<POSITION<<pos2<<":"<<endl;
    emit_rmmov(RAX,offset,RBP,s);
  }
}

void Gt_class::code(ostream &s) {
  e1->code(s);
  int addr1 = tempaddress;
  e2->code(s);
  int addr2 = tempaddress;

  emit_sub("$8", RSP, s);
  offset -= 8;
  tempaddress = offset;
  if (e1->getType()->get_string() == Int->get_string() && e2->getType()->get_string() == Int->get_string()) {
    emit_mrmov(RBP,addr1,RAX,s);
    emit_mrmov(RBP,addr2,RDX,s);
    emit_cmp(RDX,RAX,s);
    int pos1 = labelNum ++;
    int pos2 = labelNum ++;
    s<<JG<<" "<<POSITION<<pos1<<endl;
    emit_mov("$0", RAX, s);
    s<<JMP<<" "<<POSITION<<pos2<< endl;
    s<<POSITION<<pos1<<":"<<endl;
    emit_mov("$1",RAX,s);
    s<<POSITION<<pos2<<":"<<endl;
    emit_rmmov(RAX,offset,RBP,s);
  } else if (e1->getType()->get_string() == Float->get_string() && e2->getType()->get_string() == Int->get_string()) {
    emit_mrmovsd(RBP, addr1, XMM1, s);
    emit_mrmov(RBP, addr2, RAX, s);
    emit_int_to_float(RAX, XMM0, s);
    emit_ucompisd(XMM0, XMM1, s);
    int pos1 = labelNum ++;
    int pos2 = labelNum ++;
    s<<JA<<" "<<POSITION<<pos1<<endl;
    emit_mov("$0",RAX,s);
    s<<JMP<<" "<<POSITION<<pos2<< endl;
    s<<POSITION<<pos1<<":"<<endl;
    emit_mov("$1",RAX,s);
    s<<POSITION<<pos2<<":"<<endl;
    emit_rmmov(RAX,offset,RBP,s);
  } else if (e1->getType()->get_string() == Int->get_string() && e2->getType()->get_string() == Float->get_string()) {
    emit_mrmov(RBP, addr1, RAX, s);
    emit_int_to_float(RAX, XMM0, s);
    emit_mrmovsd(RBP, addr2, XMM1, s);
    emit_ucompisd(XMM0, XMM1, s);
    int pos1 = labelNum ++;
    int pos2 = labelNum ++;
    s<<JA<<" "<<POSITION<<pos1<<endl;
    emit_mov("$0", RAX, s);
    s<<JMP<<" "<<POSITION<<pos2<<endl;
    s<<POSITION<<pos1<<":"<<endl;
    emit_mov("$1",RAX,s);
    s<<POSITION<<pos2<<":"<<endl;
    emit_rmmov(RAX, 0, RBP, s);
  } else if (e1->getType()->get_string() == Float->get_string() && e2->getType()->get_string() == Float->get_string()) {
    emit_mrmovsd(RBP, addr1, XMM0, s);
    emit_mrmovsd(RBP, addr2, XMM1, s);
    emit_ucompisd(XMM0, XMM1, s);
    int pos1 = labelNum ++;
    int pos2 = labelNum ++;
    s<<JA<<" "<<POSITION<<pos1<<endl;
    emit_mov("$0", RAX, s);
    s<<JMP<<" "<<POSITION<<pos2<<endl;
    s<<POSITION<<pos1<<":"<<endl;
    emit_mov("$1",RAX,s);
    s<<POSITION<<pos2<<":"<<endl;
    emit_rmmov(RAX, offset, RBP, s);
  }
}

void And_class::code(ostream &s) {
  e1->code(s);
  int addr1 = tempaddress;
  e2->code(s);
  int addr2 = tempaddress;

  emit_sub("$8", RSP, s);
  offset -= 8;
  tempaddress = offset;

  emit_mrmov(RBP, addr1, RAX, s);
  emit_mrmov(RBP, addr2, RDX, s);
  emit_and(RAX, RDX, s);
  emit_rmmov(RDX, offset, RBP, s);
}

void Or_class::code(ostream &s) {
  e1->code(s);
  int addr1 = tempaddress;
  e2->code(s);
  int addr2 = tempaddress;

  emit_sub("$8", RSP, s);
  offset -= 8;
  tempaddress = offset;

  emit_mrmov(RBP, addr1, RAX, s);
  emit_mrmov(RBP, addr2, RDX, s);
  emit_or(RAX, RDX, s);
  emit_rmmov(RDX, offset, RBP, s);
}

void Xor_class::code(ostream &s) {
  e1->code(s);
  int addr1 = tempaddress;
  e2->code(s);
  int addr2 = tempaddress;

  emit_sub("$8", RSP, s);
  offset -= 8;
  tempaddress = offset;

  emit_mrmov(RBP, addr1, RAX, s);
  emit_mrmov(RBP, addr2, RDX, s);
  emit_xor(RAX, RDX, s);
  emit_rmmov(RDX, offset, RBP, s);
}

void Not_class::code(ostream &s) {
  e1->code(s);
  int addr1 = tempaddress;

  emit_sub("$8", RSP, s);
  offset -= 8;
  tempaddress = offset;

  emit_mrmov(RBP, addr1, RAX, s);
  emit_mov("$$0x0000000000000001", RDX, s);
  emit_xor(RDX, RAX, s);
  emit_rmmov(RAX, offset, RBX, s);
}

void Bitnot_class::code(ostream &s) {
  e1->code(s);
  int addr1 = tempaddress;

  emit_sub("$8", RSP, s);
  offset -= 8;
  tempaddress = offset;
  
  emit_mrmov(RBP, addr1, RAX, s);
  emit_not(RAX, s);
  emit_rmmov(RAX, offset, RBP, s);
}

void Bitand_class::code(ostream &s) {
  e1->code(s);
  int addr1 = tempaddress;
  e2->code(s);
  int addr2 = tempaddress;

  emit_sub("$8", RSP, s);
  offset -= 8;
  tempaddress = offset;

  emit_mrmov(RBP, addr1, RAX, s);
  emit_mrmov(RBP, addr2, RDX, s);
  emit_and(RAX, RDX, s);
  emit_rmmov(RDX, offset, RBP, s);
}

void Bitor_class::code(ostream &s) {
  e1->code(s);
  int addr1 = tempaddress;
  e2->code(s);
  int addr2 = tempaddress;

  emit_sub("$8", RSP, s);
  offset -= 8;
  tempaddress = offset;

  emit_mrmov(RBP, addr1, RAX, s);
  emit_mrmov(RBP, addr2, RDX, s);
  emit_or(RAX, RDX, s);
  emit_rmmov(RDX, offset, RBP, s);
}

void Const_int_class::code(ostream &s) {
  emit_sub("$8",RSP,s);
  offset -= 8;
  tempaddress = offset;

  s<<MOV<<"$"<<value<<COMMA<<RAX<<endl;
  
  emit_rmmov(RAX, tempaddress, RBP, s);
}

void Const_string_class::code(ostream &s) {
  emit_sub("$8", RSP, s);
  offset -= 8;
  tempaddress = offset;
  s<<MOV;
  stringtable.lookup_string(value->get_string())->code_ref(s);
  s<<COMMA<<RAX<<endl;

  emit_rmmov(RAX, tempaddress, RBP, s);
}

void Const_float_class::code(ostream &s) {
  emit_sub("$8", RSP, s);
  offset -= 8;
  tempaddress = offset;

  double d_value = atof(value->get_string());
  unsigned long long hex_value = *(unsigned long long *) &d_value;
  char test[17];
  sprintf(test, "%llx", hex_value);
  
  s<<MOV<<"$0x";
  s<<test;
  s<<COMMA<<RAX<<endl;

  emit_rmmov(RAX, tempaddress, RBP, s);
}

void Const_bool_class::code(ostream &s) {
  emit_sub("$8", RSP, s);
  offset -= 8;
  tempaddress = offset;

  s<<MOV<<"$"<<value<<COMMA<<RAX<<endl;

  emit_rmmov(RAX, tempaddress, RBP, s);
}

void Object_class::code(ostream &s) {
  variabletab.enterscope();
  tempaddress = *variabletab.lookup(var);
  variabletab.exitscope();
}

void No_expr_class::code(ostream &s) {

}