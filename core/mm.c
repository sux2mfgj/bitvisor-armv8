/*
 * Copyright (c) 2007, 2008 University of Tsukuba
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of the University of Tsukuba nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "mm.h"

#include <core.h>

#include "assert.h"
#include "initfunc.h"
#include "list.h"
#include "panic.h"

#define NUM_OF_ALLOCSIZE 13
#define NUM_OF_SMALL_ALLOCSIZE 5
#define SMALL_ALLOCSIZE(n) (0x80 << (n))
#define NUM_OF_TINY_ALLOCSIZE 3
#define TINY_ALLOCSIZE(n) (0x10 << (n))
#define FIND_NEXT_BIT(bitmap) (~(bitmap) & ((bitmap) + 1))
#define BIT_TO_INDEX(bit) (__builtin_ffs ((bit)) - 1)

enum page_type {
	PAGE_TYPE_FREE,
	PAGE_TYPE_NOT_HEAD,
	PAGE_TYPE_ALLOCATED,
	PAGE_TYPE_ALLOCATED_CONT,
	PAGE_TYPE_ALLOCATED_SMALL,
	PAGE_TYPE_RESERVED,
};

struct page {
	LIST1_DEFINE (struct page);
	phys_t phys;
	u32 small_bitmap;
	enum page_type type : 8;
	unsigned int allocsize : 8;
	unsigned int small_allocsize : 8;
};

struct tiny_allocdata {
	LIST1_DEFINE (struct tiny_allocdata);
	u32 tiny_bitmap;
	u8 tiny_allocsize;
};

static LIST1_DEFINE_HEAD (struct tiny_allocdata,
			  tiny_freelist[NUM_OF_TINY_ALLOCSIZE]);
static LIST1_DEFINE_HEAD (struct page, small_freelist[NUM_OF_SMALL_ALLOCSIZE]);

extern u8 phys_base[];
extern u8 end[], head[];

static struct page pagestruct[NUM_OF_PAGES];
static LIST1_DEFINE_HEAD (struct page, list1_freepage[NUM_OF_ALLOCSIZE]);
u32 vmm_start_phys;

static struct page *
virt_to_page (virt_t virt)
{
	unsigned int i;

	i = (virt - (virt_t)vmm_start_phys) >> PAGESIZE_SHIFT;
	// ASSERT (i < NUM_OF_PAGES);
	return &pagestruct[i];
}

// in aarch64, virtual memory is straight mapped.
virt_t
phys_to_virt (phys_t phys)
{
	return (virt_t)phys;
}

static struct page *
phys_to_page (phys_t phys)
{
	return virt_to_page (phys_to_virt (phys));
}

static phys_t
page_to_phys (struct page *p)
{
	return p->phys;
}

static virt_t
page_to_virt (struct page *p)
{
	return phys_to_virt (page_to_phys (p));
}

u32
vmm_start_inf ()
{
	return vmm_start_phys;
}

u32
vmm_term_inf ()
{
	return vmm_start_phys + VMMSIZE_ALL;
}

static int
mm_page_get_allocsize (int n)
{
	// ASSERT (n >= 0);
	// ASSERT (n < NUM_OF_ALLOCSIZE);
	return 4096 << n;
}

static int
get_alloc_pages_size (int n, int *second)
{
	int sz = n * PAGESIZE;
	for (int i = 0; i < NUM_OF_ALLOCSIZE; i++) {
		int allocsize = mm_page_get_allocsize (i);
		if (allocsize >= sz) {
			*second = -1;
			return i;
		}
		if (i > 0 && allocsize + mm_page_get_allocsize (i - 1) >= sz) {
			int secondsz = sz - allocsize;
			int j;
			for (j = 0; j < i - 1; j++)
				if (mm_page_get_allocsize (j) >= secondsz)
					break;
			*second = j;
			return i;
		}
	}
	return -1;
}

static struct page *
mm_page_free_2ndhalf (struct page *p)
{
	int n = p->allocsize;
	// ASSERT (n > 0);
	n--;
	virt_t virt = page_to_virt (p);
	int s = mm_page_get_allocsize (n);
	// ASSERT (!(virt & s));
	p->allocsize = n;
	struct page *q = virt_to_page (virt | s);
	q->type = PAGE_TYPE_FREE;
	LIST1_ADD (list1_freepage[n], q);
	return q;
}

static struct page *
mm_page_alloc_sub (int n)
{
	for (int i = n; i < NUM_OF_ALLOCSIZE; i++) {
		struct page *p = LIST1_POP (list1_freepage[i]);
		if (p) {
			while (p->allocsize > n)
				mm_page_free_2ndhalf (p);
			return p;
		}
	}
	return NULL;
}

static struct page *
mm_page_alloc (int n)
{
	struct page *p;
	// enum page_type old_type;

	// ASSERT (n < NUM_OF_ALLOCSIZE);
	// spinlock_lock (&mm_lock);
	p = mm_page_alloc_sub (n);
	if (!p) {
		// spinlock_unlock (&mm_lock);
		// panic ("%s(%d): Out of memory", __func__, n);
	}
	/* p->type must be set before unlock, because the
	 * mm_page_free() function may merge blocks if the type is
	 * PAGE_TYPE_FREE. */
	// old_type = p->type;
	p->type = PAGE_TYPE_ALLOCATED;
	// spinlock_unlock (&mm_lock);
	/* The old_type must be PAGE_TYPE_FREE, or the memory will be
	 * corrupted.  The ASSERT is called after unlock to avoid
	 * deadlocks during panic. */
	// ASSERT (old_type == PAGE_TYPE_FREE);
	return p;
}

