#include <stdio.h>
#include <stdlib.h>
#include <pin.H>
#include "tmt.h"
#include <cstring>
#include <iostream>

struct call
{
int no;
int counter;
};


TmtWriteContext * write_context;

static gchar default_program[] = "prog.bz2";
static gchar default_trace[] = "trace.bz2";
static gchar default_linestart[] = "0";
static gchar default_filestart[] = "";
static int stackfound = 0;
static int start_pin = 0;
static int line_start = 0;
static string file_start;
struct call sysnos[100];
int syscnt=1,i,noflag=0;
int syscall_number;


// Command line argument "knobs"
KNOB<std::string> KnobProgramFile(KNOB_MODE_WRITEONCE, "pintool", "program", 
				  default_program, "Program file");
KNOB<std::string> KnobTraceFile(KNOB_MODE_WRITEONCE, "pintool", "trace",
				default_trace, "Trace file");
KNOB<std::string> KnobLineStart(KNOB_MODE_WRITEONCE, "pintool", "linestart",
				default_linestart, "Line Start");
KNOB<std::string> KnobFileStart(KNOB_MODE_WRITEONCE, "pintool", "filestart",
				default_filestart, "File Start");
				
				
            	int tmtarr[300];
			int cnt=0;
int SysCallEnter(ADDRINT ip,  INT32 syscall_number, VOID *arg0)

{
      int j;
	
	tmtarr[syscall_number]++;
        fprintf(stdout," %d \n",syscall_number);
        

}







VOID SysCallExit(VOID *ret)
{
//fprintf(stdout,"= %d(%p)\n",ret);
fflush(stdout);
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
    guint p;

    int column = 0, line = 0,j=0;
    string filename;

    if(start_pin == 0)
        {
            //! if the user did not set a starting line, just start
            if((line_start == 0) && (file_start.empty()))
                {
                    start_pin = 1;
                }
            else if (line_start != 0)
                {
                    PIN_GetSourceLocation (INS_Address(ins), &column, &line, &filename);
                    if(line >= line_start)
                        {
                            if((file_start.empty()) || 
                               (filename.find(file_start, 0) != string::npos))
                                {
                            
                                    printf("Found line %d at 0x%x\n",
                                           line_start, INS_Address(ins));
                                    printf("in file %s in column %d\n\n",
                                           filename.c_str(), column);
                                    start_pin = 1;
                                }
                        }
                }
            else if (!file_start.empty())
                {
                    PIN_GetSourceLocation (INS_Address(ins), &column, &line, &filename);
                    if(filename.find(file_start, 0) != string::npos)
                        {
                            
                            printf("Found line %d at 0x%x\n",
                                   line_start, INS_Address(ins));
                            printf("in file %s in column %d\n\n",
                                   filename.c_str(), column);
                            start_pin = 1;
                        }
                        
                }
        }

    if(start_pin == 1)
        {

            //zero out structure
            memset(&soper, 0, sizeof(TmtStaticOper));

            // set fields
            soper.ip = INS_Address(ins);
            soper.instr_category = INS_Category(ins);
            soper.instr_size = INS_Size(ins);

         

          
            //
            // Instruction ALL instructions
			if(INS_IsSyscall( ins ) )
			{
			 INS_InsertCall(ins, IPOINT_BEFORE,
                                       (AFUNPTR) SysCallEnter,
                                       IARG_INST_PTR,
					 IARG_SYSCALL_NUMBER,
                                       IARG_SYSARG_VALUE,0,
                                       IARG_END);
			  		 	
			 
			INS_InsertCall(ins, IPOINT_BEFORE,
                                       (AFUNPTR) SysCallExit,
                                       IARG_INST_PTR,
                                       IARG_SYSRET_VALUE,
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
                                                        is_var_mem_write);

            // obtain opcode
            tmt_static_oper_set_opcode(&soper, (gchar *) INS_Mnemonic(ins).c_str());
  
            // add the static info to the trace
            tmt_writecontext_write_static(write_context, &soper);
        }
	        	
}


VOID Fini(int n,VOID *v)
{
  tmt_writecontext_free(write_context);
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



int main(int argc, char * argv[]) 
{
    gchar * program = NULL;
    gchar * trace = NULL;
    int j;
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
        //cout << "No program file specified... using " << program << endl;
    }
    if (!strcmp(trace,default_trace)) {
        //cout << "No trace file specified... using " << trace << endl;
    }  

    write_context = tmt_writecontext_new(trace, program);

    IMG_AddInstrumentFunction(I_Aoti,0);
    //INS_AddInstrumentFunction(Instruction, 0);
    	
    PIN_AddFiniFunction(Fini, 0);
    		
    PIN_StartProgram();
     
    return 0;
}
