/*****************************************************************************
 * TMT-IA64 Trace Generator using PIN for Itanium
 ****************************************************************************/
#include <iostream>
#include <map>
#include <ostream>
#include <set>
using namespace std;

#include <svt_sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/unistd.h>
#include <sys/time.h>
#include <sys/unistd.h>

#include <pin.h>
#include "pinpp.H"

#include "tmt.h"

TmtWriteContext * write_context;
TMTFILE         * exttmtfp;

static gchar default_program[] = "prog.bz2";
static gchar default_trace[] = "trace.bz2";
static gchar default_ext[] = "ext.bz2";

static map< UINT64, int > imgMap;

// Command line argument "knobs"
KNOB ProgramFileKnob("program", "general", KNOB_TYPE_STRING, default_program, 
		     "Program file");
KNOB TraceFileKnob("trace", "general", KNOB_TYPE_STRING, default_trace,
		   "Trace file");
KNOB ExtFileKnob("ext", "general", KNOB_TYPE_STRING, default_ext,
		 "IA64 extensions file");

void RecordAlloc(UINT64 ip, UINT64 qp, UINT64 cfm, UINT64 imm)
{
  TmtIA64SideTrace st;
  gint tmterror;
                                                                                
  st.ip = ip;
  st.cfmpfs = cfm;
  st.imm = imm;
  st.lc = 0;
  st.ec = 0;
  st.misc[0] = 0;
  st.misc[1] = 0;
  st.misc[2] = 0;
  st.misc[3] = 0;
                                                                                
  tmt_write( &tmterror, exttmtfp, &st, sizeof(TmtIA64SideTrace));
  if (tmterror != TMTIO_OK) {
    g_printerr("Error writing trace\n");
    exit(1);
  }

  tmt_writecontext_write_dynamic(write_context, 
				 ip, 
				 (gboolean) qp,
				 FALSE,
				 NULL, 
				 NULL,
				 0,             /* var_mem_src_size */
				 0              /* var_mem_dst_size */
		  		 );
}

void RecordCFM(UINT64 ip, UINT64 cfm)
{
  TmtIA64SideTrace st;
  gint tmterror;

  st.ip = ip;
  st.cfmpfs = cfm;
  st.imm = 0;
  st.lc = 0;
  st.ec = 0;
  st.misc[0] = 0;
  st.misc[1] = 0;
  st.misc[2] = 0;
  st.misc[3] = 0;
                                                                                
  tmt_write( &tmterror, exttmtfp, &st, sizeof(TmtIA64SideTrace));
  if (tmterror != TMTIO_OK) {
    g_printerr("Error writing trace\n");
    exit(1);
  }

  tmt_writecontext_write_dynamic(write_context, 
				 ip, 
				 TRUE,
				 FALSE,
				 NULL, 
				 NULL,
				 0,             /* var_mem_src_size */
				 0              /* var_mem_dst_size */
		  		 );
}

void RecordBranch(UINT64 ip, UINT64 taken)
{

//g_print("ip: 0x%016"G_GINT64_MODIFIER"X\n", ip);
//g_print("tk: 0x%016"G_GINT64_MODIFIER"X\n", taken);
//g_print("\n");
  tmt_writecontext_write_dynamic(write_context, 
				 ip,                /* ip */
				 (taken > 0),       /* qp */
				 (taken > 0),       /* taken */
				 NULL,              /* mem src */
				 NULL,              /* mem dst */
				 0,                 /* var_mem_src_size */
				 0                  /* var_mem_dst_size */
				 );
}

void RecordBranchCFMPFS(UINT64 ip, UINT64 taken, UINT64 cfmpfs)
{
  TmtIA64SideTrace st;
  gint tmterror;
                
  st.ip = ip;
  st.cfmpfs = cfmpfs;
  st.imm = 0;
  st.lc = 0;
  st.ec = 0;
  st.misc[0] = 0;
  st.misc[1] = 0;
  st.misc[2] = 0;
  st.misc[3] = 0;
                                                                                
  tmt_write( &tmterror, exttmtfp, &st, sizeof(TmtIA64SideTrace));
  if (tmterror != TMTIO_OK) {
    g_printerr("Error writing trace\n");
    exit(1);
  }

  tmt_writecontext_write_dynamic(write_context, 
				 ip,                /* ip */
				 (taken > 0),       /* qp */
				 (taken > 0),       /* taken */
				 NULL,              /* mem src */
				 NULL,              /* mem dst */
				 0,                 /* var_mem_src_size */
				 0                  /* var_mem_dst_size */
				 );
}
                                                                                