static struct page *
mm_page_alloc_cont (int n, int m)
{
	int s;
	virt_t virt;
	struct page *p, *q;
	// enum page_type old_ptype, old_qtype;

	// ASSERT (n < NUM_OF_ALLOCSIZE);
	// ASSERT (m < NUM_OF_ALLOCSIZE);
	s = mm_page_get_allocsize (n);
	// spinlock_lock (&mm_lock);
	LIST1_FOREACH (list1_freepage[n], p)
	{
		virt = page_to_virt (p);
		if (virt & s)
			goto not_found;
		q = virt_to_page (virt | s);
		if (q->type == PAGE_TYPE_FREE && q->allocsize >= m) {
			LIST1_DEL (list1_freepage[n], p);
			goto found;
		}
	}
not_found:
	p = mm_page_alloc_sub (n + 1);
	if (!p) {
		// spinlock_unlock (&mm_lock);
		// panic ("%s(%d,%d): Out of memory", __func__, n, m);
	}
	q = mm_page_free_2ndhalf (p);
found:
	LIST1_DEL (list1_freepage[q->allocsize], q);
	// old_ptype = p->type;
	// old_qtype = q->type;
	p->type = PAGE_TYPE_ALLOCATED_CONT;
	while (q->allocsize > m)
		mm_page_free_2ndhalf (q);
	q->type = PAGE_TYPE_ALLOCATED;
	// spinlock_unlock (&mm_lock);
	// ASSERT (old_ptype == PAGE_TYPE_FREE);
	// ASSERT (old_qtype == PAGE_TYPE_FREE);
	return p;
}

/* allocate n or more pages */
int
alloc_pages (void **virt, u64 *phys, int n)
{
	struct page *p;
	int i, j;

	i = get_alloc_pages_size (n, &j);
	if (i >= 0)
		goto found;
	panic ("alloc_pages (%d) failed.", n);
	return -1;
found:
	if (j < 0)
		p = mm_page_alloc (i);
	else
		p = mm_page_alloc_cont (i, j);
	if (virt)
		*virt = (void *)page_to_virt (p);
	if (phys)
		*phys = page_to_phys (p);
	return 0;
}

