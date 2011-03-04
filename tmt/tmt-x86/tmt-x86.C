#include <stdio.h>
#include <stdlib.h>
#include <pin.H>
#include "tmt.h"
#include <cstring>
#include <iostream>
#include <asm/unistd.h>

TmtWriteContext * write_context;
int tmtars[256];
static gchar default_program[] = "prog.bz2";
static gchar default_trace[] = "trace.bz2";
static gchar default_linestart[] = "0";
static gchar default_filestart[] = "";
static int stackfound = 0;
static bool start_pin = false;
static int line_start = 0;
static string file_start;
static bool syscall_function_start = false;
static int syscall_function_number = 0;

// Command line argument "knobs"
KNOB<std::string> KnobProgramFile(KNOB_MODE_WRITEONCE, "pintool", "program", 
				  default_program, "Program file");
KNOB<std::string> KnobTraceFile(KNOB_MODE_WRITEONCE, "pintool", "trace",
				default_trace, "Trace file");
KNOB<std::string> KnobLineStart(KNOB_MODE_WRITEONCE, "pintool", "linestart",
				default_linestart, "Line Start");
KNOB<std::string> KnobFileStart(KNOB_MODE_WRITEONCE, "pintool", "filestart",
				default_filestart, "File Start");
				

bool InstructionStartCheck(ADDRINT ip)
{  
  if(false == start_pin)
    {
      int column = 0;
      int line = 0;
      string filename;

      //! if the user did not set a starting line, just start
      if((line_start == 0) && (file_start.empty()))
	{		  
	  printf("Starting Dynamic Tracing\n");
	  start_pin = true;
	}
      else if (line_start != 0)
	{
	  PIN_LockClient();
	  PIN_GetSourceLocation (ip, &column, &line, &filename);
	  PIN_UnlockClient();
	  if(line >= line_start)
	    {
	      if((file_start.empty()) || 
		 (filename.find(file_start, 0) != string::npos))
		{
                            
		  printf("Found line %d at 0x%x\n",
			 line_start, (unsigned int)ip);
		  printf("in file %s in column %d\n\n",
			 filename.c_str(), column);
		  printf("Starting Dynamic Tracing\n");
		  start_pin = true;
		}
	    }
	}
      else if (!file_start.empty())
	{
	  PIN_LockClient();
	  PIN_GetSourceLocation (ip, &column, &line, &filename);
	  PIN_UnlockClient();
	 
	  if(filename.find(file_start, 0) != string::npos)
	    {                            
	      printf("Found line %d at 0x%x\n",
		     line_start, (unsigned int)ip);
	      printf("in file %s in column %d\n\n",
		     filename.c_str(), column);
	      printf("Starting Dynamic Tracing\n");
	      start_pin = true;
	    }                        
	}
    }

  return(start_pin);
}

VOID A_MemoryRead(ADDRINT ip, ADDRINT ea)
{
  if((true == start_pin) ||
     (true == InstructionStartCheck(ip)))
    {

      guint64 ea64 = ea;
      tmt_writecontext_write_dynamic(write_context, 
				     (guint64) ip,  /* ip */
				     TRUE,          /* qp */
				     FALSE,
				     FALSE,   /* is syscall */				
				     &ea64,         /* mem src */
				     NULL,          /* mem dst */
				     0,             /* var_mem_src_size */
				     0 ,             /* var_mem_dst_size */
				 
				     0   /* syscall num*/
				     );
    }
}

VOID A_SysCallMemoryRead_VR(ADDRINT ip, ADDRINT ea, UINT32 size, UINT32 syscallnum)
{

  if((true == start_pin) || 
     (true == InstructionStartCheck(ip)))
    {

      guint64 ea64 = ea;
      guint64 dst = 0;

      // Start DEBUG
      //  cout << "Syscall Read from: " << ea << " of size: " << size << endl;
      // END DEBUG
 
      tmt_writecontext_write_dynamic(write_context, 
				     (guint64) ip,  /* ip */
				     TRUE,          /* qp */
				     FALSE,         /* taken */
				     TRUE,   /* is syscall */				
				     &ea64,         /* mem src */
				     &dst,          /* mem dst */
				     size,          /* var_mem_src_size */
				     0,              /* var_mem_dst_size */
				     syscallnum
				     );
    }
}

VOID A_MemoryRead_VR(ADDRINT ip, ADDRINT ea, UINT32 size)
{

  if((true == start_pin) ||
     (true == InstructionStartCheck(ip)))
    {

      guint64 ea64 = ea;
 
      tmt_writecontext_write_dynamic(write_context, 
				     (guint64) ip,  /* ip */
				     TRUE,          /* qp */
				     FALSE,         /* taken */
				     FALSE,   /* taken */
				
				     &ea64,         /* mem src */
				     NULL,          /* mem dst */
				     size,          /* var_mem_src_size */
				     0,              /* var_mem_dst_size */
				     0 
				     );
    }
}

