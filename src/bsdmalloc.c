/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *  This product includes software developed by the University of
 *  California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)malloc.c	5.11 (Berkeley) 2/23/91";
#endif              /* LIBC_SCCS and not lint */

/*
 * malloc.c (Caltech) 2/21/82
 * Chris Kingsley, kingsley@cit-20.
 *
 * This is a very fast storage allocator.  It allocates blocks of a small
 * number of different sizes, and keeps free lists of each size.  Blocks that
 * don't exactly fit are passed up to the next larger size.  In this
 * implementation, the available sizes are 2^n-4 (or 2^n-10) bytes long.
 * This is designed for use in a virtual memory environment.
 */

#define IN_MALLOC_WRAPPER
#define NO_OPCODES
#include "std.h"
#include "main.h"

#undef NULL
#define NULL 0

#ifdef _SEQUENT_
typedef unsigned char u_char;
typedef unsigned short u_short;
typedef unsigned int u_int;

#endif

/*
 * The overhead on a block is at least 4 bytes.  When free, this space
 * contains a pointer to the next free block, and the bottom two bits must
 * be zero.  When in use, the first byte is set to MAGIC, and the second
 * byte is the size index.  The remaining bytes are for alignment.
 * If range checking is enabled then a second word holds the size of the
 * requested block, less 1, rounded up to a multiple of sizeof(RMAGIC).
 * The order of elements is critical: ov_magic must overlay the low order
 * bits of ov_next, and ov_magic can not be a valid ov_next bit pattern.
 */
union overhead {
  union overhead *ov_next;    /* when free */
  struct {
    u_char ovu_magic;   /* magic number */
    u_char ovu_index;   /* bucket # */
#ifdef RCHECK
    u_short ovu_rmagic; /* range magic number */
    u_int ovu_size;     /* actual block size */
#endif
  }      ovu;
#define ov_magic    ovu.ovu_magic
#define ov_index    ovu.ovu_index
#define ov_rmagic   ovu.ovu_rmagic
#define ov_size     ovu.ovu_size
};

#define MAGIC       0xef    /* magic # on accounting info */
#define RMAGIC      0x5555  /* magic # on range info */

#ifdef RCHECK
#define RSLOP       sizeof (u_short)
#else
#define RSLOP       0
#endif

/*
 * nextf[i] is the pointer to the next free block of size 2^(i+3).  The
 * smallest allocatable block is 8 bytes.  The overhead information
 * precedes the data area returned to the user.
 */
#define NBUCKETS 30
static union overhead *nextf[NBUCKETS];

static int pagesz;      /* page size */
static int pagebucket;      /* page size bucket */

static void morecore(int);
static int findbucket(union overhead *, int);
void *sbrkx(long);
#ifdef DO_MSTATS
/*
 * nmalloc[i] is the difference between the number of mallocs and frees
 * for a given block size.
 */
static u_int nmalloc[NBUCKETS];

#endif

#if defined(DEBUG) || defined(RCHECK)
#ifdef __STDC__
#define ASSERT(p)   if (!(p)) botch(#p)
#else
#define ASSERT(p)   if (!(p)) botch("?")
#endif
static int botch(char *s)
{
  debug_message("\r\nassertion botched: %s\r\n", s);
  (void) fflush(stderr);  /* just in case user buffered it */
  abort();
  return 0;
}
#else
#define ASSERT(p)
#endif

