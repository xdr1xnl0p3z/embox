/**
 * \file mmu_probe.c
 * \date Jun 9, 2009
 * \author anton
 * \details
 */
#include "types.h"
#include "leon.h"
#include "shell.h"
#include "mmu.h"
#include "mmu_probe.h"

/**
 * show MMU register
 */
static BOOL mmu_show_reg() {
    printf("Registers MMU:\n");
//    printf("CTLR REG:\t%X\n", srmmu_get_mmureg(SRMMU_CTRL_REG));
//    printf("CTXTBL PTR:\t%X\n", srmmu_get_mmureg(SRMMU_CTXTBL_PTR));
//    printf("CTX REG:\t%X\n", srmmu_get_mmureg(SRMMU_CTX_REG));
//    printf("FAULT STATUS:\t%X\n", srmmu_get_mmureg(SRMMU_FAULT_STATUS));
//    printf("FAULT ADDR:\t%X\n", srmmu_get_mmureg(SRMMU_FAULT_ADDR));
    return 0;
}
static BOOL mmu_show_version() {
    printf("Registers MMU:\n");
    return 0;
}


#define TLBNUM 4


static BOOL mmu_probe() {

//  ctxd_t *c0 = (ctxd_t *) &ctx;
//  pgd_t *g0 = (pgd_t *) &pg0;
//  pmd_t *m0 = (pmd_t *) &pm0;
//  pte_t *p0 = (pte_t *) &pt0;
//  unsigned long pteval, j, k;
    unsigned long j, k, val;
//  unsigned long paddr, vaddr, val;
//  unsigned long *pthaddr = &pth_addr1;
    functype func = mmu_func1;
    int i = 0;
    //alloc mem for tables
    //ctx context table
    __asm__(
            ".section .data\n\t"
            ".align %0\n\t"
            "page0: .skip %0\n\t"
            "page1: .skip %0\n\t"
            "page2: .skip %4\n\t"
            ".text\n"
            : : "i" (PAGE_SIZE),
            "i"(SRMMU_PGD_TABLE_SIZE) ,
            "i"(SRMMU_PMD_TABLE_SIZE) ,
            "i"(SRMMU_PTE_TABLE_SIZE) ,
            "i"((3)*PAGE_SIZE) );

    mmu_probe_init();
    mmu_flush_cache_all ();
    mmu_flush_tlb_all ();
    printf ("mmu_test\n");





//  /* one-on-one mapping for context 0 */
//  paddr = 0;
//  srmmu_ctxd_set(c0 + 0, (pgd_t *) g0);
    mmu_probe_map_region(0, 0, 0x1000000, MMU_PRIV);
    //pteval = ((0 >> 4) | SRMMU_ET_PTE | SRMMU_PRIV); /*0 - 1000000: ROM */
    //srmmu_set_pte(g0 + 0, pteval);
    mmu_probe_map_region(0x20000000, 0x20000000, 0x1000000, MMU_PRIV);
    //pteval = ((0x20000000 >> 4) | SRMMU_ET_PTE | SRMMU_PRIV); /*20000000 - 21000000: IOAREA */
    //srmmu_set_pte(g0 + 32, pteval);
    mmu_probe_map_region(0x40000000, 0x40000000, 0x1000000, MMU_PRIV);
    //pteval = ((0x40000000 >> 4) | SRMMU_ET_PTE | SRMMU_PRIV | SRMMU_CACHE); /*40000000 - 41000000: CRAM */
    //srmmu_set_pte(g0 + 64, pteval);

    mmu_probe_map_region(0x80000000, 0x80000000, 0x1000000, MMU_PRIV);
    //pteval = ((0x80000000 >> 4) | SRMMU_ET_PTE | SRMMU_PRIV ); /*80000000 - 81000000: CRAM */
    //srmmu_set_pte(g0 + 0x80, pteval);
/* testarea:
 *  map 0x40000000  at f0080000 [vaddr:(0) (240)(2)(-)] as pmd
 *  map page0       at f0041000 [vaddr:(0) (240)(1)(1)] as page SRMMU_PRIV_RDONLY
 *  map mmu_func1() at f0042000 [vaddr:(0) (240)(1)(2)] as page
 *  map f0043000 - f007f000 [vaddr:(0) (240)(1)(3)] - [vaddr:(0) (240)(1)(63)] as page
 * page fault test:
 *  missing pgd at f1000000 [vaddr:(0) (241)(-)(-)]
 */
    mmu_probe_map_region(0x40000000, 0xf0080000, 0x40000, MMU_PRIV);
    //not alloc here (alloc after page_fault)
    mmu_probe_map_region(0x0, 0xf0042000, 0x1000, 0);

    mmu_probe_map_region((unsigned long)&page0, 0xf0041000, 0x1000, MMU_PRIV_RDONLY);

  ((unsigned long *)&page0)[0] = 0;
  ((unsigned long *)&page0)[1] = 0x12345678;
    for (i = 3; i < TLBNUM+3; i++) {
        mmu_probe_map_region((((unsigned long) &page2) + (((i - 3) % 3) * PAGE_SIZE)), (0xf0040000 + (i * PAGE_SIZE)) , 0x1000, MMU_PRIV);
    }
    mmu_probe_repair_table_init((unsigned long)&page0);

//  //repair table
// *((unsigned long **)&pth_addr) =  pthaddr;
//  /* repair info for fault (0xf1000000)*/
//  pthaddr[0] = (unsigned long) (g0+241);
//  pthaddr[1] = ((0x40000000 >> 4) | SRMMU_ET_PTE | SRMMU_PRIV);
//  pthaddr[2] = 0xf1000000;
//  /* repair info for write protection fault (0xf0041000) */
//  pthaddr[3] = (unsigned long) (p0+1);
//  pthaddr[4] = ((((unsigned long)&page0) >> 4) | SRMMU_ET_PTE | SRMMU_PRIV);
//  pthaddr[5] = 0xf0041000;
//  /* repair info for instruction page fault (0xf0042000) */
//  pthaddr[6] = (unsigned long) (p0+2);
//  pthaddr[7] = ((((unsigned long)func) >> 4) | SRMMU_ET_PTE | SRMMU_PRIV);
//  pthaddr[8] = 0xf0042000;
//  /* repair info for priviledge protection fault (0xf0041000) */
//  pthaddr[9] = (unsigned long) (p0+1);
//  pthaddr[10] = ((((unsigned long)&page0) >> 4) | SRMMU_ET_PTE | SRMMU_EXEC | SRMMU_WRITE);
//  pthaddr[11] = 0xf0041000;

  //srmmu_set_ctable_ptr((unsigned long)c0);

  /* test reg access */
 // k = srmmu_get_mmureg(0);
  //k = srmmu_get_ctable_ptr();
  //srmmu_set_context(1);
  //k = srmmu_get_context();
  //srmmu_set_context(0);

  /* close your eyes and pray ... */
  mmu_probe_start();


  /* test reg access */
  //k = srmmu_get_mmureg(0);
  //k = srmmu_get_ctable_ptr();
  //k = srmmu_get_context();

  /* do tests*/
  if ( (*((unsigned long *)0xf0041000)) != 0 ||
       (*((unsigned long *)0xf0041004)) != 0x12345678 ) { MMU_RETURN(1); }

  if ( (*((unsigned long *)0xf0080000)) != (*((unsigned long *)0x40000000))) { MMU_RETURN(2); }

  /* page faults tests*/
  val = * ((volatile unsigned long *)0xf1000000);
  /* write protection fault */
  * ((volatile unsigned long *)0xf0041004) = 0x87654321;
  if ( (*((volatile unsigned long *)0xf0041004)) != 0x87654321 ) { MMU_RETURN(3); }

    for (j = 0xf0043000, i = 3; i < TLBNUM+3; i++, j += 0x1000) {
        *((unsigned long *) j) = j;
        if (*((unsigned long*) (((unsigned long) &page2) + (((i - 3) % 3)
                * PAGE_SIZE))) != j) {
            MMU_RETURN (FALSE);
        }
    }
    flush_data_cache();

  for (j = 0, i = 3; i < TLBNUM+3; i++) {
      pteval = (((((unsigned long) &page2) + (((i - 3) % 3) * PAGE_SIZE))
              >> 4) | SRMMU_ET_PTE | SRMMU_PRIV);
      if ((*(p0 + i)) & (SRMMU_DIRTY| SRMMU_REF))
          j++;

      if (((*(p0 + i)) & ~(SRMMU_DIRTY| SRMMU_REF)) != (pteval
              & ~(SRMMU_DIRTY| SRMMU_REF))) {
          MMU_RETURN (FALSE);
      }
  }
  //at least one entry has to have been flushed
  if (j == 0) {
      MMU_RETURN (FALSE);
  }


    /* instruction page fault */
    func = (functype) 0xf0042000;
    func();

//  /* flush */
//  srmmu_flush_whole_tlb();
//  for (j = 0, i = 3; i < TLBNUM+3; i++) {
//      if ((*(p0 + i)) & (SRMMU_DIRTY| SRMMU_REF))
//          j++;
//  }
//  if (j != TLBNUM) {
//      MMU_RETURN (FALSE);
//  }
//
//  /* check modified & ref bit */
//  if (!srmmu_pte_dirty(p0[1]) || !srmmu_pte_young(p0[1])) {
//      MMU_RETURN (FALSE);
//  }
//
//  if (!srmmu_pte_young(m0[2])) {
//      MMU_RETURN (FALSE);
//  }
//  if (!srmmu_pte_young(p0[2])) {
//      MMU_RETURN (FALSE);
//  }

    printf ("ending mmu testing");
    MMU_RETURN (TRUE);
}





#define COMMAND_NAME "mmu_probe"

static char available_keys[] = {
#include "mmu_probe_key.inc"
        };
static void show_help() {
    printf(
#include "mmu_probe_help.inc"
    );
}

/**
 * handler of command "mmu_probe"
 * It starts tests of mmu mode
 * return 0 if successed
 * return -1 another way
 */
BOOL mmu_probe_shell_handler(int argsc, char **argsv) {
    SHELL_KEY keys[MAX_SHELL_KEYS];
    char *key_value;
    int keys_amount;

    keys_amount = parse_arg(COMMAND_NAME, argsc, argsv, available_keys,
            sizeof(available_keys), keys);

    if (keys_amount < 0) {
        LOG_ERROR("during parsing params\n");
        show_help();
        return -1;
    }

    if (get_key('h', keys, keys_amount, &key_value)) {
        show_help();
        return 0;
    }

    if (get_key('v', keys, keys_amount, &key_value)) {
        mmu_show_version();
        return 0;
    }

    if (get_key('i', keys, keys_amount, &key_value)) {
        mmu_show_reg();
        return 0;
    }
    return mmu_probe();
}