VOID A_MemoryReadx2(ADDRINT ip, ADDRINT ea_r1, ADDRINT ea_r2)
{

  if((true == start_pin) ||
     (true == InstructionStartCheck(ip)))
    {

      guint64 src_ea[2];
      src_ea[0] = ea_r1;
      src_ea[1] = ea_r2;
      tmt_writecontext_write_dynamic(write_context,
				     (guint64) ip,  /* ip */
				     TRUE,          /* qp */
				     FALSE,   /* taken */
				     FALSE,   /* taken */
				
				     src_ea,        /* mem src */
				     NULL,          /* mem dst */
				     0,             /* var_mem_src_size */
				     0,              /* var_mem_dst_size */
				     0   /* syscall num*/
				     );
    }
}

VOID A_MemoryReadx2_VR(ADDRINT ip, ADDRINT ea_r1, ADDRINT ea_r2, 
		       UINT32 size)
{
  if((true == start_pin) ||
     (true == InstructionStartCheck(ip)))
    {
      guint64 src_ea[2];
      src_ea[0] = ea_r1;
      src_ea[1] = ea_r2;
      tmt_writecontext_write_dynamic(write_context,
				     (guint64) ip,  /* ip */
				     TRUE,          /* qp */
				     FALSE,         /* taken */
				     FALSE,   /* taken */
				
				     src_ea,        /* mem src */

				     NULL,          /* mem dst */
				     size,          /* var_mem_src_size */
				     0,              /* var_mem_dst_size */
				     0   /* syscall num*/
				     );
    }
}

VOID A_MemoryWrite(ADDRINT ip, ADDRINT ea)
{

  if((true == start_pin) ||
     (true == InstructionStartCheck(ip)))
    {

      guint64 ea64 = ea;
      tmt_writecontext_write_dynamic(write_context,
				     (guint64) ip,  /* ip */
				     TRUE,          /* qp */
				     FALSE,         /* taken */
				     FALSE,   /* taken */
				
				     NULL,	        /* mem src */
				     &ea64,       /* mem dst */
				     0,             /* var_mem_src_size */
				     0,              /* var_mem_dst_size */
				     0   /* syscall num*/ 
				     );
    }
}

VOID A_MemoryWrite_VW(ADDRINT ip, ADDRINT ea, UINT32 size)
{

  if((true == start_pin) ||
     (true == InstructionStartCheck(ip)))
    {

      guint64 ea64 = ea;
      tmt_writecontext_write_dynamic(write_context,
				     (guint64) ip,  /* ip */
				     TRUE,          /* qp */
				     FALSE,         /* taken */
				     FALSE,   /* taken */
				
				     NULL,	        /* mem src */
				     &ea64,         /* mem dst */
				     0,             /* var_mem_src_size */
				     size,           /* var_mem_dst_size */
				     0  /* syscall num*/
				     );
    }
}

VOID A_MemoryReadAndWrite(ADDRINT ip, ADDRINT ea_r, ADDRINT ea_w)
{

  if((true == start_pin) ||
     (true == InstructionStartCheck(ip)))
    {

      guint64 ea64_r = ea_r;
      guint64 ea64_w = ea_w;
      tmt_writecontext_write_dynamic(write_context,
				     (guint64) ip,  /* ip */
				     TRUE,          /* qp */
				     FALSE,         /* taken */
				     FALSE,   /* taken */
				
				     &ea64_r,       /* mem src */
				     &ea64_w,       /* mem dst */
				     0,             /* var_mem_src_size */
				     0,              /* var_mem_dst_size */
				     0   /* syscall num*/ 
				     );
    }
}

VOID A_MemoryReadAndWrite_VRW(ADDRINT ip, ADDRINT ea_r, ADDRINT ea_w,
			      UINT32 size_r, UINT32 size_w)
{

  if((true == start_pin) ||
     (true == InstructionStartCheck(ip)))
    {
      guint64 ea64_r = ea_r;
      guint64 ea64_w = ea_w;
      tmt_writecontext_write_dynamic(write_context,
				     (guint64) ip,  /* ip */
				     TRUE,          /* qp */
				     FALSE,         /* taken */
				     FALSE,   /*  */				
				     &ea64_r,       /* mem src */
				     &ea64_w,       /* mem dst */
				     size_r,        /* var_mem_src_size */
				     size_w,         /* var_mem_dst_size */
				     0  /* syscall num*/
				     );
    }
}


VOID A_MemoryReadAndWrite_VR(ADDRINT ip, ADDRINT ea_r, ADDRINT ea_w,
			     UINT32 size_r)
{

  if((true == start_pin) ||
     (true == InstructionStartCheck(ip)))
    {

      guint64 ea64_r = (guint64)ea_r;
      guint64 ea64_w = (guint64)ea_w;
      tmt_writecontext_write_dynamic(write_context,
				     (guint64) ip,  /* ip */
				     TRUE,          /* qp */
				     FALSE,         /* taken */
				     TRUE,   /* is syscall */				
				     &ea64_r,       /* mem src */
				     &ea64_w,       /* mem dst */
				     size_r,        /* var_mem_src_size */
				     0 ,             /* var_mem_dst_size */
				     0   /* syscall num*/
				     );
    }
}

