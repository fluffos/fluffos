/* calloc.c - C standard library routine.
   Copyright (c) 1989, 1993  Michael J. Haertel
   You may redistribute this library under the terms of the
   GNU Library General Public License (version 2 or any later
   version) as published by the Free Software Foundation.
   THIS SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY EXPRESS OR IMPLIED
   WARRANTY.  IN PARTICULAR, THE AUTHOR MAKES NO REPRESENTATION OR
   WARRANTY OF ANY KIND CONCERNING THE MERCHANTABILITY OF THIS
   SOFTWARE OR ITS FITNESS FOR ANY PARTICULAR PURPOSE. */

#include <stdlib.h>
#include <string.h>
#include "gnumalloc.h"

/* Allocate space for the given number of elements of the given
   size, initializing the whole region to binary zeroes. */
void *
gnucalloc(size_t nelem, size_t size)
{
    void *result;

    result = gnumalloc(size * nelem);
    if (result)
	memset(result, 0, nelem * size);
    return result;
}
/* free.c - C standard library routine.
   Copyright (c) 1989, 1993  Michael J. Haertel
   You may redistribute this library under the terms of the
   GNU Library General Public License (version 2 or any later
   version) as published by the Free Software Foundation.
   THIS SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY EXPRESS OR IMPLIED
   WARRANTY.  IN PARTICULAR, THE AUTHOR MAKES NO REPRESENTATION OR
   WARRANTY OF ANY KIND CONCERNING THE MERCHANTABILITY OF THIS
   SOFTWARE OR ITS FITNESS FOR ANY PARTICULAR PURPOSE. */

#include <limits.h>
#include <stddef.h>
#include <stdlib.h>

