#ifdef __cplusplus
extern "C" {
#endif
/*****************************************************************************
 *
 ****************************************************************************/
#include <stdio.h>
#include <string.h>
#include "tmthash.h"
#include "tmt.h"
#include "tmtoper_ia64.h"
#include "tmtoper_ppc64.h"
#include "tmtoper_x86.h"
#include "TCgen.h"

#include "tmt_internal.h"

#include "TCgen.c" /* So the functions get inlined */

typedef struct _TmtIpMapEntry TmtIpMapEntry;
struct _TmtIpMapEntry 
{
  guint64 ip;
  guint32 oper_id;
};

static void
tmt_writecontext_open_next_trace_file( TmtWriteContext * ctxt );

static gchar *
tmt_file_remove_extension(gchar * filename);


TmtWriteContext *
tmt_writecontext_new( gchar * trace_filename, gchar * program_filename )
{
  TmtWriteContext * ctxt;
  gint tmterror;

  TCgen_Init();

  // allocate the structure
  ctxt = g_new0(TmtWriteContext,1);

  // trace file parameters
  ctxt->trace_file_max = TMT_WRITECONTEXT_TRACE_FILE_MAX;
  ctxt->trace_file_size = 0;
  ctxt->trace_file_no = 0;
  ctxt->trace_file_prefix = tmt_file_remove_extension(trace_filename);

  // open a file for the trace
  tmt_writecontext_open_next_trace_file(ctxt);

  // open a file for the oper table
  ctxt->prog_fp = tmt_write_open(&tmterror, program_filename);
  if (tmterror == TMTIO_ERROR)
    g_error("Could not open program file");

  // create an ip-to-index mapping hash table
  ctxt->ip_hashtable = g_hash_table_new_full(tmt_uint64_hash,
					     tmt_uint64_equal,
					     NULL,
					     NULL);
  ctxt->ip_memchunk = 
    g_mem_chunk_new("ip chunk",
		    sizeof(TmtIpMapEntry),
		    sizeof(TmtIpMapEntry),
		    G_ALLOC_ONLY);
  // create the oper table
  ctxt->oper_table = g_array_new(FALSE, FALSE, sizeof(TmtStaticOper));
  ctxt->oper_table_size = 0;
  ctxt->oper_table_written = 0;
  
  return (ctxt);
}

void
tmt_writecontext_free( TmtWriteContext * ctxt )
{
  // write the oper table if pending
  tmt_writecontext_write_oper_table(ctxt);
  // close the files
  tmt_write_close(NULL, ctxt->trace_fp);
  tmt_write_close(NULL, ctxt->prog_fp);
  // free trace file name
  g_free(ctxt->trace_file_prefix);
  // free the oper table
  g_array_free(ctxt->oper_table, TRUE);
  // free the mapping hash table
  g_hash_table_destroy(ctxt->ip_hashtable);
  g_mem_chunk_destroy(ctxt->ip_memchunk);
  // free the entire structure
  g_free(ctxt);
}

void
tmt_writecontext_write_static( TmtWriteContext * ctxt,
			       TmtStaticOper * soper )
{
  TmtIpMapEntry * pentry;
  guint32 * poper_id;
  
  // lookup static oper id
  poper_id = (guint32 *) g_hash_table_lookup(ctxt->ip_hashtable, &(soper->ip));
  if (!poper_id) {
    // add to oper table (this copies the structure)
    g_array_append_vals(ctxt->oper_table, soper, 1);
    // create an ip map table entry
    pentry = g_chunk_new(TmtIpMapEntry, ctxt->ip_memchunk);
    pentry->ip = soper->ip;
    pentry->oper_id = ctxt->oper_table_size++;
    // add to map table
    g_hash_table_insert(ctxt->ip_hashtable, 
			&(pentry->ip), 
			&(pentry->oper_id));
 

 
  } 
}

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
				guint32 sysno )
{
  static guint32 writes_since_last_check = 0;
  gint tmterror;
  guint32 * poper_id;
  guint32 id, attr;
  guint64 ea;
  TmtStaticOper * soper;
  unsigned char code;
  unsigned char buf[128];
  unsigned char *bufp = buf+1;

  // lookup static oper id
  poper_id = (guint32 *) g_hash_table_lookup(ctxt->ip_hashtable, &ip);
  // it must exist or there is an error
  g_assert(poper_id);
  id = *poper_id;
  
  // lookup static oper
  soper = &g_array_index(ctxt->oper_table, TmtStaticOper, id);
  
  // setup dynamic oper id
  code = TCgen_Encode_oper_id(id);
  *bufp++ = code;
  if (code==2) {
   
	  *((guint32*)bufp) = GUINT32_TO_LE(id);
	  bufp += sizeof(guint32);
  }
  
  attr = tmt_dynamic_oper_set_attr(qp, taken, is_syscall);
  code = TCgen_Encode_attr(id, attr);
  *bufp++ = code;
  if (code==1) {
	  *((guint32*)bufp) = GUINT32_TO_LE(attr);
	  bufp += sizeof(guint32);
  }

  if (tmt_flag_get_value(soper->instr_attr, TMT_SYSCALL))
    {
      code = TCgen_Encode_sys(id, sysno);
      
      *bufp++ = code;
      
      if (code==1)
  	{
  	  *((guint32*)bufp) = GUINT32_TO_LE(sysno);
  	  bufp +=sizeof(guint32);
	}
    }

  /*     // now write out syscall memory operations */
  /*     if (NULL != mem_src)  */
  /* 	{ */
  /* 	  ea = mem_src[0]; */
  /* 	  code = TCgen_Encode_src_ea0(id, ea); */
  /* 	  *bufp++ = code; */
  /* 	  if (code==1)  */
  /* 	    { */
  /* 	      *((guint64*)bufp) = GUINT64_TO_LE(ea); */
  /* 	      bufp += sizeof(guint64); */
  /* 	    } */

  /* 	  if(0 < var_mem_src_size) */
  /* 	    { */
  /* 	      code = TCgen_Encode_mem_read_size(id, var_mem_src_size); */
  /* 	      *bufp++ = code; */
  /* 	      if (code==1)  */
  /* 		{ */
  /* 		  *((guint32*)bufp) = GUINT32_TO_LE(var_mem_src_size); */
  /* 		  bufp += sizeof(guint32); */
  /* 		} */
  /* 	    } */
  /* 	} */
  /*     else */
  /* 	{ */
  /* 	  ea = 0; */
  /* 	  code = TCgen_Encode_src_ea0(id, ea); */
  /* 	  *bufp++ = code; */
  /* 	  if (code==1)  */
  /* 	    { */
  /* 	      *((guint64*)bufp) = GUINT64_TO_LE(ea); */
  /* 	      bufp += sizeof(guint64); */
  /* 	    } */
  /* 	} */
  
  /*     if (NULL != mem_dst) */
  /* 	{ */
  /* 	  ea = mem_dst[0]; */
  /* 	  code = TCgen_Encode_dst_ea0(id, ea); */
  /* 	  *bufp++ = code; */
  /* 	  if (code==1)  */
  /* 	    { */
  /* 	      *((guint64*)bufp) = GUINT64_TO_LE(ea); */
  /* 	      bufp += sizeof(guint64); */
  /* 	    } */
	  
  /* 	  if(0 < var_mem_dst_size) */
  /* 	    { */
  /* 	      code = TCgen_Encode_mem_write_size(id, var_mem_dst_size); */
  /* 	      *bufp++ = code; */
  /* 	      if (code==1)  */
  /* 		{ */
  /* 		  *((guint32*)bufp) = GUINT32_TO_LE(var_mem_dst_size); */
  /* 		  bufp += sizeof(guint32); */
  /* 		} */
  /* 	    }      */
  /* 	} */
  /*     else */
  /* 	{ */
  /* 	  ea = 0; */
  /* 	  code = TCgen_Encode_dst_ea0(id, ea); */
  /* 	  *bufp++ = code; */
  /* 	  if (code==1)  */
  /* 	    { */
  /* 	      *((guint64*)bufp) = GUINT64_TO_LE(ea); */
  /* 	      bufp += sizeof(guint64); */
  /* 	    } */
  /* 	} */
  /*   } */

 
  // setup dynamic oper mem effective addresses

  g_assert(soper->num_mem_src <= TMT_MAX_MEM_SRC);
  if (soper->num_mem_src > 0) {
	  ea = mem_src[0];
	  code = TCgen_Encode_src_ea0(id, ea);
	  *bufp++ = code;
	  if (code==1) {
		  *((guint64*)bufp) = GUINT64_TO_LE(ea);
		  bufp += sizeof(guint64);
	  }
  }
  if (soper->num_mem_src > 1) {
	  ea = mem_src[1];
	  code = TCgen_Encode_src_ea1(id, ea);
	  *bufp++ = code;
	  if (code==1) {
		  *((guint64*)bufp) = GUINT64_TO_LE(ea);
		  bufp += sizeof(guint64);
	  }
  }
  
  g_assert(soper->num_mem_dst <= TMT_MAX_MEM_DST);
  if (soper->num_mem_dst > 0) {
	  ea = mem_dst[0];
	  code = TCgen_Encode_dst_ea0(id, ea);
	  *bufp++ = code;
	  if (code==1) {
		  *((guint64*)bufp) = GUINT64_TO_LE(ea);
		  bufp += sizeof(guint64);
	  }
  }

  if (tmt_flag_get_value(soper->instr_attr, TMT_VAR_MEM_READ)) {
	  code = TCgen_Encode_mem_read_size(id, var_mem_src_size);
	  *bufp++ = code;
	  if (code==1) {
		  *((guint32*)bufp) = GUINT32_TO_LE(var_mem_src_size);
		  bufp += sizeof(guint32);
	  }
  }
  if (tmt_flag_get_value(soper->instr_attr, TMT_VAR_MEM_WRITE)) {
	  code = TCgen_Encode_mem_write_size(id, var_mem_dst_size);
	  *bufp++ = code;
	  if (code==1) {
		  *((guint32*)bufp) = GUINT32_TO_LE(var_mem_dst_size);
		  bufp += sizeof(guint32);
	  }
  }




  // write dyn oper to the trace file
  buf[0] = (bufp-buf)-1;
  tmt_write(&tmterror, ctxt->trace_fp, buf, bufp-buf);
  if (tmterror == TMTIO_ERROR)
	  g_error("Could not write eof to trace file");

  // only check trace file size once every 'trace_file_period' writes
  if ( writes_since_last_check >= TMT_WRITECONTEXT_FILESIZE_CHECK_PERIOD ) {
    ctxt->trace_file_size = tmt_io_get_file_size(ctxt->trace_fp);
    // check if trace file size larger than limit
    if (ctxt->trace_file_size >= ctxt->trace_file_max) {
      // open a new trace file and continue
      tmt_writecontext_open_next_trace_file(ctxt);
      ctxt->trace_file_size = 0;
    }
    writes_since_last_check = 0;
  }
  // increment number of writes since last file size check
  writes_since_last_check++;



}

