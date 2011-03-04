#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <glib.h>

#include "reverser.h"


int main(int argc, gchar * argv[])
{

//Declare and Initialize the reverser configuration
    Reverser_config *rev_config;
    TempFileWriteContext *tempfileWriteCtx;
    Bzip2WriteContext *bz2_wrCtx;
    gboolean prog_error;

    prog_error = FALSE;

//check to see that the command line arguments were correct
    Check_usage(argc, argv);

    system("clear");

    fprintf(stderr,
	    "\nStep 1:Initializing Trace Reverser Components... ... ... ...");
    fprintf(stderr,
	    "\n-----------------------------------------------------------------------");
//Initialize the Reverser configuration
    fprintf(stderr, "\nInitializing Reverser Config...");
    rev_config = Reverser_config_Init();

    fprintf(stderr, "\nRemoving old Temp Files...");
    system("rm -rf *.tmp");

    fprintf(stderr, "\nInitializing Temp File Write Context...");
    tempfileWriteCtx = TempFileName_Init();

    fprintf(stderr, "\nInitializing Bzip2 Write Context...");
    bz2_wrCtx = BZ2_WrCntxt_Init();

//Set Reverser Configuration
    rev_config->program = argv[3];
    rev_config->trace = argv[5];
    rev_config->ctx =
	tmt_readcontext_new(rev_config->trace, rev_config->program);

//Read Trace file and write Tmtoper into temporary files each of size TEMP_FILE_MAX
    fprintf(stderr, "\n\nStep 2:Begun Reading Trace File ... ... ... ...");
    fprintf(stderr,
	    "\n--------------------------------------------------------------------------");
    prog_error = Reverser_read_program(rev_config, tempfileWriteCtx);
    if (prog_error == TRUE) {
	fprintf(stderr, "\n**Error**: Reading trace file failed!!!\n");
	Terminate_program(tempfileWriteCtx, rev_config);
    }
    fprintf(stderr, "\nCompleted Creating and Writing into Temp Files.\n");


//Write Reverse trace into reverse_trace.bz2 file
    fprintf(stderr,
	    "\nStep 3: Writing Reverse Trace to reverse_trace.bz2 file... ... ... ... ...");
    fprintf(stderr,
	    "\n--------------------------------------------------------------------------");
    prog_error = Reverser_write(tempfileWriteCtx, bz2_wrCtx);
    if (prog_error == TRUE) {
	fprintf(stderr, "\n**ERROR**: Failed Reversing Trace!!!\n");
	Terminate_program(tempfileWriteCtx, rev_config);
    }


    TempFileWrContext_Free(tempfileWriteCtx);
    Reverser_config_Free(rev_config);
    if (prog_error == FALSE) {
	fprintf(stderr,
		"\n*** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** ***");
	fprintf(stderr, "\nCHEERS!!!Program successfully ended!!\n");
	fprintf(stderr,
		"\n------------------------------------------------------------------");

    }
    return 0;
}