/* Return memory to the heap. */
void
gnufree(void *ptr)
{
    int block, blocks, i, type;
    struct list *prev, *next;

    if (!ptr)
	return;

    block = BLOCK(ptr);

    switch (type = _heapinfo[block].busy.type) {
    case 0:
	/* Find the free cluster previous to this one in the free list.
	   Start searching at the last block referenced; this may benefit
	   programs with locality of allocation. */
	i = _heapindex;
	if (i > block)
	    while (i > block)
		i = _heapinfo[i].free.prev;
	else {
	    do
		i = _heapinfo[i].free.next;
	    while (i > 0 && i < block);
	    i = _heapinfo[i].free.prev;
	}

	/* Determine how to link this block into the free list. */
	if (block == i + _heapinfo[i].free.size) {
	    /* Coalesce this block with its predecessor. */
	    _heapinfo[i].free.size += _heapinfo[block].busy.info.size;
	    block = i;
	} else {
	    /* Really link this block back into the free list. */
	    _heapinfo[block].free.size = _heapinfo[block].busy.info.size;
	    _heapinfo[block].free.next = _heapinfo[i].free.next;
	    _heapinfo[block].free.prev = i;
	    _heapinfo[i].free.next = block;
	    _heapinfo[_heapinfo[block].free.next].free.prev = block;
	}

	/* Now that the block is linked in, see if we can coalesce it
	   with its successor (by deleting its successor from the list
	   and adding in its size). */
	if (block + _heapinfo[block].free.size == _heapinfo[block].free.next) {
	    _heapinfo[block].free.size
		+= _heapinfo[_heapinfo[block].free.next].free.size;
	    _heapinfo[block].free.next
		= _heapinfo[_heapinfo[block].free.next].free.next;
	    _heapinfo[_heapinfo[block].free.next].free.prev = block;
	}

	/* Now see if we can return stuff to the system. */
	blocks = _heapinfo[block].free.size;
	if (blocks >= FINAL_FREE_BLOCKS && block + blocks == _heaplimit
	    && (*_morecore)(0) == ADDRESS(block + blocks)) {
	    _heaplimit -= blocks;
	    (*_morecore)(-blocks * BLOCKSIZE);
	    _heapinfo[_heapinfo[block].free.prev].free.next
		= _heapinfo[block].free.next;
	    _heapinfo[_heapinfo[block].free.next].free.prev
		= _heapinfo[block].free.prev;
	    block = _heapinfo[block].free.prev;
	}

	/* Set the next search to begin at this block. */
	_heapindex = block;
	break;

    default:
	/* Get the address of the first free fragment in this block. */
	prev = (struct list *) ((char *) ADDRESS(block)
				+ (_heapinfo[block].busy.info.frag.first
				   << type));

	if (_heapinfo[block].busy.info.frag.nfree == (BLOCKSIZE >> type) - 1
	&& _fragblocks[type] > 1) {
	    /* If all fragments of this block are free, remove them
	       from the fragment list and free the whole block. */
	    --_fragblocks[type];
	    for (next = prev, i = 1; i < BLOCKSIZE >> type; ++i)
		next = next->next;
	    prev->prev->next = next;
	    if (next)
		next->prev = prev->prev;
	    _heapinfo[block].busy.type = 0;
	    _heapinfo[block].busy.info.size = 1;
	    gnufree(ADDRESS(block));
	} else if (_heapinfo[block].busy.info.frag.nfree) {
	    /* If some fragments of this block are free, link this fragment
	       into the fragment list after the first free fragment of
	       this block. */
	    next = ptr;
	    next->next = prev->next;
	    next->prev = prev;
	    prev->next = next;
	    if (next->next)
		next->next->prev = next;
	    ++_heapinfo[block].busy.info.frag.nfree;
	} else {
	    /* No fragments of this block are free, so link this fragment
	       into the fragment list and announce that it is the first
	       free fragment of this block. */
	    prev = (struct list *) ptr;
	    _heapinfo[block].busy.info.frag.nfree = 1;
	    _heapinfo[block].busy.info.frag.first
		= (unsigned int) ((char *) ptr - (char *) NULL) % BLOCKSIZE
		  >> type;
	    prev->next = _fraghead[type].next;
	    prev->prev = &_fraghead[type];
	    prev->prev->next = prev;
	    if (prev->next)
		prev->next->prev = prev;
	}
	break;
    }
}
/* malloc.c - C standard library routine.
   Copyright (c) 1989, 1993  Michael J. Haertel
   You may redistribute this library under the terms of the
   GNU Library General Public License (version 2 or any later
   version) as published by the Free Software Foundation.
   THIS SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY EXPRESS OR IMPLIED
   WARRANTY.  IN PARTICULAR, THE AUTHOR MAKES NO REPRESENTATION OR
   WARRANTY OF ANY KIND CONCERNING THE MERCHANTABILITY OF THIS
   SOFTWARE OR ITS FITNESS FOR ANY PARTICULAR PURPOSE. */

#include <limits.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

/* How to really get more memory. */
void *(*_morecore)(long) = _default_morecore;

/* Pointer to the base of the first block. */
char *_heapbase;

/* Block information table. */
union info *_heapinfo;

/* Number of info entries. */
static int heapsize;

/* Search index in the info table. */
int _heapindex;

/* Limit of valid info table indices. */
int _heaplimit;

/* Count of large blocks allocated for each fragment size. */
int _fragblocks[BLOCKLOG];

/* Free lists for each fragment size. */
struct list _fraghead[BLOCKLOG];

/* Are we experienced? */
static int initialized;

/* Aligned allocation. */
static void *
align(size_t size)
{
    void *result;
    unsigned int adj;

    result = (*_morecore)(size);
    adj = (unsigned int) ((char *) result - (char *) NULL) % BLOCKSIZE;
    if (adj != 0) {
	(*_morecore)(adj = BLOCKSIZE - adj);
	result = (char *) result + adj;
    }
    return result;
}

/* Set everything up and remember that we have. */
static int
initialize()
{
    heapsize = HEAP / BLOCKSIZE;
    _heapinfo = align(heapsize * sizeof (union info));
    if (!_heapinfo)
	return 0;
    memset(_heapinfo, 0, heapsize * sizeof (union info));
    _heapinfo[0].free.size = 0;
    _heapinfo[0].free.next = _heapinfo[0].free.prev = 0;
    _heapindex = 0;
    _heapbase = (char *) _heapinfo;
    initialized = 1;
    return 1;
}

