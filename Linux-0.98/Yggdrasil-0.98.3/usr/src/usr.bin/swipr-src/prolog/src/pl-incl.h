/*  @(#) pl-incl.h 1.5.0 (UvA SWI) Jul 30, 1990

    Copyright (c) 1990 Jan Wielemaker. All rights reserved.
    See ../LICENCE to find out about your rights.
    jan@swi.psy.uva.nl

    Purpose: SWI-Prolog general include file
*/

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Include Machine Desciption (md-*) file.  If -DMD=md-sun.h  or  something
similar  is  passed  as  cpp  flag,  this  machine  description is used.
Otherwise "md.h" is used,  which  is  supposed  to  be  a  link  to  the
appropriate machine description file.
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifdef MD
#include MD
#else
#include "md.h"
#endif

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
		      PROLOG SYSTEM OPTIONS

These are not really options normally.  They are there because I use  to
add  new  features  conditional  using  #if ... #endif.  In many cases I
leave them in for ducumentation purposes.   Notably  O_STRING  might  be
handy for it someone wants to add a data type to the system.

  O_STRING
      Include data type string.  This  feature  does  not  rely  on  any
      system  feature.   It  hardly has any consequences for the system.
      Because of its experimental nature it is optional.  The definition
      of the predicates operating on strings might change.
      (NOTE: Currently some of the boot files rely on strings. It is NOT
      suggested to leave them out).
  O_COMPILE_OR
      Compile ->/2, ;/2 and |/2 into WAM.  This  no  longer  is  a  real
      option.   the mechanism to handle cuts without compiling ;/2, etc.
      has been taken out.
  O_COMPILE_ARITH
      Include arithmetic compiler (compiles is/2, >/2, etc. into WAM).
  O_PROLOG_FUNCTUIONS
      Include evaluatable Prolog functions into the arithmetic module.
  O_AUTOINDEX
      Include code to gues the best predicate indexing pattern.  This is
      not yet very well worked out, neither will be in the near  future.
      just left in for the case I want to return to this subject.
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#define PL_KERNEL		1
#define O_COMPILE_OR		1
#define O_COMPILE_ARITH		1
#define O_STRING		1
#define O_AUTOINDEX		0
#define O_PROLOG_FUNCTIONS	1

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
The macros below try to establish a common basis for various  compilers,
so  we  can  write  most  of the real code without having to worry about
compiler limits and differences.

The  current  version  has  prototypes  defined   for   all   functions.
Unfortunately  there  are  still a lot of old compilers around and it is
hard to write and maintain code that runs on both old and new compilers.
This has worked on TURBO_C not very long ago.
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if O_SHORT_SYMBOLS
#include "pl-ssymb.h"		/* Redefine long symbols to avoid clashes */
#endif

#include <stdio.h>
#if unix
#include <sys/types.h>
#include <signal.h>
#endif
#include <setjmp.h>

#if ANSI
#define PROTO 1
#if !mips
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#endif
#include <stdarg.h>		/* variable arity handling */
#endif

#if !ANSI || AIX		/* AIX stdarg still broken */
#if mips
#include "/usr/include/varargs.h"
#else
#include <varargs.h>
#endif
#endif

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
A common basis for C keywords.
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if !__GNUC__
#define volatile		/* volatile functions do not return */
#define inline			/* inline functions are integrated in */
				/* their caller */
#define signed			/* some compilers don't have this. */
#endif

#define forwards static		/* forwards function declarations */
#if PROTO
#define P(type) type
#else
#define P(type) ()
#endif

