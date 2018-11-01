#include <vmx.h>
#include <cpuid.h>
#include <paging.h>
#include <msr.h>
#include <cr.h>
#include <kernel.h>
#include <string.h>
#include <mm.h>
#include <math.h>
#include <list.h>

int vmx_enable()
{
	u32 regs[4] = {0};
	u64 msr;
	u64 vmx_msr;
	cpuid(0x00000001, 0x00000000, &regs[0]);
	if (regs[2] & CPUID_FEATURES_ECX_VMX == 0) {
		printk("VMX is unsupported on this CPU!.\n");
		return -1;
	}

	msr = rdmsr(MSR_IA32_FEATURE_CONTROL);
	vmx_msr = FEATURE_CONTROL_VMXON_ENABLED_OUTSIDE_SMX | FEATURE_CONTROL_LOCKED;
	if (msr & vmx_msr != vmx_msr) {
		printk("VMX is supported, but BIOS disabled VMX.\n");
		return -2;
	}

	cr0_set_bits(CR0_NE | CR0_MP);
	cr4_set_bits(CR4_VMXE);
	return 0;
}

int vmx_disable()
{
	cr4_clear_bits(CR4_VMXE);
	return 0;
}

struct vmx_vcpu *vmx_preinit()
{
	int ret;
	struct vmx_vcpu *vcpu_ptr = bootmem_alloc(sizeof(*vcpu_ptr));
	memset(vcpu_ptr, 0, sizeof(*vcpu_ptr));
	INIT_LIST_HEAD(&vcpu_ptr->guest_memory_list);
	vcpu_ptr->vmxon_region = bootmem_alloc(0x1000);
	memset(vcpu_ptr->vmxon_region, 0, 0x1000);
	vcpu_ptr->vmxon_region_phys = VIRT2PHYS(vcpu_ptr->vmxon_region);
	vcpu_ptr->vmcs = bootmem_alloc(0x1000);
	vcpu_ptr->vmcs_phys = VIRT2PHYS(vcpu_ptr->vmcs);
	memset(vcpu_ptr->vmcs, 0, 0x1000);

	ret = vmx_enable();
	if (ret) {
		return NULL;
	}

	vcpu_ptr->vmxon_region->revision_id = rdmsr(MSR_IA32_VMX_BASIC);
	vcpu_ptr->vmcs->revision_id = rdmsr(MSR_IA32_VMX_BASIC);
	vmx_on(vcpu_ptr->vmxon_region_phys);
	vmclear(vcpu_ptr->vmcs_phys);
	vmptr_load(vcpu_ptr->vmcs_phys);
	return vcpu_ptr;
}

int vmx_init(struct vmx_vcpu * vcpu)
{
	vcpu->vapic_page = bootmem_alloc(0x1000);
	memset(vcpu->vapic_page, 0, 0x1000);
	vcpu->io_bitmap_a = bootmem_alloc(0x1000);
	memset(vcpu->io_bitmap_a, 0, 0x1000);
	vcpu->io_bitmap_b = bootmem_alloc(0x1000);
	memset(vcpu->io_bitmap_b, 0, 0x1000);
	vcpu->msr_bitmap = bootmem_alloc(0x1000);
	memset(vcpu->msr_bitmap, 0, 0x1000);
	vcpu->eptp_base = bootmem_alloc(0x1000);
	memset(vcpu->eptp_base, 0, 0x1000);
	vcpu->host_state.msr = bootmem_alloc(0x1000);
	memset(vcpu->host_state.msr, 0, 0x1000);
	vcpu->guest_state.msr = bootmem_alloc(0x1000);
	memset(vcpu->guest_state.msr, 0, 0x1000);

	return 0;
}

