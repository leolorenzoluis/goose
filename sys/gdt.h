#ifndef SYS_GDT_H_
#define SYS_GDT_H_

// TODO(chrsmith): Refactor.
/* Defines a GDT entry. We say packed, because it prevents the
*  compiler from doing things that it thinks is best: Prevent
*  compiler "optimization" by packing */
struct gdt_entry
{
    unsigned short limit_low;
    unsigned short base_low;
    unsigned char base_middle;
    unsigned char access;
    unsigned char granularity;
    unsigned char base_high;
} __attribute__((packed));

/* Special pointer which includes the limit: The max bytes
*  taken up by the GDT, minus 1. Again, this NEEDS to be packed */
struct gdt_ptr
{
    unsigned short limit;
    unsigned int base;
} __attribute__((packed));

/* Our GDT, with 3 entries, and finally our special GDT pointer */
struct gdt_entry gdt[3];
struct gdt_ptr _gp;

/* This will be a function in start.asm. We use this to properly
*  reload the new segment registers */

extern void _gdt_flush();
void gdt_install();
void gdt_set_gate(int num, unsigned long base, unsigned long limit, unsigned char access, unsigned char gran);

#endif  // SYS_GDT_H_