VOID A_SysCallMemoryReadAndWrite_VRW(ADDRINT ip, ADDRINT ea_r, ADDRINT ea_w, 
				     UINT32 size_r, UINT32 size_w,  UINT32 syscallnum)
{
  if((true == start_pin) ||
     (true == InstructionStartCheck(ip)))
    {
      guint64 ea64_r = (guint64) ea_r;
      guint64 ea64_w = (guint64) ea_w;

      // Start DEBUG
      //  cout << "Syscall Read from: " << ea64_r << " to: " << ea64_w << " of size: " << size_r << endl;
      // END DEBUG

      tmt_writecontext_write_dynamic(write_context,
				     (guint64) ip,  /* ip */
				     TRUE,          /* qp */
				     FALSE,         /* taken */
				     TRUE,   /* is syscall */				
				     &ea64_r,       /* mem src */
				     &ea64_w,       /* mem dst */
				     size_r,        /* var_mem_src_size */
				     size_w,        /* var_mem_dst_size */
				     syscallnum   /* syscall num*/
				     );
    }
}
VOID A_SysCallMemoryReadAndWrite_VR(ADDRINT ip, ADDRINT ea_r, ADDRINT ea_w, 
				    UINT32 size_r, UINT32 syscallnum)
{
  if((true == start_pin) ||
     (true == InstructionStartCheck(ip)))
    {

      guint64 ea64_r = (guint64) ea_r;
      guint64 ea64_w = (guint64) ea_w;

      // Start DEBUG
      //  cout << "Syscall Read from: " << ea64_r << " to: " << ea64_w << " of size: " << size_r << endl;
      // END DEBUG

      tmt_writecontext_write_dynamic(write_context,
				     (guint64) ip,  /* ip */
				     TRUE,          /* qp */
				     FALSE,         /* taken */
				     TRUE,   /* is syscall */				
				     &ea64_r,       /* mem src */
				     &ea64_w,       /* mem dst */
				     size_r,        /* var_mem_src_size */
				     0 ,             /* var_mem_dst_size */
				     syscallnum   /* syscall num*/
				     );
    }
}

VOID A_MemoryReadAndWrite_VW(ADDRINT ip, ADDRINT ea_r, ADDRINT ea_w, 
			     UINT32 size_w)
{
  if((true == start_pin) ||
     (true == InstructionStartCheck(ip)))
    {

      guint64 ea64_r = (guint64) ea_r;
      guint64 ea64_w = (guint64) ea_w;
      tmt_writecontext_write_dynamic(write_context,
				     (guint64) ip,  /* ip */
				     TRUE,          /* qp */
				     FALSE,         /* taken */
				     FALSE,   /* syscall */				
				     &ea64_r,       /* mem src */
				     &ea64_w,       /* mem dst */
				     0,             /* var_mem_src_size */
				     size_w ,        /* var_mem_dst_size */
				     0   /* syscall num*/
				     );
    }
}

VOID A_SysCallMemoryReadAndWrite_VW(ADDRINT ip, ADDRINT ea_r, ADDRINT ea_w, 
				    UINT32 size_w, UINT32 sysNum)
{
  if((true == start_pin) ||
     (true == InstructionStartCheck(ip)))
    {

      guint64 ea64_r = (guint64) ea_r;
      guint64 ea64_w = (guint64) ea_w;

      // Start DEBUG
      //  cout << "Syscall write to: " << ea64_w << " from: " << ea64_r << " of size: " << size_w << endl;
      // END DEBUG

      tmt_writecontext_write_dynamic(write_context,
				     (guint64) ip,  /* ip */
				     TRUE,          /* qp */
				     FALSE,         /* taken */
				     TRUE,   /* is syscall */				
				     &ea64_r,       /* mem src */
				     &ea64_w,       /* mem dst */
				     0,             /* var_mem_src_size */
				     size_w ,        /* var_mem_dst_size */
				     sysNum   /* syscall num*/
				     );
    }
}

VOID A_MemoryReadx2AndWrite(ADDRINT ip, ADDRINT ea_r1, ADDRINT ea_r2, 
			    ADDRINT ea_w)
{
  if((true == start_pin) ||
     (true == InstructionStartCheck(ip)))
    {

      guint64 src_ea[2];
      guint64 ea64_w = ea_w;
      src_ea[0] = ea_r1;
      src_ea[1] = ea_r2;
      tmt_writecontext_write_dynamic(write_context,
				     (guint64) ip,   /* ip */
				     TRUE,           /* qp */
				     FALSE,          /* taken */
				     FALSE,   /* taken */
				
				     src_ea,         /* mem src */
				     &ea64_w,        /* mem dst */
				     0,              /* var_mem_src_size */
				     0,               /* var_mem_dst_size */
				     0   /* syscall num*/
				     );
    }
}

