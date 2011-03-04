#include <glib.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <tmt.h>

//
// Command line parsing variables
//
static GString * program_file = NULL;
static GString * trace_file = NULL;


//
// Other globals
//
int i,j;

static struct option long_options[] = {     
    { "program", required_argument, 0, 'p'},
    { "trace", required_argument, 0, 't'},
    { 0, 0, 0, 0 }
};

// Function Prototypes
void srcdest_work(TmtReadContext *tmtrc);


/*
  Function: Main - the main

  This is the main function of the program

  Called with: int argc - number of command line arguments
               char *argv[] - the command line arguments
               
  Returns: int - zero unless something goes wrong

  Side Effects: program_file
                trace_file
 */
int main(int argc, char *argv[])
{
    TmtReadContext *tmtrc;

    // setup and parse command line
    int c;
    while (1){
        int option_index = 0;

        c = getopt_long(argc, argv, "p:t:", 
                        long_options, &option_index);
        
        // Detect the end of the options
        if (c == -1)
            break;

        switch (c)
            {
            case 't':
                trace_file = g_string_new(optarg);
                break;
            
            case 'p':
                program_file = g_string_new(optarg);
                break;
                
            default:
                break;

            }
    }

    // check that command line is good
    if (!program_file || !trace_file) {
        fprintf(stderr, "You must specify a program file and a trace file\n");
        exit(1);
    }
    
    // read in the new trace context
    tmtrc = tmt_readcontext_new(trace_file->str, program_file->str);
    if(tmtrc == NULL) {
        g_error("Unable to create read context from trace file %s "
                "and program file %s", trace_file->str, program_file->str);
    }

    // call the branch predictor work
    srcdest_work(tmtrc);

    // cleanup
    tmt_readcontext_free(tmtrc);

    return 0;
}


/*
  Function: srcdest_work - prints out sources and destinations

  This function reads in the trace context, prints out data
  about source and destination registers and memory addresses

  Called with: TmtReadContex * tmtrc - the context of this trace data

  Returns: void

  Side Effects: None
  
 */
void srcdest_work(TmtReadContext *tmtrc)
{
    gint tmterr;
    //    gboolean is_branch, is_call, is_return;
    //    gboolean is_memory, is_var_mem_read, is_var_mem_write;
    TmtOper oper1, oper2;
    TmtOper *oper = &oper1, *next_oper = &oper2, *tmp;
    TmtStaticOper *soper;
    guint64 opcount = 0;

    // grab the current operation in as the next operation..
    // because I roll like that
    tmt_readcontext_read(&tmterr, tmtrc, next_oper);

    // this iterates through all of the trace information
    while (tmterr == TMTIO_OK) {
        tmp = oper;
        oper = next_oper;
        next_oper = tmp;

        // read the next operation in this trace
        // it could be handy
        tmt_readcontext_read(&tmterr, tmtrc, next_oper);

        // grab the static operation
        soper = oper->soper;

        // check to see if the static operation is a branch
        // a branch call, and a branch return
/*         is_branch = tmt_flag_get_value(soper->instr_attr, TMT_BRANCH); */
/*         is_call   = tmt_flag_get_value(soper->instr_attr, TMT_BRANCH_CALL); */
/*         is_return = tmt_flag_get_value(soper->instr_attr, TMT_BRANCH_RETURN); */
/*         is_memory = tmt_flag_get_value(soper->instr_attr, TMT_MEMORY); */
/*         is_var_mem_read = tmt_flag_get_value(soper->instr_attr, TMT_VAR_MEM_READ); */
/*         is_var_mem_write = tmt_flag_get_value(soper->instr_attr, TMT_VAR_MEM_WRITE); */
 
        // test to make sure everything is cool
        if(tmterr == TMTIO_OK) 
            {

                // print out the instruction pointer and the opcode
                printf("%016llx: %s (", soper->ip, soper->opcode);

                //! print out register destinations
                for(j = 0; j < soper->num_reg_dst; j++) 
                    {
                        printf("%d ", soper->reg_dst[j]);
                    }

                //! print out destination memory information
                for(j = 0; j < soper->num_mem_dst; j++)
                    {
                        printf("[%llu] ", oper->mem_dst[j]);
                    }
                //            printf("[%d] = ", soper->num_mem_dst);

                // mark the start of the sources
                printf(" , ");
            
                // print out register sources
                for(j = 0; j < soper->num_reg_src; j++) 
                    {
                        printf("%d ", soper->reg_src[j]);
                    }

                //            printf("[%d]\n", soper->num_mem_src);

                //! print out source memory information
                for(j = 0; j < soper->num_mem_src; j++)
                    {
                        printf("[%llu] ", oper->mem_src[j]);
                    }
            
                // done with this instruction
                printf(")\n");         
        
            }
    }

    if (tmterr == TMTIO_ERROR) {
        printf("**ERROR**: Trace file ended abnormally, opcount = %lld\n",
               (long long int)opcount);
        exit(-1);
    }
}