void RecordBranchRRB(UINT64 ip, UINT64 taken, UINT64 cfm, UINT64 lc, UINT64 ec)
{
  TmtIA64SideTrace st;
  gint tmterror;
                    
  st.ip = ip;
  st.cfmpfs = cfm;
  st.imm = 0;
  st.lc = lc;
  st.ec = ec;
  st.misc[0] = 0;
  st.misc[1] = 0;
  st.misc[2] = 0;
  st.misc[3] = 0;
                                                                                
  tmt_write( &tmterror, exttmtfp, &st, sizeof(TmtIA64SideTrace));
  if (tmterror != TMTIO_OK) {
    g_printerr("Error writing trace\n");
    exit(1);
  }

  tmt_writecontext_write_dynamic(write_context, 
				 ip,                /* ip */
				 (taken > 0),       /* qp */
				 (taken > 0),       /* taken */
				 NULL,              /* mem src */
				 NULL,              /* mem dst */
				 0,                 /* var_mem_src_size */
				 0                  /* var_mem_dst_size */
				 );
}

void 
RecordMemoryRead(UINT64 ip, UINT64 qp, UINT64 ea)
{
//g_print("ip: 0x%016"G_GINT64_MODIFIER"X\n", ip);
//g_print("qp: 0x%016"G_GINT64_MODIFIER"X\n", qp);
//g_print("ea: 0x%016"G_GINT64_MODIFIER"X\n", ea);
//g_print("\n");
  tmt_writecontext_write_dynamic(write_context, 
				 ip,            /* ip */
				 (gboolean) qp, /* qp */
				 FALSE,         /* taken */
				 &ea,           /* mem src */
				 NULL,          /* mem dst */
				 0,             /* var_mem_src_size */
				 0              /* var_mem_dst_size */
				 );
}

void 
RecordMemoryWrite(UINT64 ip, UINT64 qp, UINT64 ea)
{
//g_print("ip: 0x%016"G_GINT64_MODIFIER"X\n", ip);
//g_print("qp: 0x%016"G_GINT64_MODIFIER"X\n", qp);
//g_print("ea: 0x%016"G_GINT64_MODIFIER"X\n", ea);
//g_print("\n");
  tmt_writecontext_write_dynamic(write_context, 
				 ip,            /* ip */
				 (gboolean) qp, /* qp */
				 FALSE,         /* taken */
				 NULL,          /* mem src */
				 &ea,           /* mem dst */
				 0,             /* var_mem_src_size */
				 0              /* var_mem_dst_size */
				 );
}

void 
RecordMemoryReadAndWrite(UINT64 ip, UINT64 qp, UINT64 ea)
{
//g_print("ip: 0x%016"G_GINT64_MODIFIER"X\n", ip);
//g_print("qp: 0x%016"G_GINT64_MODIFIER"X\n", qp);
//g_print("ea: 0x%016"G_GINT64_MODIFIER"X\n", ea);
//g_print("\n");
  tmt_writecontext_write_dynamic(write_context, 
				 ip,             /* ip */
				 (gboolean) qp,  /* qp */
				 FALSE,          /* taken */
				 &ea,            /* mem src */
				 &ea,            /* mem dst */
				 0,             /* var_mem_src_size */
				 0              /* var_mem_dst_size */
				 );
}


void 
RecordNoQpInstruction(UINT64 ip)
{
//g_print("ip: 0x%016"G_GINT64_MODIFIER"X\n", ip);
//g_print("\n");
  tmt_writecontext_write_dynamic(write_context, 
				 ip, 
				 TRUE,
				 FALSE,
				 NULL, 
				 NULL,
				 0,             /* var_mem_src_size */
				 0              /* var_mem_dst_size */
		  		 );
}