VOID A_MemoryReadx2AndWrite_VRW(ADDRINT ip, ADDRINT ea_r1, ADDRINT ea_r2, 
				ADDRINT ea_w, UINT32 size_r, UINT32 size_w)
{
  if((true == start_pin) ||
     (true == InstructionStartCheck(ip)))
    {

      guint64 src_ea[2];
      guint64 ea64_w = ea_w;
      src_ea[0] = ea_r1;
      src_ea[1] = ea_r2;
      tmt_writecontext_write_dynamic(write_context,
				     (guint64) ip,   /* ip */
				     TRUE,           /* qp */
				     FALSE,          /* taken */
				     FALSE,   /*  */
				
				     src_ea,         /* mem src */
				     &ea64_w,        /* mem dst */
				     size_r,         /* var_mem_src_size */
				     size_w,          /* var_mem_dst_size */
				     0   /* syscall num*/
				     );
    }
}

VOID A_MemoryReadx2AndWrite_VR(ADDRINT ip, ADDRINT ea_r1, ADDRINT ea_r2, 
			       ADDRINT ea_w, UINT32 size_r)
{
  if((true == start_pin) ||
     (true == InstructionStartCheck(ip)))
    {

      guint64 src_ea[2];
      guint64 ea64_w = ea_w;
      src_ea[0] = ea_r1;
      src_ea[1] = ea_r2;
      tmt_writecontext_write_dynamic(write_context,
				     (guint64) ip,   /* ip */
				     TRUE,           /* qp */
				     FALSE,          /* taken */
				     FALSE,   /*  */
				
				     src_ea,         /* mem src */
				     &ea64_w,        /* mem dst */
				     size_r,         /* var_mem_src_size */
				     0 ,              /* var_mem_dst_size */
				     0   /* syscall num*/
				     );
    }
}

VOID A_MemoryReadx2AndWrite_VW(ADDRINT ip, ADDRINT ea_r1, ADDRINT ea_r2, 
			       ADDRINT ea_w, UINT32 size_w)
{
  if((true == start_pin) ||
     (true == InstructionStartCheck(ip)))
    {

      guint64 src_ea[2];
      guint64 ea64_w = ea_w;
      src_ea[0] = ea_r1;
      src_ea[1] = ea_r2;
      tmt_writecontext_write_dynamic(write_context,
				     (guint64) ip,   /* ip */
				     TRUE,           /* qp */
				     FALSE,          /* taken */
				     FALSE,   /*  */
				
				     src_ea,         /* mem src */
				     &ea64_w,        /* mem dst */
				     0,              /* var_mem_src_size */
				     size_w,          /* var_mem_dst_size */
				     0   /* syscall num*/
				     );
    }
}

VOID
A_UncondBr_MemR(ADDRINT ip, ADDRINT ea)
{
  if((true == start_pin) ||
     (true == InstructionStartCheck(ip)))
    {

      guint64 ea64 = ea;
      tmt_writecontext_write_dynamic(write_context, (guint64) ip, 
				     TRUE, TRUE, FALSE, &ea64, NULL, 0, 0,0);
    }
}

VOID
A_UncondBr_MemW(ADDRINT ip, ADDRINT ea)
{
  if((true == start_pin) ||
     (true == InstructionStartCheck(ip)))
    {

      guint64 ea64 = ea;
      tmt_writecontext_write_dynamic(write_context, (guint64) ip, 
				     TRUE, TRUE,FALSE, NULL, &ea64, 0, 0,0);
    }
}

VOID 
A_UncondBr_MemRW(ADDRINT ip, ADDRINT ea_r, ADDRINT ea_w)
{
  if((true == start_pin) ||
     (true == InstructionStartCheck(ip)))
    {

      guint64 ea64_r = ea_r;
      guint64 ea64_w = ea_w;
      tmt_writecontext_write_dynamic(write_context, (guint64) ip, 
				     TRUE, TRUE,FALSE, &ea64_r, &ea64_w, 0, 0,0);
    }
}

VOID 
A_CondBr(ADDRINT ip, UINT32 taken)
{
  if((true == start_pin) ||
     (true == InstructionStartCheck(ip)))
    {
      tmt_writecontext_write_dynamic(write_context,
				     (guint64) ip,   /* ip */
				     TRUE,           /* qp */
				     (taken > 0),    /* taken */
				     FALSE,
				
				     NULL,           /* mem src */
				     NULL,           /* mem dst */
				     0,              /* var_mem_src_size */
				     0,               /* var_mem_dst_size */
				     0   /* syscall num*/
				     );
    }
}



VOID A_Instruction(ADDRINT ip)
{
  if((true == start_pin) ||
     (true == InstructionStartCheck(ip)))
    {

      tmt_writecontext_write_dynamic(write_context, 
				     (guint64) ip,   /* ip */
				     TRUE,           /* qp */
				     FALSE,          /* taken */
				     FALSE,   /* taken */
				
				     NULL,           /* mem src */
				     NULL,           /* mem dst */
				     0,              /* var_mem_src_size */
				     0,               /* var_mem_dst_size */
				     0   /* syscall num*/
				     );
    }
}