void
tmt_writecontext_write_oper_table( TmtWriteContext * ctxt )
{
  TmtStaticOper * oper;
  guint32 i;
  if ( ctxt->oper_table_written < ctxt->oper_table_size ) {
    for ( i=ctxt->oper_table_written; i<ctxt->oper_table_size; ++i ) {
      oper = &g_array_index(ctxt->oper_table, TmtStaticOper, i);
      tmt_write(NULL, ctxt->prog_fp, oper, sizeof(TmtStaticOper));
    }
    ctxt->oper_table_written = ctxt->oper_table_size;
  }


}

static void
tmt_writecontext_open_next_trace_file( TmtWriteContext * ctxt )
{
  gint tmterror;
  guint32 eof_oper_id;
  TmtStaticOper eof_static_oper;
  gchar * trace_file_fullname;
  unsigned char code;
  unsigned char buf[128];
  unsigned char *bufp = buf+1;

  // build the new trace file name
  if (ctxt->trace_file_no == 0) {
    trace_file_fullname = g_strdup_printf("%s.bz2", 
					  ctxt->trace_file_prefix);
  } else {
    trace_file_fullname = g_strdup_printf("%s_%d.bz2", 
					  ctxt->trace_file_prefix, 
					  ctxt->trace_file_no);
  }
  ctxt->trace_file_no++;

  // error the file name is too big for the opcode
  if ( strlen(trace_file_fullname) > TMT_MAX_OPCODE_LEN ) {
    g_error("File name too large");
  }

  /**
   * If this is not the first file in the trace, we must link the current
   * trace file to the next trace file by using a special static oper id
   * to signify EOF.
   */
  if ((ctxt->trace_fp) && (ctxt->trace_file_size > 0)) {
    // clear static oper
    memset(&eof_static_oper, 0, sizeof(TmtStaticOper));

    // set instr category to special eof category
    eof_static_oper.instr_category = TMT_TRACE_EOF_INSTR_CATEGORY;
    eof_static_oper.instr_attr = 
      tmt_flag_set_value(eof_static_oper.instr_attr, TMT_TRACE_EOF, 1);

    // the opcode field points to the next trace file
    strncpy(eof_static_oper.opcode, trace_file_fullname, 
	    TMT_MAX_OPCODE_LEN);

    // add to oper table (this copies the structure)
    g_array_append_vals(ctxt->oper_table, &eof_static_oper, 1);
    eof_oper_id = ctxt->oper_table_size++;

    // setup dynamic oper id
    code = TCgen_Encode_oper_id(eof_oper_id);
    *bufp++ = code;
    if (code==2) {
      *((guint32*)bufp) = GUINT32_TO_LE(eof_oper_id);
      bufp += sizeof(guint32);
    }
    
    code = TCgen_Encode_attr(eof_oper_id, 0);
    *bufp++ = code;
    if (code==1) {
      *((guint32*)bufp) = 0;
      bufp += sizeof(guint32);
    }
    code = TCgen_Encode_sys(eof_oper_id, 0);
    *bufp++ = code;
    if (code==1) {
      *((guint32*)bufp) = 0;
      bufp += sizeof(guint32);
    }
    // write dyn oper to the trace file
    buf[0] = (bufp-buf)-1;
    tmt_write(&tmterror, ctxt->trace_fp, buf, bufp-buf);
    if (tmterror == TMTIO_ERROR)
      g_error("Could not write eof to trace file");

    // close the trace file
    tmt_write_close(&tmterror, ctxt->trace_fp);
    if (tmterror == TMTIO_ERROR)
      g_error("Could not close trace file");
  }

  // open the trace file
  ctxt->trace_fp = tmt_write_open(&tmterror, trace_file_fullname);
  if (tmterror == TMTIO_ERROR)
    g_error("Could not open trace file");

  // free the trace file string
  g_free(trace_file_fullname);
}