int vmx_set_ctrl_state(struct vmx_vcpu *vcpu)
{
	u64 pin_based_vm_exec_ctrl;
	u64 cpu_based_vm_exec_ctrl;
	u64 cpu_based_vm_exec_ctrl2;
	u64 vm_entry_ctrl;
	u64 vm_exit_ctrl;

	u32 pin_based_allow1_mask;
	u32 pin_based_allow0_mask;
	u32 cpu_based_allow1_mask;
	u32 cpu_based_allow0_mask;
	u32 cpu_based2_allow1_mask;
	u32 cpu_based2_allow0_mask;
	u32 vm_entry_allow1_mask;
	u32 vm_entry_allow0_mask;
	u32 vm_exit_allow1_mask;
	u32 vm_exit_allow0_mask;

	vmcs_write(IO_BITMAP_A, VIRT2PHYS(vcpu->io_bitmap_a));
	vmcs_write(IO_BITMAP_B, VIRT2PHYS(vcpu->io_bitmap_b));
	vmcs_write(MSR_BITMAP, VIRT2PHYS(vcpu->msr_bitmap));
	vmcs_write(VM_ENTRY_MSR_LOAD_ADDR, VIRT2PHYS(vcpu->guest_state.msr));
	vmcs_write(VM_EXIT_MSR_STORE_ADDR, VIRT2PHYS(vcpu->guest_state.msr));
	vmcs_write(VM_EXIT_MSR_LOAD_ADDR, VIRT2PHYS(vcpu->host_state.msr));

	vmcs_write(VIRTUAL_APIC_PAGE_ADDR, VIRT2PHYS(vcpu->vapic_page));
	vmcs_write(EPT_POINTER, VIRT2PHYS(vcpu->eptp_base) | 0x5e);

	pin_based_vm_exec_ctrl = rdmsr(MSR_IA32_VMX_PINBASED_CTLS);
	cpu_based_vm_exec_ctrl = rdmsr(MSR_IA32_VMX_PROCBASED_CTLS);
	cpu_based_vm_exec_ctrl2 = rdmsr(MSR_IA32_VMX_PROCBASED_CTLS2);
	vm_entry_ctrl = rdmsr(MSR_IA32_VMX_ENTRY_CTLS);
	vm_exit_ctrl = rdmsr(MSR_IA32_VMX_EXIT_CTLS);

	pin_based_allow1_mask = pin_based_vm_exec_ctrl >> 32;
	pin_based_allow0_mask = pin_based_vm_exec_ctrl;
	cpu_based_allow1_mask = cpu_based_vm_exec_ctrl >> 32;
	cpu_based_allow0_mask = cpu_based_vm_exec_ctrl;
	cpu_based2_allow1_mask = cpu_based_vm_exec_ctrl2 >> 32;
	cpu_based2_allow0_mask = cpu_based_vm_exec_ctrl2;
	vm_entry_allow1_mask = vm_entry_ctrl >> 32;
	vm_entry_allow0_mask = vm_entry_ctrl;
	vm_exit_allow1_mask = vm_exit_ctrl >> 32;
	vm_exit_allow0_mask = vm_exit_ctrl;

	pin_based_vm_exec_ctrl =  PIN_BASED_ALWAYSON_WITHOUT_TRUE_MSR
		| PIN_BASED_EXT_INTR_MASK
		| PIN_BASED_POSTED_INTR;
	if ((pin_based_vm_exec_ctrl | pin_based_allow1_mask) != pin_based_allow1_mask) {
		printk("Warning:setting pin_based_vm_exec_control:%x unsupported.\n", 
			(pin_based_vm_exec_ctrl & pin_based_allow1_mask) ^ pin_based_vm_exec_ctrl);
		pin_based_vm_exec_ctrl |= pin_based_allow0_mask;
		pin_based_vm_exec_ctrl &= pin_based_allow1_mask;
	}
	vmcs_write(PIN_BASED_VM_EXEC_CONTROL, pin_based_vm_exec_ctrl);
	cpu_based_vm_exec_ctrl = CPU_BASED_FIXED_ONES
		| CPU_BASED_ACTIVATE_SECONDARY_CONTROLS
		| CPU_BASED_USE_IO_BITMAPS
		| CPU_BASED_USE_MSR_BITMAPS
		| CPU_BASED_TPR_SHADOW
		//| CPU_BASED_MONITOR_TRAP_FLAG
		;
	if ((cpu_based_vm_exec_ctrl | cpu_based_allow1_mask) != cpu_based_allow1_mask) {
		printk("Warning:setting cpu_based_vm_exec_control:%x unsupported.\n", 
			(cpu_based_vm_exec_ctrl & cpu_based_allow1_mask) ^ cpu_based_vm_exec_ctrl);
		cpu_based_vm_exec_ctrl |= cpu_based_allow0_mask;
		cpu_based_vm_exec_ctrl &= cpu_based_allow1_mask;
	}
	vmcs_write(CPU_BASED_VM_EXEC_CONTROL, cpu_based_vm_exec_ctrl);

	cpu_based_vm_exec_ctrl2 = SECONDARY_EXEC_UNRESTRICTED_GUEST
		| SECONDARY_EXEC_ENABLE_EPT
		| SECONDARY_EXEC_VIRTUALIZE_APIC_ACCESSES
		| SECONDARY_EXEC_VIRTUAL_INTR_DELIVERY
		| SECONDARY_EXEC_APIC_REGISTER_VIRT
		;
	if ((cpu_based_vm_exec_ctrl2 | cpu_based2_allow1_mask) != cpu_based2_allow1_mask) {
		printk("Warning:setting secondary_vm_exec_control:%x unsupported.\n", 
			(cpu_based_vm_exec_ctrl2 & cpu_based2_allow1_mask) ^ cpu_based_vm_exec_ctrl2);
		cpu_based_vm_exec_ctrl2 |= cpu_based2_allow0_mask;
		cpu_based_vm_exec_ctrl2 &= cpu_based2_allow1_mask;
	}
	vmcs_write(SECONDARY_VM_EXEC_CONTROL, cpu_based_vm_exec_ctrl2);

	vm_entry_ctrl = VM_ENTRY_ALWAYSON_WITHOUT_TRUE_MSR
		| VM_ENTRY_LOAD_IA32_PERF_GLOBAL_CTRL
		| VM_ENTRY_LOAD_IA32_EFER;
	if ((vm_entry_ctrl | vm_entry_allow1_mask) != vm_entry_allow1_mask) {
		printk("Warning:setting vm_entry_controls:%x unsupported.\n", 
			(vm_entry_ctrl & vm_entry_allow1_mask) ^ vm_entry_ctrl);
		vm_entry_ctrl |= vm_entry_allow0_mask;
		vm_entry_ctrl &= vm_entry_allow1_mask;
	}
	vmcs_write(VM_ENTRY_CONTROLS, vm_entry_ctrl);

	vm_exit_ctrl = VM_EXIT_ALWAYSON_WITHOUT_TRUE_MSR
		| VM_EXIT_SAVE_IA32_EFER
		| VM_EXIT_LOAD_IA32_EFER
		| VM_EXIT_ACK_INTR_ON_EXIT
		| VM_EXIT_HOST_ADDR_SPACE_SIZE
		;
	if ((vm_exit_ctrl | vm_exit_allow1_mask) != vm_exit_allow1_mask) {
		printk("Warning:setting vm_exit_controls:%x unsupported.\n", 
			(vm_exit_ctrl & vm_exit_allow1_mask) ^ vm_exit_ctrl);
		vm_exit_ctrl |= vm_exit_allow0_mask;
		vm_exit_ctrl &= vm_exit_allow1_mask;
	}
	vmcs_write(VM_EXIT_CONTROLS, vm_exit_ctrl);

	return 0;
}

