/**
 * @file memory.h
 * @brief routines for tracking dynamic allocations and otherwise shared memory
 * @author Ben Blum <bblum@andrew.cmu.edu>
 */

#ifndef __LS_MEMORY_H
#define __LS_MEMORY_H

#include <simics/api.h> /* for bool, of all things... */

#include "lockset.h"
#include "rbtree.h"
#include "vector_clock.h"
#include "variable_queue.h"

struct hax;
struct stack_trace;

/******************************************************************************
 * Shared memory access tracking
 ******************************************************************************/

enum chunk_id_info { NOT_IN_HEAP, HAS_CHUNK_ID, MULTIPLE_CHUNK_IDS };

/* represents a single instance of accessing a shared memory location */
struct mem_lockset {
	unsigned int eip;
	unsigned int last_call;
	unsigned int most_recent_syscall;
	bool write;
	bool during_init;
	bool during_destroy;
	bool interrupce_enabled;
	bool during_txn;
	/* two locksets with existing but different chunk ids are a false
	 * positive for data race detection (see issues #23 and #170).
	 * however, since we aggressively combine locksets, multiple chunk
	 * ids may appear; if so, we fall back to false-positiving. */
	enum chunk_id_info any_chunk_ids;
	unsigned int chunk_id;
	struct lockset locks_held;
#ifdef PURE_HAPPENS_BEFORE
	struct vector_clock clock;
#endif
	Q_NEW_LINK(struct mem_lockset) nobe;
};

Q_NEW_HEAD(struct mem_locksets, struct mem_lockset);

/* represents a shared memory address, accessed once or more, possibly from
 * different locations in the code */
struct mem_access {
	unsigned int addr; /* byte granularity */
	bool any_writes;   /* true if any access among locksets is a write */
	/* PC is recorded per-lockset, so when there's a data race, the correct
	 * eip can be reported instead of the first one. */
	//int eip;         /* what instruction pointer */
	int other_tid;     /* does this access another thread's stack? 0 if none */
	int count;         /* how many times accessed? (stats) */
	bool conflict;     /* does this conflict with another transition? (stats) */
	struct mem_locksets locksets; /* distinct locksets used while accessing */
	struct rb_node nobe;
};

/* represents two instructions by different threads which accessed the same
 * memory location, where both threads did not hold the same lock and there was
 * not a happens-before relation between them [insert citation here]. the
 * intuition behind these criteria is that the memory accesses are "concurrent"
 * and can be interleaved at instruction granularity. in iterative deepening,
 * we use data race reports to identify places for new preemption points.
 *
 * a data race may be either "suspected" or "confirmed", the distinction being
 * whether we have yet observed that the instruction pair can actually be
 * reordered. in some cases a "suspected" data race cannot actually be reordered
 * because of an implicit HB relationship that's established through some other
 * shared variable communication. these are invisible to us during single-branch
 * analysis, because we compute HB only by runqueues, and can't see how shm
 * communication might affect flow control. fortunately, unlike single-branch
 * detectors (eraser, tsan, go test -race), we can remember data race reports
 * cross-branch and establish two tiers of "likelihood" by confirming whether
 * the race can be reordered. however, it is still possible for a non-
 * reorderable data race to indicate a bug (flow control in the same transition
 * may suppress the later access if reordered). */
struct data_race {
	int first_eip;
	int other_eip;
	/* which order were they observed in? "confirmed" iff both are true. */
	bool first_before_other;
	bool other_before_first;
	// TODO: record stack traces?
	// TODO: record originating tids (even when stack unavailable)?
	struct rb_node nobe;
};

/******************************************************************************
 * Heap state tracking
 ******************************************************************************/

/* a heap-allocated block. */
struct chunk {
	unsigned int base;
	unsigned int len;
	unsigned int id; /* distinguishes chunks in same-space-different-time */
	struct rb_node nobe;
	/* for use-after-free reporting */
	struct stack_trace *malloc_trace;
	struct stack_trace *free_trace;
	bool pages_reserved_for_malloc;
};

struct malloc_actions {
	bool in_alloc;
	bool in_realloc;
	bool in_free;
	unsigned int alloc_request_size; /* valid iff in_alloc */
	bool in_page_alloc;
	bool in_page_free;
	unsigned int palloc_request_size; /* valid iff in_page_alloc */
};

struct mem_state {
	/**** heap state tracking ****/
	struct rb_root malloc_heap;
	unsigned int heap_size;
	unsigned int heap_next_id; /* generation counter for chunks */

	/* Separate from the malloc heap because, in pintos, malloc uses
	 * palloc'ed pages as its backing arenas (the chunks will overlap).
	 * The above fields for size and generation counter are shared for
	 * simplicity of code, but others need to be duplicated. In pebbles
	 * this is deadcode. */
	struct rb_root palloc_heap;

	/* dynamic allocation request state */
	bool guest_init_done;
	bool in_mm_init; /* userspace only */
#ifndef ALLOW_REENTRANT_MALLOC_FREE
	/* otherwise, this struct appears per-thread in schedule.h. */
	struct malloc_actions flags;
#endif

	/**** userspace information ****/
	unsigned int cr3; /* 0 == uninitialized or this is for kernel mem */
	unsigned int cr3_tid; /* tid for which cr3 was registered (main tid of process) */
	unsigned int user_mutex_size; /* 0 == uninitialized or kernel mem as above */
	/* tracks the values read/written by xchgs, so we can decide when an
	 * xchg couldn't possibly unblock another xchg-looping thread. */
	bool during_xchg; /* wtb option types */
	unsigned int last_xchg_read;
	/**** shared memory conflict detection ****/
	/* set of all shared accesses that happened during this transition;
	 * cleared after each save point - done in save.c */
	struct rb_root shm;
	/* set of all chunks that were freed during this transition; cleared
	 * after each save point just like the shared memory one above */
	struct rb_root freed;
	/* set of candidate data races, maintained cross-branch */
	struct rb_root data_races;
	unsigned int data_races_suspected;
	unsigned int data_races_confirmed;
};

/******************************************************************************
 * Interface
 ******************************************************************************/

void mem_init(struct ls_state *);
void init_malloc_actions(struct malloc_actions *);

void mem_update(struct ls_state *);

void mem_check_shared_access(struct ls_state *, unsigned int phys_addr,
							 unsigned int virt_addr, bool write);
bool mem_shm_intersect(struct ls_state *ls, struct hax *h0, struct hax *h2,
                       bool in_kernel);

bool shm_contains_addr(struct mem_state *m, unsigned int addr);

bool check_user_address_space(struct ls_state *ls);

#endif