/* Get neatly aligned memory, initializing or growing the
   heap info table as necessary. */
static void *
morecore(size_t size)
{
    void *result;
    union info *newinfo, *oldinfo;
    int newsize;

    result = align(size);
    if (!result)
	return NULL;

    /* Check if we need to grow the info table. */
    if (BLOCK((char *) result + size) > heapsize) {
	newsize = heapsize;
	while (BLOCK((char *) result + size) > newsize)
	    newsize *= 2;
	newinfo = align(newsize * sizeof (union info));
	if (!newinfo) {
	    (*_morecore)(-size);
	    return NULL;
	}
	memset(newinfo, 0, newsize * sizeof (union info));
	memcpy(newinfo, _heapinfo, heapsize * sizeof (union info));
	oldinfo = _heapinfo;
	newinfo[BLOCK(oldinfo)].busy.type = 0;
	newinfo[BLOCK(oldinfo)].busy.info.size
	    = BLOCKIFY(heapsize * sizeof (union info));
	_heapinfo = newinfo;
	gnufree(oldinfo);
	heapsize = newsize;
    }

    _heaplimit = BLOCK((char *) result + size);
    return result;
}

/* Allocate memory from the heap. */
void *
gnumalloc(size_t size)
{
    void *result;
    int log, block, blocks, i, lastblocks, start;
    struct list *next;

    if (!initialized && !initialize())
	return NULL;

    if (size == 0)
	return NULL;

    if (size < sizeof (struct list))
	size = sizeof (struct list);

    /* Determine the allocation policy based on the request size. */
    if (size <= BLOCKSIZE / 2) {
	/* Small allocation to receive a fragment of a block. Determine
	   the logarithm to base two of the fragment size. */
	--size;
	for (log = 1; (size >>= 1) != 0; ++log)
	    ;

	/* Look in the fragment lists for a free fragment of the
	   desired size. */
	if ((next = _fraghead[log].next) != 0) {
	    /* There are free fragments of this size.  Pop a fragment
	       out of the fragment list and return it.  Update the block's
	       nfree and first counters. */
	    result = next;
	    next->prev->next = next->next;
	    if (next->next)
		next->next->prev = next->prev;
	    block = BLOCK(result);
	    if (--_heapinfo[block].busy.info.frag.nfree)
		_heapinfo[block].busy.info.frag.first
		    = (unsigned int) ((char *) next->next - (char *) NULL)
		      % BLOCKSIZE >> log;
	} else {
	    /* No free fragments of the desired size, so get a new block
	       and break it into fragments, returning the first. */
	    result = gnumalloc(BLOCKSIZE);
	    if (!result)
		return NULL;
	    ++_fragblocks[log];

	    /* Link all fragments but the first into the free list. */
	    for (i = 1; i < BLOCKSIZE >> log; ++i) {
		next = (struct list *) ((char *) result + (i << log));
		next->next = _fraghead[log].next;
		next->prev = &_fraghead[log];
		next->prev->next = next;
		if (next->next)
		    next->next->prev = next;
	    }

	    /* Initialize the nfree and first counters for this block. */
	    block = BLOCK(result);
	    _heapinfo[block].busy.type = log;
	    _heapinfo[block].busy.info.frag.nfree = i - 1;
	    _heapinfo[block].busy.info.frag.first = i - 1;
	}
    } else {
	/* Large allocation to receive one or more blocks.  Search
	   the free list in a circle starting at the last place visited.
	   If we loop completely around without finding a large enough
	   space we will have to get more memory from the system. */
	blocks = BLOCKIFY(size);
	start = block = _heapindex;
	while (_heapinfo[block].free.size < blocks) {
	    block = _heapinfo[block].free.next;
	    if (block == start) {
		/* Need to get more from the system.  Check to see if
		   the new core will be contiguous with the final free
		   block; if so we don't need to get as much. */
		block = _heapinfo[0].free.prev;
		lastblocks = _heapinfo[block].free.size;
		if (_heaplimit && block + lastblocks == _heaplimit
		    && (*_morecore)(0) == ADDRESS(block + lastblocks)
		    && morecore((blocks - lastblocks) * BLOCKSIZE)) {
		    /* Note that morecore() can change the location of
		       the final block if it moves the info table and the
		       old one gets coalesced into the final block. */
		    block = _heapinfo[0].free.prev;
		    _heapinfo[block].free.size += blocks - lastblocks;
		    continue;
		}
		result = morecore(blocks * BLOCKSIZE);
		if (!result)
		    return NULL;
		block = BLOCK(result);
		_heapinfo[block].busy.type = 0;
		_heapinfo[block].busy.info.size = blocks;
		return result;
	    }
	}

	/* At this point we have found a suitable free list entry.
	   Figure out how to remove what we need from the list. */
	result = ADDRESS(block);
	if (_heapinfo[block].free.size > blocks) {
	    /* The block we found has a bit left over, so relink the
	       tail end back into the free list. */
	    _heapinfo[block + blocks].free.size
		= _heapinfo[block].free.size - blocks;
	    _heapinfo[block + blocks].free.next
		= _heapinfo[block].free.next;
	    _heapinfo[block + blocks].free.prev
		= _heapinfo[block].free.prev;
	    _heapinfo[_heapinfo[block].free.prev].free.next
		= _heapinfo[_heapinfo[block].free.next].free.prev
		    = _heapindex = block + blocks;
	} else {
	    /* The block exactly matches our requirements, so
	       just remove it from the list. */
	    _heapinfo[_heapinfo[block].free.next].free.prev
		= _heapinfo[block].free.prev;
	    _heapinfo[_heapinfo[block].free.prev].free.next
		= _heapindex = _heapinfo[block].free.next;
	}

	_heapinfo[block].busy.type = 0;
	_heapinfo[block].busy.info.size = blocks;
    }

    return result;
}
/* morecore.c - C library support routine for UNIX.
   Copyright (c) 1989, 1993  Michael J. Haertel
   You may redistribute this library under the terms of the
   GNU Library General Public License (version 2 or any later
   version) as published by the Free Software Foundation.
   THIS SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY EXPRESS OR IMPLIED
   WARRANTY.  IN PARTICULAR, THE AUTHOR MAKES NO REPRESENTATION OR
   WARRANTY OF ANY KIND CONCERNING THE MERCHANTABILITY OF THIS
   SOFTWARE OR ITS FITNESS FOR ANY PARTICULAR PURPOSE. */