int vmx_set_host_state(struct vmx_vcpu *vcpu)
{
	struct gdtr host_gdtr;
	struct idtr host_idtr;
	u64 host_rsp;
	extern void vm_exit();

	vmcs_write(HOST_CR0, read_cr0());
	vmcs_write(HOST_CR3, read_cr3());
	vmcs_write(HOST_CR4, read_cr4());

	sgdt(&host_gdtr);
	sidt(&host_idtr);

	vmcs_write(HOST_GDTR_BASE, host_gdtr.base);
	vmcs_write(HOST_IDTR_BASE, host_idtr.base);

	vmcs_write(HOST_CS_SELECTOR, save_cs());
	vmcs_write(HOST_DS_SELECTOR, save_ds());
	vmcs_write(HOST_ES_SELECTOR, save_es());
	vmcs_write(HOST_FS_SELECTOR, save_fs());
	vmcs_write(HOST_GS_SELECTOR, save_gs());
	vmcs_write(HOST_SS_SELECTOR, save_ss());
	vmcs_write(HOST_TR_SELECTOR, str());
	vmcs_write(HOST_GS_BASE, rdmsr(MSR_GS_BASE));
	vmcs_write(HOST_FS_BASE, rdmsr(MSR_FS_BASE));
	vmcs_write(HOST_IA32_SYSENTER_CS, rdmsr(MSR_IA32_SYSENTER_CS));
	vmcs_write(HOST_IA32_SYSENTER_ESP, rdmsr(MSR_IA32_SYSENTER_ESP));
	vmcs_write(HOST_IA32_SYSENTER_EIP, rdmsr(MSR_IA32_SYSENTER_EIP));
	vmcs_write(HOST_IA32_EFER, rdmsr(MSR_EFER));
	vmcs_write(HOST_IA32_PAT, rdmsr(MSR_IA32_CR_PAT));
	vmcs_write(HOST_IA32_PERF_GLOBAL_CTRL, rdmsr(MSR_IA32_PERF_CTL));

	asm volatile("mov %%rsp, %0\n\t"
		:"=r"(host_rsp):);
	vmcs_write(HOST_RSP, host_rsp + 8);
	vmcs_write(HOST_RIP, (u64)vm_exit);

	return 0;
}

