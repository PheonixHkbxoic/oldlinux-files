/*  @(#) pl-prims.c 1.5.0 (UvA SWI) Jul 30, 1990

    Copyright (c) 1990 Jan Wielemaker. All rights reserved.
    See ../LICENCE to find out about your rights.
    jan@swi.psy.uva.nl

    Purpose: primitive built in
*/

#include "pl-incl.h"
#include "pl-ctype.h"

forwards int	pl_se P((Word, Word, int));
forwards void	resetVariables P((Word));
forwards bool	freeVariables P((Word, Word *, bool));
forwards bool	unifyStringWithList P((char *, Word));
forwards char 	*prependBase P((int, char *));
forwards bool	isPrefix P((char *, char *));
forwards bool	boolPlease P((bool *, Word, Word));
forwards word	copyTerm P((Word, Table));

		/********************************
		*         TYPE CHECKING         *
		*********************************/


word
pl_nonvar(k)
register Word k;
{ if (isVar(*k))
    fail;

  succeed;
}

word
pl_var(k)
register Word k;
{ if (isVar(*k))
    succeed;

  fail;
}

word
pl_integer(k)
register Word k;
{ if (isInteger(*k))
    succeed;

  fail;
}

word
pl_float(k)
register Word k;
{ if (isReal(*k))
    succeed;

  fail;
}

#if O_STRING
word
pl_string(k)
register Word k;
{ if (isString(*k))
    succeed;;

  fail;
}
#endif O_STRING

word
pl_number(k)
register Word k;
{ if ( isNumber(*k) )
    succeed;

  fail;
}

word
pl_atom(k)
register Word k;
{ if (isAtom(*k))
    succeed;

  fail;
}

word
pl_atomic(k)
register Word k;
{ if (isAtomic(*k))
    succeed;

  fail;
}

word
pl_ground(term)
register Word term;
{ register int arity;

  deRef(term);

  if (isVar(*term) )
    fail;
  if (!isTerm(*term) )
    succeed;
  arity = functorTerm(*term)->arity;
  for(term = argTermP(*term, 0); arity > 0; arity--, term++)
    TRY( pl_ground(term) );

  succeed;
}

		/********************************
		*           EQUALITY            *
		*********************************/

word
pl_unify(t1, t2)			/* =/2 */
register Word t1, t2;
{ mark m;

  Mark(m);
  if (unify(t1, t2) == FALSE)
  { Undo(m);
    fail;
  }

  succeed;  
}

word
pl_notunify(t1, t2)
register Word t1, t2;
{ bool rval;
  mark m;
  
  Mark(m);
  rval = unify(t1, t2);
  Undo(m);

  if (rval == TRUE)
    fail;

  succeed;
}

word
pl_equal(t1, t2)			/* ==/2 */
register Word t1, t2;
{ int arity, n;

  deRef(t1);
  deRef(t2);

  if (isVar(*t1) )
  { if (t1 == t2)
      succeed;
    fail;
  }

  if (*t1 == *t2)
    succeed;

  if ( isIndirect(*t1) )
  {
#if O_STRING
    if ( isString(*t1) )
    { if ( isString(*t2) && equalString(*t1, *t2) )
        succeed;
      fail;
    }
#endif O_STRING
    if (isReal(*t2) && valReal(*t1) == valReal(*t2) )
      succeed;
    fail;
  }

  if (!isTerm(*t1) || !isTerm(*t2) ||
       functorTerm(*t1) != functorTerm(*t2) )
    fail;

  arity = functorTerm(*t1)->arity;
  t1 = argTermP(*t1, 0);
  t2 = argTermP(*t2, 0);
  for(n=0; n<arity; n++, t1++, t2++)
    TRY(pl_equal(t1, t2) );

  succeed;
}

word
pl_nonequal(t1, t2)		/* \== */
Word t1, t2;
{ if (pl_equal(t1, t2) == FALSE)
    succeed;

  fail;
}


		/********************************
		*        STANDARD ORDER         *
		*********************************/

/*  Rules:

    Var < Atom < String < number < Term
    
    OldVar < NewVar	(not relyable)
    Atom:	alphabetically
    Strings:	alphabetically
    number:	value
    Term:	alphabetically / arity / recursive

 ** Tue Apr 26 16:25:50 1988  jan@swivax.UUCP (Jan Wielemaker)  */

#define LESS -1
#define EQUAL  0
#define GREATER  1

#if !O_NO_LEFT_CAST
#define w1 ((word)t1)
#define w2 ((word)t2)
#endif

