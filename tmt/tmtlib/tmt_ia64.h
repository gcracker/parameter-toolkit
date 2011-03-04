#ifndef _TMT_IA64_H_
#define _TMT_IA64_H_

#define IA64_RENBASE (0x1000)
#define IA64_GRTOBASE(reg) ((reg) - REG_GBASE)

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  guint32 framebase; /* Current reg stack frame base for IA64 translation */
  struct {          
    gint frrb;
    gint prrb;
    gint grrb;
    gint sor;
  } cfm;
} IA64RegStackInfo;

/* Don't include this stuff if this is a pin tool build */
#ifndef PINTOOL_BUILD 
/* Stuff below here is from Pin 1.79's (AKA PIN 0)  Reg.H */
typedef enum
{
    REG_INVALID = 0,
    REG_NONE = 1,
    REG_IMMBASE = 2,
    REG_IMM_LIT = REG_IMMBASE + 0,
    REG_IMM_BBL = REG_IMMBASE + 1,
    REG_IMM_REL = REG_IMMBASE + 2,

    REG_BBASE = 8, // 8 branch regoisters
    REG_B0 = REG_BBASE + 0,
    REG_B1 = REG_BBASE + 1,
    REG_B2 = REG_BBASE + 2,
    REG_B3 = REG_BBASE + 3,
    REG_B4 = REG_BBASE + 4,
    REG_B5 = REG_BBASE + 5,
    REG_B6 = REG_BBASE + 6,
    REG_B7 = REG_BBASE + 7,

    REG_PBASE = 16, // 64 predicate registers 
    REG_PR0 = REG_PBASE + 0,
    REG_PR1 = REG_PBASE + 1,
    REG_PR15 = REG_PBASE + 15,
    REG_PR63 = REG_PBASE + 63,

    REG_GBASE = 128, // 128 general registers
    REG_GZERO = REG_GBASE + 0,
    REG_GP  = REG_GBASE + 1,
    REG_G02 = REG_GBASE + 2,
    REG_G03 = REG_GBASE + 3,
    REG_G04 = REG_GBASE + 4,
    REG_G05 = REG_GBASE + 5,
    REG_G06 = REG_GBASE + 6,
    REG_G07 = REG_GBASE + 7,
    REG_G08 = REG_GBASE + 8,
    REG_G09 = REG_GBASE + 9,
    REG_G10 = REG_GBASE + 10,
    REG_G11 = REG_GBASE + 11,
    REG_SP  = REG_GBASE + 12,
    REG_TP  = REG_GBASE + 13,
    REG_G14 = REG_GBASE + 14,
    REG_G15 = REG_GBASE + 15,
    REG_G16 = REG_GBASE + 16,
    REG_G17 = REG_GBASE + 17,
    REG_G18 = REG_GBASE + 18,
    REG_G19 = REG_GBASE + 19,
    REG_G20 = REG_GBASE + 20,
    REG_G21 = REG_GBASE + 21,
    REG_G22 = REG_GBASE + 22,
    REG_G23 = REG_GBASE + 23,
    REG_G24 = REG_GBASE + 24,
    REG_G25 = REG_GBASE + 25,
    REG_G26 = REG_GBASE + 26,
    REG_G27 = REG_GBASE + 27,
    REG_G28 = REG_GBASE + 28,
    REG_G29 = REG_GBASE + 29,
    REG_G30 = REG_GBASE + 30,
    REG_G31 = REG_GBASE + 31,
    REG_G32 = REG_GBASE + 32,
    REG_G33 = REG_GBASE + 33,
    REG_G34 = REG_GBASE + 34,
    REG_G35 = REG_GBASE + 35,
    REG_G36 = REG_GBASE + 36,
    REG_G37 = REG_GBASE + 37,
    REG_G38 = REG_GBASE + 38,
    REG_G39 = REG_GBASE + 39,
    REG_G40 = REG_GBASE + 40,
    REG_G41 = REG_GBASE + 41,
    REG_G42 = REG_GBASE + 42,
    REG_G43 = REG_GBASE + 43,
    REG_G105 = REG_GBASE + 105,
    REG_G106 = REG_GBASE + 106,
    REG_G127 = REG_GBASE + 127,

    REG_FBASE = 256, // 128 floating point registers
    REG_FZERO = REG_FBASE + 0,
    REG_FONE  = REG_FBASE + 1,
    REG_F01   = REG_FBASE + 1, 
    REG_F02   = REG_FBASE + 2,
    REG_F03   = REG_FBASE + 3,
    REG_F04   = REG_FBASE + 4,
    REG_F05   = REG_FBASE + 5,
    REG_F06   = REG_FBASE + 6,
    REG_F07   = REG_FBASE + 7,
    REG_F08   = REG_FBASE + 8,
    REG_F09   = REG_FBASE + 9,
    REG_F10   = REG_FBASE + 10,
    REG_F11   = REG_FBASE + 11,
    REG_F12   = REG_FBASE + 12,
    REG_F13   = REG_FBASE + 13,
    REG_F14   = REG_FBASE + 14,
    REG_F15   = REG_FBASE + 15,
    REG_F32   = REG_FBASE + 32,
    REG_F127  = REG_FBASE + 127,


    REG_CBASE = 384, // 128 control registers 

    REG_ABASE = 512, // 128 application registers (implicit) 

    REG_AR_KR0 = REG_ABASE + 0,
    REG_AR_KR1 = REG_ABASE + 1,
    REG_AR_KR2 = REG_ABASE + 2,
    REG_AR_KR3 = REG_ABASE + 3,
    REG_AR_KR4 = REG_ABASE + 4,
    REG_AR_KR5 = REG_ABASE + 5,
    REG_AR_KR6 = REG_ABASE + 6,
    REG_AR_KR7 = REG_ABASE + 7,

    REG_AR_CSD = REG_ABASE + 25,
    
    REG_AR_RSC     = REG_ABASE + 16,
    REG_AR_BSP     = REG_ABASE + 17,
    REG_AR_BSPSTORE = REG_ABASE + 18,
    REG_AR_RNAT    = REG_ABASE + 19,
    
    REG_AR_UNAT  =  REG_ABASE + 36,

    REG_AR_FPSR  =  REG_ABASE + 40,

    REG_AR_PFS   = REG_ABASE + 64,
    REG_AR_LC    = REG_ABASE + 65,
    REG_AR_EC    = REG_ABASE + 66,

    REG_SBASE = 640, // 128 special registers

    REG_PR    = REG_SBASE + 0,
    REG_CFM   = REG_SBASE + 1,
    REG_IP    = REG_SBASE + 2,
    
    REG_COMP  = 768, // Compiler registers
    REG_PSP = REG_COMP + 0,
    REG_RA_SCR1 = REG_COMP + 1,
    REG_RA_SCR2 = REG_COMP + 2,
    REG_COMP_DC = REG_COMP + 3,
    REG_COMP_LAST = REG_COMP_DC,

    REG_VBASE    = 1000,
    REG_VUNAT    = REG_VBASE + 0,
    REG_INST_PFS1 = REG_VBASE + 1,
    REG_INST_PFS2 = REG_VBASE + 2,
    REG_INST_RA   = REG_VBASE + 3,
    REG_INST_RA_SAVE = REG_VBASE + 4,
    REG_GP_SCR1   = REG_VBASE + 5,
    REG_GP_SCR2   = REG_VBASE + 6,
    REG_PR_SCR1   = REG_VBASE + 7,
    REG_PR_TRUE   = REG_VBASE + 8,
    REG_BR_SCR1   = REG_VBASE + 9,
    REG_THREAD_ID  = REG_VBASE + 10,
    REG_IN_SIGNAL  = REG_VBASE + 11,
    REG_COMES_FROM = REG_VBASE + 12,
    REG_ORIGFUNADDR = REG_VBASE + 13,
    REG_ORIGFUNGP = REG_VBASE + 14,
    REG_NATIVECALL_RA = REG_VBASE + 15,

    REG_INST_BASE = REG_VBASE + 16,
    REG_INST_G0 = REG_INST_BASE, 
    REG_INST_G1 = REG_INST_BASE + 1,
    REG_INST_G2 = REG_INST_BASE + 2,
    REG_INST_G3 = REG_INST_BASE + 3,
    REG_INST_G4 = REG_INST_BASE + 4,
    REG_INST_G5 = REG_INST_BASE + 5,
    REG_INST_G6 = REG_INST_BASE + 6,
    REG_INST_G7 = REG_INST_BASE + 7,
    REG_INST_G8 = REG_INST_BASE + 8,
    REG_INST_G9 = REG_INST_BASE + 9,

    REG_INST_P0 = REG_INST_BASE + 10,
    REG_INST_P1 = REG_INST_BASE + 11,
    REG_INST_P2 = REG_INST_BASE + 12,
    REG_INST_P3 = REG_INST_BASE + 13,
    REG_INST_P4 = REG_INST_BASE + 14,
    REG_INST_P5 = REG_INST_BASE + 15,
    REG_INST_P6 = REG_INST_BASE + 16,
    REG_INST_P7 = REG_INST_BASE + 17,
    REG_INST_P8 = REG_INST_BASE + 18,
    REG_INST_P9 = REG_INST_BASE + 19,

    REG_INST_LAST = REG_INST_P9,

    REG_VGABASE    = REG_INST_LAST + 1,
    REG_VGALAST    = REG_VGABASE + 7,
    
    REG_VPBASE   = REG_VGALAST + 1,
    REG_VPLAST   = REG_VPBASE + 10,

    REG_VGBASE   = REG_VPLAST + 1,
    REG_VGLAST   = REG_VGBASE + 24,

    REG_VBBASE   = REG_VGLAST + 1,
    REG_VBLAST   = REG_VBBASE + 1,

    REG_VFBASE   = REG_VBLAST + 1,
    REG_VFLAST   = REG_VFBASE + 125,

    REG_VLAST    = REG_VFLAST + 1,
    REG_LAST     = REG_VLAST,

    REG_NBASE    = 2000,
    REG_NLAST    = REG_NBASE + 127,
    
    REG_TYPE_HARD,
    REG_TYPE_FP,
    REG_TYPE_GP,
    REG_TYPE_GP_NS,
    REG_TYPE_BR,
    REG_TYPE_PR,

    REG_GP_ARG0 = REG_G32,
    REG_GP_ARG1 = REG_G33,
    REG_GP_ARG2 = REG_G34,
    REG_GP_ARG3 = REG_G35,
    REG_GP_ARG4 = REG_G36,
    REG_GP_ARG5 = REG_G37,
    REG_GP_ARG6 = REG_G38,
    REG_GP_ARG7 = REG_G39,

    REG_FP_ARG0 = REG_F08,
    REG_FP_ARG1 = REG_F09,
    REG_FP_ARG2 = REG_F10,
    REG_FP_ARG3 = REG_F11,
    REG_FP_ARG4 = REG_F12,
    REG_FP_ARG5 = REG_F13,
    REG_FP_ARG6 = REG_F14,
    REG_FP_ARG7 = REG_F15,

    REG_GP_RET0 = REG_G08,
    REG_GP_RET1 = REG_G09,
    REG_GP_RET2 = REG_G10,
    REG_GP_RET3 = REG_G11,
    
    REG_FP_RET0 = REG_F08,
    REG_FP_RET1 = REG_F09,
    REG_FP_RET2 = REG_F10,
    REG_FP_RET3 = REG_F11,
    REG_FP_RET4 = REG_F12,
    REG_FP_RET5 = REG_F13,
    REG_FP_RET6 = REG_F14,
    REG_FP_RET7 = REG_F15

} REG;