#ifndef GLOBAL			/* global variables */
#define GLOBAL extern
#endif

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Booleans,  addresses,  strings  and other   goodies.   Note that  ANSI
compilers have  `Void'.   This should  be  made part  of  the  general
platform as well.
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

typedef int			bool;
typedef double			real;
#if O_NO_VOID_POINTER
typedef char *			Void;
#else
typedef void *			Void;
#endif

#if unix
#ifdef SIGNAL_HANDLER_TYPE
typedef SIGNAL_HANDLER_TYPE (*handler_t)();
#else
typedef void (*handler_t)();
#endif
#endif

#ifndef TRUE
#define TRUE			1
#define FALSE			0
#endif
#define succeed			return TRUE
#define fail			return FALSE
#define TRY(goal)		{ if ((goal) == FALSE) fail; }

/*typedef unsigned short	ushort;*/
#if !O_ULONG_PREDEFINED
typedef unsigned long		ulong;
#endif
typedef char *			caddress;

#define EOS			('\0')
#define streq(s, q)		((strcmp((s), (q)) == 0))

#ifndef abs
#define abs(x)			((x) < 0 ? -(x) : (x))
#endif
				/* n is 2^m !!! */
#define ROUND(p, n)		((((p) + (n) - 1) & ~((n) - 1)))
#define addPointer(p, n)	((char *)(p) + (long)(n))

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
			     LIMITS

Below are some arbitrary limits on object sizes.  Feel free  to  enlarge
them,  but  be aware of the fact that this increases memory requirements
and  slows  down  for  some  of  these  options.    Also,   MAXARITY   <
MAXVARIABLES, MAXVARIABLES and MAXEXTERNALS must be lower that 64 K. One
day,  I  should  try  to  get  rid  of these limits.  This requires some
redesign of parts of the compiler.
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#define LINESIZ			1024	/* size of a data line */
#define MAXARITY		128	/* arity of predicate */
#define MAXVARIABLES		256	/* number of variables/clause */
#define MAXEXTERNALS		512	/* external references of a clause */
#define MAXCODES		10000	/* number of byte codes of a clause */
#define MAXSIGNAL		32	/* highest system signal number */

				/* Prolog's largest int */
#define PLMAXINT		((1L<<(32 - MASK_BITS - LMASK_BITS - 1)) - 1)
				/* Prolog's smallest int */
#define PLMININT		(-(1L<<(32 - MASK_BITS - LMASK_BITS - 1)))
#if vax
#define MAXREAL			(1.701411834604692293e+38)
#else					/* IEEE double */
#define MAXREAL			(1.79769313486231470e+308)
#endif

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Macros to handle hash tables.  See pl-table.c for  details.   First  the
sizes  of  the  hash  tables are defined.  Note that these should all be
2^N.
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#define ATOMHASHSIZE		1024	/* global atom table */
#define FUNCTORHASHSIZE		512	/* global functor table */
#define PROCEDUREHASHSIZE	512	/* predicates in module user */
#define MODULEPROCEDUREHASHSIZE 128	/* predicates in other modules */
#define RECORDHASHSIZE		512	/* global recorda/recordz table */
#define MODULEHASHSIZE		64	/* global module table */
#define PUBLICHASHSIZE		32	/* Module export table */
#define OPERATORHASHSIZE	256	/* global operator table */
#define FLAGHASHSIZE		256	/* global flag/3 table */
#define ARITHHASHSIZE		64	/* arithmetic function table */

#define pointerHashValue(p, size) ((int)(((long)(p)>>2) & ((size)-1)))

#define for_table(s, t) for(s = firstHTable(t); s; s = nextHTable(t, s))
#define return_next_table(t, v) \
	{ for((v) = (v)->next; isRef((word)(v)) && (v); (v) = *((t *)unRef(v))) \
	  if ( (v) == (t)NULL ) \
	    succeed; \
	  ForeignRedo(v); \
	}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Foreign language interface definitions.  Note that these macros MUST  be
consistent  with  the  definitions  in  pl-itf.h, which is included with
users foreign language code.
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#define FRG_CUT 	(0x80000000L)		/* highest bit */
#define FRG_MASK	(0x40000000L)		/* Mask to indicate redo */
#define FRG_MASK_MASK	(FRG_CUT|FRG_MASK)

#define FRG_FIRST_CALL	(0)
#define FRG_CUTTED	(1)
#define FRG_REDO	(2)

#define FIRST_CALL	(0L)

#define ForeignRedo(v)		return (word) (((long)(v) & ~FRG_MASK_MASK) \
					      | FRG_MASK)
#define ForeignControl(h)	((h) == FIRST_CALL ? FRG_FIRST_CALL : \
				 (h) & FRG_CUT	   ? FRG_CUTTED : \
						     FRG_REDO)
#define ForeignContext(h)	(((long)(h) << 2) >> 2)
#define ForeignContextAddress(h) ((Void)((long)(h) & ~FRG_MASK_MASK))

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Virtual machine instruction declarations.  Prefixes:

  I_	General instructions
  B_	Body specific version
  H_	Head specific versin
  A_	Arithmetic compilation specific
  C_	Control (compilation of ;/2, etc.)

Numbering these things is arbitrary,  but  for  fast  operation  of  the
switch  in  pl-wam.c,  numbering  should start at 0 and be without gaps.
I_HIGHEST must be made equal to the highest  value  of  the  instruction
codes (only used with O_ASM_SWITCH; consistency is checked at startup).
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#define I_NOP		((code) 0)		/* nop */
#define I_ENTER		((code) 1)		/* enter body */
#define I_CALL		((code) 2)		/* call procedure */
#define I_DEPART	((code) 3)		/* last call of procedure */
#define I_EXIT		((code) 4)		/* exit procedure */
#define B_FUNCTOR	((code) 5)		/* start functor */
#define H_FUNCTOR	((code) 6)
#define I_POP		((code) 7)		/* end functor */
#define I_POPN		((code) 8)		/* end functor */
#define B_VAR		((code) 9)		/* variable */
#define H_VAR		((code)10)
#define B_CONST		((code)11)		/* constant (atomic) */
#define H_CONST		((code)12)
#define H_REAL		((code)13)		/* real in the head */
#if O_STRING
#define H_STRING	((code)14)		/* string in the head */
#endif O_STRING

#define B_FIRSTVAR	((code)15)		/* first occurrence of var */
#define H_FIRSTVAR	((code)16)
#define B_VOID		((code)17)		/* anonimous variables */
#define H_VOID		((code)18)
#define B_ARGFIRSTVAR	((code)19)		/* body vars nested in functor */
#define B_ARGVAR	((code)20)

#define H_NIL		((code)21)		/* [] in the head */
#define H_CONST0	((code)22)		/* H_CONST 0, etc. */
#define H_CONST1	((code)23)
#define H_CONST2	((code)24)

#define H_LIST		((code)25)		/* ./2 in the head */
#define H_FUNCTOR0	((code)26)		/* H_FUNCTOR 0, etc. */
#define H_FUNCTOR1	((code)27)
#define H_FUNCTOR2	((code)28)

#define B_VAR0		((code)29)		/* B_VAR 0 */
#define B_VAR1		((code)30)		/* B_VAR 1 */
#define B_VAR2		((code)31)		/* B_VAR 2 */

#define H_SINT		((code)32)		/* Small integer in the Head */
#define B_SINT		((code)33)		/* Small integer in the Body */

#define I_USERCALL	((code)34)		/* variable in body (call/1) */
#define I_CUT		((code)35)		/* ! */
#define I_APPLY		((code)36)		/* apply/2 */

#if O_COMPILE_ARITH
#define A_FUNC0		((code)37)		/* nullary arithmic function */
#define A_FUNC1		((code)38)		/* unary arithmic function */
#define A_FUNC2		((code)39)		/* binary arithmic function */
#define A_FUNC		((code)40)		/* n-ary arithmic function */
#define A_LT		((code)41)		/* < */
#define A_GT		((code)42)		/* > */
#define A_LE		((code)43)		/* =< */
#define A_GE		((code)44)		/* >= */
#define A_EQ		((code)45)		/* =:= */
#define A_NE		((code)46)		/* =\= */
#define A_IS		((code)47)		/* is */
#endif O_COMPILE_ARITH

#if O_COMPILE_OR
#define C_OR		((code)48)		/* In-clause backtract point */
#define C_JMP		((code)49)		/* Jump over code */
#define C_MARK		((code)50)		/* Sub-clause cut mark */
#define C_CUT		((code)51)		/* cut to corresponding mark */
#define C_IFTHENELSE	((code)52)		/* if-then-else start */
#define C_VAR		((code)53)		/* make a variable */
#define C_END		((code)54)		/* dummy to help decompiler */
#define C_NOT		((code)55)		/* same as C_IFTHENELSE */
#define C_FAIL		((code)56)		/* fail */
#endif O_COMPILE_OR

#define B_REAL		((code)57)		/* REAL in body */
#define B_STRING	((code)58)		/* STRING in body */

#define I_HIGHEST	((code)58)		/* largest WAM code !!! */

#if O_ASM_SWITCH
extern long wam_table[];			/* switch jump table */
GLOBAL char *dewam_table;			/* decoding table */

#define encode(wam) ((code) wam_table[wam])	/* WAM --> internal */
#define decode(c)   ((code) dewam_table[c])	/* internal --> WAM */
#else O_ASM_SWITCH
#define encode(wam) (wam)
#define decode(wam) (wam)
#endif O_ASM_SWITCH

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Arithmetic comparison
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#define LT 1
#define GT 2
#define LE 3
#define GE 4
#define NE 5
#define EQ 6

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Operator types
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#define	OP_FX	0
#define OP_FY	1
#define OP_XF	2
#define OP_YF	3
#define OP_XFX	4
#define OP_XFY	5
#define OP_YFX	6
#define	OP_YFY	7

#define OP_PREFIX  1
#define OP_INFIX   2
#define OP_POSTFIX 3

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Files and streams
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#define F_CLOSED	0		/* closed entry */
#define F_READ		1		/* open for reading */
#define F_WRITE		2		/* open for writing */
#define F_APPEND	6		/* open for append writing */

#define streamOutput(stream, goal) \
	{ int SOn = streamNo(stream, F_WRITE); \
	  int SOout = Output; \
	  word SOrval; \
	  if ( SOn < 0 ) fail; \
	  Output = SOn; \
	  SOrval = goal; \
	  Output = SOout; \
	  return SOrval; \
	}

#define streamInput(stream, goal) \
	{ int SOn = streamNo(stream, F_READ); \
	  int SOin = Input; \
	  word SOrval; \
	  if ( SOn < 0 ) fail; \
	  Input = SOn; \
	  SOrval = goal; \
	  Input = SOin; \
	  return SOrval; \
	}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Type fields.  These codes are  included  in  a  number  of  the  runtime
structures  at  a  fixed  point, so the runtime environment can tell the
difference.
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#define ATOM_TYPE	1		/* an atom */
#define FUNCTOR_TYPE	2		/* a Functor */
#define PROCEDURE_TYPE	3		/* a procedure */
#define RECORD_TYPE	4		/* a record list */

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
			  PROLOG DATA REPRESENTATION

Prolog data objects live on various places:

	- In the variable and argument slots of environment frames.
	- As arguments to complex terms on the global stack.
	- In records (recorda/recorded database) in the heap.
	- In variables in foreign language functions.

All Prolog data is packed into a `word'.  A word is  a  32  bit  entity.
The top 3 bits are used to indicate the type; the bottom 2 bits are used
for  the  garbage  collector.   The  bits  for the garbage collector are
always 0 during normal execution.  This implies we do not have  to  care
about  them  for  pointers  and  as  pointers  always  point  to 4 bytes
entities, the range is not harmed by the garbage collection bits.