int
compareStandard(t1, t2)
register Word t1, t2;
{ int rval;
  int arity;
  int n;
  FunctorDef f1, f2;
#if O_NO_LEFT_CAST
  register word w1, w2;
#endif

  deRef(t1);
  deRef(t2);

  if (isVar(*t1) )
  { if (isVar(*t2) )
      return t1 < t2 ? LESS : t1 == t2 ? EQUAL : GREATER;
    return LESS;
  }

  w1 = *t1; w2 = *t2;

  if (isVar(w2) )
    return GREATER;

  if (isAtom(w1) )
  { if (isAtom(w2) )
      return strcmp(stringAtom(w1), stringAtom(w2));
    return LESS;
  }
  if (isAtom(w2) )
    return GREATER;

#if O_STRING
  if ( isString(w1) )
  { if ( isString(w2) )
      return strcmp(valString(w1), valString(w2));
    return LESS;
  }
  if ( isString(w2) )
    return GREATER;
#endif O_STRING

  if ( isNumber(w1) )
  { if ( !isNumber(w2) )
      return LESS;

    if ( isInteger(w1) && isInteger(w2) )
    { long i1 = valNum(w1);
      long i2 = valNum(w2);

      return i1 < i2 ? LESS : i1 == i2 ? EQUAL : GREATER;
    } else
    { double f1 = (isInteger(w1) ? (double)valNum(w1) : valReal(w1));
      double f2 = (isInteger(w2) ? (double)valNum(w2) : valReal(w2));
    
      return f1 < f2 ? LESS : f1 == f2 ? EQUAL : GREATER;
    }
  }
  if ( isNumber(w2) )
    return GREATER;
  
  SECURE(if (!isTerm(w1) || !isTerm(w2)) sysError("Unknown type"));

  f1 = functorTerm(w1);
  f2 = functorTerm(w2);

  if ((rval = strcmp(f1->name->name, f2->name->name)) != EQUAL)
    return rval;
  if (f1->arity > f2->arity)
    return GREATER;
  if (f2->arity > f1->arity)
    return LESS;

  arity = f1->arity;
  t1 = argTermP(w1, 0);
  t2 = argTermP(w2, 0);

  for(n=0; n<arity; n++, t1++, t2++)
  { if ((rval = compareStandard(t1, t2)) != EQUAL)
      return rval;
  }

  return EQUAL;
}

word
pl_lessStandard(t1, t2)		/* @</2 */
Word t1, t2;
{ if (compareStandard(t1, t2) < 0)
    succeed;
  fail;
}

word
pl_lessEqualStandard(t1, t2)		/* @=</2 */
Word t1, t2;
{ if (compareStandard(t1, t2) <= 0)
    succeed;
  fail;
}

word
pl_greaterStandard(t1, t2)		/* @>/2 */
Word t1, t2;
{ if (compareStandard(t1, t2) > 0)
    succeed;
  fail;
}

word
pl_greaterEqualStandard(t1, t2)	/* @>=/2 */
Word t1, t2;
{ if (compareStandard(t1, t2) >= 0)
    succeed;
  fail;
}

		/********************************
		*     STRUCTURAL EQUIVALENCE    *
		*********************************/
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
The  idea  for  this  predicate  is  taken  from  the  usenet   network.
Unfortunately I can't recall the author of the note.

Structural equivalency is stronger then unifyable (=), but  weaker  then
pure equivalence (==). Two terms are structural equivalent if their tree
representation is equivalent. Examples:

  a =@= A		--> false
  A =@= B		--> true
  foo(A, B) =@= foo(C, D)	--> true
  foo(A, A) =@= foo(B, C)	--> false
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


static int
pl_se(t1, t2, index)
register Word t1, t2;
int index;
{ int arity, n;

  deRef(t1);
  deRef(t2);

  if (isVar(*t1))
  { if (isVar(*t2))
    { unifyFunctor(t1, FUNCTOR_var1);
      unifyFunctor(t2, FUNCTOR_var1);
      unifyAtomic(argTermP(*t1, 0), consNum(index));
      unifyAtomic(argTermP(*t2, 0), consNum(index));
      
      return ++index;
    }
    fail;
  }

  if (*t1 == *t2)
    succeed;

  if (isIndirect(*t1) )
  { 
#if O_STRING
    if (isString(*t1))
    { if ( isString(*t2) && equalString(*t1, *t2) )
        succeed;
      fail;
    }
#endif O_STRING
    if (isReal(*t2) && valReal(*t1) == valReal(*t2) )
      succeed;
    fail;
  }

  if (!isTerm(*t1) || !isTerm(*t2) ||
       functorTerm(*t1) != functorTerm(*t2) )
    fail;

  arity = functorTerm(*t1)->arity;
  t1 = argTermP(*t1, 0);
  t2 = argTermP(*t2, 0);
  for(n=0; n<arity; n++, t1++, t2++)
    if ((index = pl_se(t1, t2, index)) == FALSE)
      fail;

  return index;
}

