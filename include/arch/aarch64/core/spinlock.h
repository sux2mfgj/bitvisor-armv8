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

#ifndef __CORE_SPINLOCK_H
#define __CORE_SPINLOCK_H

#ifdef SPINLOCK_DEBUG
#include <core/panic.h>
#endif
#include <core/types.h>

typedef u8 spinlock_t;
typedef u32 rw_spinlock_t;
typedef struct {
	u32 next_ticket, now_serving;
} ticketlock_t;

#define SPINLOCK_INITIALIZER ((spinlock_t)0)

#ifdef SPINLOCK_DEBUG
#define spinlock_lock _spinlock_lock
#endif

static inline void
spinlock_lock (spinlock_t *l)
{
    //TODO
}

#ifdef SPINLOCK_DEBUG
#undef spinlock_lock
#define spinlock_debug1(a) spinlock_debug2(a)
#define spinlock_debug2(a) #a
#define spinlock_lock(l) spinlock_lock_debug (l, \
	"spinlock_lock failed." \
	" file " __FILE__ \
	" line " spinlock_debug1 (__LINE__))

static inline void
spinlock_lock_debug (spinlock_t *l, char *msg)
{
    //TODO
}
#endif

static inline void
spinlock_unlock (spinlock_t *l)
{
    //TODO
}

static inline void
spinlock_init (spinlock_t *l)
{
	spinlock_unlock (l);
}

static inline void
rw_spinlock_lock_sh (rw_spinlock_t *l)
{
    //TODO
}

static inline void
rw_spinlock_unlock_sh (rw_spinlock_t *l)
{
    //TODO
}

static inline void
rw_spinlock_lock_ex (rw_spinlock_t *l)
{
    //TODO
}

/* return value 0: lock succeeded */
static inline rw_spinlock_t
rw_spinlock_trylock_ex (rw_spinlock_t *l)
{
    //TODO
    rw_spinlock_t t;
    return t;
}

static inline void
rw_spinlock_unlock_ex (rw_spinlock_t *l)
{
    //TODO
}

static inline void
rw_spinlock_init (rw_spinlock_t *l)
{
	*l = 0;
}

static inline void
ticketlock_lock (ticketlock_t *l)
{
    //TODO
}

static inline void
ticketlock_unlock (ticketlock_t *l)
{
    //TODO
}

static inline void
ticketlock_init (ticketlock_t *l)
{
	l->next_ticket = 0;
	l->now_serving = 0;
}

#endif