typedef enum
{
    IMPL_REG_AR_KR0 =  (1LL<<0LL),
    IMPL_REG_AR_KR1 =  (1LL<<1LL),
    IMPL_REG_AR_KR2 =  (1LL<<2LL),
    IMPL_REG_AR_KR3 =  (1LL<<3LL),
    IMPL_REG_AR_KR4 =  (1LL<<4LL),
    IMPL_REG_AR_KR5 =  (1LL<<5LL),
    IMPL_REG_AR_KR6 =  (1LL<<6LL),
    IMPL_REG_AR_KR7 =  (1LL<<7LL),

    IMPL_REG_AR_RSC =       (1LL<<8LL),
    IMPL_REG_AR_BSP =       (1LL<<9LL),
    IMPL_REG_AR_BSPSTORE =  (1LL<<10LL),
    IMPL_REG_AR_RNAT =      (1LL<<11LL),

    IMPL_REG_AR_CCV =       (1LL<<12LL),
    IMPL_REG_AR_UNAT =      (1LL<<13LL),
    IMPL_REG_AR_FPSR =      (1LL<<14LL),
    IMPL_REG_AR_ITC =       (1LL<<15LL),

    IMPL_REG_AR_PFS =      (1LL<<16LL),
    IMPL_REG_AR_LC =       (1LL<<17LL),
    IMPL_REG_AR_EC =       (1LL<<18LL),

    IMPL_REG_AR_IA32 =       (1LL<<18LL), //< collective ar.ia32 regs
    
    IMPL_REG_CR =       (1LL<<18LL), //< collective cr regs
    IMPL_REG_MR =       (1LL<<18LL), //< collective rr,dbr,ibr,pkr,pmc,pmd,cpuid,dr,itr regs

    IMPL_REG_PSR =  0,
    IMPL_REG_CFM =    (1LL<<24LL),      
    IMPL_REG_IP =     (1LL<<25LL),
    IMPL_REG_USERMASK =  (1LL<<26LL),
    IMPL_REG_SYSTEMMASK =  (1LL<<26LL),


    //IMPL_REG_PMD =    (1LL<<32LL),
    //IMPL_REG_CPUID =  (1LL<<33LL),
    //IMPL_REG_PMD =    (1LL<<34LL),

    IMPL_REG_ALLAT =   (1LL<<40LL),
    IMPL_REG_MEMORY =    (1LL<<41LL),
    IMPL_REG_PREDICATES = (1LL<<41LL),
    IMPL_REG_BARRIER  =0
}IMPL_REG;


