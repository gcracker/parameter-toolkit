#ifdef __cplusplus
extern "C" {
#endif
#ifndef TMT_OPER_X86_H
#define TMT_OPER_X86_H

typedef enum  {
  X86_OTHER = 0, /* This should always be 0, since 0 also is used for
		    no type */
  X86_NOP
} X86_operation_type;

void tmt_x86_readcontext_type_opers(TmtReadContext * ctxt);

#endif /* TMT_OPER_X86_H */
#ifdef __cplusplus
}
#endif
