/*#*STARTLICENCE*#
Copyright (c) 2005-2009, Regents of the University of Colorado

All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.

Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.

Neither the name of the University of Colorado nor the names of its
contributors may be used to endorse or promote products derived from
this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
#*ENDLICENCE*#*/


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
	ERROR 	= 0,
	FULL 	= 1,
	SAMPLE 	= 2,
	REG	= 3,
	MEM	= 4
}run_mode;

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
	
}MemInfo;

Tla_config * Tla_Init(gchar * fileName, TlaOper * oper);

void Tla_free(Tla_config * tla_config);

void Tla_Read(Tla_config * tla_config, TlaOper * oper);

MemInfo * memInfo_new();

void Usage(gchar* pname);
guint Check_usage(int argc, gchar * argv[]);


#endif /* _TLAIO_H */


