// Memory layout

#define EXTMEM  0x100000            // Start of extended memory
#define PHYSTOP 0xE000000          // Top physical memory

#define DEVSPACE 0xFE000000         // Other devices are at high addresses

// Key addresses for address space layout (see kmap in vm.c for layout)
#define KERNBASE 0x80000000         // First kernel virtual address
#define KERNLINK (KERNBASE+EXTMEM)  // Address where kernel is linked

#define V2P(a) (((uint) (a)) - KERNBASE)
#define P2V(a) ((void *)(((char *) (a)) + KERNBASE))

#define V2P_WO(x) ((x) - KERNBASE)    // same as V2P, but without casts
#define P2V_WO(x) ((x) + KERNBASE)    // same as P2V, but without casts

// Changes for huge pages
#define HUGE_PAGE_START 0x1E000000  // Start of reserved region - 480 MB
#define HUGE_PAGE_END   0x3E000000  // End of reserved region - 992 MB
#define HUGE_PAGE_SIZE  0x400000    // 4MB

#define HUGE_PAGE_VSTART P2V(HUGE_PAGE_START)
#define HUGE_PAGE_VEND   P2V(HUGE_PAGE_END) //HUGE_PAGE_START + KERNBASE