#include <limits.h>
#include <stddef.h>

extern void *sbrkx(int);

/* Note that morecore has to take a signed argument so
   that negative values can return memory to the system. */
void *
_default_morecore(long size)
{
    void *result;

    result = sbrkx(size);
    if (result == (void *) -1)
	return NULL;
    return result;
}
/* realloc.c - C standard library routine.
   Copyright (c) 1989, 1993  Michael J. Haertel
   You may redistribute this library under the terms of the
   GNU Library General Public License (version 2 or any later
   version) as published by the Free Software Foundation.
   THIS SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY EXPRESS OR IMPLIED
   WARRANTY.  IN PARTICULAR, THE AUTHOR MAKES NO REPRESENTATION OR
   WARRANTY OF ANY KIND CONCERNING THE MERCHANTABILITY OF THIS
   SOFTWARE OR ITS FITNESS FOR ANY PARTICULAR PURPOSE. */

#include <limits.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define MIN(A, B) ((A) < (B) ? (A) : (B))

/* Resize the given region to the new size, returning a pointer
   to the (possibly moved) region.  This is optimized for speed;
   some benchmarks seem to indicate that greater compactness is
   achieved by unconditionally allocating and copying to a
   new region. */
void *
gnurealloc(void *ptr, size_t size)
{
    void *result, *previous;
    int block, blocks, type;
    int oldlimit;

    if (!ptr)
	return gnumalloc(size);
    if (!size) {
	gnufree(ptr);
	return gnumalloc(0);
    }

    block = BLOCK(ptr);

    switch (type = _heapinfo[block].busy.type) {
    case 0:
	/* Maybe reallocate a large block to a small fragment. */
	if (size <= BLOCKSIZE / 2) {
	    if ((result = gnumalloc(size)) != NULL) {
	    	memcpy(result, ptr, size);
	    	gnufree(ptr);
	    }
	    return result;
	}

	/* The new size is a large allocation as well; see if
	   we can hold it in place. */
	blocks = BLOCKIFY(size);
	if (blocks < _heapinfo[block].busy.info.size) {
	    /* The new size is smaller; return excess memory
	       to the free list. */
	    _heapinfo[block + blocks].busy.type = 0;
	    _heapinfo[block + blocks].busy.info.size
		= _heapinfo[block].busy.info.size - blocks;
	    _heapinfo[block].busy.info.size = blocks;
	    gnufree(ADDRESS(block + blocks));
	    return ptr;
	} else if (blocks == _heapinfo[block].busy.info.size)
	    /* No size change necessary. */
	    return ptr;
	else {
	    /* Won't fit, so allocate a new region that will.  Free
	       the old region first in case there is sufficient adjacent
	       free space to grow without moving. */
	    blocks = _heapinfo[block].busy.info.size;
	    /* Prevent free from actually returning memory to the system. */
	    oldlimit = _heaplimit;
	    _heaplimit = 0;
	    gnufree(ptr);
	    _heaplimit = oldlimit;
	    result = gnumalloc(size);
	    if (!result) {
		/* Now we're really in trouble.  We have to unfree
		   the thing we just freed.  Unfortunately it might
		   have been coalesced with its neighbors. */
		if (_heapindex == block)
		    gnumalloc(blocks * BLOCKSIZE);
		else {
		    previous = gnumalloc((block - _heapindex) * BLOCKSIZE);
		    gnumalloc(blocks * BLOCKSIZE);
		    gnufree(previous);
		}	    
		return NULL;
	    }
	    if (ptr != result)
		memmove(result, ptr, blocks * BLOCKSIZE);
	    return result;
	}
	break;

    default:
	/* Old size is a fragment; type is logarithm to base two of
	   the fragment size. */
	if ((size > 1 << (type - 1)) && (size <= 1 << type))
	    /* New size is the same kind of fragment. */
	    return ptr;
	else {
	    /* New size is different; allocate a new space, and copy
	       the lesser of the new size and the old. */
	    result = gnumalloc(size);
	    if (!result)
		return NULL;
	    memcpy(result, ptr, MIN(size, 1 << type));
	    gnufree(ptr);
	    return result;
	}
	break;
    }
}
/* valloc.c - Berkeley C Library Compatibility routine.
   Copyright (c) 1989, 1993  Michael J. Haertel
   You may redistribute this library under the terms of the
   GNU Library General Public License (version 2 or any later
   version) as published by the Free Software Foundation.
   THIS SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY EXPRESS OR IMPLIED
   WARRANTY.  IN PARTICULAR, THE AUTHOR MAKES NO REPRESENTATION OR
   WARRANTY OF ANY KIND CONCERNING THE MERCHANTABILITY OF THIS
   SOFTWARE OR ITS FITNESS FOR ANY PARTICULAR PURPOSE. */

#include <limits.h>
#include <stddef.h>
#include <stdlib.h>

#define MAX(A,B) ((A) > (B) ? (A) : (B))

/*
 * WARNING: The definition of BLOCKSIZE (in "malloc.h")
 * must be greater than or equal to the page size of
 * your machine.  We don't do getpagesize() because I
 * want to keep weird Unix dependencies out of the code.
 */
void *
valloc(size_t size)
{
    return gnumalloc(MAX(BLOCKSIZE, size));
}