int vmx_set_guest_state(struct vmx_vcpu *vcpu)
{
	u64 cr0, cr4;
	cr0 = vcpu->guest_state.cr0 | rdmsr(MSR_IA32_VMX_CR0_FIXED0) & rdmsr(MSR_IA32_VMX_CR0_FIXED1);

	if (vmcs_read(SECONDARY_VM_EXEC_CONTROL) & SECONDARY_EXEC_UNRESTRICTED_GUEST != 0) {
		cr0 &= 0x7ffffffe;
	}

	cr4 = rdmsr(MSR_IA32_VMX_CR4_FIXED0) & rdmsr(MSR_IA32_VMX_CR4_FIXED1);
	vmcs_write(GUEST_CR0, cr0);
	vmcs_write(GUEST_CR3, vcpu->guest_state.cr3);
	vmcs_write(GUEST_CR4, cr4);

	vmcs_write(GUEST_RIP, vcpu->guest_state.rip);
	vmcs_write(GUEST_RFLAGS, vcpu->guest_state.rflags);
	
	vmcs_write(GUEST_RSP, vcpu->guest_state.gr_regs.rsp);
	vmcs_write(GUEST_DR7, 0);
	vmcs_write(GUEST_IA32_DEBUGCTL, 0);

	vmcs_write(GUEST_CS_SELECTOR, vcpu->guest_state.cs.selector);
	vmcs_write(GUEST_CS_BASE, vcpu->guest_state.cs.base);
	vmcs_write(GUEST_CS_LIMIT, vcpu->guest_state.cs.limit);
	vmcs_write(GUEST_CS_AR_BYTES, vcpu->guest_state.cs.ar_bytes);

	vmcs_write(GUEST_DS_SELECTOR, vcpu->guest_state.ds.selector);
	vmcs_write(GUEST_DS_BASE, vcpu->guest_state.ds.base);
	vmcs_write(GUEST_DS_LIMIT, vcpu->guest_state.ds.limit);
	vmcs_write(GUEST_DS_AR_BYTES, vcpu->guest_state.ds.ar_bytes);

	vmcs_write(GUEST_ES_SELECTOR, vcpu->guest_state.es.selector);
	vmcs_write(GUEST_ES_BASE, vcpu->guest_state.es.base);
	vmcs_write(GUEST_ES_LIMIT, vcpu->guest_state.es.limit);
	vmcs_write(GUEST_ES_AR_BYTES, vcpu->guest_state.es.ar_bytes);

	vmcs_write(GUEST_FS_SELECTOR, vcpu->guest_state.fs.selector);
	vmcs_write(GUEST_FS_BASE, vcpu->guest_state.fs.base);
	vmcs_write(GUEST_FS_LIMIT, vcpu->guest_state.fs.limit);
	vmcs_write(GUEST_FS_AR_BYTES, vcpu->guest_state.fs.ar_bytes);

	vmcs_write(GUEST_GS_SELECTOR, vcpu->guest_state.gs.selector);
	vmcs_write(GUEST_GS_BASE, vcpu->guest_state.gs.base);
	vmcs_write(GUEST_GS_LIMIT, vcpu->guest_state.gs.limit);
	vmcs_write(GUEST_GS_AR_BYTES, vcpu->guest_state.gs.ar_bytes);

	vmcs_write(GUEST_SS_SELECTOR, vcpu->guest_state.ss.selector);
	vmcs_write(GUEST_SS_BASE, vcpu->guest_state.ss.base);
	vmcs_write(GUEST_SS_LIMIT, vcpu->guest_state.ss.limit);
	vmcs_write(GUEST_SS_AR_BYTES, vcpu->guest_state.ss.ar_bytes);	

	vmcs_write(GUEST_TR_SELECTOR, vcpu->guest_state.tr.selector);
	vmcs_write(GUEST_TR_BASE, vcpu->guest_state.tr.base);
	vmcs_write(GUEST_TR_LIMIT, vcpu->guest_state.tr.limit);
	vmcs_write(GUEST_TR_AR_BYTES, vcpu->guest_state.tr.ar_bytes);
	
	vmcs_write(GUEST_LDTR_SELECTOR, vcpu->guest_state.ldtr.selector);
	vmcs_write(GUEST_LDTR_BASE, vcpu->guest_state.ldtr.base);
	vmcs_write(GUEST_LDTR_LIMIT, vcpu->guest_state.ldtr.limit);
	vmcs_write(GUEST_LDTR_AR_BYTES, vcpu->guest_state.ldtr.ar_bytes);

	vmcs_write(GUEST_GDTR_BASE, vcpu->guest_state.gdtr.base);
	vmcs_write(GUEST_GDTR_LIMIT, vcpu->guest_state.gdtr.limit);
	vmcs_write(GUEST_IDTR_BASE, vcpu->guest_state.idtr.base);
	vmcs_write(GUEST_IDTR_LIMIT, vcpu->guest_state.idtr.limit);

	vmcs_write(GUEST_SYSENTER_ESP, 0);
	vmcs_write(GUEST_SYSENTER_EIP, 0);
	vmcs_write(GUEST_SYSENTER_CS, 0);
	vmcs_write(GUEST_IA32_DEBUGCTL, 0);
	vmcs_write(GUEST_IA32_PERF_GLOBAL_CTRL, 0);
	vmcs_write(GUEST_IA32_EFER, 0);
	vmcs_write(GUEST_IA32_PAT, 0);
	
	vmcs_write(GUEST_ACTIVITY_STATE, 0);

	vmcs_write(GUEST_PDPTR0, vcpu->guest_state.pdpte0);
	vmcs_write(GUEST_PDPTR1, vcpu->guest_state.pdpte1);
	vmcs_write(GUEST_PDPTR2, vcpu->guest_state.pdpte2);
	vmcs_write(GUEST_PDPTR3, vcpu->guest_state.pdpte3);

	vmcs_write(GUEST_INTERRUPTIBILITY_INFO, 0);
	vmcs_write(VM_ENTRY_INTR_INFO_FIELD, 0);
	vmcs_write(GUEST_PENDING_DBG_EXCEPTIONS, 0);
	vmcs_write(GUEST_BNDCFGS, 0);

	vmcs_write(VMCS_LINK_POINTER, 0xffffffffffffffff);

	vmcs_write(APIC_ACCESS_ADDR, 0xfee00000);
	return 0;
}

