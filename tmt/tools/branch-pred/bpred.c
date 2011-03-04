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

static struct option long_options[] = {     
    { "program", required_argument, 0, 'p'},
    { "trace", required_argument, 0, 't'},
    { 0, 0, 0, 0 }
};

// Function Prototypes
void branch_work(TmtReadContext *tmtrc);
int pred_work(TmtOper * oper);


/*
  Function: Main - the main

  This is the main function of the program

  Called with: int argc - number of command line arguments
               char *argv[] - the command line arguments
               
  Returns: int - zero unless something goes wrong

  Side Effects: program_file
                trace_file
                nausea, headaches, upset stomach
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
    branch_work(tmtrc);

    // cleanup
    tmt_readcontext_free(tmtrc);

    return 0;
}

/*
  Function: branch_work - prints out branch instructions and relevant data

  This function reads in the trace context, prints out branch data, and prints
  out taken/not-taken data.

  Called with: TmtReadContex * tmtrc - the context of this trace data

  Returns: void

  Side Effects: None
  
 */
void branch_work(TmtReadContext *tmtrc)
{
    gint tmterr;
    gboolean is_branch, is_call, is_return;
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
        is_branch = tmt_flag_get_value(soper->instr_attr, TMT_BRANCH);
        is_call   = tmt_flag_get_value(soper->instr_attr, TMT_BRANCH_CALL);
        is_return = tmt_flag_get_value(soper->instr_attr, TMT_BRANCH_RETURN);

        // test to make sure everything is cool
        if(tmterr == TMTIO_OK) {
            
            /* 
               Check to see if this operation
               is a branch, but not a call or
               return.
            */
            if(is_branch && !(is_call || is_return)) {

                printf("You say Branch is ");
                
                int crystal_ball = pred_work(oper);
                
                // you predict taken
                if(crystal_ball){
                    printf("taken\n");
                }
                else{
                    printf("not taken\n");
                }
                               
                printf("The correct answer is ");
                if(oper->taken){
                    printf("taken\n");
                }
                else{
                    printf("not taken\n");
                }
            }
        }
    }

    if (tmterr == TMTIO_ERROR) {
        printf("**ERROR**: Trace file ended abnormally, opcount = %lld\n",
               (long long int)opcount);
        exit(-1);
    }
}


/*
  Function: pred_work - branch predictor

  This function implements a branch predictor.  No cheating.

  Called with: TmtOper * oper - the current trace operation

  Returns: int - taken or not taken
  
  Side effects: none
  
 */
int pred_work(TmtOper * oper){

    // ! HINT !
    // oper->soper->ip contains the static ip of this instruction
    // oper contains
    
    // always taken
    return 1;
}
