/*  @(#) pl-table.c 1.5.0 (UvA SWI) Jul 30, 1990

    Copyright (c) 1990 Jan Wielemaker. All rights reserved.
    See ../LICENCE to find out about your rights.
    jan@swi.psy.uva.nl

    Purpose: generic support for (numeric) hashTables
*/

#include "pl-incl.h"

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Anonymous hash-tables.  This  is  rather  straightforward.   A  peculiar
thing  on these and a number of dedicated hashtables build in the system
is that the last symbol of  a  specific  hash  value  is  not  the  NULL
pointer,  but  a reference pointer to the next entry.  This allows us to
enumerate  all  entries  of  the  table  with  only  one   word   status
information.   This  is  more  efficient  and  simple to handle with the
interface for non-deterministic C functions (current_predicate/2, ...).

This module also can allocate from the local stack for temporary  tables
needed by foreign language functions.
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

Table
newHTable(size)
int size;
{ Symbol *p;
  int n;
  Table ht;

  DEBUG(9, printf("Creating hash table (size=%d)\n", size));
  ht = (Table) allocHeap(sizeof(struct table) + (size - 1) * sizeof(Symbol));
  DEBUG(9, printf("Allocated hash table\n"));
  ht->size = size;

  for(n=0, p = &ht->entries[0]; n < size-1; n++, p++)
    *p = (Symbol) makeRef(p+1);
  *p = (Symbol) NULL;

  DEBUG(9, printf("Returning ht=%ld\n", ht));
  return ht;
}

Symbol
lookupHTable(ht, name)
Table ht;
Void name;
{ register Symbol s = ht->entries[pointerHashValue(name, ht->size)];

  DEBUG(9, printf("lookupHTable(%ld, %ld) --> ", ht, name));
  for(;s && !isRef((word)s); s = s->next)
    if (s->name == (word)name)
    { DEBUG(9, printf("Symbol=%ld, value=%ld\n", s, s->value));
      return s;
    }

  DEBUG(9, printf("Symbol = NULL\n"));
  return (Symbol) NULL;
}

bool
addHTable(ht, name, value)
Table ht;
Void name;
Void value;
{ register Symbol s;
  register int v = pointerHashValue(name, ht->size);

  DEBUG(9, printf("addHTable(%ld, %ld, %ld) ... ", ht, name, value));
  if (lookupHTable(ht, name) != (Symbol) NULL)
    fail;
  s = (Symbol) allocHeap(sizeof(struct symbol));
  s->name = (word)name;
  s->value = (word)value;
  s->next = ht->entries[v];
  ht->entries[v] = s;

  DEBUG(9, printf("ok\n"));
  succeed;
}  

bool
deleteHTable(ht, name)
Table ht;
Void name;
{ register int v = pointerHashValue(name, ht->size);
  register Symbol *s = &ht->entries[v];
  Symbol symb = *s;

  for(;symb && !isRef((word)symb); s = &symb->next)
  { symb = *s;
    if (symb->name == (word)name)
    { *s = symb->next;
      freeHeap(symb, sizeof(struct symbol));
      succeed;
    }
  }

  fail;
}

Symbol
nextHTable(ht, s)
Table ht;
register Symbol s;
{ s = s->next;
  while(s != (Symbol) NULL && isRef((word)s) )
    s = *((Symbol *)unRef(s));

  return s;
}

Symbol
firstHTable(ht)
Table ht;
{ register Symbol s = ht->entries[0];

  while(s != (Symbol) NULL && isRef((word)s) )
    s = *((Symbol *)unRef(s));

  return s;
}  

void
clearHTable(ht)
Table ht;
{ int n;
  register Symbol s;

  for(n=0; n < ht->size; n++)
  { s = ht->entries[n];
    while(s && !isRef((word)s))
    { register Symbol q = s->next;
      freeHeap(s, sizeof(struct symbol));
      s = q;
    }
    ht->entries[n] = s;
  }
}

		/********************************
		*     TABLES ON LOCAL STACK     *
		*********************************/

Table
newLocalTable(size)
int size;
{ Symbol *p;
  int n;
  Table ht;

  ht = (Table) allocLocal(sizeof(struct table) + (size - 1) * sizeof(Symbol));
  ht->size = size;

  for(n=0, p = &ht->entries[0]; n < size; n++, p++)
    *p = (Symbol) NULL;

  return ht;
}

Symbol
lookupLocalTable(ht, name)
Table ht;
Void name;
{ register Symbol s = ht->entries[pointerHashValue(name, ht->size)];

  for( ; s; s = s->next )
    if ( s->name == (word)name )
      return s;

  return NULL;
}

bool
addLocalTable(ht, name, value)
Table ht;
Void name;
Void value;
{ register Symbol s;
  register int v = pointerHashValue(name, ht->size);

  if (lookupLocalTable(ht, name) != (Symbol) NULL)
    fail;
  s = (Symbol) allocLocal(sizeof(struct symbol));
  s->name = (word)name;
  s->value = (word)value;
  s->next = ht->entries[v];
  ht->entries[v] = s;

  succeed;
}