int vmx_run(struct vmx_vcpu *vcpu)
{	
	return vm_launch(&vcpu->host_state.gr_regs, &vcpu->guest_state.gr_regs);
}

int ept_map_page(struct vmx_vcpu *vcpu, u64 gpa, u64 hpa, u64 page_size, u64 attribute)
{
	//printk("ept map:gpa = %x hpa = %x page_size = %x\n", gpa, hpa, page_size);
	u64 index1, index2, index3, index4, offset_1g, offset_2m, offset_4k;
	u64 *pml4t, *pdpt, *pdt, *pt;
	u64 pml4e, pdpte, pde, pte;
	u64 pml4e_attr, pdpte_attr, pde_attr, pte_attr;
	u64 *virt;
	pml4e_attr = EPT_PML4E_READ | EPT_PML4E_WRITE | EPT_PML4E_EXECUTE | EPT_PML4E_ACCESS_FLAG;
	pdpte_attr = EPT_PDPTE_READ | EPT_PDPTE_WRITE | EPT_PDPTE_EXECUTE | EPT_PDPTE_ACCESS_FLAG;
	pde_attr = EPT_PDE_READ | EPT_PDE_WRITE | EPT_PDE_EXECUTE | EPT_PDE_ACCESS_FLAG;
	pte_attr = EPT_PTE_READ | EPT_PTE_WRITE | EPT_PTE_ACCESS_FLAG;
	pml4t = (u64 *)vcpu->eptp_base;

	index1 = (gpa >> 39) & 0x1ff;
	index2 = (gpa >> 30) & 0x1ff;
	index3 = (gpa >> 21) & 0x1ff;
	index4 = (gpa >> 12) & 0x1ff;
	offset_1g = gpa & ((1 << 30) - 1);
	offset_2m = gpa & ((1 << 21) - 1);
	offset_4k = gpa & ((1 << 12) - 1);

	pml4e = pml4t[index1];
	if (pml4e == 0) {
		virt = bootmem_alloc(0x1000);
		memset(virt, 0, 0x1000);
		pml4t[index1] = VIRT2PHYS(virt) | pml4e_attr;
	}

	pdpt = (u64 *)(PHYS2VIRT(pml4t[index1] & ~0xfffULL));
	pdpte = pdpt[index2];

	if (page_size == 0x40000000) {
		pdpt[index2] = hpa | attribute | EPT_PDPTE_1GB_PAGE;
		return 0;
	}

	if (pdpte == 0) {
		virt = bootmem_alloc(0x1000);
		memset(virt, 0, 0x1000);
		pdpt[index2] = VIRT2PHYS(virt) | pdpte_attr;
	}

	

	pdt = (u64 *)(PHYS2VIRT(pdpt[index2] & ~0xfffULL));
	pde = pdt[index3];

	if (page_size == 0x200000) {
		pdt[index3] = hpa | attribute | EPT_PDE_2MB_PAGE;
		return 0;
	}

	if (pde == 0) {
		virt = bootmem_alloc(0x1000);
		memset(virt, 0, 0x1000);
		pdt[index3] = VIRT2PHYS(virt) | pde_attr;
	}

	pt = (u64 *)(PHYS2VIRT(pdt[index3] & ~0xfffULL));
	pte = pdt[index4];

	if (page_size == 0x1000) {
		pt[index4] = hpa | attribute;
		return 0;
	}
	
	return 0;
}

