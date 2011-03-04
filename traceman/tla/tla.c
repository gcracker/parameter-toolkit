#include "tlaio.h"

#define SAMPLING_LENGTH 100000	//sampling length = 100000 instructions.
#define MAX_LIVE_NODES  100000

#define MAX_LIVE_MEMS  100000


int compare_ints(const void *pa, const void *pb)
{
    const int *a = pa;
    const int *b = pb;

    if (*a < *b)
	return -1;
    else if (*a > *b)
	return +1;
    else
	return 0;
}

FILE *pFile;
FILE *pFile2;
FILE *pFile3;

gint main(int argc, gchar * argv[])
{
    TlaOper oper;
    Tla_config *tla_config;
    gchar *fileName;
    guint64 opcount;
    guint64 total_opcount;
    int reg_index = 0;
    int index = 0;
    guint64 num_live_nodes = 0;
    guint64 num_live_reg = 0;
    guint64 num_live_mem = 0;
    guint64 bin_live_count = 0;
    guint64 bin_reg_live_count = 0;
    guint64 sampled_live_count = 0;
    guint16 reg_table[MAX_LIVE_NODES];
    guint table_index = 0;
    gboolean insert = TRUE;
    gboolean delete = TRUE;
    guint mode = ERROR;
    guint32 bin_index = 0;
    guint64 bin_width = 0;
    guint64 sample_width = 0;
    guint64 sample_offset = 0;
    gdouble mean_liveness = 0.0;
    guint64 max_liveness = 0;
    guint64 min_liveness = G_MAXUINT64;
    gdouble mean_reg_liveness = 0.0;
    guint64 max_reg_liveness = 0;
    guint64 min_reg_liveness = 0;
    PageTable64 *memory_pt;
    guint64 aligned_y;
    guint64 index_y;
    guint64 mem_index = 0;
    MemInfo *memInfo;
    guint16 mem_size;

    //PageTableEntry *ptentry = g_new0(PageTableEntry,1);

    memory_pt = PageTable64_new();	// create a new page table

    memInfo = memInfo_new();

    //fprintf(stderr, "\nmemInfo->isActive = %d", memInfo->isActive);

    pFile = fopen("reg_live_count.dat", "w+");
    pFile2 = fopen("mem_live_count.dat", "w+");
    pFile3 = fopen("reg_table.csv", "w+");
    //Check Usage
    mode = Check_usage(argc, argv);

    fileName = argv[3];		// get the file Name of the reversed file for reading 
    bin_width = atoi(argv[5]);

    /*if (mode == FULL) {
       bin_width = atoi(argv[5]);
       fprintf(pFile, "# MODE = FULL BINNED ; BIN_WIDTH = %lld",
       bin_width);
       } else if (mode == SAMPLE) {
       sample_width = atoi(argv[5]);
       sample_offset = atoi(argv[7]);
       } else {
       exit(-1);
       } */

    opcount = 0;
    total_opcount = 0;



    //Initialize the Tla Configuration
    fprintf(stderr,
	    "\nStep 1: Initializing Tla Runtime Configuration ... ... ... ... ... ... ...");
    fprintf(stderr,
	    "\n-------------------------------------------------------------------------\n");
    tla_config = Tla_Init(fileName, &oper);


    fprintf(stderr,
	    "\nStep 2: Reading from file %s ... ... ... ... ... ... ... ... ... ... ...",
	    fileName);
    fprintf(stderr,
	    "\n------------------------------------------------------------------------\n");
    //Read from BZ2 File
    while (tla_config->bz2error == BZ_OK) {
	PageTableEntry *ptentry;
	//PageTableEntry *ptentry = g_new(PageTableEntry,1);
	//Reading from BZ2 file and populating the Tlaoper structure

	Tla_Read(tla_config, &oper);
	if (tla_config->bz2error == BZ_STREAM_END) {
	    fprintf(stderr, "\nEnd of file %s reached successfully.",
		    fileName);
	    fprintf(pFile, "%d,%lld,%.3f,%lld\n", bin_index,
		    max_reg_liveness, mean_reg_liveness, min_reg_liveness);
	    fprintf(pFile2, "%d,%lld,%.3f,%lld\n", bin_index, max_liveness,
		    mean_liveness, min_liveness);
	    break;
	}
	//Analysis Stuff

/* *** *** *** *** *** *** *** *** *** REGISTER LIVENESS ANALYSIS *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** ****/
	//fprintf(stderr,"\n----------REG ANALYSIS-------------------------------------------");
	//fprintf(stderr,"\nOpcount = %lld",opcount + 1);               
	if (mode & REGL) {
	    if (oper.num_reg_src != 0) {
		//fprintf(stderr,"\nNUM OF REG SRC : %d\n",oper.num_reg_src);
		for (reg_index = 0; reg_index < oper.num_reg_src;
		     reg_index++) {
		    if (oper.reg_src[reg_index] > 95
			&& oper.reg_src[reg_index] < 127) {
			for (index = 0; index <= table_index; index++) {
			    insert = TRUE;
			    if (oper.reg_src[reg_index] ==
				reg_table[index]) {
				insert = FALSE;
				break;
			    }
			}
			if (insert == TRUE) {
			    reg_table[table_index] =
				oper.reg_src[reg_index];
			    //Insertion is successful
			    //fprintf(stderr,"\n\tLIVENESS :%lld INSERTED REG: %d",++num_live_nodes,reg_table[table_index]);
			    ++num_live_reg;
			    table_index++;

			}	//if
		    }		//for
		}
	    }
	    if (oper.num_reg_dst != 0) {
		//              fprintf(stderr,"\nNUM OF REG DST : %d\n",oper.num_reg_dst);
		for (reg_index = 0; reg_index < oper.num_reg_dst;
		     reg_index++) {
		    //              fprintf(stderr,"\n\tREG DST = %d",oper.reg_dst[reg_index]);
		    if (oper.reg_dst[reg_index] > 95
			&& oper.reg_dst[reg_index] < 127) {
			for (index = 0; index <= table_index; index++) {
			    delete = FALSE;
			    if (oper.reg_dst[reg_index] ==
				reg_table[index]) {
				delete = TRUE;
				break;
			    }
			}
			if (delete == TRUE) {

			    while (index <= table_index) {
				reg_table[index] = reg_table[index + 1];
				index++;
			    }
			    //Deletion  is successful
			    //fprintf(stderr,"\n\tLIVENESS :%lld DELETED REG: %d",--num_live_nodes,oper.reg_dst[reg_index]);
			    --num_live_reg;
			    table_index--;

			}	//if
		    }		//for
		}
	    }
	}

/********************************  MEMORY LIVENESS ANALYSIS ******************************************************************************/
	//fprintf(stderr,"\n-----------------MEM ANALYSIS------------------------------------");
	//fprintf(stderr,"\nOpcount = %lld",opcount + 1);

	if (mode & MEM) {
	    if (oper.num_mem_src != 0) {
		//fprintf(stderr,"\nNUM OF MEM SRC : %d\n",oper.num_mem_src);
		for (mem_index = 0; mem_index < oper.num_mem_src;
		     mem_index++) {
		    //fprintf(stderr,"\n\tMEM SRC = %lld, MEM SIZE = %d",oper.mem_src[mem_index], oper.mem_src_size[mem_index]);
		    for (mem_size = 0;
			 mem_size < oper.mem_src_size[mem_index];
			 mem_size++) {
			guint64 ea = oper.mem_src[mem_index] + mem_size;
			aligned_y = ea & ENTRY_KEY_LINE_MASK;
			index_y = ea & ENTRY_KEY_INDEX_MASK;
			ptentry = PageTable64_lookup(memory_pt, aligned_y);

			//fprintf(stderr,"\naligned_y = %lld,index_y = %lld\n",aligned_y,index_y);
			if (ptentry == NULL)	//new memory location used
			{
			    ptentry = g_new0(PageTableEntry, 1);
			    ptentry->key = aligned_y;
			    PageTable64_add(memory_pt, aligned_y, ptentry);
			    ptentry->data[index_y] = memInfo_new();
			    //fprintf(stderr,"\n\tLIVENESS :%lld INSERTED MEM: %lld",++num_live_mem,ea);
			    ++num_live_mem;
			} else if (ptentry != NULL) {

			    if (ptentry->data[index_y] == NULL)	//entry is not there
			    {
				ptentry->data[index_y] = memInfo_new();
				//fprintf(stderr,"\n\tLIVENESS :%lld INSERTED MEM: %lld",++num_live_mem,ea);
				++num_live_mem;
			    } else {
				memInfo =
				    (MemInfo *) ptentry->data[index_y];
				if (memInfo->isActive == 0)	//consider the case when it exists in the pagetable but had been killed by a previous def
				{
				    memInfo->isActive = 1;
				    //fprintf(stderr,"\n\tLIVENESS :%lld  REBORN MEM: %lld,%lld",++num_live_mem,ptentry->key,ea);
				    ++num_live_mem;
				}	// else it already exists in the page table and is alive
			    }
			}	//end of if
		    }
		}
	    }

	    if (oper.num_mem_dst != 0) {
		//fprintf(stderr,"\nNUM OF MEM DST : %d\n",oper.num_mem_dst);
		for (mem_index = 0; mem_index < oper.num_mem_dst;
		     mem_index++) {
		    for (mem_size = 0;
			 mem_size < oper.mem_dst_size[mem_index];
			 mem_size++) {
			guint64 ea = oper.mem_dst[mem_index] + mem_size;
			aligned_y = ea & ENTRY_KEY_LINE_MASK;
			index_y = ea & ENTRY_KEY_INDEX_MASK;

			//fprintf(stderr,"\n\tMEM DST = %lld",oper.mem_dst[mem_index]);
			ptentry = PageTable64_lookup(memory_pt, aligned_y);

			if (ptentry != NULL)	//does this memory location line exist in the page table?
			{
			    // There are two ways to handle this. 
			    //      1. Either kill the location by setting data[index_y] to 0 after checking to see whether 
			    //         that entry is one -- meaning it was a previously live memory location. If data[index_y] is 0, then do nothing.
			    //      2. Kill it in any case -- this save a branch instruction; but is invoked every time even if there are 200 successive
			    //         store instructions to the same memory location.

			    if (ptentry->data[index_y] != NULL) {
				memInfo =
				    (MemInfo *) ptentry->data[index_y];
				if (memInfo->isActive == 1) {
				    memInfo->isActive = 0;
				    --num_live_mem;
				    //fprintf(stderr,"\n\tLIVENESS :%lld KILLED MEM: %lld",num_live_mem,oper.mem_dst[mem_index]);
				}
			    }
			}
		    }
		}
	    }
	}

/************************************ Dump Data *******************************************************************************************/
	if (mode & (REGL | MEM)) {
	    //fprintf(stderr,"\nOpcount = %lld",opcount);
	    //if(opcount == 100)
	    //{
	    //      exit(-1);
	    //  }
	    bin_live_count = bin_live_count + num_live_mem;
	    bin_reg_live_count = bin_reg_live_count + num_live_reg;
	    max_liveness =
		num_live_mem > max_liveness ? num_live_mem : max_liveness;
	    min_liveness =
		num_live_mem < min_liveness ? num_live_mem : min_liveness;

	    max_reg_liveness =
		num_live_reg >
		max_reg_liveness ? num_live_reg : max_reg_liveness;
	    min_reg_liveness =
		num_live_reg <
		min_reg_liveness ? num_live_reg : min_reg_liveness;
	    if ((opcount % bin_width) == 0) {

		fprintf(stderr,
			"\n--------------------------------------------------------------------------");
		fprintf(stderr,
			"\n ****** OPCOUNT = %lld *****************",
			total_opcount);
		fprintf(stderr, "\nSample Number = %d *******************",
			bin_index);
		/*fprintf(stderr, "\nABSOLUTE LIVENESS = %lld",
		   num_live_mem);
		 */

		mean_liveness = ((double) bin_live_count) / bin_width;
		mean_reg_liveness =
		    ((double) bin_reg_live_count) / bin_width;

		//fprintf(stderr,"\n\tBIN = %lld, MAX = %lld, MEAN = %.3f, MIN=  %lld \n",bin_live_count, max_liveness, mean_liveness,min_liveness);
		fprintf(pFile, "%d,%lld,%.3f,%lld\n", bin_index,
			max_reg_liveness, mean_reg_liveness,
			min_reg_liveness);
		fprintf(pFile2, "%d,%lld,%.3f,%lld\n", bin_index,
			max_liveness, mean_liveness, min_liveness);

		mean_liveness = 0;
		bin_live_count = 0;
		max_liveness = 0;
		min_liveness = G_MAXUINT64;

		mean_reg_liveness = 0;
		bin_reg_live_count = 0;
		max_reg_liveness = 0;
		min_reg_liveness = G_MAXUINT64;

		bin_index++;
		//      opcount = 0;
		/*if(opcount >
		   for(reg_index=0;reg_index<table_index;reg_index++)
		   {
		   fprintf(pFile3,"%d,",reg_table[reg_index]);
		   }
		   fprintf(pFile3,"\n"); */
	    }
	    if (opcount >= 110000000 && opcount <= 120000000) {
		if ((opcount % 100000) == 0) {
		    fprintf(pFile3, "%lld,", opcount);
		    for (reg_index = 0; reg_index < table_index;
			 reg_index++) {
			fprintf(pFile3, "%d,", reg_table[reg_index]);
		    }
		    fprintf(pFile3, "\n");
		}
	    }

	    opcount++;

	}			//close if mode  
	total_opcount++;

    }				//while loop

    /*fprintf(stderr, "\nThe Number of Instructions read = %lld\n",
       total_opcount); */
    //Free Tla Runtime Configuration
    fclose(pFile);
    fclose(pFile2);
    fclose(pFile3);
    //g_free(ptentry);
    PageTable64_destroy(memory_pt);	//destroy the page table
    Tla_free(tla_config);
    return 0;
}
