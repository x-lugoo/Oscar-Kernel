#ifndef _VMX_H
#define _VMX_H

/* Most macros are copied from linux-kernel. */

/*
 * Address types:
 *
 *  gva - guest virtual address
 *  gpa - guest physical address
 *  gfn - guest frame number
 *  hva - host virtual address
 *  hpa - host physical address
 *  hfn - host frame number
 */

#include <types.h>
#include <kernel.h>
typedef unsigned long  gva_t;
typedef u64            gpa_t;
typedef u64            gfn_t;

typedef unsigned long  hva_t;
typedef u64            hpa_t;
typedef u64            hfn_t;

/*
 * vmx.h: VMX Architecture related definitions
 * Copyright (c) 2004, Intel Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place - Suite 330, Boston, MA 02111-1307 USA.
 *
 * A few random additions are:
 * Copyright (C) 2006 Qumranet
 *    Avi Kivity <avi@qumranet.com>
 *    Yaniv Kamay <yaniv@qumranet.com>
 *
 */
#include <types.h>
#include <x86_cpu.h>
#include <msr.h>
#include <segment.h>
#include <cr.h>
#include <fpu.h>

/*
 * Definitions of Primary Processor-Based VM-Execution Controls.
 */
#define CPU_BASED_VIRTUAL_INTR_PENDING          0x00000004
#define CPU_BASED_USE_TSC_OFFSETING             0x00000008
#define CPU_BASED_HLT_EXITING                   0x00000080
#define CPU_BASED_INVLPG_EXITING                0x00000200
#define CPU_BASED_MWAIT_EXITING                 0x00000400
#define CPU_BASED_RDPMC_EXITING                 0x00000800
#define CPU_BASED_RDTSC_EXITING                 0x00001000
#define CPU_BASED_CR3_LOAD_EXITING		0x00008000
#define CPU_BASED_CR3_STORE_EXITING		0x00010000
#define CPU_BASED_CR8_LOAD_EXITING              0x00080000
#define CPU_BASED_CR8_STORE_EXITING             0x00100000
#define CPU_BASED_TPR_SHADOW                    0x00200000
#define CPU_BASED_VIRTUAL_NMI_PENDING		0x00400000
#define CPU_BASED_MOV_DR_EXITING                0x00800000
#define CPU_BASED_UNCOND_IO_EXITING             0x01000000
#define CPU_BASED_USE_IO_BITMAPS                0x02000000
#define CPU_BASED_MONITOR_TRAP_FLAG             0x08000000
#define CPU_BASED_USE_MSR_BITMAPS               0x10000000
#define CPU_BASED_MONITOR_EXITING               0x20000000
#define CPU_BASED_PAUSE_EXITING                 0x40000000
#define CPU_BASED_ACTIVATE_SECONDARY_CONTROLS   0x80000000

#define CPU_BASED_ALWAYSON_WITHOUT_TRUE_MSR	0x0401e172
#define CPU_BASED_FIXED_ONES					0x04006172
#define CPU_BASED_FIXED_ZEROS					0xfff9fffe


/*
 * Definitions of Secondary Processor-Based VM-Execution Controls.
 */
#define SECONDARY_EXEC_VIRTUALIZE_APIC_ACCESSES 0x00000001
#define SECONDARY_EXEC_ENABLE_EPT               0x00000002
#define SECONDARY_EXEC_DESC			0x00000004
#define SECONDARY_EXEC_RDTSCP			0x00000008
#define SECONDARY_EXEC_VIRTUALIZE_X2APIC_MODE   0x00000010
#define SECONDARY_EXEC_ENABLE_VPID              0x00000020
#define SECONDARY_EXEC_WBINVD_EXITING		0x00000040
#define SECONDARY_EXEC_UNRESTRICTED_GUEST	0x00000080
#define SECONDARY_EXEC_APIC_REGISTER_VIRT       0x00000100
#define SECONDARY_EXEC_VIRTUAL_INTR_DELIVERY    0x00000200
#define SECONDARY_EXEC_PAUSE_LOOP_EXITING	0x00000400
#define SECONDARY_EXEC_RDRAND_EXITING		0x00000800
#define SECONDARY_EXEC_ENABLE_INVPCID		0x00001000
#define SECONDARY_EXEC_ENABLE_VMFUNC            0x00002000
#define SECONDARY_EXEC_SHADOW_VMCS              0x00004000
#define SECONDARY_EXEC_RDSEED_EXITING		0x00010000
#define SECONDARY_EXEC_ENABLE_PML               0x00020000
#define SECONDARY_EXEC_XSAVES			0x00100000
#define SECONDARY_EXEC_TSC_SCALING              0x02000000

#define PIN_BASED_EXT_INTR_MASK                 0x00000001
#define PIN_BASED_NMI_EXITING                   0x00000008
#define PIN_BASED_VIRTUAL_NMIS                  0x00000020
#define PIN_BASED_VMX_PREEMPTION_TIMER          0x00000040
#define PIN_BASED_POSTED_INTR                   0x00000080

#define PIN_BASED_ALWAYSON_WITHOUT_TRUE_MSR	0x00000016

#define VM_EXIT_SAVE_DEBUG_CONTROLS             0x00000004
#define VM_EXIT_HOST_ADDR_SPACE_SIZE            0x00000200
#define VM_EXIT_LOAD_IA32_PERF_GLOBAL_CTRL      0x00001000
#define VM_EXIT_ACK_INTR_ON_EXIT                0x00008000
#define VM_EXIT_SAVE_IA32_PAT			0x00040000
#define VM_EXIT_LOAD_IA32_PAT			0x00080000
#define VM_EXIT_SAVE_IA32_EFER                  0x00100000
#define VM_EXIT_LOAD_IA32_EFER                  0x00200000
#define VM_EXIT_SAVE_VMX_PREEMPTION_TIMER       0x00400000
#define VM_EXIT_CLEAR_BNDCFGS                   0x00800000