int map_guest_memory(struct vmx_vcpu *vcpu, u64 gpa, u64 hpa, u64 len, u64 attr)
{
	u64 page_size;
	s64 remain_len = len;
	while (remain_len > 0) {
		//if (remain_len >= 0x40000000) {
		//	ept_map_page(vcpu, gpa, hpa, 0x40000000, attr);
		//	remain_len -= 0x40000000;
		//	gpa += 0x40000000;
		//	hpa += 0x40000000;
		//} else if (remain_len >= 0x200000) {
		//	ept_map_page(vcpu, gpa, hpa, 0x200000, attr);
		//	remain_len -= 0x200000;
		//	gpa += 0x200000;
		//	hpa += 0x200000;
		//} else {
			ept_map_page(vcpu, gpa, hpa, 0x1000, attr);
			remain_len -= 0x1000;
			gpa += 0x1000;
			hpa += 0x1000;
		//}
	}
}

int alloc_guest_memory(struct vmx_vcpu *vcpu, u64 gpa, u64 size)
{
	u64 hpa;
	u64 *virt;
	struct guest_memory_zone *zone = bootmem_alloc(sizeof(*zone));
	memset(zone, 0, sizeof(*zone));
	size = roundup(size, 0x1000);
	virt = bootmem_alloc(size);
	hpa = VIRT2PHYS(virt);
	zone->hpa = hpa;
	zone->page_nr = size / 0x1000;
	zone->gpa = gpa;
	printk("hpa = %x\n", hpa);
	map_guest_memory(vcpu, gpa, hpa, size, EPT_PTE_READ | EPT_PTE_WRITE | EPT_PTE_EXECUTE | EPT_PTE_CACHE_WB);
	list_add_tail(&zone->list, &vcpu->guest_memory_list);
	return 0;
}

