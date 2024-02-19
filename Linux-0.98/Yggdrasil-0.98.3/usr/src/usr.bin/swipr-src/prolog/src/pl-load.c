/*  @(#) pl-load.c 1.5.0 (UvA SWI) Jul 30, 1990

    Copyright (c) 1990 Jan Wielemaker. All rights reserved.
    See ../LICENCE to find out about your rights.
    jan@swi.psy.uva.nl

    Purpose: load foreign files
*/

#include "pl-incl.h"

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Make sure the symbolfile and  orgsymbolfile  attributes  of  the  global
structure status are filled properly.
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

bool
getSymbols()
{ char *symbols;

  if ( loaderstatus.symbolfile != (Atom) NULL )
    succeed;
  
  if ( (symbols = Symbols()) == (char *)NULL )
  { DEBUG(1, printf("Failed to find symbol table\n"));
    symbols = mainArgv[0];
  }
  DEBUG(2, printf("Symbol file = %s\n", symbols));

  loaderstatus.symbolfile = loaderstatus.orgsymbolfile = lookupAtom(symbols);

  succeed;
}


#if O_FOREIGN

forwards bool create_a_out();
forwards int  openExec();
forwards int  sizeExec();
forwards Func loadExec();
forwards bool scanSymbols();
forwards char *symbolString();

#include <sys/file.h>
#include <a.out.h>

extern char *sbrk(/*int*/);
extern int system(/*char **/);
extern int unlink(/*char **/);
extern int lseek(/*int, long, int*/);

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Load an object file and link it to the system.  The intented  schema  is
to  call  the  standard  system  loader `ld' to proceduce an incremental
executable starting at some specified address.  As we only need 1  entry
point  (the foreign module's initialisation function) we call the loader
with -e <function> which will make the loader put the  address  of  that
function in the header of the executable, thus avoiding the need to scan
the  symbol table.  With the new dynamic linking facilities of SunOs 4.0
this appears not to work any more.  Therefore a NOENTRY  flag  has  been
introduced  to  indicate that `-e' does not work properly and the symbol
table is to be scanned for the entry point.

If the size of the executable is not provided by the user, we first make
an executable for an arbitrary base address (0) to deterimine the  size.
Next  we  allocate  memory  and  produce  an  executable to start at the
allocated memory base.  Finally, we read the text and  initialised  data
segment  from  the  executable,  clear  the  bss area and call the entry
point.

Normally, the entry point will install foreign language  functions,  but
the user is allowed to do anything (s)he likes (even take over control).

This module is a bit of a mess due to all the #ifdef.  We should  define
a better common basis to get rid of most of these things.
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if sun | vax
#define NOENTRY 1
#else
#define NOENTRY 0
#endif

#if hpux
#define N_DATOFF(x)	DATA_OFFSET(x)
#define N_TXTOFF(x)	TEXT_OFFSET(x)
#define PAGSIZ		0x1000
#endif

#if vax
#define PAGSIZ		0x400
#endif

#ifndef N_DATOFF			/* SunOs 3.4 does not define this */
#define N_DATOFF(x) ( N_TXTOFF(x) + (x).a_text )
#endif

#define LOADER	"ld"			/* Unix loader command name */

#if NOENTRY
#define MAXSYMBOL 256			/* maximum length of a function name */

typedef struct
{ char *string;				/* name of function (withouth _) */
  Func function;			/* functions address */
} textSymbol;

char *symbolString();			/* forwards */
#endif NOENTRY

static struct exec header;		/* a.out header */  

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Allocate room for text and data segment of executable.  The  SUN  has  a
special  function  for  this  called valloc(). On some systems you might
need to start the text and data segment on a page  boundary,  on  others
not.
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if hpux || vax
#define valloc malloc
#endif

long
allocText(size)
long size;
{ extern char *valloc();
  long base;

  if ( size < sizeof(word) )
    return 0;				/* test run */

  size = ROUND(size, sizeof(long));

  if ( (base = (long) valloc((malloc_t) size)) == 0L )
    fatalError("%s", OsError());

  statistics.heap += size;

  return base;
}


