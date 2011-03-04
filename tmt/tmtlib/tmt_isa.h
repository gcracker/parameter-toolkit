#ifndef _TMT_ISA_H_
#define _TMT_ISA_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef enum { PPC_isa, IA64_isa, x86_isa } TMT_ISA;

#include "tmtoper_x86.h"
#include "tmtoper_ia64.h"
#include "tmtoper_ppc64.h"

#include "tmt_ia64.h"

static inline int instruction_is_nop(TMT_ISA isa,
				     TmtOper *oper)
{
  if(isa == x86_isa) {
    switch(((TmtTypedStaticOper *)oper->soper)->type) {
    case X86_NOP:
      return 1;
    default:
      return 0;
    }
  }

  if(isa == IA64_isa) {
    switch(((TmtTypedStaticOper *)oper->soper)->type) {
    case IA64_NOP:
      return 1;
    default:
      return 0;
    }
  }

  if(isa == PPC_isa) {
    switch(((TmtTypedStaticOper *)oper->soper)->type) {
    case PPC64_NOP:
      return 1;
    default:
      return 0;
    }
  }
  return 0;
}

#ifdef __cplusplus
}
#endif

#endif /* _TMT_ISA_H_ */

