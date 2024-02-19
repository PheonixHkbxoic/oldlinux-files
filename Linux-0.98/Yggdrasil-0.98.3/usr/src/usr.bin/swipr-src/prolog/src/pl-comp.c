/*  @(#) pl-comp.c 1.5.0 (UvA SWI) Jul 30, 1990

    Copyright (c) 1990 Jan Wielemaker. All rights reserved.
    See ../LICENCE to find out about your rights.
    jan@swi.psy.uva.nl

    Purpose: compiler support
*/

#include "pl-incl.h"

#define CODE(c, n, a)	{ n, a, c }

struct code_info codeTable[] = {
  CODE(I_NOP,		"i_nop",	0),
  CODE(I_ENTER,		"i_enter",	0),
  CODE(I_CALL,		"i_call",	1),
  CODE(I_DEPART,	"i_depart",	1),
  CODE(I_EXIT,		"i_exit",	0),
  CODE(B_FUNCTOR,	"b_functor",	1),
  CODE(H_FUNCTOR,	"h_functor",	1),
  CODE(I_POP,		"i_pop",	0),
  CODE(I_POPN,		"i_popn",	1),
  CODE(B_VAR,		"b_var",	1),
  CODE(H_VAR,		"h_var",	1),
  CODE(B_CONST,		"b_const",	1),
  CODE(H_CONST,		"h_const",	1),
  CODE(H_REAL,		"h_real",	1),
  CODE(H_STRING,	"h_string",	1),
  CODE(B_FIRSTVAR,	"b_firstvar",	1),
  CODE(H_FIRSTVAR,	"h_firstvar",	1),
  CODE(B_VOID,		"b_void",	0),
  CODE(H_VOID,		"h_void",	0),
  CODE(B_ARGFIRSTVAR,	"b_argfirstvar",1),
  CODE(B_ARGVAR,	"b_argvar",	1),
  CODE(H_NIL,		"h_nil",	0),
  CODE(H_CONST0,	"h_const0",	0),
  CODE(H_CONST1,	"h_const1",	0),
  CODE(H_CONST2,	"h_const2",	0),
  CODE(H_LIST,		"h_list",	0),
  CODE(H_FUNCTOR0,	"h_functor0",	0),
  CODE(H_FUNCTOR1,	"h_functor1",	0),
  CODE(H_FUNCTOR2,	"h_functor2",	0),
  CODE(B_VAR0,		"b_var0",	0),
  CODE(B_VAR1,		"b_var1",	0),
  CODE(B_VAR2,		"b_var2",	0),
  CODE(H_SINT,		"h_sint",	1),
  CODE(B_SINT,		"b_sint",	1),
  CODE(I_USERCALL,	"i_usercall",	0),
  CODE(I_CUT,		"i_cut",	0),
  CODE(I_APPLY,		"i_apply",	0),
  CODE(A_FUNC0,		"a_func0",	1),
  CODE(A_FUNC1,		"a_func1",	1),
  CODE(A_FUNC2,		"a_func2",	1),
  CODE(A_FUNC,		"a_func",	2),
  CODE(A_LT,		"a_lt",		0),
  CODE(A_GT,		"a_gt",		0),
  CODE(A_LE,		"a_le",		0),
  CODE(A_GE,		"a_ge",		0),
  CODE(A_EQ,		"a_eq",		0),
  CODE(A_NE,		"a_ne",		0),
  CODE(A_IS,		"a_is",		0),
  CODE(C_OR,		"c_or",		1),
  CODE(C_JMP,		"c_jmp",	1),
  CODE(C_MARK,		"c_mark",	1),
  CODE(C_CUT,		"c_cut",	1),
  CODE(C_IFTHENELSE,	"c_ifthenelse",	2),
  CODE(C_VAR,		"c_var",	1),
  CODE(C_END,		"c_end",	0),
  CODE(C_NOT,		"c_not",	2),
  CODE(C_FAIL,		"c_fail",	0),
  CODE(B_REAL,		"b_real",	1),
  CODE(B_STRING,	"b_string",	1),
  CODE(0,		NULL,		0)
};

forwards void	checkCodeTable P((void));

