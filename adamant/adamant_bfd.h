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

// bfdwork.h
// 
  
#ifndef _BFDWORK_H 
#define _BFDWORK_H 

#include <glib.h>

//! Other Includes
#include "bfd.h"

typedef struct _addrSecStruct {
    bfd_vma pc;
    const char *functionname;
    const char *filename;
    unsigned int line;
    gboolean found;
    asymbol **syms;		/* Symbol table.  */
} addrSecStruct;


//! ** variables **
GString * g_BFDFile;
bfd * g_BFD;
char * g_bfdTarget;    
addrSecStruct findAddrSection;    

//! ** function prototypes **
void adamant_bfd_init(GString * bfdfile);
GString * adamant_bfd_sinInfo(guint64 sin_num);

//! ** these were derived from the GNU addr2line source
void adamant_bfd_slurp_symtab (bfd *);

GString * adamant_bfd_translate_addresses (bfd *);
GString * adamant_bfd_process_file (bfd *);

#endif 