word
pl_structural_equal(t1, t2)
Word t1, t2;
{ mark m;
  bool rval;

  Mark(m);
  rval = pl_se(t1, t2, 1);
  Undo(m);

  return rval == FALSE ? FALSE : TRUE;
}

word
pl_structural_nonequal(t1, t2)
Word t1, t2;
{ return pl_structural_equal(t1, t2) == FALSE ? TRUE : FALSE;
}


		/********************************
		*         TERM HACKING          *
		*********************************/

word
pl_functor(t, f, a)
Word t, f, a;
{ int arity;

  if (isVar(*t) )
  { if (isAtom(*f) && isInteger(*a) )
    { arity = (int) valNum(*a);
      if (arity == 0)
	return unifyAtomic(t, *f);
      if (arity < 0)
        fail;
      return unifyFunctor(t, lookupFunctorDef((Atom)*f, arity));
    }
    fail;
  }
  if (isAtom(*t) )
  { TRY(unifyAtomic(f, *t) );
    return unifyAtomic(a, consNum(0));
  }
  if (!isTerm(*t))
    fail;

  TRY(unifyAtomic(f, functorTerm(*t)->name) );
  return unifyAtomic(a, consNum(functorTerm(*t)->arity));
}

word
pl_arg(n, term, arg)
register Word n, term, arg;
{ int argn;

  if (!isInteger(*n))
    return warning("arg/3: first argument in not an integer");
  if (!isTerm(*term))
    return warning("arg/3: second argument in not a term");
  argn = (int) valNum(*n);
  if (argn < 1 || argn > functorTerm(*term)->arity)
    fail;

  return pl_unify(argTermP(*term, argn-1), arg);
}

/*  Determine the length of a list. If the list is not proper (or not
    a list at all) -1 is returned.

 ** Mon Apr 18 16:29:01 1988  jan@swivax.UUCP (Jan Wielemaker)  */

int
lengthList(list)
Word list;
{ int length = 0;

  while(!isNil(*list) )
  { if (!isList(*list) )
      return -1;			/* not a proper list */
    length++;
    list = TailList(list);
    deRef(list);
  }
  if (isNil(*list) )
    return length;

  return -1;
}

word
pl_univ(t, l)
Word t, l;
{ word term;
  int arity, a;
  Word argp;
  int n;
  Word head;

  arity = lengthList(l) - 1;

  if (isVar(*t) )
  { if (arity < 0)			/* list is not proper */
      fail;
    head = HeadList(l);
    deRef(head);
    if (arity == 0)
    { if ( isAtomic(*head) )
	return unifyAtomic(t, *head);
      fail;
    }
    if (!isAtom(*head) )
      fail;
    term = globalFunctor(lookupFunctorDef((Atom)*head, arity) );
    pl_unify(t, &term);
  } else
  { if (isAtomic(*t) )
    { APPENDLIST(l, t);
      CLOSELIST(l);
      succeed;
    }
    if (!isTerm(*t) )
      fail;
    term = *t;
  }

  a = functorTerm(term)->arity;
  if (arity >= 0 && a != arity)
    fail;

  APPENDLIST(l, (Word)&(functorTerm(term)->name));
  argp = argTermP(term, 0);
  for(n = 0; n < a; n++, argp++)
  { APPENDLIST(l, argp);
  }

  CLOSELIST(l);

  succeed;
}

int
numberVars(t, functor, n)
register Word t;
FunctorDef functor;
int n;
{ Word argp;
  int i, arity;

  deRef(t);
  
  if (isVar(*t))
  { unifyFunctor(t, functor);
    unifyAtomic(argTermP(*t, 0), consNum(n));

    return ++n;
  }
  if (isTerm(*t))
  { arity = functorTerm(*t)->arity;
    argp = argTermP(*t, 0);

    for(i=0; i<arity; i++, argp++)
      n = numberVars(argp, functor, n);
    
    return n;
  }

  return n;			/* anything else */
}