The remaining 27 bits can hold a  unique  representation  of  the  value
itself  or  can be a pointer to the global stack where the real value is
stored.  We call the latter type of data `indirect'.

Below is a description of the  representation  used  for  each  type  of
Prolog data:

INTEGER
    Integers are stored in the  27  remaining  bits  of  a  word.   This
    implies they are limited to +- 2^26.
REAL
    For a real, the 27 bits are a pointer to a 8 byte unit on the global
    stack.  For both words of the 8 byte unit, the top 3  and  bottom  2
    bits  are  reserved  for identification and garbage collection.  The
    remaining bits hold the exponent and mantisse.  See pack_real()  and
    unpack_real() in pl-alloc.c for details.
ATOM
    For atoms, the 27 bits represent a pointer  to  an  atom  structure.
    Atom  structures are cells of a hash table.  Equality of the pointer
    implies equality of the atoms and visa versa.  Atom  structures  are
    not  collected by the garbage collector and thus live for the entire
    Prolog session.
STRING
    For a string, the 27 bits are a pointer to the  global  stack.   The
    first  word  of  the  string  again reserves  the top 3 and bottom 2
    bits.  The remaining bits indicate the lenght of the  string.   Next
    follows a 0 terminated character string.  Finally a word exactly the
    same  as the header word, to allow the garbage collector to traverse
    the stack downwards and identify the string.
TERM
    For a compound term, the 27 bits are a pointer to the global  stack.
    the  first  word there is a pointer to a functordef structure, which
    determines the name and arity of the  term.   functordef  structures
    are  cells  of  a hash table like atom structures.  They to live for
    the entire Prolog session.  Next, there are just as  many  words  as
    the  arity  of the term, each word representing a normal Prolog data
    object.
VARIABLES
    An unbound variable is represented by NULL.
REFERENCES
    References are the result of sharing variables.   If  two  variables
    must  share,  the one with the shortest livetime is made a reference
    pointer to the other.  This way a tree of reference pointers can  be
    constructed.   The root of the tree is the variable with the longest
    livetime.  To bind the entire tree of variables this root is  bound.
    The  others remain reference pointers.  This implies that ANY prolog
    data object might be a reference  pointer  to  another  Prolog  data
    object,  holding  the  real  value.  To find the real value, a macro
    called deRef() is available.

    The direction of reference pointers is critical.  It MUST  point  in
    the direction of the longest living variable.  If not, the reference
    pointer  will  point  into  the  dark  if  the other end dies.  This
    implies that if both cells are part of an environment frame, the one
    in the child function (closest to the top of the stack)  must  point
    to  the  one in the parent function.  If one is on the local and one
    on the global stack, the  pointer  must  point  towards  the  global
    stack.   Inside  the global stack it is irrelevant.  If backtracking
    destroys a variable, it also will reset the reference towards it  if
    there is one.
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Masks.  Currently the top 3 bits of a word are used as  mask.   The  top
bit  is  reserved  for  references,  which  are  represented as negative
numbers.  At least the 68020 is faster in checking for negative  numbers
and  turning  negative  numbers into positive ones.  This trick gives an
overall performance increase of about 5%. The other two  bits  are  used
for  integers, reals and strings.  Both reals and strings are `indirect'
data types (tagged pointers to the real value).  This  has  consequences
in  unification an similar functions.  Therefore a macro `isIndirect(w)'
has been introduced.  If you decide to change things here make sure this
macro  operates  oppropriately  and  is  FAST  (its  used  in   critical
unification code).

(The RT under AIX uses a somewhat irregular memory model:

0x10000000 text... 0x20000000 ...data...bss...malloc... ...stack 03fffffff

This conflicts with the data representation used for SUN.  So we put the
tag bits on bits 32, 31 and 29 instead of 32, 31 and 30.   This  reduces
the range of integers to +- 2^25.  (Macros have to be rewritten))
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#define REF_MASK	0x80000000L	/* Reference (= negative) */
#define INDIRECT_MASK	0x40000000L	/* Indirect constant */
#if O_DATA_AT_0X2
#define INT_MASK	0x10000000L	/* Integer mask */
#else
#define INT_MASK	0x20000000L	/* Integer mask */
#endif
#define MARK_MASK	0x00000001L	/* GC marking bit */

#if O_16_BITS
#define MASK_BITS	4		/* high order mask bits */
#define LMASK_BITS	1		/* low order mask bits */
#define DMASK_BITS	5		/* DATA_TAG_MASK bits */
#define FIRST_MASK	0x10000000L	/* first member of relocation chain */
#define STRING_MASK	0x60000000L	/* Header mask on global stack */
#define REAL_MASK	0x68000000L	/* Header mask on global stack */
#define MASK_MASK	(INT_MASK|REF_MASK|INDIRECT_MASK|FIRST_MASK)
#define DATA_TAG_MASK	0xf8000000L	/* Indirect data type mask */
#else
#if O_DATA_AT_0X2
#define MASK_BITS	4		/* high order mask bits */
#define STRING_MASK	0x50000000L	/* Header mask on global stack */
#define REAL_MASK	0x70000000L	/* Header mask on global stack */
#else
#define MASK_BITS	3		/* high order mask bits */
#define STRING_MASK	0x60000000L	/* Header mask on global stack */
#define REAL_MASK	0x70000000L	/* Header mask on global stack */
#endif
#define LMASK_BITS	2		/* low order mask bits */
#define DMASK_BITS	4		/* DATA_TAG_MASK bits */
#define FIRST_MASK	0x00000002L	/* first member of relocation chain */
#define MASK_MASK	(INT_MASK|REF_MASK|INDIRECT_MASK)
#define DATA_TAG_MASK	0xf0000000L	/* Indirect data type mask */
#endif

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Common Prolog objects typedefs.
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

typedef ulong 			word;		/* Anonimous 4 byte object */
typedef word *			Word;		/* a pointer to anything */
typedef unsigned short		code;		/* bytes codes */
typedef code *			Code;		/* pointer to byte codes */
typedef int			Char;		/* char that can pass EOF */
typedef word			(*Func)();	/* foreign functions */

typedef struct atom *		Atom;		/* atom */
typedef struct functor *	Functor;	/* complex term */
typedef struct functorDef *	FunctorDef;	/* name/arity pair */
typedef struct procedure *	Procedure;	/* predicate */
typedef struct definition *	Definition;	/* predicate definition */
typedef struct clause *		Clause;		/* compiled clause */
typedef struct code_info *	CodeInfo;	/* WAM op-code info */
typedef struct operator *	Operator;	/* see pl-op.c, pl-read.c */
typedef struct record *		Record;		/* recorda/3, etc. */
typedef struct recordList *	RecordList;	/* list of these */
typedef struct module *		Module;		/* predicate modules */
typedef struct sourceFile *	SourceFile;	/* file adminitration */
typedef struct table *		Table;		/* (numeric) hash table */
typedef struct symbol *		Symbol;		/* symbol of hash table */
typedef struct localFrame *	LocalFrame;	/* environment frame */
typedef struct trail_entry *	TrailEntry;	/* Entry of train stack */
typedef struct data_mark	mark;		/* backtrack mark */
typedef struct index *		Index;		/* clause indexing */
typedef struct stack *		Stack;		/* machine stack */
typedef struct lock *		Lock;		/* GC data lock */

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Many of the structures have a large number of booleans  associated  with
them.   Early  versions defined these using `unsigned <name> : 1' in the
structure definition.  When I ported SWI-Prolog to a  machine  that  did
not  understand  this  construct  I  decided  to pack all the flags in a
short.  As this allows us to set, clear and test combinations  of  flags
with one operation, it turns out to be faster as well.
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#define true(s, a)		((s)->flags & (a))
#define false(s, a)		(!true((s), (a)))
#define set(s, a)		((s)->flags |= (a))
#define clear(s, a)		((s)->flags &= ~(a))
#define clearFlags(s)		((s)->flags = 0)

