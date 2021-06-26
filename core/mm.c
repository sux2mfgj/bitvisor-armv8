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
#include "list.h"
#include "initfunc.h"

#define VMMSIZE_ALL		(128 * 1024 * 1024)
#define NUM_OF_PAGES		(VMMSIZE_ALL >> PAGESIZE_SHIFT)
#define NUM_OF_ALLOCSIZE	13

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
	//ASSERT (i < NUM_OF_PAGES);
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

u32 vmm_start_inf()
{
        return vmm_start_phys ;
}

u32 vmm_term_inf()
{
        return vmm_start_phys+VMMSIZE_ALL ;
}

static int
mm_page_get_allocsize (int n)
{
	//ASSERT (n >= 0);
	//ASSERT (n < NUM_OF_ALLOCSIZE);
	return 4096 << n;
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
	//spinlock_lock (&mm_lock);
	char *fail = mm_page_free_sub (p);
	//spinlock_unlock (&mm_lock);
    // panic is not implemented
	//if (fail)
		//panic ("%s: %s", __func__, fail);
}

static void
mm_init_global (void)
{
    int i;

    vmm_start_phys = (phys_t)phys_base;

    // initailize list data structore
	for (i = 0; i < NUM_OF_ALLOCSIZE; i++)
		LIST1_HEAD_INIT (list1_freepage[i]);
	//for (i = 0; i < NUM_OF_SMALL_ALLOCSIZE; i++)
	//	LIST1_HEAD_INIT (small_freelist[i]);
	//for (i = 0; i < NUM_OF_TINY_ALLOCSIZE; i++)
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
		//if (i < panicmem_start_page + NUM_OF_PANICMEM_PAGES)
		//	continue;
		mm_page_free (&pagestruct[i]);
	}
}

INITFUNC ("global2", mm_init_global);