#define VM_EXIT_ALWAYSON_WITHOUT_TRUE_MSR	0x00036dff

#define VM_ENTRY_LOAD_DEBUG_CONTROLS            0x00000004
#define VM_ENTRY_IA32E_MODE                     0x00000200
#define VM_ENTRY_SMM                            0x00000400
#define VM_ENTRY_DEACT_DUAL_MONITOR             0x00000800
#define VM_ENTRY_LOAD_IA32_PERF_GLOBAL_CTRL     0x00002000
#define VM_ENTRY_LOAD_IA32_PAT			0x00004000
#define VM_ENTRY_LOAD_IA32_EFER                 0x00008000
#define VM_ENTRY_LOAD_BNDCFGS                   0x00010000

#define VM_ENTRY_ALWAYSON_WITHOUT_TRUE_MSR	0x000011ff

#define VMX_MISC_PREEMPTION_TIMER_RATE_MASK	0x0000001f
#define VMX_MISC_SAVE_EFER_LMA			0x00000020
#define VMX_MISC_ACTIVITY_HLT			0x00000040

/* VMFUNC functions */
#define VMX_VMFUNC_EPTP_SWITCHING               0x00000001
#define VMFUNC_EPTP_ENTRIES  512

/* VMCS Encodings */
enum vmcs_field {
	VIRTUAL_PROCESSOR_ID            = 0x00000000,
	POSTED_INTR_NV                  = 0x00000002,
	EPTP_INDEX                      = 0x00000004,
	GUEST_ES_SELECTOR               = 0x00000800,
	GUEST_CS_SELECTOR               = 0x00000802,
	GUEST_SS_SELECTOR               = 0x00000804,
	GUEST_DS_SELECTOR               = 0x00000806,
	GUEST_FS_SELECTOR               = 0x00000808,
	GUEST_GS_SELECTOR               = 0x0000080a,
	GUEST_LDTR_SELECTOR             = 0x0000080c,
	GUEST_TR_SELECTOR               = 0x0000080e,
	GUEST_INTR_STATUS               = 0x00000810,
	GUEST_PML_INDEX			= 0x00000812,
	HOST_ES_SELECTOR                = 0x00000c00,
	HOST_CS_SELECTOR                = 0x00000c02,
	HOST_SS_SELECTOR                = 0x00000c04,
	HOST_DS_SELECTOR                = 0x00000c06,
	HOST_FS_SELECTOR                = 0x00000c08,
	HOST_GS_SELECTOR                = 0x00000c0a,
	HOST_TR_SELECTOR                = 0x00000c0c,
	IO_BITMAP_A                     = 0x00002000,
	IO_BITMAP_A_HIGH                = 0x00002001,
	IO_BITMAP_B                     = 0x00002002,
	IO_BITMAP_B_HIGH                = 0x00002003,
	MSR_BITMAP                      = 0x00002004,
	MSR_BITMAP_HIGH                 = 0x00002005,
	VM_EXIT_MSR_STORE_ADDR          = 0x00002006,
	VM_EXIT_MSR_STORE_ADDR_HIGH     = 0x00002007,
	VM_EXIT_MSR_LOAD_ADDR           = 0x00002008,
	VM_EXIT_MSR_LOAD_ADDR_HIGH      = 0x00002009,
	VM_ENTRY_MSR_LOAD_ADDR          = 0x0000200a,
	VM_ENTRY_MSR_LOAD_ADDR_HIGH     = 0x0000200b,
	PML_ADDRESS			= 0x0000200e,
	PML_ADDRESS_HIGH		= 0x0000200f,
	TSC_OFFSET                      = 0x00002010,
	TSC_OFFSET_HIGH                 = 0x00002011,
	VIRTUAL_APIC_PAGE_ADDR          = 0x00002012,
	VIRTUAL_APIC_PAGE_ADDR_HIGH     = 0x00002013,
	APIC_ACCESS_ADDR		= 0x00002014,
	APIC_ACCESS_ADDR_HIGH		= 0x00002015,
	POSTED_INTR_DESC_ADDR           = 0x00002016,
	POSTED_INTR_DESC_ADDR_HIGH      = 0x00002017,
	VM_FUNCTION_CONTROL             = 0x00002018,
	VM_FUNCTION_CONTROL_HIGH        = 0x00002019,
	EPT_POINTER                     = 0x0000201a,
	EPT_POINTER_HIGH                = 0x0000201b,
	EOI_EXIT_BITMAP0                = 0x0000201c,
	EOI_EXIT_BITMAP0_HIGH           = 0x0000201d,
	EOI_EXIT_BITMAP1                = 0x0000201e,
	EOI_EXIT_BITMAP1_HIGH           = 0x0000201f,
	EOI_EXIT_BITMAP2                = 0x00002020,
	EOI_EXIT_BITMAP2_HIGH           = 0x00002021,
	EOI_EXIT_BITMAP3                = 0x00002022,
	EOI_EXIT_BITMAP3_HIGH           = 0x00002023,
	EPTP_LIST_ADDRESS               = 0x00002024,
	EPTP_LIST_ADDRESS_HIGH          = 0x00002025,
	VMREAD_BITMAP                   = 0x00002026,
	VMREAD_BITMAP_HIGH              = 0x00002027,
	VMWRITE_BITMAP                  = 0x00002028,
	VMWRITE_BITMAP_HIGH             = 0x00002029,
	XSS_EXIT_BITMAP                 = 0x0000202c,
	XSS_EXIT_BITMAP_HIGH            = 0x0000202d,
	ENCLS_EXITING_BITMAP            = 0x0000202e,
	TSC_MULTIPLIER                  = 0x00002032,
	TSC_MULTIPLIER_HIGH             = 0x00002033,
	GUEST_PHYSICAL_ADDRESS          = 0x00002400,
	GUEST_PHYSICAL_ADDRESS_HIGH     = 0x00002401,
	VMCS_LINK_POINTER               = 0x00002800,
	VMCS_LINK_POINTER_HIGH          = 0x00002801,
	GUEST_IA32_DEBUGCTL             = 0x00002802,
	GUEST_IA32_DEBUGCTL_HIGH        = 0x00002803,
	GUEST_IA32_PAT			= 0x00002804,
	GUEST_IA32_PAT_HIGH		= 0x00002805,
	GUEST_IA32_EFER			= 0x00002806,
	GUEST_IA32_EFER_HIGH		= 0x00002807,
	GUEST_IA32_PERF_GLOBAL_CTRL	= 0x00002808,
	GUEST_IA32_PERF_GLOBAL_CTRL_HIGH= 0x00002809,
	GUEST_PDPTR0                    = 0x0000280a,
	GUEST_PDPTR0_HIGH               = 0x0000280b,
	GUEST_PDPTR1                    = 0x0000280c,
	GUEST_PDPTR1_HIGH               = 0x0000280d,
	GUEST_PDPTR2                    = 0x0000280e,
	GUEST_PDPTR2_HIGH               = 0x0000280f,
	GUEST_PDPTR3                    = 0x00002810,
	GUEST_PDPTR3_HIGH               = 0x00002811,
	GUEST_BNDCFGS                   = 0x00002812,
	GUEST_BNDCFGS_HIGH              = 0x00002813,
	GUEST_IA32_RTIT_CTL             = 0x00002814,
	HOST_IA32_PAT			= 0x00002c00,
	HOST_IA32_PAT_HIGH		= 0x00002c01,
	HOST_IA32_EFER			= 0x00002c02,
	HOST_IA32_EFER_HIGH		= 0x00002c03,
	HOST_IA32_PERF_GLOBAL_CTRL	= 0x00002c04,
	HOST_IA32_PERF_GLOBAL_CTRL_HIGH	= 0x00002c05,
	PIN_BASED_VM_EXEC_CONTROL       = 0x00004000,
	CPU_BASED_VM_EXEC_CONTROL       = 0x00004002,
	EXCEPTION_BITMAP                = 0x00004004,
	PAGE_FAULT_ERROR_CODE_MASK      = 0x00004006,
	PAGE_FAULT_ERROR_CODE_MATCH     = 0x00004008,
	CR3_TARGET_COUNT                = 0x0000400a,
	VM_EXIT_CONTROLS                = 0x0000400c,
	VM_EXIT_MSR_STORE_COUNT         = 0x0000400e,
	VM_EXIT_MSR_LOAD_COUNT          = 0x00004010,
	VM_ENTRY_CONTROLS               = 0x00004012,
	VM_ENTRY_MSR_LOAD_COUNT         = 0x00004014,
	VM_ENTRY_INTR_INFO_FIELD        = 0x00004016,
	VM_ENTRY_EXCEPTION_ERROR_CODE   = 0x00004018,
	VM_ENTRY_INSTRUCTION_LEN        = 0x0000401a,
	TPR_THRESHOLD                   = 0x0000401c,
	SECONDARY_VM_EXEC_CONTROL       = 0x0000401e,
	PLE_GAP                         = 0x00004020,
	PLE_WINDOW                      = 0x00004022,
	VM_INSTRUCTION_ERROR            = 0x00004400,
	VM_EXIT_REASON                  = 0x00004402,
	VM_EXIT_INTR_INFO               = 0x00004404,
	VM_EXIT_INTR_ERROR_CODE         = 0x00004406,
	IDT_VECTORING_INFO_FIELD        = 0x00004408,
	IDT_VECTORING_ERROR_CODE        = 0x0000440a,
	VM_EXIT_INSTRUCTION_LEN         = 0x0000440c,
	VMX_INSTRUCTION_INFO            = 0x0000440e,
	GUEST_ES_LIMIT                  = 0x00004800,
	GUEST_CS_LIMIT                  = 0x00004802,
	GUEST_SS_LIMIT                  = 0x00004804,
	GUEST_DS_LIMIT                  = 0x00004806,
	GUEST_FS_LIMIT                  = 0x00004808,
	GUEST_GS_LIMIT                  = 0x0000480a,
	GUEST_LDTR_LIMIT                = 0x0000480c,
	GUEST_TR_LIMIT                  = 0x0000480e,
	GUEST_GDTR_LIMIT                = 0x00004810,
	GUEST_IDTR_LIMIT                = 0x00004812,
	GUEST_ES_AR_BYTES               = 0x00004814,
	GUEST_CS_AR_BYTES               = 0x00004816,
	GUEST_SS_AR_BYTES               = 0x00004818,
	GUEST_DS_AR_BYTES               = 0x0000481a,
	GUEST_FS_AR_BYTES               = 0x0000481c,
	GUEST_GS_AR_BYTES               = 0x0000481e,
	GUEST_LDTR_AR_BYTES             = 0x00004820,
	GUEST_TR_AR_BYTES               = 0x00004822,
	GUEST_INTERRUPTIBILITY_INFO     = 0x00004824,
	GUEST_ACTIVITY_STATE            = 0X00004826,
	GUEST_SYSENTER_CS               = 0x0000482A,
	VMX_PREEMPTION_TIMER_VALUE      = 0x0000482E,
	HOST_IA32_SYSENTER_CS           = 0x00004c00,
	CR0_GUEST_HOST_MASK             = 0x00006000,
	CR4_GUEST_HOST_MASK             = 0x00006002,
	CR0_READ_SHADOW                 = 0x00006004,
	CR4_READ_SHADOW                 = 0x00006006,
	CR3_TARGET_VALUE0               = 0x00006008,
	CR3_TARGET_VALUE1               = 0x0000600a,
	CR3_TARGET_VALUE2               = 0x0000600c,
	CR3_TARGET_VALUE3               = 0x0000600e,
	EXIT_QUALIFICATION              = 0x00006400,
	IO_RCX                          = 0x00006402,
	IO_RSI                          = 0x00006404,
	IO_RDI                          = 0x00006406,
	IO_RIP                          = 0x00006408,
	GUEST_LINEAR_ADDRESS            = 0x0000640a,
	GUEST_CR0                       = 0x00006800,
	GUEST_CR3                       = 0x00006802,
	GUEST_CR4                       = 0x00006804,
	GUEST_ES_BASE                   = 0x00006806,
	GUEST_CS_BASE                   = 0x00006808,
	GUEST_SS_BASE                   = 0x0000680a,
	GUEST_DS_BASE                   = 0x0000680c,
	GUEST_FS_BASE                   = 0x0000680e,
	GUEST_GS_BASE                   = 0x00006810,
	GUEST_LDTR_BASE                 = 0x00006812,
	GUEST_TR_BASE                   = 0x00006814,
	GUEST_GDTR_BASE                 = 0x00006816,
	GUEST_IDTR_BASE                 = 0x00006818,
	GUEST_DR7                       = 0x0000681a,
	GUEST_RSP                       = 0x0000681c,
	GUEST_RIP                       = 0x0000681e,
	GUEST_RFLAGS                    = 0x00006820,
	GUEST_PENDING_DBG_EXCEPTIONS    = 0x00006822,
	GUEST_SYSENTER_ESP              = 0x00006824,
	GUEST_SYSENTER_EIP              = 0x00006826,
	HOST_CR0                        = 0x00006c00,
	HOST_CR3                        = 0x00006c02,
	HOST_CR4                        = 0x00006c04,
	HOST_FS_BASE                    = 0x00006c06,
	HOST_GS_BASE                    = 0x00006c08,
	HOST_TR_BASE                    = 0x00006c0a,
	HOST_GDTR_BASE                  = 0x00006c0c,
	HOST_IDTR_BASE                  = 0x00006c0e,
	HOST_IA32_SYSENTER_ESP          = 0x00006c10,
	HOST_IA32_SYSENTER_EIP          = 0x00006c12,
	HOST_RSP                        = 0x00006c14,
	HOST_RIP                        = 0x00006c16,
};