VOID A_SysCallInstruction(ADDRINT ip, UINT32 sysCallNum)
{
  if((true == start_pin) ||
     (true == InstructionStartCheck(ip)))
    {
  
      guint64 src = 0;
      guint64 dst = 0;
      tmt_writecontext_write_dynamic(write_context, 
				     (guint64) ip,   /* ip */
				     TRUE,           /* qp */
				     FALSE,          /* taken */
				     TRUE,   /* is syscall */				
				     &src,           /* mem src */
				     &dst,           /* mem dst */
				     0,              /* var_mem_src_size */
				     0,               /* var_mem_dst_size */
				     sysCallNum  /* syscall num*/
				     );
    }
}



static inline gboolean RegIsValid(REG reg)
{
  if((stackfound == 0) && (reg == REG_STACK_PTR))
    {
      printf("Stack Register is %d\n", reg );
      stackfound = 1;
    }
  return(reg > REG_NONE);
}


VOID SysCallBase(ADDRINT ip, UINT32 sysCallNum, 
		 ADDRINT arg0,  ADDRINT arg1, 
		 ADDRINT arg2, ADDRINT arg3, 
		 ADDRINT arg4, ADDRINT arg5,
		 UINT32 tid)
{
  if((true == start_pin) ||
     (true == InstructionStartCheck(ip)))
    {

      // DEBUG START
      //      cout << "Sys Call: " << sysCallNum << " args: "<< arg0 << " " << arg1 <<" " << arg2 <<" " << arg3 <<" " << arg4 <<" " << arg5 << endl;
      // DEBUG END

      switch (sysCallNum)
	{
      
	case __NR_pwrite64:
	case __NR_write:      

	  /*
	    sys_write(uint fd, const char * buf, size_t count)
	  */
	  
// 	  A_SysCallMemoryReadAndWrite_VW(ip, 
// 					 arg1, 
// 					 arg0, 
// 					 (UINT64)(arg2), 
//					 sysCallNum);   

	  A_SysCallMemoryReadAndWrite_VRW(ip, 
					  arg1,
					  arg0, 
					  (UINT64)(arg2), 
					  0,
					  sysCallNum);   
	  break;	      

	case __NR_pread64:
	case __NR_read:

	  /*
	    sys_read(uint fd, const char * buf, size_t count)
	  */

	  A_SysCallMemoryReadAndWrite_VRW(ip, 
					  arg0, 
					  arg1, 
					  0, 
					  (UINT64)(arg2),
					  sysCallNum);  

// 	  A_SysCallMemoryReadAndWrite_VR(ip,  
// 					 arg0, 
// 					 arg1, 
// 					 (UINT64)(arg2), 
//					 sysCallNum);
	  break;
      
	case __NR_writev:
	  A_SysCallInstruction(ip, sysCallNum);
	  //      cout << "vector write" << endl;
	  break;
      
	case __NR_readv:
	  A_SysCallInstruction(ip, sysCallNum);
	  //      cout << "vector read" << endl;
	  break;
      
	default:
	  A_SysCallInstruction(ip, sysCallNum);
	  //      cout << "default: " << endl;	            
	  break;
	}
    }
}

