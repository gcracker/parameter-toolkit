#ifndef _TLAIO_H_
#define _TLAIO_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "tmt.h"
#include "bzlib.h"
#include "bst.h"
#include <pagetable64.h>

#define TLA_MAX_REG_SRC 9
#define TLA_MAX_REG_DST 16

#define TLA_MAX_MEM_SRC 2
#define TLA_MAX_MEM_DST 1

typedef struct
{
	FILE * pFile;
	BZFILE * pBZ2File;
	int buffer_size;
	int  bz2error;
	gboolean config_error;
}Tla_config;

enum
{
        ERROR 	= 1, /* Dump an error log file of the execution */
	SAMPLE 	= 2, /* Analyse only a suffix of the trace (prefix of the reverse trace */
	REGL	= 4, /* Do register liveness only */
	MEM	= 8,  /* Do memory liveness only */
	FULL 	= MEM | REGL /* Do memory and register liveness */
} run_mode;

typedef struct
{
	guint64  ip;
        guint16  reg_src[TLA_MAX_REG_SRC];
        guint16  reg_dst[TLA_MAX_REG_DST];
        guint16  instr_category;
        guint16  instr_attr;
        guint8   instr_size;
        guint8   num_reg_src;
        guint8   num_reg_dst;
        guint8   num_mem_src;
        guint8   num_mem_dst;
        guint32  mem_src_size[TLA_MAX_MEM_SRC];
        guint32  mem_dst_size[TLA_MAX_MEM_DST];
        guint64  mem_src[TLA_MAX_MEM_SRC];
        guint64  mem_dst[TLA_MAX_MEM_DST];

}TlaOper;

typedef struct
{
	guint16 isActive;
	
} MemInfo;

Tla_config * Tla_Init(gchar * fileName, TlaOper * oper);

void Tla_free(Tla_config * tla_config);

void Tla_Read(Tla_config * tla_config, TlaOper * oper);

MemInfo * memInfo_new();

void Usage(gchar* pname);
guint Check_usage(int argc, gchar * argv[]);

#endif /* _TLAIO_H */
