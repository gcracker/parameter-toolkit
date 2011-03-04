#ifdef __cplusplus
extern "C" {
#endif
#ifndef TMT_H
#define TMT_H

/*
 * Header dependencies
 */
#include <glib.h>
#include "tmtio.h"
#include "tmtoper.h"

G_BEGIN_DECLS

/*
 * Defines
 */
#define TMT_WRITECONTEXT_FILESIZE_CHECK_PERIOD 1000000
#define TMT_WRITECONTEXT_TRACE_FILE_MAX (G_MAXUINT32 >> 2)

/*
 * Type declarations
 */
typedef struct _TmtWriteContext TmtWriteContext;
typedef struct _TmtReadContext TmtReadContext;

/*
 * Type definitions
 */
struct _TmtWriteContext {
  TMTFILE    * trace_fp;
  TMTFILE    * prog_fp;
  GHashTable * ip_hashtable;
  GMemChunk  * ip_memchunk;
  GArray     * oper_table;
  guint32      oper_table_size;
  guint32      oper_table_written;
  guint32      trace_file_no;
  guint64      trace_file_max;
  guint64      trace_file_size;
  gchar      * trace_file_prefix;
};

struct _TmtReadContext {
  TMTFILE    * trace_fp;
  GArray     * oper_table;
  guint32      oper_table_size;
  gchar      * path;
};

#include "tmt_isa.h"

TmtWriteContext *
tmt_writecontext_new( gchar * trace_filename, gchar * program_filename );

void
tmt_writecontext_free( TmtWriteContext * ctxt );

void
tmt_writecontext_write_static( TmtWriteContext * ctxt,
			       TmtStaticOper * soper );

void
tmt_writecontext_write_dynamic( TmtWriteContext * ctxt,
				guint64 ip,
				gboolean qp,
				gboolean taken,
				gboolean is_syscall,
				guint64 * mem_src,
				guint64 * mem_dst,
				guint32 var_mem_src_size,
				guint32 var_mem_dst_size,
				guint32 sysno );

TmtReadContext *
tmt_readcontext_new( gchar * trace_filename, gchar * program_filename );

void
tmt_readcontext_free( TmtReadContext * ctxt );

void
tmt_readcontext_read( gint * tmterror, 
		      TmtReadContext * ctxt,
		      TmtOper * oper );

TMTFILE * tmt_ia64sidetrace_read_open(gchar *filename);
void tmt_ia64sidetrace_read(gint *tmterror, TMTFILE *tmtfp, 
			    TmtIA64SideTrace * st);
void tmt_ia64sidetrace_read_close(gint *tmterror, TMTFILE *tmtfp);

G_END_DECLS

#endif /* TMT_H */
#ifdef __cplusplus
}
#endif