/* allocate a page */
int
alloc_page (void **virt, u64 *phys)
{
	return alloc_pages (virt, phys, 1);
}

static char *
mm_page_free_sub (struct page *p)
{
	int s, n;
	struct page *q, *tmp;
	virt_t virt;

	n = p->allocsize;
	s = mm_page_get_allocsize (n);
	virt = page_to_virt (p);
	if (p->type == PAGE_TYPE_ALLOCATED_CONT) {
		if (virt & s)
			return "bad address";
		p->type = PAGE_TYPE_ALLOCATED;
		mm_page_free_sub (virt_to_page (virt | s));
	}
	switch (p->type) {
	case PAGE_TYPE_FREE:
		return "double free";
	case PAGE_TYPE_NOT_HEAD:
		return "not head free";
	case PAGE_TYPE_ALLOCATED:
	case PAGE_TYPE_RESERVED:
		break;
	default:
		return "bad type";
	}
	p->type = PAGE_TYPE_FREE;
	if (virt & s)
		LIST1_ADD (list1_freepage[n], p);
	else
		LIST1_PUSH (list1_freepage[n], p);
	while (n < (NUM_OF_ALLOCSIZE - 1) &&
	       (q = virt_to_page (virt ^ s))->type == PAGE_TYPE_FREE &&
	       q->allocsize == n) {
		if (virt & s) {
			tmp = p;
			p = q;
			q = tmp;
		}
		LIST1_DEL (list1_freepage[n], p);
		LIST1_DEL (list1_freepage[n], q);
		q->type = PAGE_TYPE_NOT_HEAD;
		n = ++p->allocsize;
		s = mm_page_get_allocsize (n);
		if (virt & s)
			LIST1_ADD (list1_freepage[n], p);
		else
			LIST1_PUSH (list1_freepage[n], p);
		virt = page_to_virt (p);
	}
	return NULL;
}

static void
mm_page_free (struct page *p)
{
	// spinlock_lock (&mm_lock);
	char *fail = mm_page_free_sub (p);
	// spinlock_unlock (&mm_lock);
	// panic is not implemented
	// if (fail)
	// panic ("%s: %s", __func__, fail);
}

static int
get_small_allocsize (unsigned int len)
{
	for (int i = 0; i < NUM_OF_SMALL_ALLOCSIZE; i++)
		if (len <= SMALL_ALLOCSIZE (i))
			return i;
	return -1;
}

static int
get_tiny_allocsize (unsigned int len)
{
	for (int i = 0; i < NUM_OF_TINY_ALLOCSIZE; i++)
		if (len <= TINY_ALLOCSIZE (i))
			return i;
	return -1;
}

static virt_t
small_alloc (int small_allocsize)
{
	ASSERT (small_allocsize >= 0);
	ASSERT (small_allocsize < NUM_OF_SMALL_ALLOCSIZE);
	int nbits = PAGESIZE / SMALL_ALLOCSIZE (small_allocsize);
	u32 full = (2U << (nbits - 1)) - 1;
	u32 n;
	struct page *p;
	int i;
	// spinlock_lock (&mm_small_lock);
	LIST1_FOREACH (small_freelist[small_allocsize], p)
	{
		n = FIND_NEXT_BIT (p->small_bitmap);
		p->small_bitmap |= n;
		if (p->small_bitmap == full)
			LIST1_DEL (small_freelist[small_allocsize], p);
		goto ok;
	}
	// spinlock_unlock (&mm_small_lock);
	p = mm_page_alloc (0);
	ASSERT (p);
	ASSERT (p->type == PAGE_TYPE_ALLOCATED);
	p->type = PAGE_TYPE_ALLOCATED_SMALL;
	n = 1;
	p->small_allocsize = small_allocsize;
	p->small_bitmap = n;
	// spinlock_lock (&mm_small_lock);
	LIST1_PUSH (small_freelist[small_allocsize], p);
ok:
	// spinlock_unlock (&mm_small_lock);
	i = BIT_TO_INDEX (n);
	ASSERT (i >= 0);
	ASSERT (i < nbits);
	return page_to_virt (p) + SMALL_ALLOCSIZE (small_allocsize) * i;
}