VOID Instruction(INS ins, VOID *v)
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
  gboolean is_predicated = FALSE;
  gboolean is_var_mem_read = FALSE;
  gboolean is_var_mem_write = FALSE;
  gboolean is_sys=FALSE;
  gboolean sys_no=FALSE;
  guint i;

  // //zero out structure
  // memset(&soper, 0, sizeof(TmtStaticOper));
  // if(verflag==0)
  // 	{
  // 	  soper.ip=(ADDRINT)1;
  // 	  tmt_writecontext_write_static(write_context, &soper); 
  // 	  tmt_writecontext_write_dynamic(write_context, 
  // 					 (guint64) 1,   /* ip */
  // 					 TRUE,           /* qp */
  // 					 FALSE,          /* taken */
  // 					 FALSE,   /* taken */
  // 					 NULL,           /* mem src */
  // 					 NULL,           /* mem dst */
  // 					 0,              /* var_mem_src_size */
  // 					 0,               /* var_mem_dst_size */
  // 					 0);
  // 	  verflag=1;
  // 	}

  // set fields
  memset(&soper, 0, sizeof(TmtStaticOper));
  soper.ip = INS_Address(ins);
  soper.instr_category = INS_Category(ins);
  soper.instr_size = INS_Size(ins);

  // obtain src and dst registers
  for(i=0;i<INS_MaxNumRRegs(ins);i++)
    {
      if(RegIsValid(INS_RegR(ins,i)))
	soper.reg_src[soper.num_reg_src++] = INS_RegR(ins,i);
    }
  for(i=0;i<INS_MaxNumWRegs(ins);i++)
    {
      if(RegIsValid(INS_RegW(ins,i)))
	soper.reg_dst[soper.num_reg_dst++] = INS_RegW(ins,i);
    }

  // determine memory properties
  if (INS_IsMemoryRead(ins)) 
    {
      guint32 size = INS_MemoryReadSize(ins);

      /* !!! Deprecated Code !!!
	 VARIABLE_MEMORY_REFERENCE_SIZE is no longer
	 a valid check.  All variable-sized memory accesses
	 are now handled as iterations of a normal memory access.
	 I am keeping the TMT variable memory access code because
	 it could come in handy. (GDP)
      */
      // 	  if (size == VARIABLE_MEMORY_REFERENCE_SIZE)
      // 	    {
      // 	      is_var_mem_read = TRUE;
      // 	    }
      is_memory = TRUE;
      is_stack_read = INS_IsStackRead(ins);
      soper.mem_src_size[0] = size;
      if (INS_HasMemoryRead2(ins)) 
	{
	  soper.mem_src_size[1] = size;
	  soper.num_mem_src = 2;
	} else 
	{
	  soper.num_mem_src = 1;
	}
    }
  if (INS_IsMemoryWrite(ins)) 
    {
      guint32 size = INS_MemoryWriteSize(ins);
      // 	  if (size == VARIABLE_MEMORY_REFERENCE_SIZE) 
      // 	    {
      // 	      is_var_mem_write = TRUE;
      // 	    }
      is_memory = TRUE;
      is_stack_write = INS_IsStackWrite(ins);
      soper.mem_dst_size[0] = size;
      soper.num_mem_dst = 1;
    }
  // determine branch properties
  if (INS_IsBranchOrCall(ins))
    {
      is_branch = TRUE;
      is_direct_branch = INS_IsDirectBranchOrCall(ins);
      is_indirect_branch = INS_IsIndirectBranchOrCall(ins);
      is_call = INS_IsCall(ins);
      is_return = INS_IsRet(ins);
    }

  //
  // Instruction ALL instructions
  if(INS_IsSyscall( ins ) )
    {
      is_sys = TRUE;

      soper.num_mem_src = 1;
      soper.num_mem_dst = 1;
      is_var_mem_read = TRUE;
      is_var_mem_write = TRUE;
      is_stack_read = INS_IsStackRead(ins);
      is_stack_write = INS_IsStackWrite(ins);

      INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(SysCallBase),
  		     IARG_INST_PTR, IARG_SYSCALL_NUMBER, 
  		     IARG_SYSARG_VALUE, 0,
  		     IARG_SYSARG_VALUE, 1,
  		     IARG_SYSARG_VALUE, 2,
  		     IARG_SYSARG_VALUE, 3,
  		     IARG_SYSARG_VALUE, 4,
  		     IARG_SYSARG_VALUE, 5,
  		     IARG_THREAD_ID, IARG_END);
 
    }   
  else if (!is_branch && is_memory)
    {
      //
      // Instructions that reference memory and have no control flow
      //

      if ((soper.num_mem_src == 1) && (soper.num_mem_dst == 0))
	{
	  if (is_var_mem_read) 
	    {
	      //printf("is_var_mem_read is %d",is_var_mem_read);
	      INS_InsertCall(ins, IPOINT_BEFORE,
			     (AFUNPTR) A_MemoryRead_VR,
			     IARG_INST_PTR,
			     IARG_MEMORYREAD_EA,
			     IARG_MEMORYREAD_SIZE,
			     IARG_END);
	    } else 
	    {
	      INS_InsertCall(ins, IPOINT_BEFORE,
			     (AFUNPTR) A_MemoryRead,
			     IARG_INST_PTR,
			     IARG_MEMORYREAD_EA,
			     IARG_END);                    
	    }
	} else if ((soper.num_mem_src == 0) && (soper.num_mem_dst == 1))
	{
	  if (is_var_mem_write)
	    {
	      //printf("is_varwritteread is %d",is_var_mem_read);
	      INS_InsertCall(ins, IPOINT_BEFORE, 
			     (AFUNPTR)A_MemoryWrite_VW,
			     IARG_INST_PTR,
			     IARG_MEMORYWRITE_EA,     
			     IARG_MEMORYWRITE_SIZE,
			     IARG_END);  
	    } else
	    {
	      INS_InsertCall(ins, IPOINT_BEFORE, 
			     (AFUNPTR)A_MemoryWrite,
			     IARG_INST_PTR,
			     IARG_MEMORYWRITE_EA,     
			     IARG_END); 
	    }
	}
      else if ((soper.num_mem_src == 1) && (soper.num_mem_dst == 1))
	{
	  if (is_var_mem_read) 
	    {
	      if (is_var_mem_write) 
		{
		  INS_InsertCall(ins, IPOINT_BEFORE,
				 (AFUNPTR) A_MemoryReadAndWrite_VRW,
				 IARG_INST_PTR,
				 IARG_MEMORYREAD_EA,IARG_MEMORYWRITE_EA,
				 IARG_MEMORYREAD_SIZE,
				 IARG_MEMORYWRITE_SIZE,
				 IARG_END);
		} else
		{
		  INS_InsertCall(ins, IPOINT_BEFORE,
				 (AFUNPTR) A_MemoryReadAndWrite_VR,
				 IARG_INST_PTR,
				 IARG_MEMORYREAD_EA,IARG_MEMORYWRITE_EA,
				 IARG_MEMORYREAD_SIZE,
				 IARG_END);
		}
	    } else 
	    {
	      if (is_var_mem_write) 
		{
		  INS_InsertCall(ins, IPOINT_BEFORE,
				 (AFUNPTR) A_MemoryReadAndWrite_VW,
				 IARG_INST_PTR,
				 IARG_MEMORYREAD_EA,IARG_MEMORYWRITE_EA,
				 IARG_MEMORYWRITE_SIZE,
				 IARG_END);
		} else 
		{
		  INS_InsertCall(ins, IPOINT_BEFORE,
				 (AFUNPTR) A_MemoryReadAndWrite,
				 IARG_INST_PTR,
				 IARG_MEMORYREAD_EA,IARG_MEMORYWRITE_EA,
				 IARG_END);
		}
	    }
	} else if ((soper.num_mem_src == 2) && (soper.num_mem_dst == 0))
	{
	  if (is_var_mem_read) 
	    {
	      INS_InsertCall(ins, IPOINT_BEFORE,
			     (AFUNPTR) A_MemoryReadx2_VR,
			     IARG_INST_PTR,
			     IARG_MEMORYREAD_EA,IARG_MEMORYREAD2_EA,
			     IARG_MEMORYREAD_SIZE,
			     IARG_END);
	    } else 
	    {
	      INS_InsertCall(ins, IPOINT_BEFORE,
			     (AFUNPTR) A_MemoryReadx2,
			     IARG_INST_PTR,
			     IARG_MEMORYREAD_EA,IARG_MEMORYREAD2_EA,
			     IARG_END);
	    }
	} else if ((soper.num_mem_src == 2) && (soper.num_mem_dst == 1))
	{
	  if (is_var_mem_read) 
	    {
	      if (is_var_mem_write) 
		{
		  INS_InsertCall(ins, IPOINT_BEFORE,
				 (AFUNPTR) A_MemoryReadx2AndWrite_VRW,
				 IARG_INST_PTR,
				 IARG_MEMORYREAD_EA, IARG_MEMORYREAD2_EA, 
				 IARG_MEMORYWRITE_EA,
				 IARG_MEMORYREAD_SIZE,
				 IARG_MEMORYWRITE_SIZE,
				 IARG_END);
		} else 
		{
		  INS_InsertCall(ins, IPOINT_BEFORE,
				 (AFUNPTR) A_MemoryReadx2AndWrite_VR,
				 IARG_INST_PTR,
				 IARG_MEMORYREAD_EA, IARG_MEMORYREAD2_EA, 
				 IARG_MEMORYWRITE_EA,
				 IARG_MEMORYREAD_SIZE,
				 IARG_END);
		}
	    } else 
	    {
	      if (is_var_mem_write) {
		INS_InsertCall(ins, IPOINT_BEFORE,
			       (AFUNPTR) A_MemoryReadx2AndWrite_VW,
			       IARG_INST_PTR,
			       IARG_MEMORYREAD_EA, IARG_MEMORYREAD2_EA, 
			       IARG_MEMORYWRITE_EA,
			       IARG_MEMORYWRITE_SIZE,
			       IARG_END);
	      } else {
		INS_InsertCall(ins, IPOINT_BEFORE,
			       (AFUNPTR) A_MemoryReadx2AndWrite,
			       IARG_INST_PTR,
			       IARG_MEMORYREAD_EA, IARG_MEMORYREAD2_EA, 
			       IARG_MEMORYWRITE_EA,
			       IARG_END);
	      }
	    }
	} 
    } else if (is_branch && is_memory) {
    //
    // Instructions that reference memory and have control flow
    //
    if (is_call || is_return || is_indirect_branch) {
      // instrument pin for uncond. branches
      if (soper.num_mem_src == 1 && soper.num_mem_dst == 0) {
	INS_InsertCall(ins, IPOINT_BEFORE,
		       (AFUNPTR) A_UncondBr_MemR,
		       IARG_INST_PTR,
		       IARG_MEMORYREAD_EA,
		       IARG_END);
      } else if (soper.num_mem_src == 0 && soper.num_mem_dst == 1) {
	INS_InsertCall(ins, IPOINT_BEFORE, 
		       (AFUNPTR)A_UncondBr_MemW,
		       IARG_INST_PTR,
		       IARG_MEMORYWRITE_EA,
		       IARG_END);
      } else if (soper.num_mem_src == 1 && soper.num_mem_dst == 1) {
	INS_InsertCall(ins, IPOINT_BEFORE,
		       (AFUNPTR) A_UncondBr_MemRW,
		       IARG_INST_PTR,
		       IARG_MEMORYREAD_EA,IARG_MEMORYWRITE_EA,
		       IARG_END);
      } else if (is_var_mem_read || is_var_mem_write) {
	printf("error: uncond branch with var mem read or write\n");
	printf("error: PLEASE INSTRUMENT ME!\n");
	exit(1);
      } else {
	printf("error: uncond branch with >1 mem read or >1 mem write\n");
	printf("error: PLEASE INSTRUMENT ME!\n");
	exit(1);
      }
    } else {
      printf("error: direct branch with memory read or write\n");
      printf("error: PLEASE INSTRUMENT ME!\n");
      exit(1);
    }
  } else if (is_branch && !is_memory) {
    // instrument pin for conditional branches
    INS_InsertCall(ins, IPOINT_BEFORE, 
		   (AFUNPTR) A_CondBr,
		   IARG_INST_PTR,
		   IARG_BRANCH_TAKEN,
		   IARG_END);
  } else {
    // instrument remaining instructions
    INS_InsertCall(ins, IPOINT_BEFORE,
		   (AFUNPTR) A_Instruction,
		   IARG_INST_PTR,
		   IARG_END);  
  }
            
  // set attributes
  soper.instr_attr = tmt_static_oper_set_attr(is_branch,
					      is_direct_branch,
					      is_indirect_branch,
					      is_call,
					      is_return,
					      is_memory,
					      is_stack_read,
					      is_stack_write,
					      is_predicated,
					      is_var_mem_read,
					      is_var_mem_write,
					      is_sys);

  // obtain opcode
  tmt_static_oper_set_opcode(&soper, (gchar *) INS_Mnemonic(ins).c_str());
  
  // add the static info to the trace
  tmt_writecontext_write_static(write_context, &soper);
}