typedef guint64 IMPL_REGS;

#define BOOL gboolean

//extern REG REG_GetNatReg(REG reg);
//extern REG REG_GetRegFromNat(REG reg);
static inline BOOL REG_is_none(REG reg) { return (reg == REG_NONE); }

static inline BOOL REG_is_gr(REG reg){ return (reg >= REG_GBASE) && (reg < REG_GBASE+128);}
static inline BOOL REG_is_gr_low(REG reg){ return (reg >= REG_GBASE) && (reg < REG_GBASE+4);}
static inline BOOL REG_is_fr(REG reg){ return (reg >= REG_FBASE) && (reg < REG_FBASE+128);}
static inline BOOL REG_is_fr_rot(REG reg){ return (reg >= REG_FBASE + 32) && (reg < REG_FBASE+128);}
static inline BOOL REG_is_cr(REG reg){ return (reg >= REG_CBASE) && (reg < REG_CBASE+128);}
static inline BOOL REG_is_ar(REG reg){ return (reg >= REG_ABASE) && (reg < REG_ABASE+128);}
static inline BOOL REG_is_special(REG reg){ return (reg >= REG_SBASE) && (reg < REG_SBASE+128);}
static inline BOOL REG_is_nat(REG reg){ return (reg >= REG_NBASE) && (reg <= REG_NLAST);}

