// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

struct {
  struct spinlock lock;
  uint64 cnt[INDEX(PHYSTOP) + 1];
} pageref;

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;

void
kinit()
{
  initlock(&pageref.lock, "reflock");
  acquire(&pageref.lock);
  for (int i = 0; i < INDEX(PHYSTOP) + 1; ++i)
    pageref.cnt[i] = 0;
  release(&pageref.lock);
  initlock(&kmem.lock, "kmem");
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  acquire(&pageref.lock);
  if (pageref.cnt[INDEX((uint64)pa)] > 1) {
    --pageref.cnt[INDEX((uint64)pa)];
    release(&pageref.lock);
    return;
  }
  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  pageref.cnt[INDEX((uint64)pa)] = 0;
  release(&pageref.lock);

  r = (struct run*)pa;

  acquire(&kmem.lock);
  r->next = kmem.freelist;
  kmem.freelist = r;
  release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r)
    kmem.freelist = r->next;
  release(&kmem.lock);

  if(r) {
    add_ref((uint64)r);
    memset((char*)r, 5, PGSIZE); // fill with junk
  }
  return (void*)r;
}

void
add_ref(uint64 pa) {
  acquire(&pageref.lock);
  ++pageref.cnt[INDEX(pa)];
  release(&pageref.lock);
}

void
dec_ref(uint64 pa) {
  acquire(&pageref.lock);
  --pageref.cnt[INDEX(pa)];
  release(&pageref.lock);
}