word
pl_numbervars(t, atom, start, end)
Word t, atom, start, end;
{ int n;
  FunctorDef functor;

  if (!isInteger(*start) || !isAtom(*atom) )
    fail;
    
  functor = lookupFunctorDef((Atom)*atom, 1);
  n = (int) valNum(*start);
  n = numberVars(t, functor, n);

  return unifyAtomic(end, consNum(n));
}

static void
resetVariables(t)
register Word t;
{ register int arity;

  deRef(t);
  if ( !isTerm(*t) )
    return;
  if ( functorTerm(*t) == FUNCTOR_var1 )  
  { setVar(*t);
    return;
  }
  for(arity=functorTerm(*t)->arity, t=argTermP(*t, 0); arity > 0; arity--, t++)
    resetVariables(t);
}

static bool
freeVariables(t, l, e)
register Word t, *l;
bool e;
{ int arity;
  
  deRef(t);
  if (!isTerm(*t) )
    succeed;

  if (e == TRUE && functorTerm(*t) == FUNCTOR_hat2)
  { resetVariables(argTermP(*t, 0));
    return freeVariables(argTermP(*t, 1), l, e);
  }

  if (functorTerm(*t) == FUNCTOR_var1)
  { setVar(*t);
    APPENDLIST(*l, t);
    succeed;
  }
  for(arity=functorTerm(*t)->arity, t=argTermP(*t, 0); arity > 0; arity--, t++)
    TRY(freeVariables(t, l, e) );

  succeed;
}

word
pl_free_variables(t, l)
Word t, l;
{ numberVars(t, FUNCTOR_var1, 0);
  
  TRY(freeVariables(t, &l, FALSE) );
  CLOSELIST(l);

  succeed;
}

word
pl_e_free_variables(t, l)
Word t, l;
{ numberVars(t, FUNCTOR_var1, 0);
  
  TRY(freeVariables(t, &l, TRUE) );
  CLOSELIST(l);

  succeed;
}

static word
copyTerm(f, vars)
Word f;
Table vars;
{ deRef(f);
  if ( isVar(*f) )
  { Symbol s = lookupLocalTable(vars, f);
    Word p;

    if ( s != (Symbol) NULL )
      return makeRef(s->value);
    p = allocGlobal(sizeof(word));
    setVar(*p);
    addLocalTable(vars, f, p);

    return makeRef(p);
  }
  if ( isTerm(*f) )
  { word copy = globalFunctor(functorTerm(*f));
    Word p, q;
    int n;

    p = argTermP(copy, 0);
    q = argTermP(*f, 0);

    for(n = 0; n < functorTerm(*f)->arity; n++, p++, q++)
      *p = copyTerm(q, vars);

    return copy;
  }

  return *f;			/* atoms, integers, reals and strings */
}


word
pl_copy_term(f, t)
Word f, t;
{ Table vartable;
  word copy;

  initAllocLocal();
  vartable = newLocalTable(16);
  copy = copyTerm(f, vartable);
  stopAllocLocal();

  return pl_unify(t, &copy);
}

static bool
unifyStringWithList(s, l)
char *s;
Word l;
{ word w;

  while(*s)
  { w = consNum((int)*s++);
    APPENDLIST(l, &w);
  }
  CLOSELIST(l);

  succeed;
}

word
stringToList(s)
char *s;
{ word result;
  Word arg;
  FunctorDef dot = FUNCTOR_dot2;

  if (*s == EOS)
    return (word)ATOM_nil;

  result = globalFunctor(dot);
  arg = argTermP(result, 0);
  *arg++ = consNum((int)*s++);

  while(*s)
  { *arg = globalFunctor(dot);
    arg = argTermP(*arg, 0);
    *arg++ = consNum((int)*s++);
  }

  *arg = (word)ATOM_nil;

  return result;
}

char *
listToString(list)
register word list;
{ char *result = (char *) lTop;
  char *s = result;
  int c;
  register Word arg;
  Word tail;

  while(isList(list) && !isNil(list))
  { arg = argTermP(list, 0);
    deRef(arg);
    if (isInteger(*arg) && (c=(int)valNum(*arg)) > 0 && c < 128)
    { *s++ = (char) c;
      STACKVERIFY( if (s > (char *)lMax) outOf((Stack)&stacks.local) );
      tail = argTermP(list, 1);
      deRef(tail);
      list = *tail;
      continue;
    }
    return (char *)NULL;
  }
  if (!isNil(list))
    return (char *)NULL;

  *s = EOS;

  return result;
}