/*
 * Interruption-information format
 */
#define INTR_INFO_VECTOR_MASK           0xff            /* 7:0 */
#define INTR_INFO_INTR_TYPE_MASK        0x700           /* 10:8 */
#define INTR_INFO_DELIVER_CODE_MASK     0x800           /* 11 */
#define INTR_INFO_UNBLOCK_NMI		0x1000		/* 12 */
#define INTR_INFO_VALID_MASK            0x80000000      /* 31 */
#define INTR_INFO_RESVD_BITS_MASK       0x7ffff000

#define VECTORING_INFO_VECTOR_MASK           	INTR_INFO_VECTOR_MASK
#define VECTORING_INFO_TYPE_MASK        	INTR_INFO_INTR_TYPE_MASK
#define VECTORING_INFO_DELIVER_CODE_MASK    	INTR_INFO_DELIVER_CODE_MASK
#define VECTORING_INFO_VALID_MASK       	INTR_INFO_VALID_MASK

#define INTR_TYPE_EXT_INTR              (0 << 8) /* external interrupt */
#define INTR_TYPE_NMI_INTR		(2 << 8) /* NMI */
#define INTR_TYPE_HARD_EXCEPTION	(3 << 8) /* processor exception */
#define INTR_TYPE_SOFT_INTR             (4 << 8) /* software interrupt */
#define INTR_TYPE_PRIV_SW_EXCEPTION	(5 << 8) /* ICE breakpoint - undocumented */
#define INTR_TYPE_SOFT_EXCEPTION	(6 << 8) /* software exception */