static inline BOOL REG_is_pr(REG reg){ return (reg >= REG_PBASE) && (reg < REG_PBASE+64);}
static inline BOOL REG_is_pr_rot(REG reg){ return (reg >= REG_PBASE+16) && (reg < REG_PBASE+64);}

static inline BOOL REG_is_br(REG reg){ return (reg >= REG_BBASE) && (reg < REG_BBASE+8);}
static inline BOOL REG_is_br0(REG reg){ return (reg == REG_BBASE + 0);}

static inline BOOL REG_is_sp(REG reg){ return (reg == REG_SP);}

static inline BOOL REG_is_imm(REG reg){ return (reg >= REG_IMMBASE) && (reg < REG_IMMBASE+3);}
static inline BOOL REG_is_imm_lit(REG reg) { return (reg == REG_IMM_LIT); }
static inline BOOL REG_is_reg(REG reg){ return (reg >= REG_BBASE);}
static inline BOOL REG_is_reg_or_none(REG reg){ return (reg == REG_NONE) || (reg >= REG_BBASE);}

static inline BOOL REG_is_stacked(REG reg) { return ((reg >= REG_G32) && (reg <= REG_G127)); }
static inline BOOL REG_is_retval(REG reg) { return ((reg >= REG_G08) && (reg <= REG_G11)); }
    
