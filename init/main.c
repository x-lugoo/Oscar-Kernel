/* Arch indenpendant initialization 
* Oscar
* Jul, 2018
*
*/

#include <types.h>
#include <kernel.h>
#include <string.h>
#include <fb.h>
#include <mm.h>
#include <task.h>
#include <init.h>

void boot_fb_init();
void graphic_con_init();
void mm_enumate();

void fb_test()
{
	int i,j;
	u32 image_array[0x100000];
	for (i = 0; i < 200; i++) {
		for (j = 0; j < 200; j++) {
			if ((i - 100) * (i - 100) + (j - 100) * (j - 100) <= 10000)
				image_array[i + j * 200] = 0x0000ffff + 0x00010000 * j;
			else
				image_array[i + j * 200] = 0x008f9f7f + 0x00010000 * i;
		}
	}

	struct fb_image image_test = {600, 400, 200, 200, 0, 0, 0, (char *)image_array};
	imageblit_active_fb(&image_test);
}

extern void arch_numa_init();

void do_init_call()
{
	int i,j;
	init_call_t fun, *fun_ptr;
	struct init_call_entry {
		init_call_t *start;
		init_call_t *end;
	} init_call_array[] = {
		{ &__initcall1_start, &__initcall1_end },
		{ &__initcall2_start, &__initcall2_end },
		{ &__initcall3_start, &__initcall3_end },
		{ &__initcall4_start, &__initcall4_end },
		{ &__initcall5_start, &__initcall5_end },
		{ &__initcall6_start, &__initcall6_end },
		{ &__initcall7_start, &__initcall7_end },
		{ &__initcall8_start, &__initcall8_end },
	};

	for(i = 0; i < 8; i++) {
		for (fun_ptr = init_call_array[i].start; fun_ptr < init_call_array[i].end; fun_ptr++) {
			fun = *fun_ptr;
			if (fun)
				fun();
		}
	}
}

int start_kernel()
{
	boot_fb_init();
	graphic_con_init();
	//fb_test();
	printk("Oscar Kernel init start...\n");
	printk("Build:%s %s\n", __DATE__, __TIME__);
	printk("==========================================\n");

	extern void mminfo_print();
	mminfo_print();
	do_init_call();
	//mm_enumate();
	
	return 0;
}