TmtReadContext *
tmt_readcontext_new( gchar * trace_filename, gchar * program_filename )
{
  TmtReadContext * ctxt;
  TMTFILE * prog_fp;
  gint tmterror;

  TCgen_Init();

  // allocate the structure
  ctxt = g_new0(TmtReadContext,1);

  // create the oper table
  ctxt->oper_table = g_array_new(FALSE, TRUE, sizeof(TmtTypedStaticOper));
  ctxt->oper_table_size = 0;

  // get the path to the trace file in case of multiple trace files
  ctxt->path = g_path_get_dirname(trace_filename);

  // open a file for the oper table
  prog_fp = tmt_read_open(&tmterror, program_filename);
  if (tmterror == TMTIO_ERROR) {
    g_array_free(ctxt->oper_table, TRUE);
    g_free(ctxt);    
    return (NULL);
  }
  // read the program file into the oper table
  tmt_readcontext_build_oper_table(ctxt, prog_fp);
  // close the program file
  tmt_read_close(NULL, prog_fp);

  // open a file for the trace
  ctxt->trace_fp = tmt_read_open(&tmterror, trace_filename);
  if (tmterror == TMTIO_ERROR) {
    g_free(ctxt);
    return (NULL);
  }
  return (ctxt);
}

void
tmt_readcontext_free( TmtReadContext * ctxt )
{
  // free strings
  g_free(ctxt->path);
  // free the oper table
  g_array_free(ctxt->oper_table, TRUE);
  // close the trace file
  tmt_read_close(NULL, ctxt->trace_fp);
  // free the entire structure
  g_free(ctxt);  
}

