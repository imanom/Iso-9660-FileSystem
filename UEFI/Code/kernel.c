/*
 * boot.c - a kernel template (Assignment 1, ECE 6504)
 * Copyright 2021 Ruslan Nikolaev <rnikola@vt.edu>
 */

#include <fb.h>
#include <printf.h>

typedef unsigned long long u64;

// Write to the CR3 register with the base address of the PML4 Table

void write_cr3(unsigned long long cr3_value)
{
	asm volatile("mov %0, %%cr3"
				 :
				 : "r"(cr3_value)
				 : "memory");
}

void page_table(u64 *p)
{
	u64 next_page = 0x0;

	//kernel

		for (int i = 0; i < 1048576; i++)
		{
			p[i] = (next_page + 0x3);

			next_page += 0x1000;
		}
	
}

void page_directory(u64 *pd, u64 *p)
{

	//kernel
	
		for (int j = 0; j < 2048; j++)
		{
			u64 *start_pte = p + 512 * j;
			u64 page_addr = (u64)start_pte;
			pd[j] = page_addr + 0x3;
		}
	

}

void page_directory_pointer(u64 *pdp, u64 *pd)
{
	for (int j = 0; j < 512; j++)
	{

		//first 4 entries in kernel
		if (j < 4)
		{
			u64 *start_pde = pd + 512 * j;
			u64 page_addr = (u64)start_pde;
			pdp[j] = page_addr + 0x3;
		}

		//other entries are null
		else
		{
			pdp[j] = 0x0ULL;
		}
	}
}

void pml4_table(u64 *pml4, u64 *pdp)
{
	for (int j = 0; j < 512; j++)
	{
		//Level 4 containing only kernel page table
		
			//first entry for kernel
			if (j == 0)
			{
				pml4[j] = (u64)pdp + 0x3;
			}
			//other entries are null
			else
			{
				pml4[j] = 0x0ULL;
			}
		
		
	}
}

void setup_kernel_pagetable(void *addr)
{
	//PTE
	u64 *p = (u64 *)addr;
	page_table(p);

	//PDE
	u64 *pd = (p + 1048576);
	page_directory(pd, p);

	//PDPE
	u64 *pdp = (pd + 2048);
	page_directory_pointer(pdp, pd);

	//PML4E
	u64 *pml4 = (pdp + 512);
	pml4_table(pml4, pdp);

	//CR3 register
	u64 *start_pml4e = pml4;
	u64 page_addr = (u64)start_pml4e;
	write_cr3(page_addr);

}



void kernel_start(void *addr, unsigned int *fb)
{
	fb_init(fb, 800, 600);
	setup_kernel_pagetable(addr);
	printf("\nAllocated initial kernel page table.\n");
	

	/* Never exit! */
	while (1)
	{
	};
}