void 
RecordInstruction(UINT64 ip, UINT64 qp)
{
//g_print("ip: 0x%016"G_GINT64_MODIFIER"X\n", ip);
//g_print("qp: 0x%016"G_GINT64_MODIFIER"X\n", qp);
//g_print("\n");
  tmt_writecontext_write_dynamic(write_context, 
				 ip, 
				 (gboolean) qp,
				 FALSE,
				 NULL, 
				 NULL,
				 0,             /* var_mem_src_size */
				 0              /* var_mem_dst_size */
		  		 );
}

static inline gboolean
RegIsValid(REG reg)
{
  return (reg > REG_NONE);
}

void Instruction(INS ins, VOID *v)
{
  TmtStaticOper soper;
  gboolean is_branch = FALSE;
  gboolean is_direct_branch = FALSE;
  gboolean is_indirect_branch = FALSE;
  gboolean is_call = FALSE;
  gboolean is_return = FALSE;
  gboolean is_memory = FALSE;
  gboolean is_stack_read = FALSE;
  gboolean is_stack_write = FALSE;
  gboolean is_predicated = TRUE;

  REG reg_src[TMT_MAX_REG_SRC];
  REG reg_dst[TMT_MAX_REG_DST];
  guint i;

  // zero out structure
  memset(&soper, 0, sizeof(TmtStaticOper));
  memset(reg_src, 0, sizeof(reg_src));
  memset(reg_dst, 0, sizeof(reg_dst));

  // set fields
  soper.ip = INS_Address(ins);
  soper.instr_category = INS_Category(ins);
  soper.instr_size = 16; // per bundle

  // obtain src and dst registers
  reg_src[0] = INS_Regr1(ins);
  reg_src[1] = INS_Regr2(ins);
  reg_src[2] = INS_Regr3(ins);
  reg_src[3] = INS_Regr4(ins);
  reg_dst[0] = INS_Regw1(ins);
  reg_dst[1] = INS_Regw2(ins);
  reg_dst[2] = INS_Regw3(ins);  
  
  // filter out invalid registers
  for (i=0; i<TMT_MAX_REG_SRC; ++i) {
    if ( RegIsValid(reg_src[i]) ) {
      soper.reg_src[soper.num_reg_src++] = reg_src[i];
    }
  }
  for (i=0; i<TMT_MAX_REG_DST; ++i) {
    if ( RegIsValid(reg_dst[i]) ) {
      soper.reg_dst[soper.num_reg_dst++] = reg_dst[i];
    }
  }
  
  // set attributes based on instruction information
  switch(soper.instr_category) {
  case TYPE_CAT_ALLOC:
    {
      UINT64 imm;
      imm = INS_ImmLit(ins);
      PIN_InsertCall(IPOINT_BEFORE, ins,
                     (AFUNPTR) RecordAlloc,
                     IARG_IP_SLOT,
		     IARG_QP_VALUE,
                     IARG_REG_VALUE, REG_CFM,
                     IARG_UINT64, imm,
                     IARG_END);
    }
    break;
  case TYPE_CAT_COVER:
  case TYPE_CAT_CLEAR_RRB:
    PIN_InsertCall(IPOINT_BEFORE, ins,
		   (AFUNPTR) RecordCFM,
		   IARG_IP_SLOT,
		   IARG_REG_VALUE, REG_CFM,
		   IARG_END);
    break;
  case TYPE_CAT_FETCH:
  case TYPE_CAT_FETCHADD:
  case TYPE_CAT_LOAD:
    is_memory = TRUE;
    soper.num_mem_src = 1;
    soper.mem_src_size[0] = INS_SizeType(ins);
    PIN_InsertCall(IPOINT_BEFORE, ins,
		   (AFUNPTR) RecordMemoryRead,
		   IARG_IP_SLOT,
		   IARG_QP_VALUE,
		   IARG_EA,
		   IARG_END);
    break;
  case TYPE_CAT_STORE:
    is_memory = TRUE;
    soper.num_mem_dst = 1;
    soper.mem_dst_size[0] = INS_SizeType(ins);
    PIN_InsertCall(IPOINT_BEFORE, ins,
		   (AFUNPTR) RecordMemoryWrite,
		   IARG_IP_SLOT,
		   IARG_QP_VALUE,
		   IARG_EA,
		   IARG_END);
    break;
  case TYPE_CAT_CMPXCHG:
  case TYPE_CAT_XCHG:
    is_memory = TRUE;
    soper.num_mem_src = 1;
    soper.num_mem_dst = 1;
    soper.mem_src_size[0] = INS_SizeType(ins);
    soper.mem_dst_size[0] = INS_SizeType(ins);
    PIN_InsertCall(IPOINT_BEFORE, ins,
		   (AFUNPTR) RecordMemoryReadAndWrite,
		   IARG_IP_SLOT,
		   IARG_QP_VALUE,
		   IARG_EA,
		   IARG_END);
    break;
  case TYPE_CAT_BRANCH:
  case TYPE_CAT_CBRANCH:
  case TYPE_CAT_JUMP:
  case TYPE_CAT_CJUMP:
    // collect branch instruction attributes
    is_branch = TRUE;
    if ((TYPE_CAT_BRANCH == soper.instr_category) ||
	(TYPE_CAT_CBRANCH == soper.instr_category)) {
      is_direct_branch = TRUE;
    } else if ((TYPE_CAT_JUMP == soper.instr_category) ||
	       (TYPE_CAT_CJUMP == soper.instr_category)) {
      is_indirect_branch = TRUE;
    }

    // instrument branch instructions
    switch(INS_BraType(ins)) {
    case TYPE_BRA_CALL:
      is_call = TRUE;
      PIN_InsertCall(IPOINT_BEFORE, ins,
                     (AFUNPTR) RecordBranchCFMPFS,
                     IARG_IP_SLOT,
		     IARG_BRANCH_TAKEN,
                     IARG_REG_VALUE, REG_CFM,
                     IARG_END);
      break;
    case TYPE_BRA_RET:
      is_indirect_branch = TRUE;
      is_return = TRUE;
      PIN_InsertCall(IPOINT_BEFORE, ins,
                     (AFUNPTR) RecordBranchCFMPFS,
                     IARG_IP_SLOT,
		     IARG_BRANCH_TAKEN,
                     IARG_REG_VALUE, REG_AR_PFS,
                     IARG_END);
      break;
    case TYPE_BRA_CTOP:
    case TYPE_BRA_CEXIT:
    case TYPE_BRA_WTOP:
    case TYPE_BRA_WEXIT:
      PIN_InsertCall(IPOINT_BEFORE, ins,
                     (AFUNPTR) RecordBranchRRB,
                     IARG_IP_SLOT,
		     IARG_BRANCH_TAKEN,
                     IARG_REG_VALUE, REG_CFM,
                     IARG_REG_VALUE, REG_AR_LC,
                     IARG_REG_VALUE, REG_AR_EC,
                     IARG_END);
      break;
    default:
      PIN_InsertCall(IPOINT_BEFORE, ins, 
		     (AFUNPTR) RecordBranch,
		     IARG_IP_SLOT,
		     IARG_BRANCH_TAKEN,  
		     IARG_END);
      break;
    }
    break;
  case TYPE_CAT_CHECK:
    is_branch = TRUE;
    is_direct_branch = TRUE;
    PIN_InsertCall(IPOINT_BEFORE, ins, 
		   (AFUNPTR) RecordBranch,
		   IARG_IP_SLOT,
		   IARG_BRANCH_TAKEN,  
		   IARG_END);
    break;
  case TYPE_CAT_RFI:
    is_branch = TRUE;
    is_indirect_branch = TRUE;
    is_return = TRUE;
    PIN_InsertCall(IPOINT_BEFORE, ins,
		   (AFUNPTR) RecordBranchCFMPFS,
		   IARG_IP_SLOT,
		   IARG_BRANCH_TAKEN,
		   IARG_REG_VALUE, REG_CFM,
		   IARG_END);
    break;
  case TYPE_CAT_BRANCH_PREDICT:
  case TYPE_CAT_JUMP_PREDICT:
  case TYPE_CAT_RSE_CONTROL:
  case TYPE_CAT_EPC:
  case TYPE_CAT_BSW:
    PIN_InsertCall(IPOINT_BEFORE, ins,
		   (AFUNPTR) RecordNoQpInstruction,
		   IARG_IP_SLOT,
		   IARG_END);
    break;
  default:
    PIN_InsertCall(IPOINT_BEFORE, ins,
		   (AFUNPTR) RecordInstruction,
		   IARG_IP_SLOT,
		   IARG_QP_VALUE,
		   IARG_END);
  }

  // set attribute
  soper.instr_attr = tmt_static_oper_set_attr(is_branch,
					      is_direct_branch,
					      is_indirect_branch,
					      is_call,
					      is_return,
					      is_memory,
					      is_stack_read,
					      is_stack_write,
					      is_predicated,
					      FALSE, FALSE);

  // obtain opcode
  tmt_static_oper_set_opcode(&soper, INS_Mnemonic(ins));

  // add the static info to the trace
  tmt_writecontext_write_static(write_context, &soper);
}