/* GUEST_INTERRUPTIBILITY_INFO flags. */
#define GUEST_INTR_STATE_STI		0x00000001
#define GUEST_INTR_STATE_MOV_SS		0x00000002
#define GUEST_INTR_STATE_SMI		0x00000004
#define GUEST_INTR_STATE_NMI		0x00000008

/* GUEST_ACTIVITY_STATE flags */
#define GUEST_ACTIVITY_ACTIVE		0
#define GUEST_ACTIVITY_HLT		1
#define GUEST_ACTIVITY_SHUTDOWN		2
#define GUEST_ACTIVITY_WAIT_SIPI	3

/*
 * Exit Qualifications for MOV for Control Register Access
 */
#define CONTROL_REG_ACCESS_NUM          0x7     /* 2:0, number of control reg.*/
#define CONTROL_REG_ACCESS_TYPE         0x30    /* 5:4, access type */
#define CONTROL_REG_ACCESS_REG          0xf00   /* 10:8, general purpose reg. */
#define LMSW_SOURCE_DATA_SHIFT 16
#define LMSW_SOURCE_DATA  (0xFFFF << LMSW_SOURCE_DATA_SHIFT) /* 16:31 lmsw source */
#define REG_EAX                         (0 << 8)
#define REG_ECX                         (1 << 8)
#define REG_EDX                         (2 << 8)
#define REG_EBX                         (3 << 8)
#define REG_ESP                         (4 << 8)
#define REG_EBP                         (5 << 8)
#define REG_ESI                         (6 << 8)
#define REG_EDI                         (7 << 8)
#define REG_R8                         (8 << 8)
#define REG_R9                         (9 << 8)
#define REG_R10                        (10 << 8)
#define REG_R11                        (11 << 8)
#define REG_R12                        (12 << 8)
#define REG_R13                        (13 << 8)
#define REG_R14                        (14 << 8)
#define REG_R15                        (15 << 8)

