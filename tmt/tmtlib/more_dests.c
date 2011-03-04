#ifdef __cplusplus
extern "C" {
#endif
#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tmt.h"
#include "tmt_internal.h"

int main(int argc, char *argv[])
{
  TMTFILE * prog_fp;
  TmtStaticOper soper;
  gint tmterror;

  TmtWriteContext wctxt;
  int i;
  char *bak;

  // create the oper table
  GArray *table = g_array_new(FALSE, TRUE, sizeof(TmtStaticOperOld));

  prog_fp = tmt_read_open(&tmterror, argv[1]);
  tmt_readcontext_build_oper_table_old(table, prog_fp);
  tmt_read_close(NULL, prog_fp);

  bak = malloc(strlen(argv[1]) + 5);
  strcpy(bak, argv[1]);
  strcat(bak, ".bak2");
  rename(argv[1], bak);

  wctxt.oper_table_written = 0;
  wctxt.oper_table_size = table->len;
  wctxt.oper_table = g_array_new(FALSE, TRUE, sizeof(TmtStaticOper));
  for(i = 0; i < wctxt.oper_table_size; i++) {
    TmtStaticOperOld *soperold = 
      &g_array_index(table, TmtStaticOperOld, i);
    soper.ip = soperold->ip;
    memcpy(soper.reg_src, soperold->reg_src, TMT_MAX_REG_SRC*sizeof(guint16));
    memcpy(soper.reg_dst, soperold->reg_dst, TMT_MAX_REG_DST_OLD*sizeof(guint16));
    soper.instr_category = soperold->instr_category;
    soper.instr_attr = soperold->instr_attr;
    soper.instr_size = soperold->instr_size;
    soper.num_reg_src = soperold->num_reg_src;
    soper.num_reg_dst = soperold->num_reg_dst;
    soper.num_mem_src = soperold->num_mem_src;
    soper.num_mem_dst = soperold->num_mem_dst;
    memcpy(soper.mem_src_size, soperold->mem_src_size, TMT_MAX_MEM_SRC*sizeof(guint32));
    memcpy(soper.mem_dst_size, soperold->mem_dst_size, TMT_MAX_MEM_DST*sizeof(guint32));
    memcpy(soper.opcode, soperold->opcode, TMT_MAX_OPCODE_LEN*sizeof(gchar));

    g_array_append_vals(wctxt.oper_table, &soper, 1);
  }
  wctxt.prog_fp = tmt_write_open(&tmterror, argv[1]);
  tmt_writecontext_write_oper_table(&wctxt);
  tmt_write_close(NULL, wctxt.prog_fp);

  return 0;
}
#ifdef __cplusplus
}
#endif