#ifdef malloc
/* linux */
#undef malloc
#endif
void *bsdmalloc_malloc(size_t nbytes)
{
  register union overhead *op;
  register long bucket, n;
  register unsigned amt;

  /*
   * First time malloc is called, setup page size and align break pointer
   * so all data will be page aligned.
   */
  if (pagesz == 0) {
#ifdef MEMPAGESIZE
    pagesz = n = MEMPAGESIZE;
#else
    pagesz = n = getpagesize();
#endif
    op = (union overhead *) sbrkx(0);
    n = n - sizeof(*op) - ((long) op & (n - 1));
    if (n < 0) {
      n += pagesz;
    }
    if (n) {
      if ((char *)sbrkx(n) == (char *) - 1) {
        return (NULL);
      }
    }
    bucket = 0;
    amt = 8;
    while (pagesz > amt) {
      amt <<= 1;
      bucket++;
    }
    pagebucket = bucket;
  }
  /*
   * Convert amount of memory requested into closest block size stored in
   * hash buckets which satisfies request. Account for space used per block
   * for accounting.
   */
  if (nbytes <= (n = pagesz - sizeof(*op) - RSLOP)) {
#ifndef RCHECK
    amt = 8;        /* size of first bucket */
    bucket = 0;
#else
    amt = 16;       /* size of first bucket */
    bucket = 1;
#endif
    n = -(sizeof(*op) + RSLOP);
  } else {
    amt = pagesz;
    bucket = pagebucket;
  }
  while (nbytes > amt + n) {
    amt <<= 1;
    if (amt == 0) {
      return (NULL);
    }
    bucket++;
  }
  /*
   * If nothing in hash bucket right now, request more memory from the
   * system.
   */
  if ((op = nextf[bucket]) == NULL) {
    morecore(bucket);
    if ((op = nextf[bucket]) == NULL) {
      return (NULL);
    }
  }
  /* remove from linked list */
  nextf[bucket] = op->ov_next;
  op->ov_magic = MAGIC;
  op->ov_index = bucket;
#ifdef DO_MSTATS
  nmalloc[bucket]++;
#endif
#ifdef RCHECK
  /*
   * Record allocated size of block and bound space with magic numbers.
   */
  op->ov_size = (nbytes + RSLOP - 1) & ~(RSLOP - 1);
  op->ov_rmagic = RMAGIC;
  *(u_short *)((caddr_t)(op + 1) + op->ov_size) = RMAGIC;
#endif
  return ((char *)(op + 1));
}

/*
 * Allocate more memory to the indicated bucket.
 */
static void
morecore(int bucket)
{
  register union overhead *op;
  register int sz;        /* size of desired block */
  int amt;            /* amount to allocate */
  int nblks;          /* how many blocks we get */

  /*
   * sbrk_size <= 0 only for big, FLUFFY, requests (about 2^30 bytes on a
   * VAX, I think) or for a negative arg.
   */
  sz = 1 << (bucket + 3);
#ifdef DEBUG
  ASSERT(sz > 0);
#else
  if (sz <= 0) {
    return;
  }
#endif
  if (sz < pagesz) {
    amt = pagesz;
    nblks = amt / sz;
  } else {
    amt = sz + pagesz;
    nblks = 1;
  }
  op = (union overhead *) sbrkx(amt);
  /* no more room! */
  if ((long) op == -1) {
    return;
  }
  /*
   * Add new memory allocated to that on free list for this hash bucket.
   */
  nextf[bucket] = op;
  while (--nblks > 0) {
    op->ov_next = (union overhead *)((caddr_t) op + sz);
    op = (union overhead *)((caddr_t) op + sz);
  }
}

void
bsdmalloc_free(void *cp)
{
  register int size;
  register union overhead *op;

  if (cp == NULL) {
    return;
  }
  op = (union overhead *)((caddr_t) cp - sizeof(union overhead));
#ifdef DEBUG
  ASSERT(op->ov_magic == MAGIC);  /* make sure it was in use */
#else
  if (op->ov_magic != MAGIC) {
    return;    /* sanity */
  }
#endif
#ifdef RCHECK
  ASSERT(op->ov_rmagic == RMAGIC);
  ASSERT(*(u_short *)((caddr_t)(op + 1) + op->ov_size) == RMAGIC);
#endif
  size = op->ov_index;
  ASSERT(size < NBUCKETS);
  op->ov_next = nextf[size];  /* also clobbers ov_magic */
  nextf[size] = op;
#ifdef DO_MSTATS
  nmalloc[size]--;
#endif
}

/*
 * When a program attempts "storage compaction" as mentioned in the
 * old malloc man page, it realloc's an already freed block.  Usually
 * this is the last block it freed; occasionally it might be farther
 * back.  We have to search all the free lists for the block in order
 * to determine its bucket: 1st we make one pass thru the lists
 * checking only the first block in each; if that fails we search
 * ``realloc_srchlen'' blocks in each list for a match (the variable
 * is extern so the caller can modify it).  If that fails we just copy
 * however many bytes was given to realloc() and hope it's not huge.
 */
int realloc_srchlen = 4;    /* 4 should be plenty, -1 =>'s whole list */

