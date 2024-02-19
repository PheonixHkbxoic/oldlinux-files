/*  @(#) pl-modul.c 1.5.0 (UvA SWI) Jul 30, 1990

    Copyright (c) 1990 Jan Wielemaker. All rights reserved.
    See ../LICENCE to find out about your rights.
    jan@swi.psy.uva.nl

    Purpose: module management
*/

#include "pl-incl.h"

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Definition of modules.  A module consists of a  set  of  predicates.   A
predicate  can be private or public.  By default predicates are private.
A module contains two hash tables.  One that holds  all  predicates  and
one that holds the public predicates of the module.

On trapping undefined  predicates  SWI-Prolog  attempts  to  import  the
predicate  from  the  super  module  of the module.  The module `system'
holds all system predicates and has no super module.  Module  `user'  is
the  global  module  for  the  user  and imports from `system' all other
modules import from `user' (and indirect from `system').
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

Module
lookupModule(name)
Atom name;
{ Symbol s;
  Module m;

  if ((s = lookupHTable(moduleTable, name)) != (Symbol) NULL)
    return (Module) s->value;

  m = (Module) allocHeap(sizeof(struct module));
  m->name = name;
  m->file = (SourceFile) NULL;
  clearFlags(m);
  set(m, UNKNOWN);

  if ( name == ATOM_user || name == ATOM_system )
    m->procedures = newHTable(PROCEDUREHASHSIZE);
  else
    m->procedures = newHTable(MODULEPROCEDUREHASHSIZE);

  m->public = newHTable(PUBLICHASHSIZE);

  if ( name == ATOM_user || stringAtom(name)[0] == '$' )
    m->super = MODULE_system;
  else if ( name == ATOM_system )
    m->super = NULL;
  else
    m->super = MODULE_user;

  if ( name == ATOM_system || stringAtom(name)[0] == '$' )
    set(m, SYSTEM);

  addHTable(moduleTable, name, m);
  statistics.modules++;
  
  return m;
}

Module
isCurrentModule(name)
Atom name;
{ Symbol s;

  if ((s = lookupHTable(moduleTable, name)) != (Symbol) NULL)
    return (Module) s->value;

  return (Module) NULL;
}