static virt_t
tiny_alloc (int tiny_allocsize)
{
	ASSERT (tiny_allocsize >= 0);
	ASSERT (tiny_allocsize < NUM_OF_TINY_ALLOCSIZE);
	int nbits = 32;
	u32 n;
	struct tiny_allocdata *p;
	ASSERT (sizeof *p <= 2 * TINY_ALLOCSIZE (0));
	ASSERT (sizeof *p <= TINY_ALLOCSIZE (1));
	u32 empty_bitmap = tiny_allocsize ? 1 : 3;
	int i;
	// spinlock_lock (&mm_tiny_lock);
	LIST1_FOREACH (tiny_freelist[tiny_allocsize], p)
	{
		n = FIND_NEXT_BIT (p->tiny_bitmap);
		p->tiny_bitmap |= n;
		if (!~p->tiny_bitmap)
			LIST1_DEL (tiny_freelist[tiny_allocsize], p);
		goto ok;
	}
	// spinlock_unlock (&mm_tiny_lock);
	int plen = TINY_ALLOCSIZE (tiny_allocsize) * nbits;
	p = (void *)small_alloc (get_small_allocsize (plen));
	n = FIND_NEXT_BIT (empty_bitmap);
	p->tiny_allocsize = tiny_allocsize;
	p->tiny_bitmap = empty_bitmap | n;
	// spinlock_lock (&mm_tiny_lock);
	LIST1_PUSH (tiny_freelist[tiny_allocsize], p);
ok:
	// spinlock_unlock (&mm_tiny_lock);
	i = BIT_TO_INDEX (n);
	ASSERT (i > 0);
	ASSERT (i < nbits);
	return (virt_t)p + TINY_ALLOCSIZE (tiny_allocsize) * i;
}

static unsigned int
tiny_getlen (virt_t start_of_block)
{
	struct tiny_allocdata *p = (void *)start_of_block;
	int tiny_allocsize = p->tiny_allocsize;
	ASSERT (tiny_allocsize >= 0);
	ASSERT (tiny_allocsize < NUM_OF_TINY_ALLOCSIZE);
	return TINY_ALLOCSIZE (tiny_allocsize);
}

static void
small_free (struct page *p, u32 page_offset)
{
	ASSERT (p->type == PAGE_TYPE_ALLOCATED_SMALL);
	int small_allocsize = p->small_allocsize;
	ASSERT (small_allocsize >= 0);
	ASSERT (small_allocsize < NUM_OF_SMALL_ALLOCSIZE);
	int nbits = PAGESIZE / SMALL_ALLOCSIZE (small_allocsize);
	u32 full = (2U << (nbits - 1)) - 1;
	ASSERT (!(page_offset % SMALL_ALLOCSIZE (small_allocsize)));
	int i = page_offset / SMALL_ALLOCSIZE (small_allocsize);
	ASSERT (i < nbits);
	u32 n = 1 << i;
	// spinlock_lock (&mm_small_lock);
	if (p->small_bitmap == full)
		LIST1_PUSH (small_freelist[small_allocsize], p);
	u32 new_small_bitmap = p->small_bitmap ^ n;
	p->small_bitmap = new_small_bitmap;
	if (!new_small_bitmap)
		LIST1_DEL (small_freelist[small_allocsize], p);
	// spinlock_unlock (&mm_small_lock);
	if (new_small_bitmap & n)
		panic ("%s: double free", __func__);
	if (!new_small_bitmap) {
		p->type = PAGE_TYPE_ALLOCATED;
		mm_page_free (p);
	}
}

