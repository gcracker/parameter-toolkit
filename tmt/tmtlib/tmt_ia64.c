#include "tmt.h"
#include "tmt_isa.h"

#include <stdlib.h>

void ia64_update_renameinfo(TMTFILE *ia64_stfp, IA64RegStackInfo * runtime, TmtOper * oper)
{
  TmtIA64SideTrace st;
  gint tmterr;
  gboolean qp;
  guint loc;
  guint sor;

  qp=oper->qp;

  switch(((TmtTypedStaticOper *)(oper->soper))->type) {
  case IA64_ALLOC:
  case IA64_BR_CALL:
  case IA64_BRL_CALL:
  case IA64_BR_RET:
  case IA64_CLRRRB:
  case IA64_CLRRRB_PR:
  case IA64_BR_CTOP:
  case IA64_BR_CEXIT:
  case IA64_BR_WTOP:
  case IA64_BR_WEXIT:
    tmt_ia64sidetrace_read(&tmterr, ia64_stfp, &st);

    if(tmterr == TMTIO_ERROR) {
      printf("**ERROR**: Error reading sidetrace file\n");
      exit(-1);
    }
    if(oper->soper->ip != st.ip) {
      printf("**ERROR**:"
	     " Sidetrace IP=%llx does not match main trace IP=%llx, opc=%s\n",
	     st.ip, oper->soper->ip, oper->soper->opcode);
      exit(-1);
    }
    break;
  case IA64_COVER:
    printf("**ERROR**: Sorry, not sure how to handle cover\n");
    exit(-1);
    break;
  case IA64_RFI:
    printf("**ERROR**: Sorry, not sure how to handle rfi\n");
    exit(-1);
    break;
  default:
    return;
  }

  switch(((TmtTypedStaticOper *)(oper->soper))->type) {
  case IA64_ALLOC:
    runtime->cfm.sor = extract_sor_from_imm(st.imm);
    break;
  case IA64_BR_CALL:
  case IA64_BRL_CALL:
    if(qp) {
      loc=extract_sol_from_cfm(st.cfmpfs);
      g_assert(runtime->framebase < (0xffffffff - 128 - loc));
      runtime->framebase += loc;
      runtime->cfm.frrb=0;
      runtime->cfm.prrb=0;
      runtime->cfm.grrb=0;
    }
    break;
  case IA64_BR_RET:
    if(qp) {
      runtime->framebase -= extract_sol_from_pfs(st.cfmpfs);
      runtime->cfm.sor = extract_sor_from_pfs(st.cfmpfs);
      runtime->cfm.frrb = extract_frrb_from_pfs(st.cfmpfs);
      runtime->cfm.grrb = extract_grrb_from_pfs(st.cfmpfs);
      runtime->cfm.prrb = extract_prrb_from_pfs(st.cfmpfs);
      g_assert(runtime->framebase >= IA64_RENBASE);
    }
    break;
  case IA64_CLRRRB:
    if(qp) {
      runtime->cfm.frrb=0;
      runtime->cfm.prrb=0;
      runtime->cfm.grrb=0;
    }
    break;
  case IA64_CLRRRB_PR:
    if(qp)
      runtime->cfm.prrb=0;
    break;
  case IA64_BR_CTOP:
  case IA64_BR_CEXIT:
    sor = extract_sor_from_cfm(st.cfmpfs);
    if(sor != runtime->cfm.sor) {
      printf("**ERROR**: IP=%llx (opc=%s), cfm=%llx, sor (%d) != runtime->cfm.sor (%d)\n", 
	     oper->soper->ip, oper->soper->opcode, st.cfmpfs, sor, runtime->cfm.sor);
      exit(-1);
    }
    if(qp) {
      if(st.lc) {
	runtime->cfm.frrb--;
	if(sor)
	  runtime->cfm.grrb = ((runtime->cfm.grrb-1 + sor*8) % sor*8);
	runtime->cfm.prrb--;
      } else if (st.ec > 1) {
	runtime->cfm.frrb--;
	if(sor)
	  runtime->cfm.grrb = ((runtime->cfm.grrb-1 + sor*8) % sor*8);
	runtime->cfm.prrb--;
      } else if (st.ec == 1) {
	runtime->cfm.frrb--;
	if(sor)
	  runtime->cfm.grrb = ((runtime->cfm.grrb-1 + sor*8) % sor*8);
	runtime->cfm.prrb--;
      } else if (st.ec == 0) {
	// Do nothing to RRB
      }
    }
    break;
  case IA64_BR_WTOP:
  case IA64_BR_WEXIT:
    sor = extract_sor_from_cfm(st.cfmpfs);
    if(sor != runtime->cfm.sor) {
      printf("**ERROR**: IP=%llx (opc=%s), cfm=%llx, sor (%d) != runtime->cfm.sor (%d)\n", 
	     oper->soper->ip, oper->soper->opcode, st.cfmpfs, sor, runtime->cfm.sor);
      exit(-1);
    }

    if(qp) {
      runtime->cfm.frrb--;
      if(sor)
	runtime->cfm.grrb = ((runtime->cfm.grrb-1 + sor*8) % sor*8);
      runtime->cfm.prrb--;
    } else if (st.ec > 1) {
      runtime->cfm.frrb--;
      if(sor)
	runtime->cfm.grrb = ((runtime->cfm.grrb-1 + sor*8) % sor*8);
      runtime->cfm.prrb--;
    } else if (st.ec == 1) {
      runtime->cfm.frrb--;
      if(sor)
	runtime->cfm.grrb = ((runtime->cfm.grrb-1 + sor*8) % sor*8);
      runtime->cfm.prrb--;
    } else if (st.ec == 0) {
      // Do nothing to RRB
    }
    break;
  }

  return;
}

guint ia64_translate_regs(IA64RegStackInfo * runtime, guint reg)
{
  int renreg;

  renreg=reg;
  /* Register stack renaming */
  if (REG_is_gr(reg)) {
    if (IA64_GRTOBASE(reg) >= 32) {
      if (runtime->cfm.sor && 
	  (IA64_GRTOBASE(reg) <= (32 + runtime->cfm.sor)))
	renreg = (((reg - 32) + runtime->cfm.grrb) % 
		  (runtime->cfm.sor * 8)) + 32;

      renreg = reg + runtime->framebase;
    }
  } else if(REG_is_fr_rot(reg)) {
    renreg = (((reg - 32) + runtime->cfm.frrb) % 96) + 32;
  } else if(REG_is_pr_rot(reg)) {
    renreg = (((reg - 16) + runtime->cfm.prrb) % 48) + 16;
  }

  return renreg;
}

void ia64_init_regstackinfo(IA64RegStackInfo *runtime)
{
  runtime->framebase = IA64_RENBASE;
  runtime->cfm.frrb =0;
  runtime->cfm.prrb =0;
  runtime->cfm.grrb =0;
  runtime->cfm.sor =0;
}