char *
primitiveToString(w, save)
word w;
bool save;
{ static char tmp[25];
 
  if (isAtom(w) )
    return stringAtom(w);
  if (isInteger(w) )
  { sprintf(tmp, "%ld", valNum(w) );
    return save ? store_string_local(tmp) : tmp;
  }
  if (isReal(w) )
  { sprintf(tmp, "%f", valReal(w) );
    return save ? store_string_local(tmp) : tmp;
  }
#if O_STRING
  if (isString(w))
    return valString(w);
#endif O_STRING

  return (char *) NULL;
}

word
pl_atom_length(w, n)
Word w, n;
{ char *s = primitiveToString(*w, FALSE);
  return unifyAtomic(n, consNum(strlen(s)));
}

static char *
prependBase(b, s)
int b;
char *s;
{ *s-- = '\'';
  while(b > 0)
  { *s-- = digitName(b % 10, TRUE);
    b /= 10;
  }

  return s;
}

word
pl_int_to_atom(number, base, atom)
Word number, base, atom;
{ long n, b;
  char result[100];
  char *s = &result[99];

  *s-- = EOS;
  if ( wordToInteger(*number, &n) == FALSE ||
       wordToInteger(*base, &b) == FALSE)
  { warning("int_to_atom/3: instantiation fault");
    fail;
  }

  if (b == 0 && n > 0 && n < 128)
  { *s-- = (char) n;
    *s-- = '\'';
    *s = '0';
    return unifyAtomic(atom, lookupAtom(s));
  }

  if (b > 36 || b < 2)
    return warning("int_to_atom/3: Illegal base: %d", b);

  if (n == 0)
  { *s-- = '0';
    s = prependBase((int)b, s);
    return unifyAtomic(atom, lookupAtom(s+1));
  }
  while(n > 0)
  { *s-- = digitName((int)(n % b), TRUE);
    n /= b;
  }
  if (b != 10)
    s = prependBase((int)b, s);

  return unifyAtomic(atom, lookupAtom(s+1));
}

/*  format an integer according to  a  number  of  modifiers  at various
    radius.   `split'  is a boolean asking to put ',' between each group
    of three digits (e.g. 67,567,288).  `div' askes to divide the number
    by radix^`div' before printing.   `radix'  is  the  radix  used  for
    conversion.  `n' is the number to be converted.

 ** Fri Aug 19 22:26:41 1988  jan@swivax.UUCP (Jan Wielemaker)  */

char *
formatInteger(split, div, radix, small, n)
bool split;
int div;
int radix;
bool small;
long n;
{ static char tmp[100];
  char *s = tmp + 99;
  int before = (div == 0);
  int digits = 0;
  bool negative = FALSE;

  *s = EOS;
  if ( n < 0 )
  { n = -n;
    negative = TRUE;
  }
  if ( n == 0 && div == 0 )
  { *--s = '0';
    return s;
  }
  while( n > 0 || div >= 0 )
  { if ( div-- == 0 && !before )
    { *--s = '.';
      before = 1;
    }
    if ( split && before && (digits++ % 3) == 0 && digits != 1 )
      *--s = ',';
    *--s = digitName((int)(n % radix), small);
    n /= radix;
  }
  if ( negative )
    *--s = '-';  

  return s;
}	  

word
pl_format_number(format, number, string)
Word format, number, string;
{ char *fmt;
  int arg;
  char conv;
  word list;

  if (!isAtom(*format) )
    return warning("format_number/2: instantiation fault");
  fmt = stringAtom(*format);
  if (*fmt == EOS)
    return warning("format_number/3: illegal format");
  arg = atoi(fmt);
  conv = fmt[strlen(fmt)-1];

  switch(conv)
  { case 'D':
    case 'd':
    case 'r':
    case 'R':
      { long i;

	if (wordToInteger(*number, &i) == FALSE)
	  return warning("format_number/3: 2nd argument is not an integer");
	if (conv == 'd' || conv == 'D')
	  list = stringToList(formatInteger(conv == 'D', arg, 10, TRUE, i) );
	else
	  list = stringToList(formatInteger(FALSE, 0, arg, conv == 'r', i) );
	return pl_unify(string, &list);
      }
    case 'e':
    case 'E':
    case 'f':
    case 'g':
    case 'G':
      { real f;
	char tmp[100];
	char form2[10];

	if (fmt[1] == EOS)
	  arg = 6;
	if (wordToReal(*number, &f) == FALSE)
	  return warning("format_number/3: 2nd argument is not a float");
	sprintf(form2, "%%.%d%c", arg, conv);
	sprintf(tmp, form2, f);
	list = stringToList(tmp);
	return pl_unify(string, &list);
      }
    default:
      return warning("format_number/3: illegal conversion code");
  }
}

