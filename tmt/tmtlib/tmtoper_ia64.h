#ifdef __cplusplus
extern "C" {
#endif
#ifndef TMT_OPER_IA64_H
#define TMT_OPER_IA64_H

typedef enum  {
  IA64_OTHER = 0, /* This should always be 0, since 0 also is used for
		     no type */
  IA64_ALLOC,
  IA64_CLRRRB,
  IA64_CLRRRB_PR,
  IA64_BR_CALL,
  IA64_BRL_CALL,
  IA64_COVER,
  IA64_BR_RET,
  IA64_RFI,
  IA64_BR_CTOP,
  IA64_BR_CEXIT,
  IA64_BR_WTOP,
  IA64_BR_WEXIT,

  IA64_CMP,
  IA64_CMP4,
  IA64_TBIT,
  IA64_TNAT,
  IA64_FCMP,
  IA64_FCLASS,
  IA64_FRCPA,
  IA64_FPRCPA,
  IA64_FRSQRTA,
  IA64_FPRSQRTA,

  IA64_NOP,
  IA64_SYS
} IA64_operation_type;

typedef enum {
  IA64_CMP_OTHER=0,
  IA64_CMP_UNC
} IA64_cmp_type;

void tmt_ia64_readcontext_type_opers(TmtReadContext * ctxt);

#endif /* TMT_OPER_IA64_H */
#ifdef __cplusplus
}
#endif