INT32 Usage() {
  g_print("\n");
  g_print("Usage:  tmt-x86 {-program <program> -trace <trace> -linestart <int>}\n");
  g_print("\n");
  g_print("        <program>  bz2 output file containing the program data\n");
  g_print("        <trace>    bz2 output file containing the trace data\n");
  g_print("        <linestart> the line number at which the trace should begin\n");
  g_print("        <filestart> the file in which the trace should begin\n");
  g_print("\n");
  return (-1);
}

static VOID
I_Aoti(IMG img, VOID *v)
{
  for (SEC sec = IMG_SecHead(img); SEC_Valid(sec); sec = SEC_Next(sec)) {

    for (RTN rtn = SEC_RtnHead(sec); RTN_Valid(rtn); rtn = RTN_Next(rtn)) {
      // Prepare for processing of RTN, an  RTN is not broken up into BBLs,
      // it is merely a sequence of INSs 
      RTN_Open(rtn);

      for (INS ins = RTN_InsHead(rtn); INS_Valid(ins); ins = INS_Next(ins)) {
	Instruction(ins,0);
      }

      // to preserve space, release data associated with RTN after we have 
      // processed it
      RTN_Close(rtn);
    }
  }
}

static void syscall_entry(THREADID tid, CONTEXT *ctx, SYSCALL_STANDARD std, VOID * v)
{
  g_print("System call entry\n");

  syscall_function_start = true;
  syscall_function_number = (int)PIN_GetSyscallNumber(ctx, std);
}