static bool
isPrefix(s, q)
register char *s, *q;
{ while(*s && *s == *q)
    s++, q++;

  return *s == EOS;
}

word
pl_name(atom, string)
Word atom, string;
{ register char *s;

  if ((s = primitiveToString(*atom, FALSE)) != (char *)NULL)
    return unifyStringWithList(s, string);

  if (isVar(*atom) )
  { register char *q;
    int n;

    if ((s = listToString(*string)) == (char *)NULL)
      return warning("name/2: 2nd argument is not a string");
    if ( isDigit(*s) )
    { for(q=s; *q && isDigit(*q); q++) ;
      if (*q == EOS)
      { n = atoi(s);
	return unifyAtomic(atom, consNum(n));
      }
    }
    return unifyAtomic(atom, lookupAtom(s) );
  }

  return warning("name/2: instantiation fault");
}

word
pl_concat(a1, a2, a3)
Word a1, a2, a3;
{ char *s1, *s2, *s3;
  long l1, l2, l3;
  char *tmp;

  initAllocLocal();

  s1 = primitiveToString(*a1, TRUE);
  s2 = primitiveToString(*a2, TRUE);
  s3 = primitiveToString(*a3, TRUE);

  if (s1 && s2)
  { l1 = strlen(s1);
    tmp = (char *)allocLocal(l1 + strlen(s2));
    strcpy(tmp, s1);
    strcpy(tmp+l1, s2);
    stopAllocLocal();
    return unifyAtomic(a3, lookupAtom(tmp));
  }

  stopAllocLocal();

  if (!s3)
    return warning("concat/3: instantiation fault");

  if (s1)
  { if (isPrefix(s1, s3) )
      return unifyAtomic(a2, lookupAtom(s3+strlen(s1)) );
    fail;
  }

  if (s2)
  { char end;
    int rval;

    l2 = strlen(s2);
    l3 = strlen(s3);
    if (l2 > l3 || !streq(s3+l3-l2, s2) )
      fail;
    end = s3[l3-l2], s3[l3-l2] = EOS;
    rval = unifyAtomic(a1, lookupAtom(s3));
    s3[l3-l2] = end;
    
    return rval;
  }

  return warning("concat/3: instantiation fault");
}

word
pl_concat_atom(list, atom)
Word list, atom;
{ char *tmp = (char *) lTop;
  char *base = tmp;
  Word arg;
  char *s;
  long l;

  *tmp = EOS;
  while(!isNil(*list) )
  { if (!isList(*list) )
      return warning("concat_atom/2: instantiation fault");
    arg = HeadList(list);
    deRef(arg);
    if ((s = primitiveToString(*arg, FALSE)) == (char *) NULL)
      return warning("concat_atom/2: instantiation fault");
    l = strlen(s);
    STACKVERIFY( if (tmp + l > (char *) lMax) outOf((Stack)&stacks.local) );
    strcpy(tmp, s);
    tmp += l;
    list = TailList(list);
  }
  
  return unifyAtomic(atom, lookupAtom(base) );
}

word
pl_apropos_match(a1, a2)
Word a1, a2;
{ char *s1, *s2, *q, *s;

  initAllocLocal();
  s1 = primitiveToString(*a1, TRUE);
  s2 = primitiveToString(*a2, TRUE);
  stopAllocLocal();
  if ( s1 == NULL || s2 == NULL )
    return warning("$apropos_match/2: instantiation fault");

  for (; *s2; s2++)
  { for(q=s1, s=s2; *q && *s; q++, s++)
    { if ( *q != *s && *q != toLower(*s) )
        break;
    }
    if ( *q == EOS )
      succeed;
  }

  fail;
}

#if O_STRING
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Provisional String manipulation functions.
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

word
pl_string_length(str, l)
Word str, l;
{ char *s;

  if ( isString(*str) )
    return unifyAtomic(l, consNum(sizeString(*str)));

  if ( (s=primitiveToString(*str, FALSE)) == NULL )
    return warning("string_length/2: instantiation fault");

  return unifyAtomic(l, consNum(strlen(s)));
}

word
pl_string_to_atom(str, a)
Word str, a;
{ char *s;

  if ( (s = primitiveToString(*str, FALSE)) != (char *) NULL )
    return unifyAtomic(a, lookupAtom(s));
  if ( (s = primitiveToString(*a, FALSE)) != (char *) NULL )
    return unifyAtomic(str, globalString(s));

  return warning("string_to_atom/2: instantiation fault");
}