/*********************** Instrumentation Functions **********************/

static VOID
I_Aoti(PIN_IMAGE *img, PROC head, void *v)
{
  g_print("image: [0x%016"G_GINT64_MODIFIER"X,0x%016"G_GINT64_MODIFIER"X]",
	  img->LowAddress(), img->HighAddress());

  if ( imgMap.find(img->LowAddress()) == imgMap.end() ) {
    imgMap[img->LowAddress()] = 1;
    g_print(" (instrumenting)\n");

    for( PROC proc = head; proc != PROC_Invalid(); proc = PROC_Next(proc) ) {
      for( INS ins = PROC_FirstIns(proc); 
	   ins != INS_INVALID();
	   ins = INS_Next(ins) ) {
	// Instrument all instructions
	Instruction(ins,0);
      }
    }
  } else {
    g_print(" (ignoring)\n");
  }
}

/**************************** Fini **************************************/

void Fini(int n, VOID *v)
{
  gint tmterror;

  tmt_write_close( &tmterror, exttmtfp );
  if (tmterror != TMTIO_OK)
    g_printerr("Error closing output file\n");

  tmt_writecontext_free(write_context);
}

void usage(gchar * toolname) {
  g_print("\n");
  g_print("Usage:  %s {-program <program> -trace <trace> -ext <ext>}\n", 
	  toolname);
  g_print("\n");
  g_print("        <program>  bz2 output file containing the program data\n");
  g_print("        <trace>    bz2 output file containing the trace data\n");
  g_print("        <ext>      bz2 output file containing ia64 extension data\n");
  g_print("\n");
  exit(1);
}