#define NONDETERMINISTIC	(0x0002)
#define DISCONTIGUOUS		(0x0004)
#define DYNAMIC			(0x0008)
#define ERASED			(0x0010)
#define FOREIGN			(0x0020)
#define HIDE_CHILDS		(0x0040)
#define MULTIFILE		(0x0080)
#define PROFILE_TICKED		(0x0100)
#define SPY_ME			(0x0200)
#define SYSTEM			(0x0400)
#define TRACE_ME		(0x0800)
#define TRANSPARENT		(0x1000)
#define AUTOINDEX		(0x2000)
#define INDEXABLE		(0x4000)
#define UNKNOWN			(0x8000)

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Handling environment (or local stack) frames.
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#define FR_LEVEL		(0xFFFFFFF0L)
#define FR_CUT			(0x00000001L)
#define FR_NODEBUG		(0x00000002L)
#define FR_SKIPPED		(0x00000004L)
#define FR_MARKED		(0x00000008L)

#define ARGOFFSET		((int) sizeof(struct localFrame))

#define setLevelFrame(fr, l)	{ (fr)->flags &= ~FR_LEVEL;   \
				  (fr)->flags |= ((l) << 4); \
				}
#define levelFrame(fr)		(fr->flags >> 4)
#define incLevel(fr)		(fr->flags += 0x10)
#define argFrameP(f, n)		((Word)((f)+1) + (n))
#define argFrame(f, n)		(*argFrameP((f), (n)) )
#define varFrameP(f, n)		((Word)(f) + (n))
#define varFrame(f, n)		(*varFrameP((f), (n)) )
#define parentFrame(f)		((f)->parent ? (f)->parent\
					     : (LocalFrame)varFrame((f), -1))