/*
 * Exit Qualifications for MOV for Debug Register Access
 */
#define DEBUG_REG_ACCESS_NUM            0x7     /* 2:0, number of debug reg. */
#define DEBUG_REG_ACCESS_TYPE           0x10    /* 4, direction of access */
#define TYPE_MOV_TO_DR                  (0 << 4)
#define TYPE_MOV_FROM_DR                (1 << 4)
#define DEBUG_REG_ACCESS_REG(eq)        (((eq) >> 8) & 0xf) /* 11:8, general purpose reg. */


/*
 * Exit Qualifications for APIC-Access
 */
#define APIC_ACCESS_OFFSET              0xfff   /* 11:0, offset within the APIC page */
#define APIC_ACCESS_TYPE                0xf000  /* 15:12, access type */
#define TYPE_LINEAR_APIC_INST_READ      (0 << 12)
#define TYPE_LINEAR_APIC_INST_WRITE     (1 << 12)
#define TYPE_LINEAR_APIC_INST_FETCH     (2 << 12)
#define TYPE_LINEAR_APIC_EVENT          (3 << 12)
#define TYPE_PHYSICAL_APIC_EVENT        (10 << 12)
#define TYPE_PHYSICAL_APIC_INST         (15 << 12)

/* segment AR in VMCS -- these are different from what LAR reports */
#define VMX_SEGMENT_AR_L_MASK (1 << 13)

#define VMX_AR_TYPE_ACCESSES_MASK 1
#define VMX_AR_TYPE_READABLE_CODE_MASK (1 << 1)
#define VMX_AR_TYPE_WRITABLE_DATA_MASK (1 << 1)
#define VMX_AR_TYPE_CONFORMING_MASK (1 << 2)
#define VMX_AR_TYPE_EXPAND_DOWN_MASK (1 << 2)

#define VMX_AR_TYPE_CODE_MASK (1 << 3)
#define VMX_AR_TYPE_MASK 0x0f
#define VMX_AR_TYPE_BUSY_64_TSS 11
#define VMX_AR_TYPE_BUSY_32_TSS 11
#define VMX_AR_TYPE_BUSY_16_TSS 3
#define VMX_AR_TYPE_LDT 2

#define VMX_AR_UNUSABLE_MASK (1 << 16)
#define VMX_AR_S_MASK (1 << 4)
#define VMX_AR_P_MASK (1 << 7)
#define VMX_AR_L_MASK (1 << 13)
#define VMX_AR_DB_MASK (1 << 14)
#define VMX_AR_G_MASK (1 << 15)
#define VMX_AR_DPL_SHIFT 5
#define VMX_AR_DPL(ar) (((ar) >> VMX_AR_DPL_SHIFT) & 3)

#define VMX_AR_RESERVD_MASK 0xfffe0f00

#define TSS_PRIVATE_MEMSLOT			(KVM_USER_MEM_SLOTS + 0)
#define APIC_ACCESS_PAGE_PRIVATE_MEMSLOT	(KVM_USER_MEM_SLOTS + 1)
#define IDENTITY_PAGETABLE_PRIVATE_MEMSLOT	(KVM_USER_MEM_SLOTS + 2)

#define VMX_NR_VPIDS				(1 << 16)
#define VMX_VPID_EXTENT_INDIVIDUAL_ADDR		0
#define VMX_VPID_EXTENT_SINGLE_CONTEXT		1
#define VMX_VPID_EXTENT_ALL_CONTEXT		2
#define VMX_VPID_EXTENT_SINGLE_NON_GLOBAL	3

#define VMX_EPT_EXTENT_CONTEXT			1
#define VMX_EPT_EXTENT_GLOBAL			2
#define VMX_EPT_EXTENT_SHIFT			24

#define VMX_EPT_EXECUTE_ONLY_BIT		(1ull)
#define VMX_EPT_PAGE_WALK_4_BIT			(1ull << 6)
#define VMX_EPT_PAGE_WALK_5_BIT			(1ull << 7)
#define VMX_EPTP_UC_BIT				(1ull << 8)
#define VMX_EPTP_WB_BIT				(1ull << 14)
#define VMX_EPT_2MB_PAGE_BIT			(1ull << 16)
#define VMX_EPT_1GB_PAGE_BIT			(1ull << 17)
#define VMX_EPT_INVEPT_BIT			(1ull << 20)
#define VMX_EPT_AD_BIT				    (1ull << 21)
#define VMX_EPT_EXTENT_CONTEXT_BIT		(1ull << 25)
#define VMX_EPT_EXTENT_GLOBAL_BIT		(1ull << 26)