int main(int argc, char * argv[]) 
{
  gchar *program = NULL;
  gchar *trace = NULL;
  gchar *ext = NULL;
  gint tmterror;

  PIN_ParseKnobs(usage);

  program = (gchar *) ProgramFileKnob.ValueString();
  trace = (gchar *) TraceFileKnob.ValueString();
  ext = (gchar *) ExtFileKnob.ValueString();

  if (!strcmp(program,default_program)) {
    g_printerr("No program file specified... using %s\n", program);
  }
  if (!strcmp(trace,default_trace)) {
    g_printerr("No trace file specified... using %s\n", trace);
  }
  if (!strcmp(ext,default_ext)) {
    g_printerr("No extension file specified... using %s\n", ext);
  }

  // open a write context for the trace and program files
  write_context = tmt_writecontext_new(trace, program);

  // open a bzip file stream for the extensions file
  exttmtfp = tmt_write_open( &tmterror, ext );
  if (tmterror != TMTIO_OK) {
    g_printerr("Error opening outfile: %s\n", ext);
    exit(1);
  }

  PIN_InitializeSymbolTable();
  //PIN_AddInstrumentImageFunction(I_Aoti, 0);
  PIN_AddInstrumentInstructionFunction(Instruction, 0);
  PIN_AddFiniFunction(Fini, 0);
  PIN_StartProgram();

  return EXIT_SUCCESS;
}