#define slotsFrame(f)		(true((f)->procedure->definition, FOREIGN) ? \
				      (f)->procedure->functor->arity : \
				      (f)->clause->slots)
#define contextModule(f)	((f)->context)

#define leaveClause(clause) { if ( --clause->references == 0 && \
				   true(clause, ERASED) ) \
				unallocClause(clause); \
			    }

#define leaveFrame(fr) { if ( true(fr->procedure->definition, FOREIGN) ) \
			   leaveForeignFrame(fr); \
			 else \
			 { if ( fr->clause ) \
			     leaveClause(fr->clause); \
			 } \
		       }

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Macros to turn pointers into Prolog integers and  vice-versa.   Used  to
pass  references  for  recorda,  erase, clause/3, etc.  As AIX addresses
range from 0x2000000, which is above the maximum integer value  for  the
AIX  version  we  substract  this value and add it again when converting
integers to pointers.
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if O_DATA_AT_0X2
#define pointerToNum(p)		consNum(((long)(p)-0x20000000)/sizeof(int))
#define numToPointer(n)		((Word)(valNum(n)*sizeof(int)+0x20000000))
#elif O_DATA_AT_OX1
#define pointerToNum(p)		consNum(((long)(p)-0x10000000)/sizeof(int))
#define numToPointer(n)		((Word)(valNum(n)*sizeof(int)+0x10000000))
#else
#define pointerToNum(p)		consNum((long)(p)/sizeof(int))
#define numToPointer(n)		((Word)(valNum(n)*sizeof(int)))
#endif

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Macros to handle the anonimous types.  'w' implies we expect a word, 'p'
for a pointer.
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#define unMask(w)		((w) & ~MASK_MASK)
#define mask(w)			(w & MASK_MASK)
#define consNum(n)		((word) (unMask((n)<<LMASK_BITS) | INT_MASK))
#define valNum(w)		((long) ((w)<<MASK_BITS)>>(MASK_BITS+LMASK_BITS))
#define consNumFromCode(c)	consNum((signed short)(c))
#define valReal(w)		unpack_real((Word)unMask(w))
#if O_STRING
#define allocSizeString(l)	(ROUND(l+1, sizeof(word)) + 2 * sizeof(word))
#define valString(w)		((char *)((Word)unMask(w)+1))
#define sizeString(w)		(((long)(*(Word)unMask(w))<<DMASK_BITS)>> \
						(DMASK_BITS+LMASK_BITS))
#define equalString(w1,w2)	(sizeString(w1) == sizeString(w2) && \
				 streq(valString(w1), valString(w2)))
#endif O_STRING

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Handling references.
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#define makeRef(p)	((word)(-(long)(p)))
#define unRef(w)	((Word)(-(long)(w)))
#define isRef(w)	((long)(w) < 0)
#define deRef(p)	{ while(isRef(*(p))) (p) = unRef(*(p)); }
#define deRef2(p, d)	{ (d) = (p); deRef((d)); }

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Handling dereferenced arbitrary Prolog runtime objects.
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#define isMasked(w)	(mask(w) != (word) NULL)
#define isIndirect(w)	((w) & INDIRECT_MASK)
#define isInteger(w)	((w) & INT_MASK)
#define isReal(w)	(isIndirect(w) && \
			 (*((Word)unMask(w)) & DATA_TAG_MASK) == REAL_MASK)
#if O_STRING
#define isString(w)	(isIndirect(w) && \
			 (*((Word)unMask(w)) & DATA_TAG_MASK) == STRING_MASK)
#endif O_STRING
#define isNumber(w)	(isInteger(w) || isReal(w))
#define isVar(w)	((w) == (word) NULL)
#define nonVar(w)	((w) != (word) NULL)
#define isPointer(w)	(nonVar(w) && !isMasked(w))
#define pointerIsAtom(w) (((Atom)(w))->type == ATOM_TYPE)
#define pointerIsFunctor(w) (((Functor)(w))->type == FUNCTOR_TYPE)
#define isAtom(w)	(isPointer(w) && pointerIsAtom(w))
#define isFunctor(w)	(isPointer(w) && pointerIsFunctor(w))
#define stringAtom(w)	(((Atom)(w))->name)
#define isPrimitive(w)  (isVar(w) || isAtomic(w))
#define isAtomic(w)	(nonVar(w) && (isMasked(w) || pointerIsAtom(w)))
#define isTerm(w)	(isPointer(w) && !pointerIsAtom(w))
#define isList(w)	(isPointer(w) && functorTerm(w) == FUNCTOR_dot2)
#define functorTerm(w)	(((Functor)(w))->definition)
#define argTerm(w, n)	(*argTermP((w), (n)))
#define argTermP(w, n)	(((Word)(w)+1+(n)))
#define isProcedure(w)	(((Procedure)(w))->type == PROCEDURE_TYPE)
#define isRecordList(w)	(((RecordList)(w))->type == RECORD_TYPE)

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Heuristics functions to determine whether an integer reference passed to
erase and assert/2, clause/3, etc.  really points to a clause or record.
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#define inCore(a)	((char *)(a) >= hBase && (char *)(a) <= hTop)
#define isClause(c)	(inCore(((Clause)(c))->procedure) && \
			  isProcedure(((Clause)(c))->procedure))