static void
tiny_free (struct page *page, virt_t start_of_block, u32 block_offset)
{
	struct tiny_allocdata *p = (void *)start_of_block;
	int tiny_allocsize = p->tiny_allocsize;
	ASSERT (tiny_allocsize >= 0);
	ASSERT (tiny_allocsize < NUM_OF_TINY_ALLOCSIZE);
	int nbits = 32;
	ASSERT (!(block_offset % TINY_ALLOCSIZE (tiny_allocsize)));
	int i = block_offset / TINY_ALLOCSIZE (tiny_allocsize);
	ASSERT (i < nbits);
	u32 n = 1 << i;
	ASSERT (sizeof *p <= 2 * TINY_ALLOCSIZE (0));
	ASSERT (sizeof *p <= TINY_ALLOCSIZE (1));
	u32 empty_bitmap = tiny_allocsize ? 1 : 3;
	ASSERT (!(empty_bitmap & n));
	// spinlock_lock (&mm_tiny_lock);
	if (!~p->tiny_bitmap)
		LIST1_PUSH (tiny_freelist[tiny_allocsize], p);
	u32 new_tiny_bitmap = p->tiny_bitmap ^ n;
	p->tiny_bitmap = new_tiny_bitmap;
	if (new_tiny_bitmap == empty_bitmap)
		LIST1_DEL (tiny_freelist[tiny_allocsize], p);
	// spinlock_unlock (&mm_tiny_lock);
	if (new_tiny_bitmap & n)
		panic ("%s: double free", __func__);
	if (new_tiny_bitmap == empty_bitmap)
		small_free (page, start_of_block & PAGESIZE_MASK);
}

/* allocate n bytes */
void *
alloc (uint len)
{
	void *r;
	int small = get_small_allocsize (len);
	if (small < 0) {
		alloc_pages (&r, NULL, (len + 4095) / 4096);
	} else {
		int tiny = !small ? get_tiny_allocsize (len) : -1;
		if (tiny < 0)
			r = (void *)small_alloc (small);
		else
			r = (void *)tiny_alloc (tiny);
	}
	return r;
}

void
free (void *virt)
{
	virt_t v = (virt_t)virt;
	struct page *p = virt_to_page (v);
	if (p->type == PAGE_TYPE_ALLOCATED_SMALL) {
		int small_allocsize = p->small_allocsize;
		u32 page_offset = v & PAGESIZE_MASK;
		u32 block_offset =
			page_offset % SMALL_ALLOCSIZE (small_allocsize);
		if (block_offset)
			tiny_free (p, v - block_offset, block_offset);
		else
			small_free (p, page_offset);
	} else {
		mm_page_free (p);
	}
}

static void
mm_init_global (void)
{
	int i;

	vmm_start_phys = (phys_t)phys_base;

	// initailize list data structore
	for (i = 0; i < NUM_OF_ALLOCSIZE; i++)
		LIST1_HEAD_INIT (list1_freepage[i]);
	// for (i = 0; i < NUM_OF_SMALL_ALLOCSIZE; i++)
	//	LIST1_HEAD_INIT (small_freelist[i]);
	// for (i = 0; i < NUM_OF_TINY_ALLOCSIZE; i++)
	//	LIST1_HEAD_INIT (tiny_freelist[i]);
	for (i = 0; i < NUM_OF_PAGES; i++) {
		pagestruct[i].type = PAGE_TYPE_RESERVED;
		pagestruct[i].allocsize = 0;
		pagestruct[i].phys = vmm_start_phys + PAGESIZE * i;
	}

	for (i = 0; i < NUM_OF_PAGES; i++) {
		if ((u64)(virt_t)head <= page_to_virt (&pagestruct[i]) &&
		    page_to_virt (&pagestruct[i]) < (u64)(virt_t)end)
			continue;
		// if (i < panicmem_start_page + NUM_OF_PANICMEM_PAGES)
		//	continue;
		mm_page_free (&pagestruct[i]);
	}
}

INITFUNC ("global3", mm_init_global);