#define VMX_VPID_INVVPID_BIT                    (1ull << 0) /* (32 - 32) */
#define VMX_VPID_EXTENT_INDIVIDUAL_ADDR_BIT     (1ull << 8) /* (40 - 32) */
#define VMX_VPID_EXTENT_SINGLE_CONTEXT_BIT      (1ull << 9) /* (41 - 32) */
#define VMX_VPID_EXTENT_GLOBAL_CONTEXT_BIT      (1ull << 10) /* (42 - 32) */
#define VMX_VPID_EXTENT_SINGLE_NON_GLOBAL_BIT   (1ull << 11) /* (43 - 32) */

#define VMX_EPT_MT_EPTE_SHIFT			3
#define VMX_EPTP_PWL_MASK			0x38ull
#define VMX_EPTP_PWL_4				0x18ull
#define VMX_EPTP_PWL_5				0x20ull
#define VMX_EPTP_AD_ENABLE_BIT			(1ull << 6)
#define VMX_EPTP_MT_MASK			0x7ull
#define VMX_EPTP_MT_WB				0x6ull
#define VMX_EPTP_MT_UC				0x0ull
#define VMX_EPT_READABLE_MASK			0x1ull
#define VMX_EPT_WRITABLE_MASK			0x2ull
#define VMX_EPT_EXECUTABLE_MASK			0x4ull
#define VMX_EPT_IPAT_BIT    			(1ull << 6)
#define VMX_EPT_ACCESS_BIT			(1ull << 8)
#define VMX_EPT_DIRTY_BIT			(1ull << 9)
#define VMX_EPT_RWX_MASK                        (VMX_EPT_READABLE_MASK |       \
						 VMX_EPT_WRITABLE_MASK |       \
						 VMX_EPT_EXECUTABLE_MASK)
#define VMX_EPT_MT_MASK				(7ull << VMX_EPT_MT_EPTE_SHIFT)

/* The mask to use to trigger an EPT Misconfiguration in order to track MMIO */
#define VMX_EPT_MISCONFIG_WX_VALUE		(VMX_EPT_WRITABLE_MASK |       \
						 VMX_EPT_EXECUTABLE_MASK)

#define VMX_EPT_IDENTITY_PAGETABLE_ADDR		0xfffbc000ul

#pragma pack(16)
struct vmx_msr_entry {
	u32 index;
	u32 reserved;
	u64 value;
};
#pragma pack(0)

/*
 * Exit Qualifications for entry failure during or after loading guest state
 */
#define ENTRY_FAIL_DEFAULT		0
#define ENTRY_FAIL_PDPTE		2
#define ENTRY_FAIL_NMI			3
#define ENTRY_FAIL_VMCS_LINK_PTR	4

/*
 * Exit Qualifications for EPT Violations
 */
#define EPT_VIOLATION_ACC_READ_BIT	0
#define EPT_VIOLATION_ACC_WRITE_BIT	1
#define EPT_VIOLATION_ACC_INSTR_BIT	2
#define EPT_VIOLATION_READABLE_BIT	3
#define EPT_VIOLATION_WRITABLE_BIT	4
#define EPT_VIOLATION_EXECUTABLE_BIT	5
#define EPT_VIOLATION_GVA_TRANSLATED_BIT 8
#define EPT_VIOLATION_ACC_READ		(1 << EPT_VIOLATION_ACC_READ_BIT)
#define EPT_VIOLATION_ACC_WRITE		(1 << EPT_VIOLATION_ACC_WRITE_BIT)
#define EPT_VIOLATION_ACC_INSTR		(1 << EPT_VIOLATION_ACC_INSTR_BIT)
#define EPT_VIOLATION_READABLE		(1 << EPT_VIOLATION_READABLE_BIT)
#define EPT_VIOLATION_WRITABLE		(1 << EPT_VIOLATION_WRITABLE_BIT)
#define EPT_VIOLATION_EXECUTABLE	(1 << EPT_VIOLATION_EXECUTABLE_BIT)
#define EPT_VIOLATION_GVA_TRANSLATED	(1 << EPT_VIOLATION_GVA_TRANSLATED_BIT)

/*
 * VM-instruction error numbers
 */
enum vm_instruction_error_number {
	VMXERR_VMCALL_IN_VMX_ROOT_OPERATION = 1,
	VMXERR_VMCLEAR_INVALID_ADDRESS = 2,
	VMXERR_VMCLEAR_VMXON_POINTER = 3,
	VMXERR_VMLAUNCH_NONCLEAR_VMCS = 4,
	VMXERR_VMRESUME_NONLAUNCHED_VMCS = 5,
	VMXERR_VMRESUME_AFTER_VMXOFF = 6,
	VMXERR_ENTRY_INVALID_CONTROL_FIELD = 7,
	VMXERR_ENTRY_INVALID_HOST_STATE_FIELD = 8,
	VMXERR_VMPTRLD_INVALID_ADDRESS = 9,
	VMXERR_VMPTRLD_VMXON_POINTER = 10,
	VMXERR_VMPTRLD_INCORRECT_VMCS_REVISION_ID = 11,
	VMXERR_UNSUPPORTED_VMCS_COMPONENT = 12,
	VMXERR_VMWRITE_READ_ONLY_VMCS_COMPONENT = 13,
	VMXERR_VMXON_IN_VMX_ROOT_OPERATION = 15,
	VMXERR_ENTRY_INVALID_EXECUTIVE_VMCS_POINTER = 16,
	VMXERR_ENTRY_NONLAUNCHED_EXECUTIVE_VMCS = 17,
	VMXERR_ENTRY_EXECUTIVE_VMCS_POINTER_NOT_VMXON_POINTER = 18,
	VMXERR_VMCALL_NONCLEAR_VMCS = 19,
	VMXERR_VMCALL_INVALID_VM_EXIT_CONTROL_FIELDS = 20,
	VMXERR_VMCALL_INCORRECT_MSEG_REVISION_ID = 22,
	VMXERR_VMXOFF_UNDER_DUAL_MONITOR_TREATMENT_OF_SMIS_AND_SMM = 23,
	VMXERR_VMCALL_INVALID_SMM_MONITOR_FEATURES = 24,
	VMXERR_ENTRY_INVALID_VM_EXECUTION_CONTROL_FIELDS_IN_EXECUTIVE_VMCS = 25,
	VMXERR_ENTRY_EVENTS_BLOCKED_BY_MOV_SS = 26,
	VMXERR_INVALID_OPERAND_TO_INVEPT_INVVPID = 28,
};