int read_guest_memory_gpa(struct vmx_vcpu *vcpu, u64 gpa, u64 size, void *buffer)
{
	struct guest_memory_zone *zone; 
	list_for_each_entry(zone, &vcpu->guest_memory_list, list) {
		if (gpa >= zone->gpa && gpa < zone->gpa + zone->page_nr * 0x1000) {
			memcpy(buffer, (void *)PHYS2VIRT(zone->hpa + gpa - zone->gpa), size);
			return 0;
		}
	}
	return -1;
}

int write_guest_memory_gpa(struct vmx_vcpu *vcpu, u64 gpa, u64 size, void *buffer)
{
	struct guest_memory_zone *zone; 
	list_for_each_entry(zone, &vcpu->guest_memory_list, list) {
		if (gpa >= zone->gpa && gpa < zone->gpa + zone->page_nr * 0x1000) {
			memcpy((void *)PHYS2VIRT(zone->hpa + gpa - zone->gpa), buffer, size);
			return 0;
		}
	}
	return -1;
}



int vm_exit_handler(struct vmx_vcpu *vcpu)
{
	u32 exit_reason = vmcs_read(VM_EXIT_REASON);
	u32 instruction_error = vmcs_read(VM_INSTRUCTION_ERROR);
	printk("vm_exit reason:%d\n", exit_reason);
	printk("vm instruction error:%d\n", instruction_error);
	printk("Exit qualification:%x\n", vmcs_read(EXIT_QUALIFICATION));
	printk("Guest RIP:%x\n", vmcs_read(GUEST_RIP));
	printk("GPA:%x\n", vmcs_read(GUEST_PHYSICAL_ADDRESS));
}