word
pl_string_to_list(str, list)
Word str, list;
{ char *s;

  if ( (s = primitiveToString(*str, FALSE)) != (char *) NULL )
    return unifyStringWithList(s, list);

  if ( (s = listToString(*list)) != (char *) NULL )
    return unifyAtomic(str, globalString(s));

  return warning("string_to_list/2 instantiation fault");
}

word
pl_substring(str, offset, length, sub)
Word str, offset, length, sub;
{ long off, l, size, end;
  char *s, c;
  word ss;

  if ( !isString(*str) || !isInteger(*offset) || !isInteger(*length) )
    return warning("substring/4: instantiation fault");

  size = sizeString(*str);
  off = valNum(*offset);
  l = valNum(*length);
  end = off + l - 1;
  if ( off < 1 || off > size || l < 0 || end > size )
    return warning("substring/4: index out of range");

  s = valString(*str);
  c = s[end];
  s[end] = EOS;

  if ( isString(*sub) )
  { if ( streq(&s[off-1], valString(*sub)) )
    { s[end] = c;
      succeed;
    }
    s[end] = c;
    fail;
  }
  if ( !isVar(*sub) )
  { s[end] = c;
    fail;
  }

  ss = globalString(&s[off-1]);
  s[end] = c;

  return unifyAtomic(sub, ss);
}
#endif O_STRING

word
pl_write_on_atom(goal, atom)
Word goal, atom;
{ char string[10240];
  bool rval;

  tellString(string, 10240);
  rval = callGoal(MODULE_user, *goal, FALSE);
  toldString();
  TRY(rval);
  return unifyAtomic(atom, lookupAtom(string) );
} 

#if O_STRING
word
pl_write_on_string(goal, string)
Word goal, string;
{ char tmp[10240];
  bool rval;

  tellString(tmp, 10240);
  rval = callGoal(MODULE_user, *goal, FALSE);
  toldString();
  TRY(rval);
  return unifyAtomic(string, globalString(tmp));
} 
#endif O_STRING

word
pl_write_on_list(goal, string)
Word goal, string;
{ char tmp[10240];
  word list;
  bool rval;

  tellString(tmp, 10240);
  rval = callGoal(MODULE_user, *goal, FALSE);
  toldString();
  TRY(rval);
  list = stringToList(tmp);
  return pl_unify(string, &list);
} 

word
pl_term_to_atom(term, atom, bindings)
Word term, atom, bindings;
{ char *s;

  if ( isVar(*atom) )
  { word rval;
    
    s = (char *) lTop;
#if O_DYNAMIC_STACKS
    tellString(s, 10000000L);
#else
    tellString(s, (char *)lMax - (char *)lTop);
#endif
    rval = pl_writeq(term);
    toldString();
    TRY(rval);
    return unifyAtomic(atom, lookupAtom(s) );
  }

  if ( (s = primitiveToString(*atom, FALSE)) != (char *)NULL )
  { word rval;

    seeString(s);
    if ( isVar(*bindings) )
      rval = pl_read_variables(term, bindings);
    else
      rval = pl_read(term);
    seenString();
    return rval;
  }

  return warning("term_to_atom/2: instantiation fault");
}

		/********************************
		*            CONTROL            *
		*********************************/

word
pl_repeat(h)
word h;
{ switch( ForeignControl(h) )
  { case FRG_FIRST_CALL:
    case FRG_REDO:
      ForeignRedo(2L);
    case FRG_CUTTED:
    default:
      succeed;
  }
}

word
pl_fail()		/* just to define it */
{ fail;
}

word
pl_halt()
{ Halt(0);
  /*NOTREACHED*/
  fail;
}

		/********************************
		*          STATISTICS           *
		*********************************/

/*  Return various runtime statistics.

 ** Sun Apr 17 15:38:46 1988  jan@swivax.UUCP (Jan Wielemaker)  */

#define makeNum(n)	((n) < PLMAXINT ? consNum(n) : globalReal((real)n))