#define VMX_EXIT_REASONS_FAILED_VMENTRY         0x80000000

#define EXIT_REASON_EXCEPTION_NMI       0
#define EXIT_REASON_EXTERNAL_INTERRUPT  1
#define EXIT_REASON_TRIPLE_FAULT        2

#define EXIT_REASON_PENDING_INTERRUPT   7
#define EXIT_REASON_NMI_WINDOW          8
#define EXIT_REASON_TASK_SWITCH         9
#define EXIT_REASON_CPUID               10
#define EXIT_REASON_HLT                 12
#define EXIT_REASON_INVD                13
#define EXIT_REASON_INVLPG              14
#define EXIT_REASON_RDPMC               15
#define EXIT_REASON_RDTSC               16
#define EXIT_REASON_VMCALL              18
#define EXIT_REASON_VMCLEAR             19
#define EXIT_REASON_VMLAUNCH            20
#define EXIT_REASON_VMPTRLD             21
#define EXIT_REASON_VMPTRST             22
#define EXIT_REASON_VMREAD              23
#define EXIT_REASON_VMRESUME            24
#define EXIT_REASON_VMWRITE             25
#define EXIT_REASON_VMOFF               26
#define EXIT_REASON_VMON                27
#define EXIT_REASON_CR_ACCESS           28
#define EXIT_REASON_DR_ACCESS           29
#define EXIT_REASON_IO_INSTRUCTION      30
#define EXIT_REASON_MSR_READ            31
#define EXIT_REASON_MSR_WRITE           32
#define EXIT_REASON_INVALID_STATE       33
#define EXIT_REASON_MSR_LOAD_FAIL       34
#define EXIT_REASON_MWAIT_INSTRUCTION   36
#define EXIT_REASON_MONITOR_TRAP_FLAG   37
#define EXIT_REASON_MONITOR_INSTRUCTION 39
#define EXIT_REASON_PAUSE_INSTRUCTION   40
#define EXIT_REASON_MCE_DURING_VMENTRY  41
#define EXIT_REASON_TPR_BELOW_THRESHOLD 43
#define EXIT_REASON_APIC_ACCESS         44
#define EXIT_REASON_EOI_INDUCED         45
#define EXIT_REASON_GDTR_IDTR           46
#define EXIT_REASON_LDTR_TR             47
#define EXIT_REASON_EPT_VIOLATION       48
#define EXIT_REASON_EPT_MISCONFIG       49
#define EXIT_REASON_INVEPT              50
#define EXIT_REASON_RDTSCP              51
#define EXIT_REASON_PREEMPTION_TIMER    52
#define EXIT_REASON_INVVPID             53
#define EXIT_REASON_WBINVD              54
#define EXIT_REASON_XSETBV              55
#define EXIT_REASON_APIC_WRITE          56
#define EXIT_REASON_RDRAND              57
#define EXIT_REASON_INVPCID             58
#define EXIT_REASON_VMFUNC              59
#define EXIT_REASON_ENCLS               60
#define EXIT_REASON_RDSEED              61
#define EXIT_REASON_PML_FULL            62
#define EXIT_REASON_XSAVES              63
#define EXIT_REASON_XRSTORS             64

#define KVM_REG_RAX 0
#define KVM_REG_RCX 1
#define KVM_REG_RDX 2
#define KVM_REG_RBX 3
#define KVM_REG_RSP 4
#define KVM_REG_RBP 5
#define KVM_REG_RSI 6
#define KVM_REG_RDI 7
#define KVM_REG_R8 8
#define KVM_REG_R9 9
#define KVM_REG_R10 10
#define KVM_REG_R11 11
#define KVM_REG_R12 12
#define KVM_REG_R13 13
#define KVM_REG_R14 14
#define KVM_REG_R15 15

struct vmcs_hdr {
	u32 revision_id;
	u32 abort;
	char data[0];
};

enum vmcs_field_width {
	VMCS_FIELD_WIDTH_U16 = 0,
	VMCS_FIELD_WIDTH_U64 = 1,
	VMCS_FIELD_WIDTH_U32 = 2,
	VMCS_FIELD_WIDTH_NATURAL_WIDTH = 3
};

static inline int vmcs_field_width(unsigned long field)
{
	if (0x1 & field)
		return VMCS_FIELD_WIDTH_U32;
	return (field >> 13) & 0x3;
}