void
tmt_readcontext_read( gint * tmterror, 
		      TmtReadContext * ctxt,
		      TmtOper * oper )
{
  TmtStaticOper * soper;
  gint err;
  gchar * next_trace_file;
  guint32 id=0, attr=0;
  unsigned char code;
  unsigned char buf[64];
  unsigned char *bufp = buf+1;

  // oper must be valid
  g_assert(oper);

  // read dynamic oper from trace file
  tmt_read(&err, ctxt->trace_fp, buf, 1);
  if (err != TMTIO_ERROR) {
	  g_assert(buf[0]<64);
	  tmt_read(&err, ctxt->trace_fp, bufp, buf[0]);
  }
  if (tmterror)
	*tmterror = err;
  if (err == TMTIO_ERROR)
    return;

  code = *bufp++;
  if (code==2) {
	  id = GUINT32_FROM_LE(*((guint32*)bufp));
	  bufp += sizeof(guint32);
  }
  id = TCgen_Decode_oper_id(code, id);

  // lookup static oper
  soper = (TmtStaticOper*)&g_array_index(ctxt->oper_table, 
					 TmtTypedStaticOper, id);

  // build oper from static oper
  tmt_oper_build(oper, soper);

  code = *bufp++;

  if (code==1) {
	  attr = GUINT32_FROM_LE(*((guint32*)bufp));
	  bufp += sizeof(guint32);
  }
  attr = TCgen_Decode_attr(code, id, attr);

  tmt_dynamic_oper_get_attr( attr, &(oper->qp), &(oper->taken),&(oper->is_syscall) );

  // check for end of trace file marker
  if (soper->instr_category == TMT_TRACE_EOF_INSTR_CATEGORY) {
    // close the current trace file
    tmt_read_close(NULL, ctxt->trace_fp);    
    // construct full path to next trace file
    next_trace_file = g_build_filename(ctxt->path, soper->opcode, NULL);
    // open new trace file (filename is in opcode field)
    ctxt->trace_fp = tmt_read_open(&err, next_trace_file);
    g_free(next_trace_file);
    if (tmterror)
      *tmterror = err;
    if (err == TMTIO_ERROR)
      return;
  }


  if (tmt_flag_get_value(soper->instr_attr, TMT_SYSCALL))
    {
      guint32 sys = 0;
      code = *bufp++;
      if (code==1)
  	{
  	  sys = GUINT32_FROM_LE(*((guint32*)bufp));
  	  bufp += sizeof(guint32);
  	}
      oper->sysno = TCgen_Decode_sys(code, id, sys);
    }


  // now read the memory addresses
  g_assert(soper->num_mem_src <= TMT_MAX_MEM_SRC);
  if (soper->num_mem_src > 0) {
    guint64 ea = 0;
    code = *bufp++;
    if (code==1) {
      ea = GUINT64_FROM_LE(*((guint64*)bufp));
      bufp += sizeof(guint64);
    }
    oper->mem_src[0] = TCgen_Decode_src_ea0(code, id, ea);
  }

  if (soper->num_mem_src > 1) {
    guint64 ea = 0;
    code = *bufp++;
    if (code==1) {
      ea = GUINT64_FROM_LE(*((guint64*)bufp));
      bufp += sizeof(guint64);
    }
    oper->mem_src[1] = TCgen_Decode_src_ea1(code, id, ea);
  }
  
  g_assert(soper->num_mem_dst <= TMT_MAX_MEM_DST);
  if (soper->num_mem_dst > 0) {
    guint64 ea = 0;
    code = *bufp++;
    if (code==1) {
      ea = GUINT64_FROM_LE(*((guint64*)bufp));
      bufp += sizeof(guint64);
    }
    oper->mem_dst[0] = TCgen_Decode_dst_ea0(code, id, ea);
  }

  if (tmt_flag_get_value(soper->instr_attr, TMT_VAR_MEM_READ)) {
    gsize i;
    guint32 size = 0;
    code = *bufp++;
    if (code==1) {
      size = GUINT32_FROM_LE(*((guint32*)bufp));
      bufp += sizeof(guint32);
    }
    for (i=0; i<soper->num_mem_src; ++i) {
      oper->mem_src_size[i] = TCgen_Decode_mem_read_size(code, id, size);
    }
  }
  
  if (tmt_flag_get_value(soper->instr_attr, TMT_VAR_MEM_WRITE)) {
    gsize i;
    guint32 size = 0;
    code = *bufp++;
    if (code==1) {
      size = GUINT32_FROM_LE(*((guint32*)bufp));
      bufp += sizeof(guint32);
    }
    for (i=0; i<soper->num_mem_dst; ++i) {
      oper->mem_dst_size[i] = 
	TCgen_Decode_mem_write_size(code, id, size);
    }
  }

  /*     // look up memory source info */
  /*     guint64 src_ea = 0; */
  /*     code = *bufp++; */
  /*     if (code==1)  */
  /* 	{ */
  /* 	  src_ea = GUINT64_FROM_LE(*((guint64*)bufp)); */
  /* 	  bufp += sizeof(guint64); */
  /* 	} */

  /*     if(0 != src_ea) */
  /* 	{ */
  /* 	  oper->mem_src[0] = TCgen_Decode_src_ea0(code, id, src_ea); */

  /* 	  guint32 size = 0; */
  /* 	  code = *bufp++; */
  /* 	  if (code==1)  */
  /* 	    { */
  /* 	      size = GUINT32_FROM_LE(*((guint32*)bufp)); */
  /* 	      bufp += sizeof(guint32); */
  /* 	    } */
  /* 	  oper->mem_src_size[0] = TCgen_Decode_mem_read_size(code, id, size); */
  /* 	} */
  /*     else */
  /* 	{ */
  /* 	  oper->mem_src[0] = 0; */
  /* 	} */

  /*     // look up memory dest info */
  /*     guint64 dest_ea = 0; */
  /*     code = *bufp++; */
  /*     if (code==1)  */
  /* 	{ */
  /* 	  dest_ea = GUINT64_FROM_LE(*((guint64*)bufp)); */
  /* 	  bufp += sizeof(guint64); */
  /* 	} */

  /*     if(0 != dest_ea) */
  /* 	{ */
  /* 	  oper->mem_dst[0] = TCgen_Decode_dst_ea0(code, id, dest_ea); */

  /* 	  guint32 size = 0; */
  /* 	  code = *bufp++; */
  /* 	  if (code==1)  */
  /* 	    { */
  /* 	      size = GUINT32_FROM_LE(*((guint32*)bufp)); */
  /* 	      bufp += sizeof(guint32); */
  /* 	    } */
  /* 	  oper->mem_dst_size[0] = TCgen_Decode_mem_write_size(code, id, size); */
  /* 	} */
  /*     else */
  /* 	{ */
  /* 	  oper->mem_dst[0] = 0; */
  /* 	}       */
  /* } */

  g_assert((bufp-buf)-1 == buf[0]);
}

