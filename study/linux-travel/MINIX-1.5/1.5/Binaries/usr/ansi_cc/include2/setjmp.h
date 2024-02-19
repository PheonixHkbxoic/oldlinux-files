/* The <setjmp.h> header relates to the C phenomenon known as setjmp/longjmp.
 * It is used to escape out of the current situation into a previous one.
 * A typical example is in an editor, where hitting DEL breaks off the current 
 * command and puts the editor back in the main loop.
 */
/* In a jmp_buf, there is room for: 1 mask (long), 1 flag (int) and 3
 * pointers (stack-pointer, local base and program-counter). This may be
 * too big, but that doesn't matter. It could also be too small, when
 * sigset_t is larger than a long. Soit.
 * The identifier __setjmp has a special meaning to the compiler.
 */

#ifndef _SETJMP_H
#define _SETJMP_H

#ifndef _ANSI_H
#include <ansi.h>
#endif

typedef char jmp_buf[ sizeof(long) + sizeof(int) + 3 * sizeof(void *)];

_PROTOTYPE(int __setjmp, (jmp_buf _env, int _savemask)			);

#define setjmp(env)	__setjmp(env, 0)
_PROTOTYPE(void longjmp, (jmp_buf _env, int _val)			);

#ifdef _POSIX_SOURCE
typedef jmp_buf sigjmp_buf;

#define sigsetjmp(env, savemask)	__setjmp(env, savemask)
_PROTOTYPE(int siglongjmp, (sigjmp_buf _env, int _val)			);

#endif

#endif /* _SETJMP_H */
