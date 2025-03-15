// Physical memory allocator, intended to allocate
// memory for user processes, kernel stacks, page table pages,
// and pipe buffers. Allocates 4096-byte pages.

#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "spinlock.h"

void freerange(void *vstart, void *vend);
void khugeinit(void);
void khugefree(char *v);
char* khugealloc(void);

extern char end[]; // first address after kernel loaded from ELF file
                   // defined by the kernel linker script in kernel.ld

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  int use_lock;
  struct run *freelist;
} kmem;

static struct {
  struct spinlock lock;
  int use_lock;
  struct run *freelist;
} khugemem;
// Initialization happens in two phases.
// 1. main() calls kinit1() while still using entrypgdir to place just
// the pages mapped by entrypgdir on free list.
// 2. main() calls kinit2() with the rest of the physical pages
// after installing a full page table that maps them on all cores.
void
kinit1(void *vstart, void *vend)
{
  initlock(&kmem.lock, "kmem");
  kmem.use_lock = 0;
  freerange(vstart, vend);
}

void
kinit2(void *vstart, void *vend)
{
  freerange(vstart, vend);
  kmem.use_lock = 1;
}

void
freerange(void *vstart, void *vend)
{
  char *p;
  p = (char*)PGROUNDUP((uint)vstart);
  for(; p + PGSIZE <= (char*)vend; p += PGSIZE)
    kfree(p);
}


//PAGEBREAK: 21
// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(char *v)
{
  struct run *r;

  if((uint)v % PGSIZE || v < end || V2P(v) >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(v, 1, PGSIZE);

  if(kmem.use_lock)
    acquire(&kmem.lock);
  r = (struct run*)v;
  r->next = kmem.freelist;
  kmem.freelist = r;
  if(kmem.use_lock)
    release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
char*
kalloc(void)
{
  struct run *r;

  if(kmem.use_lock)
    acquire(&kmem.lock);
  r = kmem.freelist;
  if(r)
    kmem.freelist = r->next;
  if(kmem.use_lock)
    release(&kmem.lock);
  return (char*)r;
}

void
khugeinit(void)
{
  char *p;

  initlock(&khugemem.lock, "khugemem");
  khugemem.use_lock = 1;

  p = (char*)PGROUNDUP(HUGE_PAGE_START);
  acquire(&khugemem.lock);
  for(; p + HUGE_PAGE_SIZE <= (char*)HUGE_PAGE_END; p += HUGE_PAGE_SIZE)
    khugefree(p);
  release(&khugemem.lock);
}

void
khugefree(char *v)
{
  struct run *r;

  if (((uint)v % HUGE_PAGE_SIZE) || v < (char*)HUGE_PAGE_START || v + HUGE_PAGE_SIZE > (char*)HUGE_PAGE_END)
    panic("invalid huge page ptr (khugefree)");

  memset(v, 1, HUGE_PAGE_SIZE);

  r = (struct run*)v;
  if(khugemem.use_lock)
    acquire(&khugemem.lock);
  r->next = khugemem.freelist;
  khugemem.freelist = r;
  if(khugemem.use_lock)
    release(&khugemem.lock);
}

char*
khugealloc(void)
{
  struct run *r;

  if(khugemem.use_lock)
  {
    acquire(&khugemem.lock);
  }
  r = khugemem.freelist;
  if(r)
  {
    khugemem.freelist = r->next;
  }
  if(khugemem.use_lock)
  {
    release(&khugemem.lock);
  }

  if(r)
  {
    memset((char*)r, 5, HUGE_PAGE_SIZE);
    return (char*)r;
  }
  return 0;
}
