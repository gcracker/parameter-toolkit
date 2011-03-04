#ifdef __cplusplus
extern "C" {
#endif
#ifndef TMT_INTERNAL_H
#define TMT_INTERNAL_H

void
tmt_readcontext_build_oper_table_old(GArray *table, TMTFILE * prog_fp);
void
tmt_readcontext_build_oper_table(TmtReadContext *ctxt, TMTFILE * prog_fp);
void
tmt_writecontext_write_oper_table( TmtWriteContext * ctxt );

#endif /* TMT_INTERNAL_H */
#ifdef __cplusplus
}
#endif