void vmx_set_bist_state(struct vmx_vcpu *vcpu)
{
	vcpu->guest_state.cs.selector = 0;
	vcpu->guest_state.cs.base = 0;
	vcpu->guest_state.cs.limit = 0xffff;
	vcpu->guest_state.cs.ar_bytes = VMX_AR_P_MASK 
		| VMX_AR_TYPE_READABLE_CODE_MASK 
		| VMX_AR_TYPE_CODE_MASK 
		| VMX_AR_TYPE_ACCESSES_MASK 
		| VMX_AR_S_MASK;

	vcpu->guest_state.ds.selector = 0;
	vcpu->guest_state.ds.base = 0;
	vcpu->guest_state.ds.limit = 0xffff;
	vcpu->guest_state.ds.ar_bytes = VMX_AR_P_MASK
		| VMX_AR_TYPE_WRITABLE_DATA_MASK
		| VMX_AR_TYPE_ACCESSES_MASK
		| VMX_AR_S_MASK;

	vcpu->guest_state.es.selector = 0;
	vcpu->guest_state.es.base = 0;
	vcpu->guest_state.es.limit = 0xffff;
	vcpu->guest_state.es.ar_bytes = VMX_AR_P_MASK
		| VMX_AR_TYPE_WRITABLE_DATA_MASK
		| VMX_AR_TYPE_ACCESSES_MASK
		| VMX_AR_S_MASK;

	vcpu->guest_state.fs.selector = 0;
	vcpu->guest_state.fs.base = 0;
	vcpu->guest_state.fs.limit = 0xffff;
	vcpu->guest_state.fs.ar_bytes = VMX_AR_P_MASK
		| VMX_AR_TYPE_WRITABLE_DATA_MASK
		| VMX_AR_TYPE_ACCESSES_MASK
		| VMX_AR_S_MASK;

	vcpu->guest_state.gs.selector = 0;
	vcpu->guest_state.gs.base = 0;
	vcpu->guest_state.gs.limit = 0xffff;
	vcpu->guest_state.gs.ar_bytes = VMX_AR_P_MASK
		| VMX_AR_TYPE_WRITABLE_DATA_MASK
		| VMX_AR_TYPE_ACCESSES_MASK
		| VMX_AR_S_MASK;

	vcpu->guest_state.ss.selector = 0;
	vcpu->guest_state.ss.base = 0;
	vcpu->guest_state.ss.limit = 0xffff;
	vcpu->guest_state.ss.ar_bytes = VMX_AR_P_MASK
		| VMX_AR_TYPE_WRITABLE_DATA_MASK
		| VMX_AR_TYPE_ACCESSES_MASK
		| VMX_AR_S_MASK;

	vcpu->guest_state.tr.selector = 0;
	vcpu->guest_state.tr.base = 0;
	vcpu->guest_state.tr.limit = 0xffff;
	vcpu->guest_state.tr.ar_bytes = VMX_AR_P_MASK | VMX_AR_TYPE_BUSY_16_TSS;

	vcpu->guest_state.ldtr.selector = 0;
	vcpu->guest_state.ldtr.base = 0;
	vcpu->guest_state.ldtr.limit = 0xffff;
	vcpu->guest_state.ldtr.ar_bytes = VMX_AR_UNUSABLE_MASK;

	vcpu->guest_state.gdtr.base = 0;
	vcpu->guest_state.gdtr.limit = 0xffff;
	vcpu->guest_state.idtr.base = 0;
	vcpu->guest_state.idtr.limit = 0xffff;
	
	vcpu->guest_state.cr0 = 0;
	vcpu->guest_state.cr3 = 0;
	vcpu->guest_state.cr4 = 0;

	vcpu->guest_state.pdpte0 = 0;
	vcpu->guest_state.pdpte1 = 0;
	vcpu->guest_state.pdpte2 = 0;
	vcpu->guest_state.pdpte3 = 0;

	vcpu->guest_state.rip = 0;
	vcpu->guest_state.rflags = BIT1;
	vcpu->guest_state.gr_regs.rsp = 0;
}

void vm_init_test()
{
	int ret;
	u8 buf[4];
	extern u64 test_guest, test_guest_end;
	struct vmx_vcpu *vcpu = vmx_preinit();
	vmx_init(vcpu);
	vmx_set_ctrl_state(vcpu);
	vmx_set_host_state(vcpu);
	vmx_set_bist_state(vcpu);
	alloc_guest_memory(vcpu, 0, 0x1000000);
	//alloc_guest_memory(vcpu, 0xff000000, 0x1000000);
	write_guest_memory_gpa(vcpu, 0, (u64)&test_guest_end - (u64)&test_guest, &test_guest);
	u8 *ptr = (u8 *)&test_guest;
	printk("%02x %02x %02x %02x\n", ptr[0], ptr[1], ptr[2], ptr[3]);
	read_guest_memory_gpa(vcpu, 0, 4, buf);
	printk("%02x %02x %02x %02x\n", buf[0], buf[1], buf[2], buf[3]);
	vmx_set_guest_state(vcpu);
	ret = vmx_run(vcpu);
	printk("vm-exit.ret = %d\n", ret);
	vm_exit_handler(vcpu);
}