struct ctrl_regs {
	u64 cr0;
	u64 cr2;
	u64 cr3;
	u64 cr4;
	u64 cr8;
	u64 xcr0;
};

struct debug_regs {
	u64 dr0;
	u64 dr1;
	u64 dr2;
	u64 dr3;
	u64 dr4;
	u64 dr5;
	u64 dr6;
	u64 dr7;
};

struct segment {
	u64 selector;
	u64 base;
	u64 limit;
	u64 ar_bytes;
};

struct guest_memory_zone {
	u64 gpa;
	u64 hpa;
	u64 page_nr;
	u64 attributes;
	struct list_head list;
};

struct vmcs12;
struct vmx_vcpu {
	u64 state;
	u64 guest_mode;
	u64 virtual_processor_id;
	struct vmcs_hdr *vmxon_region;
	struct vmcs_hdr *vmcs01;
	struct vmcs12 *vmcs12;
	struct vmcs_hdr *vmcs02;
	struct vmcs_hdr *shadow_vmcs;
	u64 shadow_vmcs_enabled;
	u64 *vmread_bitmap;
	u64 *vmwrite_bitmap;
	u64 *io_bitmap_a;
	u64 *io_bitmap_b;
	u64 *vapic_page;
	u64 *msr_bitmap;
	u64 *eptp_base;
	u64 *posted_intr_addr;

	struct list_head list;

	struct host_state {
		struct general_regs gr_regs;
		struct xsave_area *fp_regs;
		struct ctrl_regs ctrl_regs;
		struct debug_regs debug_regs;
		u64 *msr;
	} host_state;

	struct guest_state {
		u64 rip;
		u64 rflags;
		struct general_regs gr_regs;
		struct xsave_area *fp_regs;
		struct segment cs, ds, es, fs, gs, ss, tr, ldtr;
		struct ctrl_regs ctrl_regs;
		struct debug_regs debug_regs;
		
		u64 cr0_read_shadow;
		u64 cr4_read_shadow;

		struct gdtr gdtr;
		struct idtr idtr;

		u64 *msr;
		u64 pdpte0;
		u64 pdpte1;
		u64 pdpte2;
		u64 pdpte3;
	} guest_state;

	struct list_head guest_memory_list;
};


static inline void vmx_on(u64 paddr)
{
	asm volatile("vmxon (%%rax)"
		::"r"(&paddr), "m"(paddr)
		);
}

static inline void vmx_off()
{
	asm volatile("vmxoff");
}

static inline void vmptr_load(u64 paddr)
{
	asm volatile("vmptrld (%%rax)"
		::"a"(&paddr), "m"(paddr)
		);
}

static inline void vmptr_store(u64 *paddr)
{
	asm volatile("vmptrst (%0)"
		::"r"(paddr)
		);
}

static inline void vmclear(u64 paddr)
{
	asm volatile("vmclear (%%rax)"
		::"a"(&paddr), "m"(paddr)
		);
}


static inline u64 vmcs_read(u64 field)
{
	u64 ret;
	asm volatile("vmread %%rdi, %%rax\n\t"
		:"=a"(ret):"rdi"(field)
		);
	return ret;
}

static inline void vmcs_write(u64 field, u64 value)
{
	u64 rflags;
	asm volatile("vmwrite %%rsi, %%rdi\n\t"
		"pushf \n\t"
		"pop %0"
		:"=r"(rflags)
		:"rdi"(field), "rsi"(value)
		);

	if (rflags & BIT0) {
		//printk("writing vmcs field %x failed.\n", field);
	}
	if (rflags & BIT6) {
		//printk("writing vmcs field %x failed.\n", field);
	}
}

static inline void invvpid(unsigned long ext, u16 vpid, gva_t gva)
{
	struct {
		u64 vpid : 16;
		u64 rsvd : 48;
		u64 gva;
	} operand = { vpid, 0, gva };
	bool error;

	asm volatile ("invvpid %1, %0"
		      :: "r"(ext), "m"(operand));
}

static inline void invept(unsigned long ext, u64 eptp, gpa_t gpa)
{
	struct {
		u64 eptp, gpa;
	} operand = {eptp, gpa};
	bool error;

	asm volatile ("invept %1, %0"
		      :: "r"(ext), "m"(operand));
}


int vm_launch(void *host_reg, void *guest_reg);
int vm_resume(void *host_reg, void *guest_reg);

int vm_run(struct vmx_vcpu *vcpu);

u64 kvm_reg_read(struct vmx_vcpu *vcpu, int index);
void kvm_reg_write(struct vmx_vcpu *vcpu, int index, u64 value);
int ept_gpa_to_hpa(struct vmx_vcpu *vcpu, gpa_t gpa, hpa_t *hpa);
int ept_gpa_to_hpa_any(u64 *eptp_base, gpa_t gpa, hpa_t *hpa);
int paging64_gva_to_gpa(struct vmx_vcpu *vcpu, gva_t gva, gpa_t *gpa);
int nested_ept_l2gpa_to_l1gpa(struct vmx_vcpu *vcpu, struct vmcs12 *vmcs12, gpa_t l2gpa, gpa_t *l1gpa);
int ept_map_page(u64 *eptp_base, u64 gpa, u64 hpa, u64 page_size, u64 attribute);
int nested_vm_exit_reflected(struct vmx_vcpu *vcpu, u32 exit_reason);
int nested_vmx_reflect_vmexit(struct vmx_vcpu *vcpu, u32 exit_reason);
int nested_vmx_handle_ept_volation(struct vmx_vcpu *vcpu);




#endif
