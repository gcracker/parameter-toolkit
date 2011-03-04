#ifdef __cplusplus
extern "C" {
#endif
/*****************************************************************************
 *
 ****************************************************************************/
#ifndef TMT_OPER_H
#define TMT_OPER_H

#include <glib.h>

G_BEGIN_DECLS

/*
 * Defines
 */

/* TODO: 
  IA64 max reg src 4
       max reg dst 3
  X86  max reg src 6 (9)
       max reg dst 3 (7)
*/
#define TMT_MAX_REG_SRC 9
#define TMT_MAX_REG_DST 16
#define TMT_MAX_REG_DST_OLD 7

// TCgen changes will be required if TMT_MAX_MEM_SRC or
// TMT_MAX_MEM_DST change from 2,1;
#define TMT_MAX_MEM_SRC 2 
#define TMT_MAX_MEM_DST 1 
#define TMT_MAX_OPCODE_LEN 32 
#define TMT_MAX_SYSNO 2
#define TMT_TRACE_EOF_INSTR_CATEGORY G_MAXUINT16

/*
 * Static Instruction Attribute Flags
 */
enum
{
  
  TMT_BRANCH          = 0,
  TMT_BRANCH_DIRECT   = 1,
  TMT_BRANCH_INDIRECT = 2,
  TMT_BRANCH_CALL     = 3,
  TMT_BRANCH_RETURN   = 4,
  TMT_MEMORY          = 5,
  TMT_STACK_READ      = 6,
  TMT_STACK_WRITE     = 7,
  TMT_PREDICATED      = 8,
  TMT_VAR_MEM_READ    = 9,
  TMT_VAR_MEM_WRITE   = 10,
  TMT_TRACE_EOF       = 11,
  TMT_SYSCALL         = 12
};

/*
 * Dynamic Instruction Attribute Flags
 */
enum
{
  TMT_PREDICATE_VALUE = 0,
  TMT_BRANCH_TAKEN    = 1,
   TMT_IS_SYSCALL      = 3
};

#define tmt_flag_get_value(flag, bit_index) \
  (((flag) & (1 << (bit_index))) >> (bit_index))

#define tmt_flag_set_value(flag, bit_index, value) \
  ((flag) | ((value != 0) << (bit_index)))

typedef struct _TmtStaticOper TmtStaticOper;
struct _TmtStaticOper {
  guint64  ip;
  guint16  reg_src[TMT_MAX_REG_SRC];
  guint16  reg_dst[TMT_MAX_REG_DST];
  guint16  instr_category;
  guint16  instr_attr;
  guint8   instr_size;
  guint8   num_reg_src;
  guint8   num_reg_dst;
  guint8   num_mem_src;
  guint8   num_mem_dst;
  guint32  mem_src_size[TMT_MAX_MEM_SRC];
  guint32  mem_dst_size[TMT_MAX_MEM_DST];
  gchar    opcode[TMT_MAX_OPCODE_LEN];
} __attribute__((__packed__));

/* This structure exists so that a conversion program can be written
   converting traces with only TMT_MAX_REG_DST_OLD dest registers to
   ones with TMT_MAX_REG_DST registers.  It is assumed that
   TMT_MAX_REG_DST > TMT_MAX_REG_DST_OLD. */
typedef struct _TmtStaticOperOld TmtStaticOperOld;
struct _TmtStaticOperOld {
  guint64  ip;
  guint16  reg_src[TMT_MAX_REG_SRC];
  guint16  reg_dst[TMT_MAX_REG_DST_OLD];
  guint16  instr_category;
  guint16  instr_attr;
  guint8   instr_size;
  guint8   num_reg_src;
  guint8   num_reg_dst;
  guint8   num_mem_src;
  guint8   num_mem_dst;
  guint32  mem_src_size[TMT_MAX_MEM_SRC];
  guint32  mem_dst_size[TMT_MAX_MEM_DST];
  gchar    opcode[TMT_MAX_OPCODE_LEN];
} __attribute__((__packed__));

/* This struct is "inheriting" from TmtStaticOper to add a type field.
   Currently, it is used for IA64 to identify certain special instructions when
   reading a side-trace */
typedef struct _TmtTypedStaticOper TmtTypedStaticOper;
struct _TmtTypedStaticOper {
  TmtStaticOper soper;
  guint32       type;
  guint32       ia64_cmp_type; /* OUCH, this IA64ism here is UGLY!! */
  guint64       *window_count; /* You think that's ugly, look at this
				  adamantium specific stuff here! Keep
				  count of which window this soper was
				  scheduled into in multi-window
				  runs*/
};

typedef struct _TmtOper TmtOper;
struct _TmtOper {
  TmtStaticOper *soper;
  guint32  mem_src_size[TMT_MAX_MEM_SRC];
  guint32  mem_dst_size[TMT_MAX_MEM_DST];
  gboolean qp;
  gboolean taken;
  gboolean is_syscall;
  guint64  mem_src[TMT_MAX_MEM_SRC];
  guint64  mem_dst[TMT_MAX_MEM_DST];
  guint32 sysno;
};

typedef struct _TmtIA64SideTrace TmtIA64SideTrace;
struct _TmtIA64SideTrace {
  guint64 ip;
  guint64 cfmpfs;  /* CFM or PFS depending upon instr type */
  guint64 imm;     /* Immediate field of alloc instruction */
  guint64 lc;      /* Loop counter */
  guint64 ec;      /* Epilog counter */
  guint64 misc[4]; /* Dummy info */
} __attribute__((__packed__));

/* Old stuff */
/* typedef struct _TmtOper TmtOper; */
/* struct _TmtOper { */
/*   guint64  ip; */
/*   guint16  reg_src[TMT_MAX_REG_SRC]; */
/*   guint16  reg_dst[TMT_MAX_REG_DST]; */
/*   guint16  instr_category; */
/*   guint16  instr_attr; */
/*   guint8   instr_size; */
/*   guint8   num_reg_src; */
/*   guint8   num_reg_dst; */
/*   guint8   num_mem_src; */
/*   guint8   num_mem_dst; */
/*   guint32  mem_src_size[TMT_MAX_MEM_SRC]; */
/*   guint32  mem_dst_size[TMT_MAX_MEM_DST]; */
/*   gchar    opcode[TMT_MAX_OPCODE_LEN]; */
/*   gboolean qp; */
/*   gboolean taken; */
/*   guint64  mem_src[TMT_MAX_MEM_SRC]; */
/*   guint64  mem_dst[TMT_MAX_MEM_DST]; */
/* }; */

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
			  gboolean is_sys
			 );

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
			  gboolean * is_sys
			 );

void
tmt_static_oper_set_opcode( TmtStaticOper * soper,
			    gchar * opcode );
  
  guint32
  tmt_dynamic_oper_set_attr( gboolean qp,
			     gboolean taken ,
			     gboolean syscall);

void
tmt_dynamic_oper_get_attr( guint32 attr,
			   gboolean * qp,
			   gboolean * taken ,
			   gboolean *syscall);

void
tmt_oper_build( TmtOper * oper,
		TmtStaticOper * soper );

G_END_DECLS

#endif /* TMT_OPER_H  */
#ifdef __cplusplus
}
#endif