void
tmt_readcontext_build_oper_table(TmtReadContext * ctxt, TMTFILE * prog_fp)
{
  TmtTypedStaticOper soper;
  gint tmterror;
  gint i;

  while (TRUE) {
    tmt_read(&tmterror, prog_fp, &soper, sizeof(TmtStaticOper));
    soper.type = 0;
    if (tmterror == TMTIO_ERROR)
      g_error("Error reading program file");

    ctxt->oper_table_size++;
    
    /* Fix the endianess */
    soper.soper.ip=GUINT64_FROM_LE(soper.soper.ip);
    for(i=0;i<TMT_MAX_REG_SRC;i++)
      soper.soper.reg_src[i]=GUINT16_FROM_LE(soper.soper.reg_src[i]);
    for(i=0;i<TMT_MAX_REG_DST;i++)
      soper.soper.reg_dst[i]=GUINT16_FROM_LE(soper.soper.reg_dst[i]);
    soper.soper.instr_category=GUINT16_FROM_LE(soper.soper.instr_category);
    soper.soper.instr_attr=GUINT16_FROM_LE(soper.soper.instr_attr);
    for(i=0;i<TMT_MAX_MEM_SRC;i++)
      soper.soper.mem_src_size[i]=GUINT32_FROM_LE(soper.soper.mem_src_size[i]);
    for(i=0;i<TMT_MAX_MEM_DST;i++)
      soper.soper.mem_dst_size[i]=GUINT32_FROM_LE(soper.soper.mem_dst_size[i]);
    
    // add to oper table (this copies the structure)
    g_array_append_vals(ctxt->oper_table, &soper, 1);
    if (tmterror == TMTIO_EOF)
      break;
  }
}