static void syscall_exit(THREADID tid, CONTEXT *ctx, SYSCALL_STANDARD std, VOID * v)
{
  g_print("System call return\n");
  
  syscall_function_start = false;
  syscall_function_number = 0;
}


VOID Fini(int n,VOID *v)
{
  tmt_writecontext_free(write_context);
}

int main(int argc, char * argv[]) 
{
  gchar * program = NULL;
  gchar * trace = NULL;

  PIN_InitSymbols();
  PIN_Init(argc, argv);

  /*
  // Not required with Pin 2.0-3235 (GDP)	
  if ( ParseCommandLine(argc,argv) ) {
  return Usage();
  }
  */

  program = (gchar *) KnobProgramFile.Value().c_str();
  trace = (gchar *) KnobTraceFile.Value().c_str();
  line_start = atoi(KnobLineStart.Value().c_str());
  file_start = KnobFileStart.Value();
    
  if (!strcmp(program,default_program)) {
    cout << "No program file specified... using " << program << endl;
  }
  if (!strcmp(trace,default_trace)) {
    cout << "No trace file specified... using " << trace << endl;
  }  

  write_context = tmt_writecontext_new(trace, program);

  // Test out syscall stuff
  //  PIN_AddSyscallEntryFunction(syscall_entry, 0);
  //  PIN_AddSyscallExitFunction(syscall_exit, 0);

  //  INS_AddInstrumentFunction(InstructionStartCheck, 0);
  IMG_AddInstrumentFunction(I_Aoti,0);
  //INS_AddInstrumentFunction(Instruction, 0);
    
  PIN_AddFiniFunction(Fini, 0);
  PIN_StartProgram();

  return 0;
}
