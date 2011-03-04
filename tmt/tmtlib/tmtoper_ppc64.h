#ifdef __cplusplus
extern "C" {
#endif
#ifndef TMT_OPER_PPC64_H
#define TMT_OPER_PPC64_H

#define PPC64_R0 96
#define PPC64_R31 (PPC64_R0 + 31)

#define PPC64_is_gp(reg) (((reg) >= PPC64_R0) && ((reg) <= PPC64_R31))

/* Keep this in sync with PPC_operation_type */
static char *PPC64_operation_type_to_name[] = {
  "Other",
  "addi",
  "mov",
  "extend",
  "nop"
};

typedef enum  {
  PPC64_OTHER = 0, /* This should always be 0, since 0 also is used for
		     no type */
  PPC64_ADDI,
  PPC64_MOV,
  PPC64_EXTEND,
  PPC64_NOP,

  PPC64_STORE,
  PPC64_STORE_INDEXED,
  PPC64_STORE_UPDATE,
  PPC64_STORE_INDEXED_UPDATE,

  PPC64_LOAD_ARITHMETIC,
  PPC64_LOAD_ARITHMETIC_INDEXED,
  PPC64_LOAD_ARITHMETIC_UPDATE,
  PPC64_LOAD_ARITHMETIC_INDEXED_UPDATE,
  PPC64_LOAD_ZERO,
  PPC64_LOAD_ZERO_INDEXED,
  PPC64_LOAD_ZERO_UPDATE,
  PPC64_LOAD_ZERO_INDEXED_UPDATE,
  PPC64_LOAD,
  PPC64_LOAD_INDEXED,
  PPC64_LOAD_UPDATE,
  PPC64_LOAD_INDEXED_UPDATE,
} PPC64_operation_type;

void tmt_ppc64_readcontext_type_opers(TmtReadContext * ctxt);

#endif /* TMT_OPER_PPC64_H */
#ifdef __cplusplus
}
#endif