word
pl_statistics(k, value)
Word k, value;
{ word result;
  Atom key;

  if (!isAtom(*k) )
    return warning("statistics/2: instantiation fault");
  key = (Atom) *k;

  if      (key == ATOM_cputime)				/* time */
    result = globalReal(CpuTime());
  else if (key == ATOM_inferences)			/* inferences */
    result = makeNum(statistics.inferences);
  else if (key == ATOM_local)				/* local stack */
    result = makeNum((long)lMax - (long)lBase);
  else if (key == ATOM_localused)
    result = makeNum((long)lTop - (long)lBase);
  else if (key == ATOM_locallimit)
    result = makeNum(stacks.local.limit);
  else if (key == ATOM_heapused)			/* heap */
    result = makeNum(statistics.heap);
  else if (key == ATOM_trail)				/* trail */
    result = makeNum((long)tMax - (long)tBase);
  else if (key == ATOM_trailused)
    result = makeNum((long)tTop - (long)tBase);
  else if (key == ATOM_traillimit)
    result = makeNum(stacks.trail.limit);
  else if (key == ATOM_global)				/* global */
    result = makeNum((long)gMax - (long)gBase);
  else if (key == ATOM_globalused)
    result = makeNum((long)gTop - (long)gBase);
  else if (key == ATOM_globallimit)
    result = makeNum(stacks.global.limit);
  else if (key == ATOM_atoms)				/* atoms */
    result = consNum(statistics.atoms);
  else if (key == ATOM_functors)			/* functors */
    result = consNum(statistics.functors);
  else if (key == ATOM_predicates)			/* predicates */
    result = consNum(statistics.predicates);
  else if (key == ATOM_modules)				/* modules */
    result = consNum(statistics.modules);
  else if (key == ATOM_externals)			/* externals */
    result = consNum(statistics.externals);
  else if (key == ATOM_codes)				/* codes */
    result = consNum(statistics.codes);
  else if (key == ATOM_gctime)
    result = globalReal(gc_status.time);
  else if (key == ATOM_collections)
    result = consNum(gc_status.collections);
  else if (key == ATOM_collected)
    result = makeNum(gc_status.trail_gained + gc_status.global_gained);
  else if (key == ATOM_core_left)			/* core left */
#if tos
    result = consNum((long)coreleft());
#else
    fail;
#endif
  else
    return warning("statistics/2: unknown key");

  return unifyAtomic(value, result);
}

		/********************************
		*            VERSION            *
		*********************************/

word
pl_version(v)
Word v;
{ return unifyAtomic(v, lookupAtom(systemDefaults.version));
}

word
pl_arch(m, os)
Word m, os;
{ TRY(   unifyAtomic(m,  lookupAtom(systemDefaults.machine)) );
  return unifyAtomic(os, lookupAtom(systemDefaults.operating_system));
}

word
pl_home(h)
Word h;
{ return unifyAtomic(h, lookupAtom(systemDefaults.home));
}


		/********************************
		*            OPTIONS            *
		*********************************/

/*   Obtain those options we need in the Prolog code from the option
     structure.
*/

word
pl_option(key, value)
Word key, value;
{ Atom result;
  Atom k;

  if (!isAtom(*key))
    fail;
  k = (Atom) *key;

  if (     k == ATOM_goal)	result = lookupAtom(options.goal);
  else if (k == ATOM_top_level) result = lookupAtom(options.topLevel);
  else if (k == ATOM_init_file) result = lookupAtom(options.initFile);
  else fail;

  return unifyAtomic(value, result);
}

static bool
boolPlease(b, old, new)
bool *b;
register Word old, new;
{ Atom a;

  TRY( unifyAtomic(old, *b ? ATOM_on : ATOM_off) );
  a = (Atom) *new;

  if      ( a == ATOM_on )	*b = TRUE;
  else if ( a == ATOM_off )	*b = FALSE;
  else return warning("please/3: 3rd must be `on' or `off'");

  succeed;
}

word
pl_please(key, old, new)
Word key, old, new;
{ Atom k;

  if ( !isAtom(*key) )
    fail;
  k = (Atom) *key;

  if   ( k == ATOM_optimise )
    return boolPlease(&status.optimise, old, new);
  else
    return warning("please/3: unknown key: %s", stringAtom(*key));
}

		/********************************
		*         STYLE CHECK           *
		*********************************/

word
pl_style_check(old, new)
Word old, new;
{ TRY(unifyAtomic(old, consNum(debugstatus.styleCheck)) );
  if (!isInteger(*new) )
    fail;
  debugstatus.styleCheck = (int) valNum(*new);
  systemMode(debugstatus.styleCheck & DOLLAR_STYLE);

  succeed;
}

		/********************************
		*        USER MODELLING?        *
		*********************************/

word
pl_novice(old, new)
Word old, new;
{ TRY(unifyAtomic(old, novice == TRUE ? ATOM_on : ATOM_off) );

  if (!isAtom(*new))
    fail;
  if (*new == (word) ATOM_on)
    novice = TRUE;
  else if (*new == (word) ATOM_off)
    novice = FALSE;
  else
    fail;

  succeed;
}