#define isRecord(r)	(inCore(((Record)(r))->list) && \
			  isRecordList(((Record)(r))->list))

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
At times an abort is not allowed because the heap  is  inconsistent  the
programmer  should  call  startCritical  to start such a code region and
endCritical to end it.
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#define startCritical { critical++; }
#define endCritical   { if (--critical == 0 && aborted == TRUE) pl_abort(); }

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
LIST processing macros.

    isNil(w)		word is the nil list ([]).
    isList(w)		word is a './2' term.
    HeadList(p)		Pointer to the head of list *p (NOT dereferenced).
    TailList(p)		Pointer to the tail of list *p (NOT dereferenced).
    APPENDLIST(l, p)	Append *p to list *l. l points to the tail afterwards.
    CLOSELIST(l)	Unify the tail of the list with [].
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#define isNil(w)	((w) == (word) ATOM_nil)
#define HeadList(p)	(argTermP(*(p), 0) )
#define TailList(p)	(argTermP(*(p), 1) )

#define APPENDLIST(l, p) { TRY(unifyFunctor(l, FUNCTOR_dot2) ); \
			   TRY(pl_unify(HeadList(l), p) ); \
			   l = TailList(l); deRef(l); \
			 }
#define CLOSELIST(l)     { TRY(unifyAtomic(l, ATOM_nil)); }


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Handling variables: creating a variable, trailing its assignment, making
a snap shot of the runtime environment and backtrack back to it.
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#define setVar(w)	((w) = (word) NULL)
#define Trail(p)	{ (tTop++)->address = (p); \
			  verifyStack(trail); \
			}
#define Mark(b)		{ (b).trailtop = tTop; \
			  (b).globaltop = gTop; \
			}
#define Undo(b)		{ register TrailEntry tt = tTop; \
			  while(tt > (b).trailtop) \
			    setVar(*(--tt)->address); \
			  tTop = tt; \
			  gTop = (b).globaltop; \
			}


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Structure declarations that must be shared across multiple files.
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

struct atom
{ Atom		next;		/* next in chain */
  int		type;		/* ATOM_TYPE */
  char *	name;		/* name associated with atom */
};

struct index
{ ulong		key;		/* key of index */
  ulong		varmask;	/* variable field mask */
};

struct functorDef
{ FunctorDef	next;		/* next in chain */
  int		type;		/* FUNCTOR_TYPE */
  Atom		name;		/* Name of functor */
  int		arity;		/* arity of functor */
};

struct clause
{ Procedure	procedure;	/* procedure we belong to */
  Clause	next;		/* next clause of procedure */
  Word		externals;	/* External references */
  Code		codes;		/* byte codes of clause */
  struct index	index;		/* index key of clause */
  unsigned int	references;	/* no of. references from interpreter */
  short		code_size;	/* size of byte code array */
  short		subclauses;	/* number of subclauses in body (decompiler) */
  code		XR_size;	/* size of external reference table */
  code		variables;	/* number of variables */
  code		slots;		/* # variables holding Prolog data */
  ushort	flags;		/* Flag field holding: */
		/* ERASED	   Clause is retracted, but referenced */
		/* INDEXABLE	   Clause has indexable 1st argument */
};

struct code_info
{ char		*name;		/* name of the code */
  char		arguments;	/* # arguments code takes */
  code		code;		/* number of the code */
};

struct data_mark
{ TrailEntry	trailtop;	/* top of the trail stack */
  Word		globaltop;	/* top of the global stack */
};

struct functor
{ FunctorDef	definition;	/* Name/Arity */
  word		arguments[1];	/* arguments vector */
};

struct operator
{ Operator	next;		/* next of chain */
  Atom		name;		/* name of operator */
  short		type;		/* OP_FX, ... */
  short		priority;	/* priority of operator */
};

struct procedure
{ FunctorDef	functor;	/* Name/Arity of procedure */
  int		type;		/* PROCEDURE_TYPE */
  Definition	definition;	/* definition of procedure */
};

struct definition
{ union
  { Clause	clauses;		/* clause list of procedure */
    Func	function;		/* function pointer of procedure */
  } definition;
  Clause	lastClause;		/* last clause of list */
  Module	module;			/* module of the predicate */
  SourceFile	source;			/* source file of predicate */
#if O_PROFILE
  int		profile_ticks;		/* profiler: call times active */
  int		profile_calls;		/* profiler: number of calls */
#endif O_PROFILE
  ulong		indexPattern;		/* indexed argument pattern */
  char		indexCardinality;	/* cardinality of index pattern */
#if O_AUTOINDEX
  int		indexMerit;		/* how badly do we want it? */
#endif
  short		source_count;		/* times (re)consulted */
  ushort	flags;			/* booleans: */
		/*	FOREIGN		   foreign predicate? */
		/*	PROFILE_TICKED	   has been ticked this time ? */
		/*	TRACE_ME	   is my call visible? */
		/*	HIDE_CHILDS	   hide childs for the debugger? */
		/*	SPY_ME		   spy point set? */
		/*	DYNAMIC		   dynamic predicate? */
		/*	MULTIFILE	   defined over more files? */
		/*	SYSTEM		   system predicate */
		/*	TRANSPARENT	   procedure transparent to modules */
		/*	DISCONTIGUOUS	   procedure might be discontiguous */
		/*	DETERMINISTIC	   deterministic foreign (not used) */
		/*	AUTOINDEX	   Apply heuristically best index */
};

