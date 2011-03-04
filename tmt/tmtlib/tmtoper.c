#ifdef __cplusplus
extern "C" {
#endif
/*****************************************************************************
 *
 ****************************************************************************/
#include <string.h>
#include "tmtoper.h"

guint16
tmt_static_oper_set_attr( gboolean is_branch,
			  gboolean is_direct_branch,
			  gboolean is_indirect_branch,
			  gboolean is_call,
			  gboolean is_return,
			  gboolean is_memory,
			  gboolean is_stack_read,
			  gboolean is_stack_write,
			  gboolean is_predicated,
			  gboolean is_var_mem_read,
			  gboolean is_var_mem_write,
			  gboolean is_sys )
{
  guint16 attr = 0;
  attr = tmt_flag_set_value(attr, TMT_BRANCH, is_branch);
  attr = tmt_flag_set_value(attr, TMT_BRANCH_DIRECT, is_direct_branch);
  attr = tmt_flag_set_value(attr, TMT_BRANCH_INDIRECT, is_indirect_branch);
  attr = tmt_flag_set_value(attr, TMT_BRANCH_CALL, is_call);
  attr = tmt_flag_set_value(attr, TMT_BRANCH_RETURN, is_return);
  attr = tmt_flag_set_value(attr, TMT_MEMORY, is_memory);
  attr = tmt_flag_set_value(attr, TMT_STACK_READ, is_stack_read);
  attr = tmt_flag_set_value(attr, TMT_STACK_WRITE, is_stack_write);
  attr = tmt_flag_set_value(attr, TMT_PREDICATED, is_predicated);
  attr = tmt_flag_set_value(attr, TMT_VAR_MEM_READ, is_var_mem_read);
  attr = tmt_flag_set_value(attr, TMT_VAR_MEM_WRITE, is_var_mem_write);
  attr = tmt_flag_set_value(attr, TMT_SYSCALL, is_sys);
  return (attr);
}

void
tmt_static_oper_get_attr( guint16 attr,
			  gboolean * is_branch,
			  gboolean * is_direct_branch,
			  gboolean * is_indirect_branch,
			  gboolean * is_call,
			  gboolean * is_return,
			  gboolean * is_memory,
			  gboolean * is_stack_read,
			  gboolean * is_stack_write,
			  gboolean * is_predicated,
			  gboolean * is_var_mem_read,
			  gboolean * is_var_mem_write,
			  gboolean * is_sys)
{
  *is_branch = tmt_flag_get_value(attr,TMT_BRANCH);
  *is_direct_branch = tmt_flag_get_value(attr,TMT_BRANCH_DIRECT);
  *is_indirect_branch = tmt_flag_get_value(attr,TMT_BRANCH_INDIRECT);
  *is_call = tmt_flag_get_value(attr,TMT_BRANCH_CALL);
  *is_return = tmt_flag_get_value(attr,TMT_BRANCH_RETURN);
  *is_memory = tmt_flag_get_value(attr,TMT_MEMORY);
  *is_stack_read = tmt_flag_get_value(attr,TMT_STACK_READ);
  *is_stack_write = tmt_flag_get_value(attr,TMT_STACK_WRITE);
  *is_predicated = tmt_flag_get_value(attr,TMT_PREDICATED);
  *is_var_mem_read = tmt_flag_get_value(attr,TMT_VAR_MEM_READ);
  *is_var_mem_write = tmt_flag_get_value(attr,TMT_VAR_MEM_WRITE);
  *is_sys = tmt_flag_get_value(attr,TMT_SYSCALL);
}

void
tmt_static_oper_set_opcode( TmtStaticOper * soper,
			    gchar * opcode )
{
  g_assert(soper && opcode);
  strncpy(soper->opcode, opcode, TMT_MAX_OPCODE_LEN);
  soper->opcode[TMT_MAX_OPCODE_LEN-1] = '\0';
}

guint32
tmt_dynamic_oper_set_attr( gboolean qp,
			   gboolean taken,gboolean is_syscall )
{
  guint32 attr = 0;
  attr = tmt_flag_set_value(attr, TMT_PREDICATE_VALUE, qp);
  attr = tmt_flag_set_value(attr, TMT_BRANCH_TAKEN, taken);
  attr = tmt_flag_set_value(attr, TMT_IS_SYSCALL, is_syscall);
  return (attr);
}

void
tmt_dynamic_oper_get_attr( guint32 attr,
			   gboolean * qp,
			   gboolean * taken,gboolean *is_syscall )
{
  *qp = tmt_flag_get_value(attr, TMT_PREDICATE_VALUE);
  *taken = tmt_flag_get_value(attr, TMT_BRANCH_TAKEN);
  *is_syscall = tmt_flag_get_value(attr, TMT_IS_SYSCALL);
}

void
tmt_oper_build( TmtOper * oper,
		TmtStaticOper * soper )
{
  //  guint i;

  g_assert(oper && soper);
  memset(oper, 0, sizeof(TmtOper));

  oper->soper = soper;
  //  oper->ip = soper->ip;
  //oper->instr_category = soper->instr_category;
  //oper->instr_attr = soper->instr_attr;
  //oper->instr_size = soper->instr_size;
  //oper->num_reg_src = soper->num_reg_src;
  //oper->num_reg_dst = soper->num_reg_dst;
  //oper->num_mem_src = soper->num_mem_src;
  //oper->num_mem_dst = soper->num_mem_dst;
  //strcpy(oper->opcode, soper->opcode);

  g_assert(soper->num_reg_src<=TMT_MAX_REG_SRC);
  //for ( i=0; i<oper->num_reg_src; ++i ) {
  //  oper->reg_src[i] = soper->reg_src[i];
  //}
  g_assert(soper->num_reg_dst<=TMT_MAX_REG_DST);
  //for ( i=0; i<oper->num_reg_dst; ++i ) {
  //  oper->reg_dst[i] = soper->reg_dst[i];
  //}
  g_assert(soper->num_mem_src<=TMT_MAX_MEM_SRC);
  memcpy(oper->mem_src_size,soper->mem_src_size,
	 sizeof(guint32)*soper->num_mem_src);
  g_assert(soper->num_mem_dst<=TMT_MAX_MEM_DST);
  memcpy(oper->mem_dst_size,soper->mem_dst_size,
	 sizeof(guint32)*soper->num_mem_dst);
  
}

/* Old stuff */
/* void */
/* tmt_oper_build( TmtOper * oper, */
/* 		TmtStaticOper * soper ) */
/* { */
/*   guint i; */

/*   g_assert(oper && soper); */
/*   memset(oper, 0, sizeof(TmtOper)); */

/*   oper->ip = soper->ip; */
/*   oper->instr_category = soper->instr_category; */
/*   oper->instr_attr = soper->instr_attr; */
/*   oper->instr_size = soper->instr_size; */
/*   oper->num_reg_src = soper->num_reg_src; */
/*   oper->num_reg_dst = soper->num_reg_dst; */
/*   oper->num_mem_src = soper->num_mem_src; */
/*   oper->num_mem_dst = soper->num_mem_dst; */
/*   strcpy(oper->opcode, soper->opcode); */

/*   g_assert(oper->num_reg_src<=TMT_MAX_REG_SRC); */
/*   for ( i=0; i<oper->num_reg_src; ++i ) { */
/*     oper->reg_src[i] = soper->reg_src[i]; */
/*   } */
/*   g_assert(oper->num_reg_dst<=TMT_MAX_REG_DST); */
/*   for ( i=0; i<oper->num_reg_dst; ++i ) { */
/*     oper->reg_dst[i] = soper->reg_dst[i]; */
/*   } */
/*   g_assert(oper->num_mem_src<=TMT_MAX_MEM_SRC); */
/*   for ( i=0; i<oper->num_mem_src; ++i ) { */
/*     oper->mem_src_size[i] = soper->mem_src_size[i]; */
/*   } */
/*   g_assert(oper->num_mem_dst<=TMT_MAX_MEM_DST); */
/*   for ( i=0; i<oper->num_mem_dst; ++i ) { */
/*     oper->mem_dst_size[i] = soper->mem_dst_size[i]; */
/*   } */
/* } */
#ifdef __cplusplus
}
#endif