void
tmt_readcontext_build_oper_table_old(GArray * table, TMTFILE * prog_fp)
{
  TmtTypedStaticOper soper;
  gint tmterror;
  gint i;
  
  while (TRUE) {
    tmt_read(&tmterror, prog_fp, &soper, sizeof(TmtStaticOperOld));
    soper.type = 0;
    if (tmterror == TMTIO_ERROR)
      g_error("Error reading program file");
    
    /* Fix the endianess */
    soper.soper.ip=GUINT64_FROM_LE(soper.soper.ip);
    for(i=0;i<TMT_MAX_REG_SRC;i++)
      soper.soper.reg_src[i]=GUINT16_FROM_LE(soper.soper.reg_src[i]);
    for(i=0;i<TMT_MAX_REG_DST_OLD;i++)
      soper.soper.reg_dst[i]=GUINT16_FROM_LE(soper.soper.reg_dst[i]);
    soper.soper.instr_category=GUINT16_FROM_LE(soper.soper.instr_category);
    soper.soper.instr_attr=GUINT16_FROM_LE(soper.soper.instr_attr);
    for(i=0;i<TMT_MAX_MEM_SRC;i++)
      soper.soper.mem_src_size[i]=GUINT32_FROM_LE(soper.soper.mem_src_size[i]);
    for(i=0;i<TMT_MAX_MEM_DST;i++)
      soper.soper.mem_dst_size[i]=GUINT32_FROM_LE(soper.soper.mem_dst_size[i]);
    
    // add to oper table (this copies the structure)
    g_array_append_vals(table, &soper, 1);
    if (tmterror == TMTIO_EOF)
      break;
  }
}

void
tmt_ia64_readcontext_type_opers(TmtReadContext * ctxt)
{
  TmtTypedStaticOper *typed_soper;
  guint i;

  for(i = 0; i < ctxt->oper_table->len; i++) {
    typed_soper = &g_array_index (ctxt->oper_table, TmtTypedStaticOper, i);

    if(!strcmp(typed_soper->soper.opcode, "clrrrb"))
      typed_soper->type = IA64_CLRRRB;
    else if(!strcmp(typed_soper->soper.opcode, "clrrrb.pr"))
      typed_soper->type = IA64_CLRRRB_PR;
    else if(!strncmp(typed_soper->soper.opcode, "br.call", 7) ||
	    !strncmp(typed_soper->soper.opcode, "jmp.call", 8) || 
	    !strncmp(typed_soper->soper.opcode, "bad.jmp.call", 12))
      typed_soper->type = IA64_BR_CALL;
    else if(!strncmp(typed_soper->soper.opcode, "brl.call", 8))
      typed_soper->type = IA64_BRL_CALL;
    else if(!strcmp(typed_soper->soper.opcode, "cover"))
      typed_soper->type = IA64_COVER;
    else if(!strncmp(typed_soper->soper.opcode, "jmp.ret", 7))
      typed_soper->type = IA64_BR_RET;
    else if(!strncmp(typed_soper->soper.opcode, "rfi", 3))
      typed_soper->type = IA64_RFI;
    else if(!strncmp(typed_soper->soper.opcode, "br.ctop", 7) ||
	    !strncmp(typed_soper->soper.opcode, "brl.ctop", 8))
      typed_soper->type = IA64_BR_CTOP;
    else if(!strncmp(typed_soper->soper.opcode, "br.cexit", 8) ||
	    !strncmp(typed_soper->soper.opcode, "brl.cexit", 9))
      typed_soper->type = IA64_BR_CEXIT;
    else if(!strncmp(typed_soper->soper.opcode, "br.wtop", 7) ||
	    !strncmp(typed_soper->soper.opcode, "brl.wtop", 8))
      typed_soper->type = IA64_BR_WTOP;
    else if(!strncmp(typed_soper->soper.opcode, "br.wexit", 8) ||
	    !strncmp(typed_soper->soper.opcode, "brl.wexit", 9))
      typed_soper->type = IA64_BR_WEXIT;
    else if(!strcmp(typed_soper->soper.opcode, "alloc"))
      typed_soper->type = IA64_ALLOC;
    else if(!strncmp(typed_soper->soper.opcode, "cmp",3))
      typed_soper->type = IA64_CMP;
    else if(!strncmp(typed_soper->soper.opcode, "cmp4",4))
      typed_soper->type = IA64_CMP4;
    else if(!strncmp(typed_soper->soper.opcode, "tbit",4))
      typed_soper->type = IA64_TBIT;
    else if(!strncmp(typed_soper->soper.opcode, "tnat",4))
      typed_soper->type = IA64_TNAT;
    else if(!strncmp(typed_soper->soper.opcode, "fcmp",4))
      typed_soper->type = IA64_FCMP;
    else if(!strncmp(typed_soper->soper.opcode, "fclass",6))
      typed_soper->type = IA64_FCLASS;
    else if(!strncmp(typed_soper->soper.opcode, "frcpa",5))
      typed_soper->type = IA64_FRCPA;
    else if(!strncmp(typed_soper->soper.opcode, "fprcpa",6))
      typed_soper->type = IA64_FPRCPA;
    else if(!strncmp(typed_soper->soper.opcode, "frsqrta",7))
      typed_soper->type = IA64_FRSQRTA;
    else if(!strncmp(typed_soper->soper.opcode, "fprsqrta",8))
      typed_soper->type = IA64_FPRSQRTA;
    else if(!strncmp(typed_soper->soper.opcode, "nop",3))
      typed_soper->type = IA64_NOP;

    {
      gchar *mystr;
      gchar **tokens;
      int i;

      typed_soper->ia64_cmp_type=IA64_CMP_OTHER;
      /* Determine if this is a normal write or not */
      switch(typed_soper->type) {
      case IA64_CMP:
      case IA64_CMP4:
      case IA64_TBIT:
      case IA64_TNAT:
      case IA64_FCMP:
      case IA64_FCLASS:
      case IA64_FRCPA:
      case IA64_FPRCPA:
      case IA64_FRSQRTA:
      case IA64_FPRSQRTA:
	mystr=g_strdup(typed_soper->soper.opcode);
	g_strchug(mystr);
	g_strchomp(mystr);
	tokens=g_strsplit(mystr,".",5);

	typed_soper->ia64_cmp_type=IA64_CMP_OTHER;
	for(i=0;tokens[i]!=NULL;i++) {
	  if(!strcmp(tokens[i],"unc") ||
	     !strcmp(tokens[i],"unc_i")) {
	    typed_soper->ia64_cmp_type=IA64_CMP_UNC;
	  }
	}
	g_free(mystr);
	g_strfreev(tokens);
	break;
      default:
	break;
      }
    }


  }
}