struct localFrame
{ Code		programPointer;	/* pointer into program */
  LocalFrame	parent;		/* parent local frame */
  Clause	clause;		/* Current clause of frame */
  LocalFrame	backtrackFrame;	/* Frame for backtracking */
  Procedure	procedure;	/* Procedure we are running */
  mark		mark;		/* data backtrack mark */
  Module	context;	/* context module of frame */
  ulong		flags;		/* packet long holding: */
		/*	LEVEL	   recursion level (28 bits) */
		/*	FR_CUT     has frame been cut ? */
		/*	FR_NODEBUG don't debug this frame ? */
};  

struct record
{ RecordList	list;		/* list I belong to */
  Record	next;		/* next of chain */
  word		term;		/* term associated */
  int		n_vars;		/* number of variables */
  Word		variables;	/* array of variables */
};

struct recordList
{ RecordList	next;		/* next record chain with same key */
  int		type;		/* RECORD_TYPE */
  word		key;		/* key of record */
  Record	firstRecord;	/* first record associated with key */
  Record	lastRecord;	/* last record associated with key */
};

struct sourceFile
{ Atom		name;		/* name of source file */
  SourceFile	next;		/* next of chain */
  int		count;		/* number of times loaded */
  long		time;		/* load time of file */
  bool		system;		/* system sourcefile: do not reload */
};

struct module
{ Atom		name;		/* name of module */
  SourceFile	file;		/* file from which module is loaded */
  Table		procedures;	/* predicates associated with module */
  Table		public;		/* public predicates associated */
  Module	super;		/* Import predicates from here */
  ushort	flags;		/* booleans: */
		/*	SYSTEM	   system module */
		/*	UNKNOWN	   trap unknown predicates */
};

struct trail_entry
{ Word		address;	/* address of the variable */
};

struct table
{ int		size;		/* size of hash table */
  Symbol	entries[1];	/* array of hash symbols */
};

struct symbol
{ Symbol	next;		/* next in chain */
  word		name;		/* name entry of symbol */
  word		value;		/* associated value with name */
};

struct lock
{ unsigned	type  : 2;	/* Type of lock */
  unsigned	value : 30;	/* Anonymous value */
};  

		/********************************
		*             STACKS            *
		*********************************/

#if O_CAN_MAP || O_SHARED_MEMORY
#define O_DYNAMIC_STACKS 1
#endif

#if O_SHARED_MEMORY
#define O_SHM_FREE_IMMEDIATE 1
#define MAX_STACK_SEGMENTS  20
#define STACK(type) \
	{ type		base;		/* base address of the stack */     \
	  type		top;		/* current top of the stack */      \
	  type		max;		/* allocated maximum */		    \
	  long		limit;		/* how big it is allowed to grow */ \
	  long		maxlimit;	/* maximum limit */                 \
	  char		*name;		/* Symbolic name of the stack */    \
	  long		segment_initial;/* initial size */		    \
	  int		segment_double;	/* times to double */		    \
	  int		segment_top;	/* Next segment to be allocated */  \
	  struct							    \
	  { caddress	base;		/* Base of this segment */	    \
	    long	size;		/* Size of this segment */	    \
	  } segments[MAX_STACK_SEGMENTS];				    \
	}
#else !O_SHARED_MEMORY
#define STACK(type) \
	{ type		base;		/* base address of the stack */     \
	  type		top;		/* current top of the stack */      \
	  type		max;		/* allocated maximum */		    \
	  long		limit;		/* how big it is allowed to grow */ \
	  long		maxlimit;	/* maximum limit */                 \
	  char		*name;		/* Symbolic name of the stack */    \
	}
#endif

struct stack STACK(caddress);		/* Anonymous stack */

GLOBAL struct
{ struct STACK(LocalFrame) local;	/* local (environment) stack */
  struct STACK(Word)	   global;	/* local (environment) stack */
  struct STACK(TrailEntry) trail;	/* trail stack */
  struct STACK(Word *)	   argument;	/* argument stack */
  struct STACK(Lock)	   lock;	/* Foreign code locks */
} stacks;

#define tBase	(stacks.trail.base)
#define tTop	(stacks.trail.top)
#define tMax	(stacks.trail.max)

#define lBase	(stacks.local.base)
#define lTop	(stacks.local.top)
#define lMax	(stacks.local.max)

#define gBase	(stacks.global.base)
#define gTop	(stacks.global.top)
#define gMax	(stacks.global.max)

#define aBase	(stacks.argument.base)
#define aTop	(stacks.argument.top)
#define aMax	(stacks.argument.max)

#define pBase	(stacks.lock.base)
#define pTop	(stacks.lock.top)
#define pMax	(stacks.lock.max)

GLOBAL char *	hTop;			/* highest allocated heap address */
GLOBAL char *	hBase;			/* lowest allocated heap address */

#if O_DYNAMIC_STACKS
#define STACKVERIFY(g)			/* hardware stack verify */
#define verifyStack(s)
#else
#define STACKVERIFY(g)	{ g; }
#define verifyStack(s)	{ if ( stacks.s.top >= stacks.s.max ) \
			    outOf((Stack)&stacks.s); }
#endif

		/********************************
		*       GLOBAL VARIABLES        *
		*********************************/

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
General global variables  to  indicate  status  or  communicate  between
modules.
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

GLOBAL int 	  critical;		/* are we in critical code for abort? */
GLOBAL bool	  aborted;		/* have we been aborted */
GLOBAL LocalFrame environment_frame;	/* current context frame */
GLOBAL bool	  novice;		/* novice user */

		/********************************
		*            STATUS             *
		*********************************/

