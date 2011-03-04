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





INT32 global1=0,global2=0,global3=0,global4=0;
struct call sysnos[100];
int syscnt=1,i,noflag=0;
int syscall_number;

int tmtarr[256];

				
				
           
int SysCallEnter(ADDRINT ip,  INT32 syscall_number,  INT32 arg0)

{
      int j;
	
	tmtarr[syscall_number]++;
	//if(syscall_number==3)
        //fprintf(stdout," %d %d   \n",syscall_number,arg0);
        

}

int SysCallEnter1(ADDRINT ip,  INT32 syscall_number, INT32 *argx)

{
      int j;
	
	tmtarr[syscall_number]++;
	//if(syscall_number==3)
       // fprintf(stdout," %d %p %d %d %d %d %d\n",syscall_number,argx,*argx,*(argx+1),*(argx+2),*(argx+3),*(argx+4));
        

}

int SysCallEnter2(ADDRINT ip,  INT32 syscall_number, INT32 argx, INT32 arg1,INT32 arg2)

{
      int j;
	
	tmtarr[syscall_number]++;
	if(syscall_number==3)
        fprintf(stdout,"sys call vals  %d  %d %d  %d\n",syscall_number,argx,arg1,arg1+arg2);
        	if(syscall_number==4)
        fprintf(stdout,"sys call vals %d  %d %d  %d\n",syscall_number,argx,arg1,arg1+arg2);
   global1=arg1;
  global2=arg1+arg2;
 global3=global1;
global4=global2;
}





VOID SysCallExit(VOID *ret)
{
//fprintf(stdout,"= %d(%p)\n",ret);
fflush(stdout);
}

VOID RecordMemWrite(VOID * ip, INT32 * addr)
{
INT32 mime=0;
mime=(INT32)addr;
//fprintf(stdout,"trace %d %d",global3,global4);
if(global3<=mime & mime <=  global4)
    fprintf(stdout,"trace,%p: W %p mime %d   \n", ip, addr,mime);
}

VOID RecordMemRead(VOID * ip, INT32 * addr)
{
INT32 mime=0;
mime=(INT32)addr;
 if(global1<=mime & mime <=  global2)
    fprintf(stdout,"trace,%p: R %p global %d %d  mime %d\n", ip, addr,global1,global2,mime);
}







VOID Instruction(INS ins, VOID *v)
{
   
			if(INS_IsSyscall( ins ) )
			{
			 INS_InsertCall(ins, IPOINT_BEFORE,
                                       (AFUNPTR) SysCallEnter,
                                       IARG_INST_PTR,
		IARG_SYSCALL_NUMBER ,
		IARG_SYSARG_VALUE,
		  IARG_END);
			 INS_InsertCall(ins, IPOINT_BEFORE,
                                       (AFUNPTR) SysCallEnter1,
                                       IARG_INST_PTR,
		IARG_SYSCALL_NUMBER ,
		IARG_SYSARG_REFERENCE,
		  IARG_END);  		 	
                                  
			 INS_InsertCall(ins, IPOINT_BEFORE,
                                       (AFUNPTR) SysCallEnter2,
                                       IARG_INST_PTR,
		IARG_SYSCALL_NUMBER ,
		IARG_SYSCALL_ARG0 ,IARG_SYSCALL_ARG1 , IARG_SYSCALL_ARG2 , 
		  IARG_END);  			 
		
			

			INS_InsertCall(ins, IPOINT_BEFORE,
                                       (AFUNPTR) SysCallExit,
                                       IARG_INST_PTR,
                                       IARG_SYSRET_VALUE,
                                       IARG_END);
			}
          
                     if (INS_IsMemoryRead(ins))
    {
        INS_InsertPredicatedCall(
            ins, IPOINT_BEFORE, (AFUNPTR)RecordMemRead,
            IARG_INST_PTR,
            IARG_MEMORYREAD_EA,
            IARG_END);
    }

    if (INS_IsMemoryWrite(ins))
    {
        INS_InsertPredicatedCall(
            ins, IPOINT_BEFORE, (AFUNPTR)RecordMemWrite,
            IARG_INST_PTR,
            IARG_MEMORYWRITE_EA,
            IARG_END);
    }

          
      
	        	
}


VOID Fini(int n,VOID *v)
{
 
}







int main(int argc, char * argv[]) 
{
   

   
PIN_InitSymbols();
    PIN_Init(argc, argv);

   INS_AddInstrumentFunction(Instruction, 0);
    	
    PIN_AddFiniFunction(Fini, 0);
    		
    PIN_StartProgram();
     
    return 0;
}