void
tmt_x86_readcontext_type_opers(TmtReadContext * ctxt)
{
  TmtTypedStaticOper * typed_soper;
  guint i;

  for(i = 0; i < ctxt->oper_table->len; i++) {
    typed_soper = &g_array_index (ctxt->oper_table, TmtTypedStaticOper, i);
    typed_soper->type = X86_OTHER;
    if (!g_ascii_strcasecmp(typed_soper->soper.opcode, "NOP")) {
      typed_soper->type = X86_NOP;
    }
  }
}

void
tmt_ppc64_readcontext_type_opers(TmtReadContext * ctxt)
{
  TmtTypedStaticOper *typed_soper;
  guint i;

  for(i = 0; i < ctxt->oper_table->len; i++) {
    typed_soper = &g_array_index (ctxt->oper_table, TmtTypedStaticOper, i);
    typed_soper->type = PPC64_OTHER;

    if(!strcmp(typed_soper->soper.opcode, "addi") ||
       !strcmp(typed_soper->soper.opcode, "addis") ||
       !strcmp(typed_soper->soper.opcode, "addic") ||
       !strcmp(typed_soper->soper.opcode, "addic.")) {
      typed_soper->type = PPC64_ADDI;
    } else if(!strcmp(typed_soper->soper.opcode, "ori")) {
      int j;
      gboolean is_nop = TRUE;

      for(j = 0; j < typed_soper->soper.num_reg_dst; j++) 
	if(typed_soper->soper.reg_dst[j] != PPC64_R0) is_nop = FALSE;

      for(j = 0; j < typed_soper->soper.num_reg_src; j++)
	if(typed_soper->soper.reg_src[j] != PPC64_R0) is_nop = FALSE;
      
      if(is_nop) typed_soper->type = PPC64_NOP;
    } else if(!strcmp(typed_soper->soper.opcode, "or") ||
	      !strcmp(typed_soper->soper.opcode, "or.")) {
      if(typed_soper->soper.num_reg_src == 2 &&
	 typed_soper->soper.reg_src[0] == typed_soper->soper.reg_src[1]) {
	typed_soper->type = PPC64_MOV;
      }
    } else if(!strncmp(typed_soper->soper.opcode, "extsw", 5) ||
	      !strncmp(typed_soper->soper.opcode, "extsb", 5) ||
	      !strncmp(typed_soper->soper.opcode, "extsh", 5)) {
      typed_soper->type = PPC64_EXTEND;
    } else if(!strcmp(typed_soper->soper.opcode, "stb") ||
	      !strcmp(typed_soper->soper.opcode, "sth") ||
	      !strcmp(typed_soper->soper.opcode, "stw") ||
	      !strcmp(typed_soper->soper.opcode, "std")) {
      typed_soper->type = PPC64_STORE;
    } else if(!strcmp(typed_soper->soper.opcode, "stbu") ||
	      !strcmp(typed_soper->soper.opcode, "sthu") ||
	      !strcmp(typed_soper->soper.opcode, "stwu") ||
	      !strcmp(typed_soper->soper.opcode, "stdu")) {
      typed_soper->type = PPC64_STORE_UPDATE;
    } else if(!strcmp(typed_soper->soper.opcode, "stbx") ||
	      !strcmp(typed_soper->soper.opcode, "sthx") ||
	      !strcmp(typed_soper->soper.opcode, "stwx") ||
	      !strcmp(typed_soper->soper.opcode, "stdx")) {
      typed_soper->type = PPC64_STORE_INDEXED;
    } else if(!strcmp(typed_soper->soper.opcode, "stbux") ||
	      !strcmp(typed_soper->soper.opcode, "sthux") ||
	      !strcmp(typed_soper->soper.opcode, "stwux") ||
	      !strcmp(typed_soper->soper.opcode, "stdux")) {
      typed_soper->type = PPC64_STORE_INDEXED_UPDATE;
    } else if(!strcmp(typed_soper->soper.opcode, "lbz") ||
	      !strcmp(typed_soper->soper.opcode, "lhz") ||
	      !strcmp(typed_soper->soper.opcode, "lwz")) {
      typed_soper->type = PPC64_LOAD_ZERO;
    } else if(!strcmp(typed_soper->soper.opcode, "lbzu") ||
	      !strcmp(typed_soper->soper.opcode, "lhzu") ||
	      !strcmp(typed_soper->soper.opcode, "lwzu")) {
      typed_soper->type = PPC64_LOAD_ZERO_UPDATE;
    } else if(!strcmp(typed_soper->soper.opcode, "lbzx") ||
	      !strcmp(typed_soper->soper.opcode, "lhzx") ||
	      !strcmp(typed_soper->soper.opcode, "lwzx")) {
      typed_soper->type = PPC64_LOAD_ZERO_INDEXED;
    } else if(!strcmp(typed_soper->soper.opcode, "lbzux") ||
	      !strcmp(typed_soper->soper.opcode, "lhzux") ||
	      !strcmp(typed_soper->soper.opcode, "lwzux")) {
      typed_soper->type = PPC64_LOAD_ZERO_INDEXED_UPDATE;
    } else if(!strcmp(typed_soper->soper.opcode, "lba") ||
	      !strcmp(typed_soper->soper.opcode, "lha") ||
	      !strcmp(typed_soper->soper.opcode, "lwa")) {
      typed_soper->type = PPC64_LOAD_ARITHMETIC;
    } else if(!strcmp(typed_soper->soper.opcode, "lbau") ||
	      !strcmp(typed_soper->soper.opcode, "lhau") ||
	      !strcmp(typed_soper->soper.opcode, "lwau")) {
      typed_soper->type = PPC64_LOAD_ARITHMETIC_UPDATE;
    } else if(!strcmp(typed_soper->soper.opcode, "lbax") ||
	      !strcmp(typed_soper->soper.opcode, "lhax") ||
	      !strcmp(typed_soper->soper.opcode, "lwax")) {
      typed_soper->type = PPC64_LOAD_ARITHMETIC_INDEXED;
    } else if(!strcmp(typed_soper->soper.opcode, "lbaux") ||
	      !strcmp(typed_soper->soper.opcode, "lhaux") ||
	      !strcmp(typed_soper->soper.opcode, "lwaux")) {
      typed_soper->type = PPC64_LOAD_ARITHMETIC_INDEXED_UPDATE;
    } else if(!strcmp(typed_soper->soper.opcode, "ld")) {
      typed_soper->type = PPC64_LOAD;
    } else if(!strcmp(typed_soper->soper.opcode, "ldu")) {
      typed_soper->type = PPC64_LOAD_UPDATE;
    } else if(!strcmp(typed_soper->soper.opcode, "ldx")) {
      typed_soper->type = PPC64_LOAD_INDEXED;
    } else if(!strcmp(typed_soper->soper.opcode, "ldux")) {
      typed_soper->type = PPC64_LOAD_INDEXED_UPDATE;
    }
  }
}