GLOBAL struct
{ bool		requested;		/* GC is requested by stack expander */
  int		blocked;		/* GC is blocked now */
  bool		active;			/* Currently running? */
  LocalFrame	segment;		/* Collected segment boundary */
  long		collections;		/* # garbage collections */
  long		global_gained;		/* global stack bytes collected */
  long		trail_gained;		/* trail stack bytes collected */
  real		time;			/* time spent in collections */
} gc_status;

GLOBAL struct
{ Atom		symbolfile;		/* current symbol file */
  Atom		orgsymbolfile;		/* symbol file we started with */
} loaderstatus;

#if O_PCE
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
PCE runs with the SunView notifier package.  This  causes  trouble  with
abort(),  which  performs  a  long jump.  The flags below have to ensure
everything goes well.
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#define	mayNotify()   { if (notify_status.dispatching) \
			  notify_status.active++; \
		      }
#define hasNotified() { if (aborted) \
			{ notify_status.active = 0; \
			  aborted = FALSE; \
			  pl_abort(); \
			} \
			notify_status.active--; \
		      }

GLOBAL struct
{ int		active;			/* are we in notify code? */
  bool		dispatching;		/* is notify_do_dispatch() on? */
  jmp_buf	context;		/* save context of in Get0() */
  bool		called;			/* are we called from Pce? */
  bool		abort_is_save;		/* can abort when handling events */
} notify_status;
#else
#define mayNotify()
#define hasNotified()
#endif O_PCE

#define NO_PROFILING		0
#define CUMULATIVE_PROFILING	1
#define PLAIN_PROFILING		2

/*  statistics information */

GLOBAL struct
{ long		inferences;		/* logical inferences made */
  long		heap;			/* heap in use */
  int		atoms;			/* No. of atoms defined */
  int		functors;		/* No. of functors defined */
  int		predicates;		/* No. of predicates defined */
  int		modules;		/* No. of modules in the system */
  long		externals;		/* No. of clause external references */
  long		codes;			/* No. of byte codes generated */
  long		collections;		/* No. of garbage collections */
  long		global_gained;		/* No. of cells global stack gained */
  long		trail_gained;		/* No. of cells trail stack gained */
#if O_PROFILE
  int		profiling;		/* profiler is on? */
  long		profile_ticks;		/* profile ticks total */
#endif O_PROFILE
} statistics;

		/********************************
		*            MODULES            *
		*********************************/

#define MODULE_user	(modules.user)
#define MODULE_system	(modules.system)

GLOBAL struct
{ Module	typein;			/* module for type in goals */
  Module	source;			/* module we are reading clauses in */
  Module	user;			/* user module */
  Module	system;			/* system predicate module */
} modules;

GLOBAL Table	moduleTable;		/* hash table of available modules */

		/********************************
		*         PREDICATES            *
		*********************************/

GLOBAL Procedure	PROCEDURE_alt1;	/* $alt/1, see C_OR */
GLOBAL Procedure	PROCEDURE_garbage_collect0;

extern struct code_info	codeTable[];

		/********************************
		*            DEBUGGER           *
		*********************************/

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Tracer communication declarations.
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#define ACTION_CONTINUE	0
#define ACTION_RETRY	1
#define ACTION_FAIL	2
#define ACTION_IGNORE	3
#define ACTION_AGAIN	4

#define CALL_PORT	0x1		/* port masks */
#define EXIT_PORT	0x2
#define FAIL_PORT	0x4
#define REDO_PORT	0x8
#define UNIFY_PORT	0x10
#define VERY_DEEP	10000000L	/* deep skiplevel */

#define LONGATOM_CHECK	    0x1		/* read/1: error on long atoms */
#define SINGLETON_CHECK	    0x2		/* read/1: check singleton vars */
#define DOLLAR_STYLE	    0x4		/* dollar is lower case */
#define DISCONTIGUOUS_STYLE 0x8		/* warn on discontiguous predicates */
#if O_STRING
#define O_STRING_STYLE	    0x10	/* read ".." as string instead of list */
#endif O_STRING
#define MAXNEWLINES	    5		/* maximum number of newlines in atom */
#define SYSTEM_MODE	    (debugstatus.styleCheck & DOLLAR_STYLE)

GLOBAL struct debuginfo
{ long		skiplevel;		/* current skip level */
  bool		tracing;		/* are we tracing? */
  bool		debugging;		/* are we debugging? */
  ulong		leashing;		/* ports we are leashing */
  ulong		visible;		/* ports that are visible */
  int		style;			/* print style of tracer */
  bool		showContext;		/* tracer shows context module */
  int		styleCheck;		/* source style checking */
  int		suspendTrace;		/* tracing is suspended now */
} debugstatus;

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Administration of loaded intermediate code files  (see  pl-wic.c).  Used
with the -c option to include all these if necessary.
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

typedef struct state * State;

struct state
{ char *	name;			/* name of state */
  State		next;			/* next state loaded */
};

GLOBAL State stateList;			/* list of loaded states */

#if unix
GLOBAL struct
{ handler_t os;				/* Os signal handler */
  handler_t user;			/* User signal handler */
  bool catched;				/* Prolog catches this one */
} signalHandlers[MAXSIGNAL];
#endif

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Include debugging info to make it (very) verbose.  SECURE adds  code  to
check  consistency mainly in the WAM interpreter.  Prolog gets VERY slow
if SECURE is  used.   DEBUG  is  not  too  bad  (about  20%  performance
decrease).
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#define REL(a)		((Word)(a) - (Word)(lBase))

#if O_DEBUG
#define DEBUG(n, g) { if (status.debugLevel >= n) { g; fflush(stdout); } }
#else
#define DEBUG(a, b) 
#endif

/* #define SECURE(g) g */
#define SECURE(g)

#include "pl-os.h"			/* OS dependencies */
#include "pl-funcs.h"			/* global functions */
#include "pl-main.h"			/* Declarations needed by pl-main.c */

extern struct atom atoms[];
extern struct functorDef functors[];

#include "pl-atom.ih"
#include "pl-funct.ih"