static inline BOOL REG_valid(REG reg){ return reg != REG_INVALID;}

#endif /* PINTOOL_BUILD */

static inline guint extract_sor_from_cfm(guint64 val)
{
  return ((val >> 14) & 0xf); 
}

static inline guint extract_sol_from_cfm(guint64 val) 
{
  return ((val >> 7) & 0x7f);
}

static inline guint extract_sol_from_pfs(guint64 val) {
  return extract_sol_from_cfm(val);
}

static inline guint extract_sor_from_pfs(guint64 val) {
  return extract_sor_from_cfm(val);
}

static inline guint extract_sor_from_imm(guint64 val)
{
  return ((val >> 27) & 0xf); /* FIXME later */
}

static inline guint extract_frrb_from_pfs(guint64 val) {
  return ((val >> 25) & 0x7f);
}

static inline guint extract_grrb_from_pfs(guint64 val) {
  return ((val >> 18) & 0x7f);
}

static inline guint extract_prrb_from_pfs(guint64 val) {
  return ((val >> 32) & 0x3f);
}

static inline int instruction_has_effect_when_qp0(TMT_ISA isa,
						  TmtOper *oper) {
  gboolean ans;
  gboolean is_branch;
  gboolean is_direct_branch;
  gboolean is_indirect_branch;
  gboolean is_call;
  gboolean is_return;
  gboolean is_memory;
  gboolean is_stack_read;
  gboolean is_stack_write;
  gboolean is_predicated;
  gboolean is_var_mem_read;
  gboolean is_var_mem_write;
  gboolean is_sys;
  gboolean sys_no; 
  
  ans=oper->qp; // Always 1 for non-predicated architectures

  if (isa == x86_isa) {
    // todo: are there any predicated ops for x86?
    // todo: remove once x86 traces are fixed
    return (1);
  } else if (isa == IA64_isa) {
    tmt_static_oper_get_attr( oper->soper->instr_attr,
			      &is_branch,
			      &is_direct_branch,
			      &is_indirect_branch,
			      &is_call,
			      &is_return,
			      &is_memory,
			      &is_stack_read,
			      &is_stack_write,
			      &is_predicated,
			      &is_var_mem_read,
			      &is_var_mem_write,
			      &is_sys);

    if(is_branch || is_call || is_return || is_sys) {
      return 1;
    } 

    switch(((TmtTypedStaticOper *)oper->soper)->type) {
    case IA64_RFI:
      return 1;
    case IA64_CMP:
    case IA64_CMP4:
    case IA64_TBIT:
    case IA64_TNAT:
    case IA64_FCMP:
    case IA64_FCLASS:
    case IA64_FRCPA:
    case IA64_FPRCPA:
    case IA64_FRSQRTA:
    case IA64_FPRSQRTA:
      if(((TmtTypedStaticOper *)oper->soper)->ia64_cmp_type ==
	 IA64_CMP_UNC) {
	return 1;
      }
    }
  }
  
  return ans;
}

void ia64_update_renameinfo(TMTFILE *ia64_stfp, IA64RegStackInfo * runtime, TmtOper * oper);
guint ia64_translate_regs(IA64RegStackInfo * runtime, guint reg);
void ia64_init_regstackinfo(IA64RegStackInfo *runtime);

#ifdef __cplusplus
}
#endif

#endif  /* _TMT_IA64_H_ */