word
pl_load_foreign(file, entry, options, libraries, size)
Word file, entry, options, libraries, size;
{ char *sfile, *sentry, *soptions, *slibraries;
  int sz, nsz, n;
  Atom execName;
  char *execFile;
  long base;
  int fd;

  if ( !isAtom(*file) ||
       !isAtom(*entry) ||
       !isAtom(*options) ||
       !isAtom(*libraries) ||
       !isInteger(*size) )
    return warning("pl_load_foreign/5: instantiation fault");

  sfile = stringAtom(*file);
  sentry = stringAtom(*entry);
  soptions = stringAtom(*options);
  slibraries = stringAtom(*libraries);
  sz = valNum(*size);
  if ( sz < 0 )
    sz = 0;
  
  TRY( getSymbols() );
  execName = (Atom) TemporaryFile("ld");
  execFile = stringAtom(execName);

  for( n=0; n<2; n++)
  { base = (long) allocText(sz);
#if NOENTRY
    TRY( create_a_out(sfile, soptions, slibraries, base, execFile) );
#else
    TRY( create_a_out(sfile, sentry, soptions, slibraries, base, execFile) );
#endif
    if ( (fd = openExec(execFile)) < 0 )
      fail;

    if ( sizeExec() <= sz )
    { Func entry;
#if NOENTRY
      if ( (entry = loadExec(fd, base, sentry)) == NULL )
    	fail;
#else
      if ( (entry = loadExec(fd, base)) == NULL )
	fail;
#endif
      loaderstatus.symbolfile = execName;
      DEBUG(1, printf("Calling entry point at 0x%x\n", entry));
      (*entry)();
      DEBUG(1, printf("Entry point returned successfully\n"));

      succeed;
    }

    if ( base > 0 )			/* used for test runs */
      freeHeap(base, sz);
    nsz = sizeExec();
    if ( sz > 0 )
    { Putf("! Executable %s does not fit in %d bytes\n", sfile, sz);
      Putf("Size: %d bytes (%d text %d data, %d bss) (reloading ...)\n",
		nsz, header.a_text, header.a_data, header.a_bss);
    }
    sz = nsz;
  }

  return sysError("Can't fit executable %s", execFile);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Create an a.out file from a .o file.
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static
bool
#if NOENTRY
create_a_out(files, options, libraries, base, outfile)
#else
create_a_out(files, entry, options, libraries, base, outfile)
char *entry;
#endif
char *files;
char *options;
char *libraries;
long base;
char *outfile;
{ char command[10240];

#if NOENTRY
  sprintf(command, "%s -N -A %s -T %x -o %s %s %s %s -lc",
#else
  sprintf(command, "%s -N -A %s -R %x -e _%s -o %s %s %s %s -lc",
#endif
	   LOADER, 				/* name of loader */
	   stringAtom(loaderstatus.symbolfile),	/* name of symbol file */
	   base, 				/* base address */
#if !NOENTRY
	   entry, 				/* entry point */
#endif
	   outfile,				/* temp. executable */
	   options,				/* additional options */
	   files,				/* files to be loaded */
	   libraries);				/* libraries */
  
  DEBUG(1, printf("Calling loader: %s\n", command) );
  if (system(command) == 0)
    succeed;

  unlink(outfile);
  return warning("load_foreign/5: Failed to create an executable from %s",
								files);
}


static
int
openExec(execFile)
char *execFile;
{ int fd;

  if ((fd=open(execFile, O_RDONLY)) < 0)
  { warning("load_foreign/5: Cannot open %s", execFile);
    return -1;
  }

  if (read(fd, &header, sizeof(struct exec)) != sizeof(struct exec) ||
      N_BADMAG(header) != 0)
  { warning("load_foreign/5: Bad magic number in %s", execFile);
    close(fd);
    return -1;
  }

  return fd;
}


static
int
sizeExec()
{ return ROUND(header.a_text, 4) +
	 ROUND(header.a_data, 4) +
	 ROUND(header.a_bss, 4);
}


static Func
#if NOENTRY
loadExec(fd, base, sentry)
char *sentry;
#else
loadExec(fd, base)
#endif
int fd;
ulong base;
{ Func entry;
  long *text, text_off, text_size;
  long *data, data_off, data_size;
  long *bss, bss_size;

  text = (long *)base;			/* address of text in memory */
  text_size = header.a_text;		/* size of text area */
  data = (long *)(base+text_size);	/* address of data in memory */
  data_size = header.a_data;		/* size of data area */
  text_off = N_TXTOFF(header);		/* offset of text in file */
  data_off = N_DATOFF(header);		/* offset of data in file */
  bss = (long *)(base + text_size + data_size);
  bss_size = header.a_bss;

  DEBUG(1, printf("Text offset = %d, Data offset = %d\n", text_off, data_off));
  DEBUG(1, printf("Base = 0x%x (= %d), text at 0x%x, %d bytes, data at 0x%x, %d bytes\n",
		    base, base, text, text_size, data, data_size) );

  if ( lseek(fd, text_off, 0) < 0 ||
       text_size != read(fd, text, text_size) ||
       lseek(fd, data_off, 0) < 0 ||
       data_size != read(fd, data, data_size) )
  { warning("load_foreign/5: Failed to read text segment");
    close(fd);
    return NULL;
  }

#if NOENTRY
  { textSymbol ts[1];
    ts[0].string = sentry;
    ts[0].function = (Func) NULL;

    TRY( scanSymbols(fd, 1, ts) );
    entry = ts[0].function;
  }
#else
#  if hpux
  entry = (Func)(header.a_entry + (long)text);
  DEBUG(2, printf("a_entry = 0x%x; text = 0x%x, entry = 0x%x\n",
				header.a_entry, text, entry));
#  else
  entry = (Func)(header.a_entry);
#  endif
#endif

  close(fd);

  DEBUG(1, printf("Cleaning BSS %d bytes from 0x%x (=%d)\n", 
	      bss_size, bss, bss));
  bzero(bss, bss_size);

  return entry;
}

#if NOENTRY

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Scan the symbol table and try to resolve all textSymbols given  in  `tv'
(target  vector).   The  first `tc' (target count) members of this array
are valid.  TRUE is returned if  all  functions  are  found.   Otherwise
FALSE is returned.

Searching starts at the end of the symbol table, as this  is  the  place
were the incrementally loaded symbols normally lives.

It assumes a global struct exec `header'  to  hold  the  header  of  the
symbol  file and the argument `fd' to be an open file descriptor on that
file.
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static
bool
scanSymbols(fd, tc, tv)
int fd;
int tc;
textSymbol * tv;
{ long symbols, strings;
  long next_symbol;
  struct nlist name;
  char *s;
  int n, left = tc;

  symbols = N_SYMOFF(header);
  strings = N_STROFF(header);

  n = (strings - symbols)/sizeof(struct nlist);
  next_symbol = symbols+(n-1)*sizeof(struct nlist);

  for(; next_symbol >= symbols; next_symbol -= sizeof(struct nlist) )
  { if (lseek(fd, next_symbol, 0) < 0)
      return warning("seek on executables' symbol table failed");
    if (read(fd, &name, sizeof(struct nlist) ) != sizeof(struct nlist) )
      return warning("failed to read symbol in executable");

    if (name.n_type == (unsigned char)(N_TEXT|N_EXT))
    { s = symbolString(fd, name.n_un.n_strx+strings);

      for(n = 0; n < tc; n++)
      { if ( streq(tv[n].string, s+1) )
	{ tv[n].function = (Func) name.n_value;
	  if ( --left <= 0 )
	    succeed;
	}
      }
    }
  }

  if ( left > 0 )
  { for(n = 0; n < tc; n++)
    { if ( tv[n].function == (Func) NULL )
        warning("Dynamic loader: undefined: %s", tv[n].string);
    }
    fail;
  }
  succeed;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Return the char string at offset `n' in the string table.   The  strings
are supposed not to be longer than MAXSYMBOL characters.
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static
char *
symbolString(fd, n)
int fd;
long n;
{ static char temp[MAXSYMBOL+1];
  int l;

  if (n == 0)
    return "";
  if (lseek(fd, n, 0) < 0)
  { warning("Failed to seek to string in executable");
    return (char *) NULL;
  }
  l = read(fd, temp, MAXSYMBOL);
  temp[l] = EOS;

  return temp;
}

#endif NOENTRY

#elif O_RIOS_FOREIGN			/* Does not work yet */

word
pl_load_foreign(file, entry, options, libraries, size)
Word file, entry, options, libraries, size;
{ char *sfile, *sentry, *soptions, *slibraries;
  int sz, rval;
  int (*entryFunction)();

  if ( !isAtom(*file) ||
       !isAtom(*entry) ||
       !isAtom(*options) ||
       !isAtom(*libraries) ||
       !isInteger(*size) )
    return warning("pl_load_foreign/5: instantiation fault");

  sfile = stringAtom(*file);
  sentry = stringAtom(*entry);
  soptions = stringAtom(*options);
  slibraries = stringAtom(*libraries);
  sz = valNum(*size);

  if ( sentry[0] != '\0'    ||
       soptions[0] != '\0'  ||
       slibraries[0] != '\0' ||
       sz != 0 )
     return warning("pl_load_foreign/5: rios only knows file");

  if ((entryFunction = load(sfile, 0, NULL)) == NULL)
     return warning("%s", OsError());
  else {
      
      DEBUG(1, printf("Calling entry point at 0x%x (=%d)\n",
	              entryFunction, entryFunction) );

      rval = (*entryFunction)();

      DEBUG(1, printf("Entry point returned %d\n", rval) );
  }
  succeed;
}

#else					/* No foreign language interface */

word
pl_load_foreign(file, entry, options, libraries, size)
Word file, entry, options, libraries, size;
{ warning("Foreign language loader not (yet) available for this machine");

  fail;
}

#endif O_FOREIGN