static void
checkCodeTable()
{ CodeInfo ci;
  int n;

  for(ci = codeTable, n = 0; ci->name != NULL; ci++, n++ )
  { if ( ci->code != n )
      sysError("Wrong entry in codeTable: %d", n);
  }

  if ( --n != I_HIGHEST )
    sysError("Mismatch in checkCodeTable()");
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
			MAPPING VIRTUAL INSTRUCTIONS

The virtual machine interpreter can be optimised considerably by storing
the code addressen with the clauses  rather  than  the  virtual  machine
codes.  Normally the switch in translated (in pseudo assembler) to:

next_instruction:
	r1 = *PC;
	PC += sizeof(code);
	if ( r1 > I_HIGHEST ) goto default;
	r1 = jmp_table[r1 * 4];
	goto r1;

This is rather silly.  Suppose  we  store  the  addresses  of  the  code
segments  with  the  clauses  rather than the codes themselves, than the
loop overhead can be reduced to:

next_instruction:
	r1 = *PC;
	PC += sizeof(code);
	goto r1;

This beeing very short we can also forget about the  jump  back  to  the
next_instruction  label  at  the  end of the virtual machine's code, but
just include these three instructions.

To use this trick the  flag  O_ASM_SWITCH  must  be  set.   You  compile
pl-wam.c  into  assembler, put a global label _wam_table at the start of
the switches table, so the compiler can pick up the addresses and  patch
the  switch code and jump back as described above.  This trick is rather
problematic if your machine does program relocation (680x0 based  micros
without  MMU)  as  the  addresses change each run, making compiled files
useless.
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if O_ASM_SWITCH
void
initWamTable()
{ int n;
  int maxcoded = 0;

  for(n = 0; n <= I_HIGHEST; n++)
    if ( wam_table[n] > maxcoded )
      maxcoded = wam_table[n];

  if ( maxcoded >= 64 * 1024 )
    fatalError("Cannot use direct jumps: maxcoded = %d (see pl-comp.c)", maxcoded);
  dewam_table = (char *)allocHeap((maxcoded + 1) * sizeof(char));
  
  for(n = 0; n <= I_HIGHEST; n++)
    dewam_table[wam_table[n]] = (char) n;

  checkCodeTable();
}

#else O_ASM_SWITCH

void
initWamTable()
{ checkCodeTable();
}

#endif O_ASM_SWITCH

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
This module forms together  with  the  module  'pl-wam.c'  the  complete
kernel  of  SWI-Prolog.   It  contains  the  compiler, the predicates to
interface the compiler to Prolog and the  decompiler.   SWI-Prolog  does
not  offer  a  Prolog  interpreter,  which  implies that common database
predicates such as assert/1 and retract/1 have to do  compilation  resp.
decompilation between the term representation used on the runtime stacks
and the compiled representation used in the heap.

Compiling a clause takes three different stages.  First the variables of
the clause are analysed.   This  phases  determines  `void'  (singleton)
variables  and assigns offsets in the environment frame to each variable
occurring in the clause that is not  singleton.   Variables  serving  on
their  own as an argument in the head are allocated in the corresponding
argument entry of the environment frame.  The others are allocated above
the arguments in the environment frame.   Singleton  variables  are  not
allocated at all.

Second  unification  code  for  the  head  is  produced.   Finally   the
subclauses  are  translated.   Most  vital  from  the  point  of view of
performance is to distinguis between the first time an  entry  from  the
variable  array  is addressed and the following times: the first time we
KNOW the field should be a variable and copying the value  or  making  a
reference  is  the  appropriate action.  This both saves us the variable
test and the need to turn the variable array of  the  environment  frame
really into an array of variables.

			ANALYSING VARIABLES

First of all the clause is scanned and all  variables  are  instantiated
with  a  structure  that  mimics  a term, but isn't one.  For historical
reasons this is the term $VAR$/1.  Future versions will  use  a  functor
which  is  impossible  to  conflict  with  the user's program.  For each
variable it's address is stored, as well  as  the  number  of  times  it
occurred in the clause.
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

forwards bool	analyse_variables P((Word, Word, int, int*));
forwards int	analyseVariables2 P((Word, int, int, int));

#if O_COMPILE_ARITH
#define A_NOTARITH	0
#define A_OK		1
#define A_ERROR		2
#endif O_COMPILE_ARITH

static struct vardef
{ FunctorDef	functor;		/* mimic a functor (FUNCTOR_var1) */
  Word		address;		/* address of the variable */
  int		times;			/* occurences */
  int		offset;			/* offset in environment frame */
} vars[MAXVARIABLES];

static int	filledVars;		/* vardef structures filled */

#define VAROFFSET(var) ( (var) + ARGOFFSET / (int) sizeof(word) )

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Split a  clause  into  its  head  and  body.   For  facts  the  body  is
instantiated to NULL.  This is the only place of the compiler that knows
about the :-/2 operator.
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

bool
splitClause(term, head, body)
register Word term;
Word *head, *body;
{ if (isAtom(*term) )
  { *head = term;
    *body = (Word) NULL;
    succeed;
  }
  if (!isTerm(*term) )
    fail;
  if (functorTerm(*term) != FUNCTOR_prove2)		/* :-/2 */
  { *head = term;
    *body = (Word) NULL;
    succeed;
  }
  *head = argTermP(*term, 0);
  *body = argTermP(*term, 1);
  deRef(*head);
  deRef(*body);

  succeed;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Analyse the variables of a clause.  `term' is the term to  be  analysed, 
which  is  either  a  fact  or  a  clause (:-/2) term.  First of all the
functor and arity of the predicate are determined.   The  first  `arity'
elements  of  the variable definition array are then cleared.  This part
is used for sharing variables that occurr on their own in the head  with
the  argument  part  of the environment frame instead of putting them in
the variable part.

AnalyseVariables2() just scans the term, fills the  variable  definition
array  and  binds  found  variables  to entries of this array.  The last
argument indicates which plain argument we are processing.  It is set to
-1 when called with the head.  While scaning the head  arguments  it  is
set  to  the argument number.  For all other code it is arity (body code
and nested terms of the head).  This is used for  the  argument/variable
block merging.

After this scan the variable definition records are  scanned  to  assign
offsets  and delete singleton variables.  We cannot leave out singletons
that are sharing with the argument block.  Offset `0' is the first entry
of the argument block, offset `arity' of the variable block.  Singletons
are made variables again.
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static bool
analyse_variables(head, body, arity, nv)
Word head, body;
int arity;
int *nv;
{ int nvars = 0;
  register struct vardef * vd;
  register int n;
  int body_voids = 0;

  for(n=0, vd = vars; n<arity; n++, vd++)
    vd->address = (Word) NULL;

  if ( (nvars = analyseVariables2(head, 0, arity, -1)) < 0 )
    fail;
  if (body != (Word) NULL)
    if ( (nvars = analyseVariables2(body, nvars, arity, arity)) < 0 )
      fail;

  for(n=0, vd = vars; n<arity+nvars; n++, vd++)
  { if (vd->address == (Word) NULL)
      continue;
    if (vd->times == 1)				/* ISVOID */
    { setVar(*(vd->address));
      vd->address = (Word) NULL;
      if (n >= arity)
	body_voids++;
    } else
      vd->offset = n - body_voids;
  }

  filledVars = arity + nvars;
  *nv = nvars - body_voids;
  succeed;
}

static int
analyseVariables2(head, nvars, arity, argn)
register Word head;
int nvars, arity;
int argn;
{ int ar;

  deRef(head);

  if (isVar(*head) )
  { register struct vardef *vd;
    int index = ((argn >= 0 && argn < arity) ? argn : (arity + nvars++));

    if ( index >= MAXVARIABLES-1 )
    { warning("Compiler: Too many variables in clause");
      return -1;
    }
    vd = &vars[index];
    vd->functor = FUNCTOR_var1;
    vd->address = head;
    vd->times = 1;
    *head = (word) vd;

    return nvars;
  }

  if ( isAtomic(*head) )
    return nvars;

  if (functorTerm(*head) == FUNCTOR_var1)
  { ((struct vardef *)*head)->times++;
    return nvars;
  }

  ar = functorTerm(*head)->arity;
  head = argTermP(*head, 0);

  argn = ( argn < 0 ? 0 : arity );

  for(; ar > 0; ar--, head++, argn++)
    nvars = analyseVariables2(head, nvars, arity, argn);

  return nvars;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
The compiler  itself.   First  it  calls  analyseVariables().  Next  the
arguments  of  the  head  and  the subclauses are compiled.  Finally the
bindings made by analyseVariables() are undone and the clause  is  saved
in the heap.

compile() maintains an array of `used_var' (used variables).  This is to
determine when a variable is used for the first time and thus a FIRSTVAR
instruction is to be generated instead of a VAR one.

Note that the `variables' field of a clause is filled with the number of
variables in the frame AND the arity.   This  saves  us  the  frame-size
calculation at runtime.
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#define isConjunction(w) (isTerm(w) && functorTerm(w) == FUNCTOR_comma2)

#define HEAD    2			/* compileArgument on head argument */
#define HEADARG 3			/* ... on functor arg in head */
#define BODY    4			/* compileArgument on body argument */
#define BODYARG 5			/* ... on functor arg in body */

#define ISVOID 0				/* compileArgument produced H_VOID */
#define NONVOID 1			/* ... anything else */

#define Output_0(ci, c)		((ci)->codes[(ci)->tc++] = encode(c))
#define Output_a(c1, c)		((ci)->codes[(ci)->tc++] = (c))
#define Output_1(ci, c, a)	Output_0(ci, c), Output_a(ci, a)
#define Output_2(ci, c, a0, a1)	Output_1(ci, c, a0), Output_a(ci, a1)

#define BITSPERINT (sizeof(int)*8)

struct vartable
{ int	entry[MAXVARIABLES/BITSPERINT];
} empty_var_table;

typedef struct
{ Module	module;			/* module to compile into */
  int		arity;			/* arity of top-goal */
  Clause	clause;			/* clause we are constructing */
  int		tc;			/* index in tc table */
  int		tx;			/* index in XR table */
  struct vartable used_var;		/* boolean array of used variables */
  word		XR[MAXEXTERNALS];	/* scratch XR table */
  code		codes[MAXCODES];	/* scratch code table */
} compileInfo;

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Variable table operations.
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#define addXRtable(entry, ci)	add_xr_table((word)(entry), (ci))

forwards bool	compileBody P((Word, code, compileInfo *));
forwards int	compileArgument P((Word, int, compileInfo *));
forwards int	add_xr_table P((word, compileInfo *));
forwards int	addRealXRtable P((word, compileInfo *));
forwards int	addStringXRtable P((word, compileInfo *));
forwards bool	compileSubClause P((Word, code, compileInfo *));
forwards bool	isFirstVar P((struct vartable *vt, int n));
forwards void	balanceVars P((struct vartable *, struct vartable *, compileInfo *));
forwards void	orVars P((struct vartable *, struct vartable *));
forwards void	setVars P((Word t, struct vartable *));
forwards Clause	compile P((Word, Module));
#if O_COMPILE_ARITH
forwards int	compileArith P((Word, compileInfo *));
forwards bool	compileArithArgument P((Word, compileInfo *));
#endif

#define isIndexedVarTerm(var) ( functorTerm(var) == FUNCTOR_var1 ? \
					((struct vardef *)var)->offset : \
					-1)

#define ClearVarTable(ci)	((ci)->used_var = empty_var_table)

static bool
isFirstVar(vt, n)
struct vartable *vt;
register int n;
{ register int m  = 1 << (n % BITSPERINT);
  register int *p = &vt->entry[n / BITSPERINT];
  register int result;
  
  result = ((*p & m) == 0);
  *p |= m;

  return result;
}

static void
balanceVars(valt1, valt2, ci)
struct vartable *valt1, *valt2;
compileInfo *ci;
{ int *p1 = &valt1->entry[0];
  int *p2 = &valt2->entry[0];
  register int n;

  for( n = 0; n < MAXVARIABLES/BITSPERINT; p1++, p2++, n++ )
  { register int m = (~(*p1) & *p2);

    if ( m )
    { register int i;

      for(i = 0; i < BITSPERINT; i++)
	if ( m & (1 << i) )
	  Output_1(ci, C_VAR, VAROFFSET(n * BITSPERINT + i));
    }
  }
}

static void
orVars(valt1, valt2)
struct vartable *valt1, *valt2;
{ register int *p1 = &valt1->entry[0];
  register int *p2 = &valt2->entry[0];
  register int n;

  for( n = 0; n < MAXVARIABLES/BITSPERINT; n++ )
    *p1++ |= *p2++;
}

static void
setVars(t, vt)
register Word t;
register struct vartable *vt;
{ deRef(t);

  if ( isTerm(*t) )
  { int index;
    register int arity;

    if ( (index = isIndexedVarTerm(*t)) >= 0 )
    { isFirstVar(vt, index);
      return;
    }
    arity = functorTerm(*t)->arity;
    for(t = argTermP(*t, 0); arity > 0; t++, arity--)
      setVars(t, vt);
  }
}

static Clause
compile(term, module)
Word term;
Module module;
{ compileInfo ci;			/* data base for the compiler */
  Word head, body;
  Procedure proc;
  Clause clause;
  int nvars;

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Split the clause into its head and body and determine the procedure  the
clause should belong to.
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  if (splitClause(term, &head, &body) == FALSE)
  { warning("compiler: illegal clause");
    return (Clause) NULL;
  }
  if (isAtom(*head) )
    proc = lookupProcedure(lookupFunctorDef((Atom)*head, 0), module);
  else if (isTerm(*head) )
    proc = lookupProcedure(functorTerm(*head), module);
  else
  { warning("compiler: illegal clause head");
    return (Clause) NULL;
  }

  if ( (ci.arity = proc->functor->arity) > MAXARITY )
    return (Clause) warning("Compiler: arity too high (%d)\n", ci.arity);

  DEBUG(9, printf("Splitted and found proc\n"));

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Allocate the clause and fill initialise the field we already know.
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  clause = (Clause) allocHeap(sizeof(struct clause));
  clause->next = (Clause) NULL;
  clause->references = 0;
  clear(clause, ERASED|INDEXABLE);
  clause->XR_size = clause->code_size = 0;
  clause->subclauses = 0;
  clause->procedure = proc;

#if O_AUTOINDEX
  if ( ci.arity > 0 )
  { register Word a = argTermP(*head, 0);

    deRef(a);
    DEBUG(9, printf("a = 0x%lx, *a = 0x%lx\n", a, *a));
    if ( isAtom(*a) || isInteger(*a) || isTerm(*a) )
      set(clause, INDEXABLE);
  }
#endif O_AUTOINDEX

  DEBUG(9, printf("clause struct initialised\n"));

  { register Definition def = proc->definition;

    if ( def->indexPattern )
      clause->index = getIndex(argTermP(*head, 0), def->indexPattern, 
						   def->indexCardinality);
    else
      clause->index.key = clause->index.varmask = 0L;
  }

  TRY( analyse_variables(head, body, ci.arity, &nvars) );
  clause->variables = clause->slots = nvars + ci.arity;

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Initialise the `compileInfo' structure.
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  ci.tx = ci.tc = 0;
  ci.module = module;
  ci.clause = clause;
  ClearVarTable(&ci);

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
First compile  the  head  of  the  term.   The  arguments  are  compiled
left-to-right. `lastnonvoid' is maintained to delete void variables just
before the I_ENTER instructions.
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  { int n;
    int lastnonvoid = 0;
    Word arg;

    for ( arg = argTermP(*head, 0), n = 0; n < ci.arity; n++, arg++ )
      if ( compileArgument(arg, HEAD, &ci) == NONVOID )
	lastnonvoid = ci.tc;
    ci.tc = lastnonvoid;
  }

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Now compile the body.
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  if (body != (Word) NULL)
  { Output_0(&ci, I_ENTER);
    compileBody(body, I_DEPART, &ci);
  }
  Output_0(&ci, I_EXIT);

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Reset all variables we initialised to the variable analysis  functor  to
become variables again.
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  { register struct vardef * vd;
    register int n;

    for(vd=vars, n=0; n < filledVars; n++, vd++)
      if (vd->address != (Word) NULL)
	setVar(*(vd->address));
  }

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Finish up the clause.
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  if ( ci.tx > 0 )
  { clause->externals = (Word) allocHeap(sizeof(word) * ci.tx);
    bcopy(ci.XR, clause->externals, sizeof(word) * ci.tx);
  } else
    clause->externals = NULL;
  clause->XR_size = ci.tx;
  clause->codes = (Code) allocHeap(sizeof(code) * ci.tc);
  bcopy(ci.codes, clause->codes, sizeof(code) * ci.tc);
  clause->code_size = ci.tc;
  statistics.externals += clause->XR_size;
  statistics.codes += clause->code_size;

  return clause;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
compileBody() compiles the clause's body.  Within a body,  a  number  of
constructs are recognised:

SUBGOAL
    For a subgoal we generate code to push the  arguments  on  the  next
    stack  frame  and finally generate either I_CALL for normal calls or
    I_DEPART for the last subgoal  of  the  clause  to  allow  for  tail
    recursion optimisation.

VARIABLE or META CALL
    Single variables or constructs  of  the  form  term:term  imply  the
    generation of a metacall.

A ; B, A -> B, A -> B ; C, \+ A
    The compilation of these statements are  a  bit  more  tricky.   Two
    mechanisms support this compilation:
    
	C_MARK var	Mark for `soft-cut'
	C_CUT  var	Cut alternatives generated since C_MARK var

    and
	
	C_OR jmp	Generate a choicepoint.  It the continuation
			fails skip `jmp' instructions and continue
			there.
	C_JMP jmp	Just skip `jmp' instructions.

    This set  is  augmented  with  some  compound  statements  and  some
    statements  with  different  names,  but equal semantics to help the
    decompiler.  See pl-wam.c for more details.

    NOTE: A tricky bit now is that we  can  reach  the  same  point  via
    different  paths.   Each of these paths may result in another set of
    variabled  already  instantiated.   This  gives  troubles  with  the
    FIRSTVAR  type  of instructions.  to avoid such trouble the compiler
    generates  SETVAR  instructions  to  balance  both   brances.    See
    balanceVars();
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if PROTO
static bool
compileBody(register Word body, code call, register compileInfo *ci)
#else
static bool
compileBody(body, call, ci)
register Word body;
code call;
register compileInfo *ci;
#endif
{ deRef(body);

  if ( isTerm(*body) )
  { FunctorDef fd = functorTerm(*body);

    if ( fd == FUNCTOR_comma2 )			/* A , B */
    { TRY( compileBody(argTermP(*body, 0), I_CALL, ci) );
      return compileBody(argTermP(*body, 1), call, ci);
#if O_COMPILE_OR
    } else if ( fd == FUNCTOR_semicolon2 ||
		fd == FUNCTOR_bar2 )		/* A ; B and (A -> B ; C) */
    { register Word a0 = argTermP(*body, 0);
      struct vartable vsave, valt1, valt2;

      vsave = valt1 = valt2 = ci->used_var;
      setVars(argTermP(*body, 0), &valt1);
      setVars(argTermP(*body, 1), &valt2);

      deRef(a0);
      if ( isTerm(*a0) && functorTerm(*a0) == FUNCTOR_ifthen2 ) /* A -> B ; C */
      { int var = VAROFFSET(ci->clause->variables++);
	int tc_or, tc_jmp;

	Output_2(ci, C_IFTHENELSE, var, (code)0);
	tc_or = ci->tc;
	TRY( compileBody(argTermP(*a0, 0), I_CALL, ci) );	
	Output_1(ci, C_CUT, var);
	TRY( compileBody(argTermP(*a0, 1), I_CALL, ci) );	
	balanceVars(&valt1, &valt2, ci);
	Output_1(ci, C_JMP, (code)0);
	tc_jmp = ci->tc;
	ci->codes[tc_or-1] = (code)(ci->tc - tc_or);
	ci->used_var = vsave;
	TRY( compileBody(argTermP(*body, 1), call, ci) );
	balanceVars(&valt2, &valt1, ci);
	ci->codes[tc_jmp-1] = (code)(ci->tc - tc_jmp);
      } else					/* A ; B */
      { int tc_or, tc_jmp;

	Output_1(ci, C_OR, (code)0);
	tc_or = ci->tc;
	TRY( compileBody(argTermP(*body, 0), I_CALL, ci) );
	balanceVars(&valt1, &valt2, ci);
	Output_1(ci, C_JMP, (code)0);
	tc_jmp = ci->tc;
	ci->codes[tc_or-1] = (code)(ci->tc - tc_or);
	ci->used_var = vsave;
	TRY( compileBody(argTermP(*body, 1), call, ci) );
	balanceVars(&valt2, &valt1, ci);
	ci->codes[tc_jmp-1] = (code)(ci->tc - tc_jmp);
      }

      orVars(&valt1, &valt2);
      ci->used_var = valt1;

      succeed;
    } else if ( fd == FUNCTOR_ifthen2 )		/* A -> B */
    { int var = VAROFFSET(ci->clause->variables++);

      Output_1(ci, C_MARK, var);
      TRY( compileBody(argTermP(*body, 0), I_CALL, ci) );
      Output_1(ci, C_CUT, var);

      TRY( compileBody(argTermP(*body, 1), call, ci) );
      Output_0(ci, C_END);
      
      succeed;
    } else if ( fd == FUNCTOR_not_provable1 )		/* \+/1 */
    { int var = VAROFFSET(ci->clause->variables++);
      int tc_or;
      struct vartable vsave;

      vsave = ci->used_var;

      Output_2(ci, C_NOT, var, (code)0);
      tc_or = ci->tc;
      TRY( compileBody(argTermP(*body, 0), I_CALL, ci) );	
      Output_1(ci, C_CUT, var);
      Output_0(ci, C_FAIL);
      ci->codes[tc_or-1] = (code)(ci->tc - tc_or);
      ci->used_var = vsave;
      
      succeed;
#endif O_COMPILE_OR
    }
  }

  TRY( compileSubClause(body, call, ci) );
  ci->clause->subclauses++;

  succeed;
}


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
compileArgument() is the key function of the compiler.  Its function  is
to   generate  the  term  matching/construction  instructions  both  for
arguments of the head as for arguments to subclauses.   It  distinguises
three  different  places:  compiling plain arguments to the head (HEAD),
arguments of terms occurring in the head (HEADARG) and body arguments
(BODY).

The  isIndexedVar()  macro  detects  a   term   has   been   filled   by
analyseVariables()  and  returns the offset of the variable, or -1 if it
is not produced by this function.

compileArgument() returns ISVOID if a void instruction resulted from the
compilation.  This is used to detect  the  ...ISVOID,  [I_ENTER,  I_POP]
sequences,  in  which  case  we  can leave out the VOIDS just before the
I_ENTER or I_POP instructions.
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static int lastPopped;		/* how many contiguous pops? */

static int
compileArgument(arg, where, ci)
register Word arg;
register int where;
register compileInfo *ci;
{ int index;
  bool first;

  lastPopped = 0;		/* going to produce something else */
  deRef(arg);

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
A void.  Generate either B_VOID or H_VOID.  Note that the  return  value
ISVOID  is reserved for head variables only (B_VOID sets the location to
be a variable, and thus cannot be removed if it is before an I_POP.
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  if ( isVar(*arg) )
  { if (where & BODY)
    { Output_0(ci, B_VOID);
      return NONVOID;
    }
    Output_0(ci, H_VOID);
    return ISVOID;
  }


  if ( isAtomic(*arg) )
  { int index;

    if (isNil(*arg) && (where & HEAD))
    { Output_0(ci, H_NIL);
      return NONVOID;
    }

    if ( isIndirect(*arg) )
    { if ( isReal(*arg) )
      { Output_1(ci, where & HEAD ? H_REAL : B_REAL,
		     addRealXRtable(*arg, ci));
	return NONVOID;
      }
#if O_STRING
      if ( isString(*arg) )
      { Output_1(ci, where & HEAD ? H_STRING : B_STRING,
		     addStringXRtable(*arg, ci));
	return NONVOID;
      }
#endif O_STRING
    }

    if ( isInteger(*arg) )
    { long val = valNum(*arg);
      if ( val >= -32768L && val <= 32767L )
      { Output_1(ci, (where & BODY) ? B_SINT : H_SINT, (code) val);
        return NONVOID;
      }
    }

    index = addXRtable(*arg, ci);
    if (index < 3 && (where & HEAD))
    { Output_0(ci, H_CONST0 + index);
      return NONVOID;
    }
    Output_1(ci, (where & BODY) ? B_CONST : H_CONST, index);
    return NONVOID;
  }
    
  SECURE(				/* should be a term when here */
	if (!isTerm(*arg))
	  return sysError("Illegal type in compileArgument()"));

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Non-void variables. There are many cases for this.
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  if ( (index = isIndexedVarTerm(*arg)) >= 0 )
  { first = isFirstVar(&ci->used_var, index);

    if ( index < ci->arity )		/* variable on its own in the head */
    { switch ( where )
      { case BODY:	if ( index < 3 )
			{ Output_0(ci, B_VAR0 + index);
			  return NONVOID;
			}
			Output_0(ci, B_VAR);	break;
	case BODYARG:	Output_0(ci, B_ARGVAR);	break;
	case HEAD:	if ( first )
			{ Output_0(ci, H_VOID);
			  return ISVOID;
			} /*FALLTHROUGH*/
	case HEADARG:	Output_0(ci, H_VAR);	break;
      }
      Output_a(ci, VAROFFSET(index));
      return NONVOID;
    }

    /* normal variable (i.e. not shared in the head and non-void) */
    switch(where)
    { case BODY:	if ( index < 3 && !first )
			{ Output_0(ci, B_VAR0 + index);
			  return NONVOID;
			}
			Output_0(ci, first ? B_FIRSTVAR    : B_VAR);	break;
      case BODYARG:	Output_0(ci, first ? B_ARGFIRSTVAR : B_ARGVAR); break;
      default:		Output_0(ci, first ? H_FIRSTVAR    : H_VAR);	break;
    }
    Output_a(ci, VAROFFSET(index));
    return NONVOID;
  }

  { int ar;
    int lastnonvoid;
    FunctorDef fdef;

    fdef = functorTerm(*arg);
    if ( fdef == FUNCTOR_dot2 && (where & HEAD) )
    { Output_0(ci, H_LIST);
    } else
    { index = addXRtable(fdef, ci);
      if (index < 3 && (where & HEAD))
      { Output_0(ci, H_FUNCTOR0 + index);
      } else
      { Output_1(ci, where & BODY ? B_FUNCTOR : H_FUNCTOR, index);
      }
    }
    lastnonvoid = ci->tc;
    ar = fdef->arity;
    for(arg = argTermP(*arg, 0); ar > 0; ar--, arg++)
    { if ( compileArgument(arg, (where & BODY) ? BODYARG : HEADARG, ci)
							== NONVOID )
	lastnonvoid = ci->tc;
    }
    ci->tc = lastnonvoid;
    switch(lastPopped)
    { case 0:		Output_0(ci, I_POP);
    			break;
      case 1:		ci->codes[ci->tc-1] = encode(I_POPN);
			Output_a(ci, 2);
			break;
      case 65535L:	Output_0(ci, I_POP);	/* I_POPN 65535, I_POP... */
			lastPopped = 0;
			break;
      default:		ci->codes[ci->tc-1]++;
    }
    lastPopped++;
    return NONVOID;
  }
}

#define CheckMaxExternals(ci)	\
	{ if ( ci->tx >= MAXEXTERNALS ) \
	  { warning("Compiler limit: too many external references"); \
	    pl_abort(); \
	  } \
	}

static int
add_xr_table(entry, ci)
register word entry;
register compileInfo *ci;
{ register int n;
  register Word XR = ci->XR;

  for(n=0; n<ci->tx; n++, XR++)
    if (entry == *XR)
      return n;
  CheckMaxExternals(ci);
  *XR = entry;

  return ci->tx++;
}

static int
addRealXRtable(entry, ci)
register word entry;
register compileInfo *ci;
{ register int n;
  register Word XR = ci->XR;

  for(n=0; n<ci->tx; n++, XR++)
    if (isReal(*XR) && valReal(entry) == valReal(*XR) )
      return n;
  CheckMaxExternals(ci);
  *XR = heapReal(valReal(entry));

  return ci->tx++;
}


#if O_STRING
static int
addStringXRtable(entry, ci)
register word entry;
register compileInfo *ci;
{ register int n;
  register Word XR = ci->XR;

  for(n=0; n<ci->tx; n++, XR++)
    if ( isString(*XR) && equalString(entry, *XR) )
      return n;
  CheckMaxExternals(ci);
  *XR = heapString(valString(entry));

  return ci->tx++;
}
#endif O_STRING


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
The task of compileSubClause() is to  generate  code  for  a  sunclause.
First  it will call compileArgument for each argument to the call.  Then
an instruction to call the procedure is added.  Before doing all this it
will check for the subclause just beeing a variable or the cut.
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if PROTO
static bool
compileSubClause(register Word arg, code call, compileInfo *ci)
#else
static bool
compileSubClause(arg, call, ci)
register Word arg;
code call;
compileInfo *ci;
#endif
{ Module tm = ci->module;

  deRef(arg);

  if ( isTerm(*arg) )
  {
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
A non-void variable. Create a I_USERCALL instruction for it.
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
    if ( isIndexedVarTerm(*arg) >= 0 )
    { compileArgument(arg, BODY, ci);
      Output_0(ci, I_USERCALL);
      succeed;
    }

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
If the argument is of the form <Module>:<Goal>, <Module> is an atom  and
<Goal>  is  nonvar  then compile to the specified module.  Otherwise use
the meta-call mechanism (BUG: `user:hello:foo' is called  via  meta-call
mechanism, but this only is a bit slower).

This is a bit more complex then expected: foo:assert(baz) should  assert
baz/0  into module foo.  In general: the context module should be set to
the appropriate value.  This needs a  new  virtual  machine  instruction
that  handles  calls  with  specified context module.  For the moment we
will use the meta-call mechanism for all these types of calls.
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
    if ( functorTerm(*arg) == FUNCTOR_module2 )
    {
  /*							SEE COMMENT ABOVE
      register Word mp, g;

      mp = argTermP(*arg, 0); deRef(mp);
      if ( isAtom(*mp) )
      { g = argTermP(*arg, 1); deRef(g);
	if ( isIndexedVarTerm(*g) < 0 )
	{ arg = g;
	  tm = lookupModule(*mp);
	  goto cont;
	}
      }
  */

      compileArgument(arg, BODY, ci);
      Output_0(ci, I_USERCALL);
      succeed;
    }
/*  cont: */

#if O_COMPILE_ARITH
    if ( status.optimise )
    { switch( compileArith(arg, ci) )
      { case A_OK:	succeed;
	case A_ERROR:	fail;
      }
    }
#endif O_COMPILE_ARITH

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Term, not a variable and not a module call.  Compile the  arguments  and
generate  the  call  instruction.   Note  this  codes traps the $apply/2
operator.
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
    { Procedure proc;
      int ar;

      proc = lookupProcedure(functorTerm(*arg), tm);
      ar = functorTerm(*arg)->arity;

      for(arg = argTermP(*arg, 0); ar > 0; ar--, arg++)
	compileArgument(arg, BODY, ci);
      if ( proc->functor == FUNCTOR_apply2 )
      { Output_0(ci, I_APPLY);
	succeed;
      }
      Output_1(ci, call, addXRtable(proc, ci));

      succeed;
    }
  }

  if ( isAtom(*arg) )
  { if ( *arg == (word) ATOM_cut )
    { Output_0(ci, I_CUT);
      succeed;
    }

    Output_1(ci,
	     call,
	     addXRtable(lookupProcedure(lookupFunctorDef((Atom)*arg, 0), tm), ci));

    succeed;
  }
    
  return warning("assert/1: illegal clause");
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Arithmetic compilation compiles is/2, >/2, etc.  Instead of building the
compound terms holding the arithmetic expression as  a  whole  and  then
calling  is/2,  etc.  to evaluate the result, a stack machine is used to
compute the value.  The ARGP virtual machine register, normally used  in
body  mode to push the arguments to the next functioncall now is used to
push the arguments to the arithmetic functions.  Normally, a term f(a,b)
is translated to:

	* Create f and set ARGP to point to first argument of f
	* Push a and b via ARGP
	* pop ARGP

This constructs a term.  In arithmetic mode, we generate:

	* Push a and b via ARGP
	* Call f/2 to pick the top two words from the stack and push
	  the result back onto it.

This has two advantages: No term is created on the global stack and  the
mapping  between  the  term  and  the arithmetic function is done by the
compiler rather than the evaluation routine.
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if O_COMPILE_ARITH
static int
compileArith(arg, ci)
Word arg;
register compileInfo *ci;
{ code a_func;
  register FunctorDef fdef = functorTerm(*arg);

  if      ( fdef == FUNCTOR_ar_equals2 )	a_func = A_EQ;	/* =:= */
  else if ( fdef == FUNCTOR_ar_not_equal2 )	a_func = A_NE;	/* =\= */
  else if ( fdef == FUNCTOR_smaller2 )	 	a_func = A_LT;	/* < */
  else if ( fdef == FUNCTOR_larger2 )		a_func = A_GT;	/* > */
  else if ( fdef == FUNCTOR_smaller_equal2 )	a_func = A_LE;	/* =< */
  else if ( fdef == FUNCTOR_larger_equal2 )	a_func = A_GE;	/* >= */
  else if ( fdef == FUNCTOR_is2 )		a_func = A_IS;	/* is */
  else return A_NOTARITH;			/* not arith function */

  if ( compileArithArgument(argTermP(*arg, 0), ci) == FALSE )
    return A_ERROR;
  if ( compileArithArgument(argTermP(*arg, 1), ci) == FALSE )
    return A_ERROR;
  Output_0(ci, a_func);

  return A_OK;
}

static
bool
compileArithArgument(arg, ci)
register Word arg;
register compileInfo *ci;
{ int index;

  deRef(arg);

  if ( isInteger(*arg) )		/* integer */
  { Output_1(ci, B_CONST, addXRtable(*arg, ci));
    succeed;
  }
  if ( isReal(*arg) )			/* real */
  { Output_1(ci, B_REAL, addRealXRtable(*arg, ci));
    succeed;
  }
					/* variable */
  if ( isTerm(*arg) && (index = isIndexedVarTerm(*arg)) >= 0 )
  { int first = isFirstVar(&ci->used_var, index);

    if ( index < ci->arity )	/* shared in the head */
    { if ( index < 3 )
      { Output_0(ci, B_VAR0 + index);
	succeed;
      }
      Output_0(ci, B_VAR);
    } else
    { if ( index < 3 && !first )
      { Output_0(ci, B_VAR0 + index);
        succeed;
      }
      Output_0(ci, first ? B_FIRSTVAR : B_VAR);
    }          
    Output_a(ci, VAROFFSET(index));
    succeed;
  }

  { FunctorDef fdef;
    int ar;
    Word a;

    if ( isAtom(*arg) )
    { fdef = lookupFunctorDef((Atom)*arg, 0);
      ar = 0;
      a = NULL;
    } else if ( isTerm(*arg) )
    { fdef = functorTerm(*arg);
      ar = fdef->arity;
      a = argTermP(*arg, 0);      
    } else
      return warning("Illegal argument to arithmic function");

    if ( (index = indexArithFunction(fdef, ci->module)) < 0 )
      return warning("%s/%d: unknown arithmetic operator",
		     stringAtom(fdef->name), fdef->arity);

    for(; ar > 0; a++, ar--)
      TRY( compileArithArgument(a, ci) );

    switch(fdef->arity)
    { case 0:	Output_1(ci, A_FUNC0, index); break;
      case 1:	Output_1(ci, A_FUNC1, index); break;
      case 2:	Output_1(ci, A_FUNC2, index); break;
      default:  Output_2(ci, A_FUNC,  index, (code) fdef->arity); break;
    }

    succeed;
  }
}
#endif O_COMPILE_ARITH


		/********************************
		*  PROLOG DATA BASE MANAGEMENT  *
		*********************************/

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Assert is used by assert[az] and record_clause/2 (used by  the  compiler
toplevel).  It asserts a term in the database, either at the start or at
the  end  of  the predicate and if a file is present, updates the source
administration, checks for reconsults, etc.

The warnings should help explain what is going on here.
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if PROTO
Clause
assert(Word term, char where, Atom file)
#else
Clause
assert(term, where, file)
Word term;
char where;
Atom file;
#endif
{ Clause clause;
  Procedure proc;
  Definition def;
  Module source_module = (file != (Atom) NULL ? modules.source : (Module) NULL);
  Module module = source_module;
  static Procedure current = (Procedure) NULL;

  term = stripModule(term, &module);

  DEBUG(9, printf("compiling "); pl_write(term); printf(" ... "););
  if ((clause = compile(term, module)) == (Clause) NULL)
    return (Clause) NULL;
  DEBUG(9, printf("ok\n"));
  proc = clause->procedure;
  def = proc->definition;

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
If file is defined, we are called from record_clause/2.  This code takes
care of reconsult, redefinition, etc.
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  if (file != (Atom) NULL)
  { SourceFile sf;

    sf = lookupSourceFile(file);

    if ( true(def, MULTIFILE) )
    { if (sf->count != 1)
      { warning("Multifile predicate %s not updated", procedureName(proc) );
	freeClause(clause);
	return (Clause) NULL;
      }
      return assertProcedure(proc, clause, where) == FALSE ? (Clause) NULL
							   : clause;
    }

    if (def->module != module)
    { if ( true(def->module, SYSTEM) )
        warning("Attempt to redefine a system predicate: %s", 
				procedureName(proc));
      else
	warning("%s/%d already imported from module %s", 
				stringAtom(proc->functor->name), 
				proc->functor->arity, 
				stringAtom(proc->definition->module->name) );
      freeClause(clause);
      return (Clause) NULL;
    }

    if (def->source == sf && def->source_count == sf->count)
    { if (proc != current)
      { if ( (debugstatus.styleCheck & DISCONTIGUOUS_STYLE) &&
	     false(def, DISCONTIGUOUS) )
	  warning("Clauses of %s are not together in the source file", 
				procedureName(proc) );
	current = proc;
      }

      return assertProcedure(proc, clause, where) == FALSE ? (Clause) NULL
							   : clause;
    }

    current = proc;

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
This `if' locks predicates as system predicates  if  we  are  in  system
mode, the predicate is still undefined and is not dynamic or multifile.
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

    if ( SYSTEM_MODE &&
	 false(def, SYSTEM) &&
	 false(def, DYNAMIC) &&
	 false(def, MULTIFILE) &&
	 def->definition.clauses == (Clause) NULL)
    { set(def, SYSTEM);
      set(def, HIDE_CHILDS);

      def->source = sf;
      def->source_count = sf->count;

      return assertProcedure(proc, clause, where) == FALSE ? (Clause) NULL
							   : clause;
    }

    if ( true(def, SYSTEM) && !SYSTEM_MODE )
    { warning("Attempt to redefine a system predicate: %s", 
				procedureName(proc) );
      freeClause(clause);
      return (Clause) NULL;
    }

    if (def->source != sf)
    { if (def->definition.clauses != (Clause) NULL)
      {	abolishProcedure(proc, module);
	warning("Redefined: %s", procedureName(proc) );
      }
      def->source = sf;
      def->source_count = sf->count;
    } else
    { if (def->source_count < sf->count)	/* reconsult */
      { removeClausesProcedure(proc);
        def->source = sf;
      	def->source_count = sf->count;
      }
    }
    return assertProcedure(proc, clause, where) == FALSE ? (Clause) NULL
							 : clause;
  }

  /* assert[az]/1 */

  if ( true(def, SYSTEM) && false(def, DYNAMIC) )
  { warning("Attempt to redefine a system predicate: %s", 
				procedureName(proc) );
    freeClause(clause);
    return (Clause) NULL;
  }

  if ( def->module != module && false(def, DYNAMIC) )
  { warning("Attempt to redefined an imported predicate %s", 
			      procedureName(proc) );
    freeClause(clause);
    return (Clause) NULL;
  }
  set(def, DYNAMIC);			/* Make dynamic on first assert */

  return assertProcedure(proc, clause, where) == FALSE ? (Clause) NULL
						       : clause;
}

word
pl_assertz(term)
Word term;
{ return assert(term, 'z', (Atom)NULL) == (Clause)NULL ? FALSE : TRUE;
}

word
pl_asserta(term)
Word term;
{ return assert(term, 'a', (Atom)NULL) == (Clause)NULL ? FALSE : TRUE;
}

word
pl_assertz2(term, ref)
Word term, ref;
{ Clause clause = assert(term, 'z', (Atom)NULL);

  if (clause == (Clause)NULL)
    fail;

  return unifyAtomic(ref, pointerToNum(clause));
}

word
pl_asserta2(term, ref)
Word term, ref;
{ Clause clause = assert(term, 'a', (Atom)NULL);

  if (clause == (Clause)NULL)
    fail;

  return unifyAtomic(ref, pointerToNum(clause));
}

word
pl_record_clause(term, file)
Word term, file;
{ if (!isAtom(*file) )
    fail;

  return assert(term, 'z', (Atom)*file) == (Clause)NULL ? FALSE : TRUE;
}  


		/********************************
		*          DECOMPILER           *
		*********************************/

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
The decompiler is rather straightforwards.  First it  will  construct  a
term  with  variables  for  the  head  and an array of variables for all
variables in  the  clause.   Next  the  head  arguments  are  filled  by
decompiling  the head code.  Finally the body is decompiled.  The latter
is slightly more complex as it is given in reverse polish notation.   We
first  will  skip  the  argument  filling  code,  looking for the actual
calling code.  This provides us the functor and arity of the  subclause.
Then we create a term, back up and fill the arguments.
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#define PC	(di->pc)
#define XR	(di->xr)
#define ARGP	(di->argp)

typedef struct
{ Code	pc;				/* pc for decompilation */
  Word  xr;				/* xr table for decompilation */
  Word	argp;				/* argument pointer */
  Word	variables[MAXVARIABLES];	/* variable table */
} decompileInfo;

forwards bool	unifyVar P((Word, Word *, int));
forwards bool	decompile_head P((Clause, Word, decompileInfo *));
forwards bool	decompileBody P((decompileInfo *, code, Code));
forwards void	build_term P((FunctorDef, decompileInfo *));

static bool
unifyVar(var, vars, i)
register Word var;
register Word vars[];
register int i;
{ DEBUG(3, printf("unifyVar(%d, %d, %d)\n", var, vars, i) );
  if (vars[i] == (Word)NULL)
  { vars[i] = var;
    succeed;
  }
  return (bool) pl_unify(var, vars[i]);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
decompileHead()  is  public  as  it  is  needed  to  update  the   index
information  for  clauses  if this changes when the predicate is already
defined.  Also for intermediate  code  file  loaded  clauses  the  index
information  is  recalculated  as  the  constants in the XR table may be
different accross runs.
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

bool
decompileHead(clause, head)
Clause clause;
Word head;
{ decompileInfo di;

  return decompile_head(clause, head, &di);
}

static bool
decompile_head(clause, head, di)
Clause clause;
Word head;
register decompileInfo *di;
{ int arity;
  Word argp, argp0;

  deRef(head);

  DEBUG(5, printf("Decompiling head of %s\n", procedureName(clause->procedure)));
  arity = clause->procedure->functor->arity;
  if (arity == 0)
  { TRY(unifyAtomic(head, clause->procedure->functor->name) );
  } else
  { TRY(unifyFunctor(head, clause->procedure->functor) );
  }
  argp0 = argp = argTermP(*head, 0);
  PC = clause->codes;
  XR = clause->externals;

  { register int m;
    register Word *p;

    m = VAROFFSET(clause->variables);	/* index of highest var + 1 */
    for(p = di->variables; m-- > 0;)
      *p++ = (Word) NULL;
  }

  for(;;)
  { switch(decode(*PC++))
    { case I_NOP:
	  continue;
      case H_NIL:
	  TRY(unifyAtomic(argp++, ATOM_nil) );
	  continue;
      case H_REAL:
	  TRY( unifyAtomic(argp++, globalReal(valReal(XR[*PC++]))) );
	  continue;
#if O_STRING
      case H_STRING:
	  TRY( unifyAtomic(argp++, globalString(valString(XR[*PC++]))) );
	  continue;
#endif O_STRING      
      case H_CONST:
	  TRY(unifyAtomic(argp++, XR[*PC++]) );
	  continue;
      case H_CONST0:
	  TRY(unifyAtomic(argp++, XR[0]) );
	  continue;
      case H_CONST1:
	  TRY(unifyAtomic(argp++, XR[1]) );
	  continue;
      case H_CONST2:
	  TRY(unifyAtomic(argp++, XR[2]) );
	  continue;
      case H_SINT:
	  TRY(unifyAtomic(argp++, consNumFromCode(*PC++)));
	  continue;
      case H_FIRSTVAR:
      case H_VAR:
	  TRY(unifyVar(argp++, di->variables, *PC++) );
	  continue;
      case H_VOID:
	{ int arg;		/* FIRSTVAR in the head */
	  if ((arg = (int)(argp - argp0)) < arity && arg >= 0)
	    TRY(unifyVar(argp, di->variables, VAROFFSET(arg)) );
	  argp++;
	  continue;
	}
      case H_FUNCTOR:
	{ register FunctorDef fdef = (FunctorDef) XR[*PC++];

      common_functor:
	  TRY(unifyFunctor(argp, fdef) );
	  *aTop++ = argp + 1;
	  verifyStack(argument);
	  deRef(argp);
	  argp = argTermP(*argp, 0);
	  continue;
      case H_FUNCTOR0:	  fdef = (FunctorDef) XR[0]; 	goto common_functor;
      case H_FUNCTOR1:	  fdef = (FunctorDef) XR[1]; 	goto common_functor;
      case H_FUNCTOR2:	  fdef = (FunctorDef) XR[2]; 	goto common_functor;
      case H_LIST:	  fdef = FUNCTOR_dot2;		goto common_functor;
	}
      case I_POP:
	  argp = *--aTop;
	  continue;
      case I_POPN:
	  aTop -= *PC++;
	  argp = *aTop;
	  continue;
      case I_EXIT:			/* fact */
      case I_ENTER:			/* fix H_VOID, H_VOID, I_ENTER */
	{ int arg;

	  for(arg = (int)(argp - argp0); arg < arity; arg++)
	   TRY(unifyVar(argp++, di->variables, VAROFFSET(arg)) );

	  succeed;
	}
      default:
	  sysError("Illegal instruction in clause head: %d = %d", PC[-1], decode(PC[-1]));
	  fail;
    }
  }
}

#define isVarRef(w)	(isRef(w) && (int)unRef(w) < MAXVARIABLES \
						? (int)unRef(w) : -1)

bool
decompile(clause, term)
Clause clause;
Word term;
{ decompileInfo dinfo;
  register decompileInfo *di = &dinfo;
  Word head, body;

  deRef(term);

  if ((clause->subclauses) == 0)			/* fact */
  { return decompileHead(clause, term);
  } else
  { TRY(unifyFunctor(term, FUNCTOR_prove2) );
    head = argTermP(*term, 0);
    body = argTermP(*term, 1); deRef(body);
  }

  TRY( decompile_head(clause, head, di) );
  ARGP = (Word) lTop;

  decompileBody(di, I_EXIT, (Code) NULL);

  { word b;
    int var;

    setVar(b);
    ARGP--;
    if ( (var = isVarRef(*ARGP)) >= 0 )
      unifyVar(&b, di->variables, var);
    else
      b = *ARGP;

    return (bool) pl_unify(body, &b);
  }
}


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Body decompilation.  A previous version of this part of the code  worked
top-down,  refining the term given using unification.  This approach has
three advantages:

  - Decompilation will fail as soon as  unification  of  generated  code
    fails.
  - If the body is instantiated no copy will be created  on  the  global
    stack, thus saving memory.
  - Handling variables is somewhat simpler as no intermediate storage is
    needed.

Unfortunately it also has some serious disadvantages:

  - The call/depart code is written in reverse polish notation.   If  we
    work  top-down  we  will need the functor of the subclause before we
    can start working on the arguments.  This implies we  have  to  skip
    the  argument instructions first to find the call/depart instruction
    and then back-up to fill the arguments, introducing one  more  place
    where we need to know the WAM code semantics.
  - With the  introduction  of  nested  reverse  polish  constructs  for
    arithmic  it  gets  very  difficult  to do the decompilation without
    using a stack for  intermediate  data  storage,  building  the  term
    bottom-up.

In the current implementation the head is decompiled in the  unification
style  and the head is decompiled using a stack machine.  This takes the
best of both approaches: the head is not in reverse polish notation  and
is  not  unlikely  to be instantiated (retract/1), while it is very rare
that clause/retract are used with instantiated body.

The decompilation stack is located on top of the local  stack,  as  this
area is not in use during decompilation.
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if PROTO
static bool
decompileBody(register decompileInfo *di, code end, Code until)
#else
static bool
decompileBody(di, end, until)
register decompileInfo *di;
code end;
Code until;
#endif
{ int nested = 0;		/* nesting in FUNCTOR ... POP */
  int pushed = 0;		/* Subclauses pushed on the stack */

  for(; decode(*PC) != end && PC != until; )
  { switch(decode(*PC++))
    {   case I_NOP:	    continue;
	case B_CONST:
			    *ARGP++ = XR[*PC++];
			    continue;
	case B_REAL:
			    *ARGP++ = globalReal(valReal(XR[*PC++]));
			    continue;
	case B_STRING:
			    *ARGP++ = globalString(valString(XR[*PC++]));
			    continue;
	case B_SINT:
			    *ARGP++ = consNumFromCode(*PC++);
			    continue;
      { register int index;      

	case B_ARGVAR:
	case B_ARGFIRSTVAR:
	case B_FIRSTVAR:
	case B_VAR:	    index = *PC++;		goto var_common;
	case B_VAR0:	    index = VAROFFSET(0);	goto var_common;
	case B_VAR1:	    index = VAROFFSET(1);	goto var_common;
	case B_VAR2:	    index = VAROFFSET(2);	var_common:
			    if ( nested )
			      unifyVar(ARGP++, di->variables, index);
			    else
			      *ARGP++ = makeRef(index);
			    continue;
      }
      case B_VOID:
			    setVar(*ARGP++);
			    continue;
      case B_FUNCTOR:
			    *ARGP = globalFunctor((FunctorDef)XR[*PC++]);
			    *aTop++ = ARGP + 1;
			    verifyStack(argument);
			    ARGP = argTermP(*ARGP, 0);
			    nested++;
			    continue;
      case I_POP:
			    ARGP = *--aTop;
			    nested--;
			    continue;
      case I_POPN:
			    aTop -= *PC;
			    nested -= *PC++;
			    ARGP = *aTop;
			    continue;
#if O_COMPILE_ARITH
      case A_FUNC0:
      case A_FUNC1:
      case A_FUNC2:
			    build_term(functorArithFunction(*PC++), di);
			    continue;
      case A_FUNC:
      			    build_term(functorArithFunction(*PC++), di);
      			    PC++;
			    continue;
#endif O_COMPILE_ARITH
      { FunctorDef f;
#if O_COMPILE_ARITH
	case A_LT:	    f = FUNCTOR_smaller2;	goto f_common;
	case A_LE:	    f = FUNCTOR_smaller_equal2;	goto f_common;
	case A_GT:	    f = FUNCTOR_larger2;	goto f_common;
	case A_GE:	    f = FUNCTOR_larger_equal2;	goto f_common;
	case A_EQ:	    f = FUNCTOR_ar_equals2;	goto f_common;
	case A_NE:	    f = FUNCTOR_ar_not_equal2;	goto f_common;
	case A_IS:	    f = FUNCTOR_is2;		goto f_common;
#endif O_COMPILE_ARITH
	case I_APPLY:	    f = FUNCTOR_apply2;		f_common:
			    build_term(f, di);
			    pushed++;
			    continue;
      }
      case I_CUT:	    *ARGP++ = (word) ATOM_cut;
			    pushed++;
			    continue;
      case I_DEPART:
      case I_CALL:
			    build_term(((Procedure)XR[*PC++])->functor, di);
			    pushed++;
			    continue;
      case I_USERCALL:
			    pushed++;
			    continue;
#if O_COMPILE_OR
#define DECOMPILETOJUMP { int to_jump = (int) *PC++; \
			  decompileBody(di, (code)-1, PC+to_jump); \
			}
      case C_CUT:
      case C_VAR:
      case C_JMP:
			    PC++;
			    continue;
      case C_OR:				/* A ; B */
			    DECOMPILETOJUMP;	/* A */
			    PC--;		/* get C_JMP argument */
			    DECOMPILETOJUMP;	/* B */
			    build_term(FUNCTOR_semicolon2, di);
			    pushed++;
			    continue;
      case C_NOT:				/* \+ A */
			  { PC += 2;		/* skip the two arguments */
			    decompileBody(di, C_CUT, (Code)NULL);   /* A */
			    PC += 3;		/* skip C_CUT <n> and C_FAIL */
			    build_term(FUNCTOR_not_provable1, di);
			    pushed++;
			    continue;
			  }
      case C_IFTHENELSE:			/* A -> B ; C */
			  { Code adr1;
			    int jmp;

			    PC++;		/* skip the 'MARK' variable */
			    jmp  = (int) *PC++;
			    adr1 = PC+jmp;

			    decompileBody(di, C_CUT, (Code)NULL);   /* A */
			    PC += 2;		/* skip the cut */
			    decompileBody(di, (code)-1, adr1);	    /* B */
			    build_term(FUNCTOR_ifthen2, di);
			    PC--;
			    DECOMPILETOJUMP;	/* C */
			    build_term(FUNCTOR_semicolon2, di);
			    pushed++;
			    continue;
			  }
      case C_MARK:				/* A -> B */
			    PC++;
			    decompileBody(di, C_CUT, (Code)NULL);   /* A */
			    PC += 2;
			    decompileBody(di, C_END, (Code)NULL);   /* B */
			    PC++;
			    build_term(FUNCTOR_ifthen2, di);
			    pushed++;
			    continue;
#endif O_COMPILE_OR
      case I_EXIT:
			    break;
      default:
	  sysError("Illegal instruction in clause body: %d", PC[-1]);
	  /*NOTREACHED*/
    }
  }

  while( pushed-- > 1)
    build_term(FUNCTOR_comma2, di);

  succeed;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Build the actual term.  The arguments are on  the  decompilation  stack.
We  construct a term of requested arity and name, copy `arity' arguments
from the stack into the term and finally  push  the  term  back  on  the
stack.
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static void
build_term(f, di)
register FunctorDef f;
register decompileInfo *di;
{ word term;
  int arity;
  register Word a;

  if ( f->arity == 0 )
  { *ARGP++ = (word) f->name;
    return;
  }    

  term = globalFunctor(f);
  arity = f->arity;
  a = argTermP(term, arity-1);

  ARGP--;
  for( ; arity-- > 0; a--, ARGP-- )
  { register int var;

    if ( (var = isVarRef(*ARGP)) >= 0 )
      unifyVar(a, di->variables, var);
    else
      *a = *ARGP;
  }
  ARGP++;

  *ARGP++ = term;
}


word
pl_clause(p, term, ref, h)
Word p, term, ref;
word h;
{ Procedure proc;
  Clause clause;
  Module module = (Module)NULL;

  if ( ForeignControl(h) == FRG_CUTTED )
    succeed;

  if (!isVar(*ref))  
  { Module defModule;
    Word head, body;

    if (!isInteger(*ref))
      return warning("clause/3: illegal reference");

    clause = (Clause) numToPointer(*ref);
    
    if (!inCore(clause) || !isClause(clause))
      return warning("clause/3: Invalid integer reference");
	
    if (decompile(clause, term) == FALSE)
      fail;

    proc = clause->procedure;
    defModule = proc->definition->module;

    if (isTerm(*term) && functorTerm(*term) == FUNCTOR_module2)
    { p = stripModule(p, &module);
      if (module != defModule)
	fail;
    }

    if (isVar(*p))
    { if (defModule != MODULE_user &&
	   defModule != contextModule(environment_frame) )
      { unifyFunctor(p, FUNCTOR_module2);
	unifyAtomic(argTermP(*p, 0), defModule->name);
	p = argTermP(*p, 1);
      }
    }

    TRY(splitClause(term, &head, &body));
    return pl_unify(p, head);
  }

  if ( ForeignControl(h) == FRG_FIRST_CALL)
  { if ( (proc = findProcedure(p)) == (Procedure) NULL ||
         true(proc->definition, FOREIGN) )
      fail;
    clause = proc->definition->definition.clauses;
  } else
  { clause = (Clause) ForeignContextAddress(h);
    proc = clause->procedure;
  }

  p = stripModule(p, &module);

  for(; clause; clause = clause->next)
  { bool det;

    if ((clause = findClause(clause, 
			     proc->functor->arity == 0 ? (Word)NULL
						       : argTermP(*p, 0), 
			     proc->definition, &det)) == (Clause)NULL)
      fail;

    if (decompile(clause, term) == FALSE)
      continue;
    if (unifyAtomic(ref, pointerToNum(clause)) == FALSE)
      continue;

    if ( det == TRUE )
      succeed;

    ForeignRedo(clause->next);
  }

  fail;
}