void *bsdmalloc_realloc(void *cp, size_t nbytes)
{
  register u_int onb;
  register int i;
  union overhead *op;
  char *res;
  int was_alloced = 0;

  if (cp == NULL) {
    return (bsdmalloc_malloc(nbytes));
  }
  op = (union overhead *)((caddr_t) cp - sizeof(union overhead));
  if (op->ov_magic == MAGIC) {
    was_alloced++;
    i = op->ov_index;
  } else {
    /*
     * Already free, doing "compaction".
     *
     * Search for the old block of memory on the free list.  First, check
     * the most common case (last element free'd), then (this failing)
     * the last ``realloc_srchlen'' items free'd. If all lookups fail,
     * then assume the size of the memory block being realloc'd is the
     * largest possible (so that all "nbytes" of new memory are copied
     * into).  Note that this could cause a memory fault if the old area
     * was tiny, and the moon is gibbous.  However, that is very
     * unlikely.
     */
    if ((i = findbucket(op, 1)) < 0 &&
        (i = findbucket(op, realloc_srchlen)) < 0) {
      i = NBUCKETS;
    }
  }
  onb = 1 << (i + 3);
  if (onb < pagesz) {
    onb -= sizeof(*op) + RSLOP;
  } else {
    onb += pagesz - sizeof(*op) - RSLOP;
  }
  /* avoid the copy if same size block */
  if (was_alloced) {
    if (i) {
      i = 1 << (i + 2);
      if (i < pagesz) {
        i -= sizeof(*op) + RSLOP;
      } else {
        i += pagesz - sizeof(*op) - RSLOP;
      }
    }
    if (nbytes <= onb && nbytes > i) {
#ifdef RCHECK
      op->ov_size = (nbytes + RSLOP - 1) & ~(RSLOP - 1);
      *(u_short *)((caddr_t)(op + 1) + op->ov_size) = RMAGIC;
#endif
      return (cp);
    } else {
      bsdmalloc_free(cp);
    }
  }
  if ((res = bsdmalloc_malloc(nbytes)) == NULL) {
    return (NULL);
  }
  if (cp != res) {    /* common optimization if "compacting" */
    memcpy(res, cp, (nbytes < onb) ? nbytes : onb);
  }
  return (res);
}

/*
 * Search ``srchlen'' elements of each free list for a block whose
 * header starts at ``freep''.  If srchlen is -1 search the whole list.
 * Return bucket number, or -1 if not found.
 */
static int
findbucket(union overhead *freep, int srchlen)
{
  register union overhead *p;
  register int i, j;

  for (i = 0; i < NBUCKETS; i++) {
    j = 0;
    for (p = nextf[i]; p && j != srchlen; p = p->ov_next) {
      if (p == freep) {
        return (i);
      }
      j++;
    }
  }
  return (-1);
}

#ifdef DO_MSTATS
/*
 * mstats - print out statistics about malloc
 *
 * Prints two lines of numbers, one showing the length of the free list
 * for each size category, the second showing the number of mallocs -
 * frees for each size category.
 */
void
show_mstats(outbuffer_t *ob, char *s)
{
  register int i, j;
  register union overhead *p;
  int totfree = 0, totused = 0;

  outbuf_addv(ob, "Memory allocation statistics %s\nfree:\t", s);
  for (i = 0; i < NBUCKETS; i++) {
    for (j = 0, p = nextf[i]; p; p = p->ov_next, j++) { ; }
    outbuf_addv(ob, " %d", j);
    totfree += j * (1 << (i + 3));
  }
  outbuf_add(ob, "\nused:\t");
  for (i = 0; i < NBUCKETS; i++) {
    outbuf_addv(ob, " %d", nmalloc[i]);
    totused += nmalloc[i] * (1 << (i + 3));
  }
  outbuf_addv(ob, "\n\tTotal in use: %d, total free: %d\n",
              totused, totfree);
}
#endif

/* calloc was originally in its own source file */
#ifdef calloc
/* linux */
#undef calloc
#endif
void *bsdmalloc_calloc(size_t num, register size_t size)
{
  register void *p;

  size *= num;
  if ((p = bsdmalloc_malloc(size))) {
    memset(p, 0, size);
  }
  return (p);
}
