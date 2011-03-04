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

#ifndef __ADAMANT_DD_HOT_H__
#define __ADAMANT_DD_HOT_H__

#include <glib.h>
#include "adamant_dd_types.h"

G_BEGIN_DECLS


adamantHotManager * adamant_hot_init(void);
void adamant_hot_setTop(adamantHotManager * manager, guint64 sin, guint64 value);
guint64 adamant_hot_getTopValue(adamantHotManager * manager);
guint64 adamant_hot_getTopSin(adamantHotManager * manager);
void adamant_hot_setBottom(adamantHotManager * manager, guint64 sin, guint64 value);
guint64 adamant_hot_getBottomValue(adamantHotManager * manager);
guint64 adamant_hot_getBottomSin(adamantHotManager * manager);
guint64 adamant_hot_sinLookup( adamantHotManager * manager, guint64 sin);
void adamant_hot_sinInc(adamantHotManager * manager, guint64 sin);
void adamant_hot_free(adamantHotManager * manager);
void adamant_hot_buffer_close(adamantHotManager * manager);
void adamant_hot_buffer_writebuffer(adamantHotManager * manager, adamantHotBuffer * member);
int adamant_hot_buffer_readbuffer(adamantHotManager * manager, adamantHotBuffer * member);
void adamant_hot_buffer_setfile(adamantHotManager * manager, const gchar * filename);
void adamant_hot_bufferInit(adamantHotManager * manager);
void adamant_hot_buffer_writetuple(adamantHotManager * manager, guint64 x, guint64 y);
guint64 adamant_hot_sinLookupInc(adamantHotManager * manager, guint64 sin);
void adamant_hot_buffer_write2read(adamantHotManager * manager);
G_END_DECLS

#endif
