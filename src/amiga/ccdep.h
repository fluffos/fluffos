#ifndef AMIGA_CCDEP_H
#define AMIGA_CCDEP_H

#ifdef LATTICE
/* stuff to make the DICE code happy */

#define __stkargs __stdargs
#define __sigfunc void *
#define __geta4 __asm
#define __D0 register __d0
#endif

#endif