void
initModules()
{ moduleTable    = newHTable(MODULEHASHSIZE);
  modules.system = lookupModule(ATOM_system);
  modules.user   = lookupModule(ATOM_user);
  modules.typein = modules.user;
  modules.source = modules.user;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
stripModule() takes an atom or term, possible embedded in the :/2 module
term.  It will assing *module with the associated module and return  the
remaining term.
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

Word
stripModule(term, module)
register Word term;
Module *module;
{ while(isTerm(*term) && functorTerm(*term) == FUNCTOR_module2)
  { register Word mp;
    mp = argTermP(*term, 0);
    deRef(mp);
    if (!isAtom(*mp) )
    { warning("Illegal module specification");

      return (Word) NULL;
    }
    *module = lookupModule((Atom) *mp);
    term = argTermP(*term, 1);
    deRef(term);
  }

  if (*module == (Module) NULL)
    *module = contextModule(environment_frame);

  return term;
}

bool
isPublicModule(module, proc)
Module module;
Procedure proc;
{ return lookupHTable(module->public, proc->functor) == (Symbol) NULL ? FALSE
								      : TRUE;
}

bool
isSuperModule(m, s)
Module m, s;
{ for( ; m; m = m->super )
    if ( m == s )
      succeed;

  fail;
}

		/********************************
		*       PROLOG CONNECTION       *
		*********************************/

word
pl_default_module(me, old, new)
Word me, old, new;
{ Module m, s;

  if ( isVar(*me) )
  { m = contextModule(environment_frame);
    TRY( unifyAtomic(me, m->name) );
  } else if ( isAtom(*me) )
  { m = lookupModule((Atom) *me);
  } else
    return warning("super_module/2: instantiation fault");

  TRY( unifyAtomic(old, m->super ? m->super->name : ATOM_nil) );

  if ( !isAtom(*new) )
    return warning("super_module/2: instantiation fault");

  s = (*new == (word) ATOM_nil ? (Module) NULL : lookupModule((Atom) *new));
  m->super = s;

  succeed;
}


word
pl_current_module(module, file, h)
Word module, file;
word h;
{ Module m;
  Atom f;
  Symbol symb;

  switch( ForeignControl(h) )
  { case FRG_FIRST_CALL:
      symb = firstHTable(moduleTable);
      break;
    case FRG_REDO:
      symb = (Symbol) ForeignContextAddress(h);
      break;
    case FRG_CUTTED:
    default:
      succeed;
  }

  for(; symb; symb = nextHTable(moduleTable, symb) )
  { m = (Module) symb->value;
    if ( stringAtom(m->name)[0] == '$' && !SYSTEM_MODE && isVar(*module) )
      continue;
    if (unifyAtomic(module, m->name) == FALSE)
      continue;
    f = (m->file == (SourceFile) NULL ? ATOM_nil : m->file->name);
    if (unifyAtomic(file, f) == FALSE)
      continue;

    if ((symb = nextHTable(moduleTable, symb)) == (Symbol) NULL)
      succeed;

    ForeignRedo(symb);
  }

  fail;
}

word
pl_strip_module(spec, module, term)
Word spec, module, term;
{ Module m = (Module) NULL;

  if ( (spec = stripModule(spec, &m)) == (Word) NULL )
    fail;
  TRY(unifyAtomic(module, m->name) );

  return pl_unify(spec, term);
}  

word
pl_module(old, new)
Word old, new;
{ TRY(unifyAtomic(old, modules.typein->name) );
  if (!isAtom(*new) )
    return warning("module/1: argument should be an atom");
  modules.typein = lookupModule((Atom)*new);
  
  succeed;
}

word
pl_set_source_module(old, new)
Word old, new;
{ TRY(unifyAtomic(old, modules.source->name) );
  if (!isAtom(*new) )
    return warning("$source_module/1: argument should be an atom");
  modules.source = lookupModule((Atom)*new);
  
  succeed;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Declare `name' to be a module with `file' as its source  file.   If  the
module was already loaded its public table is cleared and all procedures
in it are abolished.
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


word
pl_declare_module(name, file)
Word name, file;
{ Module module;
  Symbol s;
  SourceFile sf;

  if (!isAtom(*name) || !isAtom(*file) )
    return warning("$declare_module/2: instantiation fault");

  module = lookupModule((Atom)*name);

  sf = lookupSourceFile((Atom)*file);
  if (module->file != (SourceFile) NULL && module->file != sf)
    return warning("module/2: module %s already loaded from file %s (abandoned)", 
				stringAtom(module->name), 
				stringAtom(module->file->name) );
  module->file = sf;

  modules.source = module;

  for_table(s, module->procedures)
  { Procedure proc = (Procedure) s->value;
    Definition def = proc->definition;
    if ( def->module == module &&
	 false(def, DYNAMIC) &&
	 false(def, MULTIFILE) )
      abolishProcedure(proc, module);
  }
  clearHTable(module->public);
  
  succeed;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
pl_export() exports a procedure specified by its name and arity from the
context module.
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

word
pl_export(head)
Word head;
{ Procedure proc;
  Module module = (Module) NULL;

  if ((head = stripModule(head, &module)) == (Word) NULL)
    fail;

  if ( isAtom(*head) )
    proc = lookupProcedure(lookupFunctorDef((Atom)*head, 0), module);
  else if ( isTerm(*head) )
    proc = lookupProcedure(functorTerm(*head), module);
  else
    return warning("export/1: illegal predicate specification");

  addHTable(module->public, proc->functor, proc);

  succeed;
}

word
pl_check_export()
{ Module module = contextModule(environment_frame);
  Symbol s;

  for_table(s, module->public)
  { Procedure proc = (Procedure) s->value;
    if (isDefinedProcedure(proc) == FALSE)
    { warning("Exported procedure %s:%s/%d is not defined", 
				  stringAtom(module->name), 
				  stringAtom(proc->functor->name), 
				  proc->functor->arity);
    }
  }

  succeed;
}

word
pl_context_module(module)
Word module;
{ return unifyAtomic(module, contextModule(environment_frame)->name);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
pl_import() imports the predicate specified with its argument  into  the
current  context  module.   If  the  predicate is already defined in the
context a warning is displayed and the predicate is  NOT  imported.   If
the  predicate  is  not  on  the  public  list of the exporting module a
warning is displayed, but the predicate is imported nevertheless.
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

word
pl_import(pred)
Word pred;
{ Module source = (Module) NULL;
  Module destination = contextModule(environment_frame);
  Procedure proc, old;

  if ((pred = stripModule(pred, &source)) == (Word) NULL)
    fail;

  if (isAtom(*pred) )
    proc = lookupProcedure(lookupFunctorDef((Atom)*pred, 0), source);
  else if (isTerm(*pred) )
    proc = lookupProcedure(functorTerm(*pred), source);
  else
    return warning("import/1: illegal predicate specification");

  if ((old = isCurrentProcedure(proc->functor, destination)) != (Procedure) NULL)
  { if (!isDefinedProcedure(old) )
    { old->definition = proc->definition;

      succeed;
    }
    if (old->definition->module == destination)
    { warning("Cannot import %s into module %s: name clash", 
				procedureName(proc), 
				stringAtom(destination->name) );
      fail;
    } else if (old->definition->module != source)
    { warning("Cannot import %s into module %s: already imported from %s", 
				procedureName(proc), 
				stringAtom(destination->name), 
				stringAtom(old->definition->module->name) );
      fail;
    } else if ( old->definition == proc->definition )
    { succeed;			/* already done this! */
    } else
    { sysError("Unknown problem importing %s into module %s",
				procedureName(proc),
				stringAtom(destination->name));
      fail;
    }
  }

  if (isPublicModule(source, proc) == FALSE)
  { warning("import/1: %s is not declared public (still imported)", 
				procedureName(proc));
  }
  
  { Procedure nproc = (Procedure)  allocHeap(sizeof(struct procedure));
  
    nproc->type = PROCEDURE_TYPE;
    nproc->functor = proc->functor;
    nproc->definition = proc->definition;
  
    addHTable(destination->procedures, proc->functor, nproc);
  }

  succeed;
}