static gchar *
tmt_file_remove_extension(gchar * filename)
{
  gchar * ext;
  gchar * prefix;

  prefix=g_strdup(filename);
  ext = g_strrstr(filename, ".bz2");
  if (ext) {
    prefix[GPOINTER_TO_UINT(ext-filename)] = '\0';
  }
  return (prefix);
}

/************************************************/
/* Ugly IA64 side trace things with cfm and pfs */
TMTFILE * tmt_ia64sidetrace_read_open(gchar *filename)
{
  gint tmterr;
  TMTFILE *tmtfp;

  tmtfp=tmt_read_open(&tmterr, filename);
  if(tmterr != TMTIO_OK) return NULL;
  return (tmtfp);
}

void tmt_ia64sidetrace_read(gint *tmterror, TMTFILE *tmtfp, 
			    TmtIA64SideTrace * st)
{
  tmt_read(tmterror, tmtfp, st, sizeof(TmtIA64SideTrace));
  st->ip=GUINT64_FROM_LE(st->ip);
  st->cfmpfs=GUINT64_FROM_LE(st->cfmpfs);
  st->imm=GUINT64_FROM_LE(st->imm);
  st->lc=GUINT64_FROM_LE(st->lc);
  st->ec=GUINT64_FROM_LE(st->ec);
}

void tmt_ia64sidetrace_read_close(gint *tmterror, TMTFILE *tmtfp)
{
  tmt_read_close(tmterror, tmtfp);
}
#ifdef __cplusplus
}
#endif
