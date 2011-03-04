#include "reverser.h"

#define BZ_BLOCKSIZE100K_MIN 1
#define BZ_BLOCKSIZE100K_MAX 9
#define BZ_BLOCKSIZE100K 5

#define BZ_VERBOSITY_SILENT 0
#define BZ_WORKFACTOR_DEFAULT 30

#define BZ_READ_SLOW 1
#define BZ_READ_FAST 0

gchar *Error_mssgs[4] = {
    "ERROR:Wrong Number or Types of Arguments",
    "Help List for Reverser Usage",
    "ERROR:Program File Missing",
    "ERROR:Trace File Missing"
};

void Usage(gchar * pname)
{
    fprintf(stderr, "\nUSAGE\n");
    fprintf(stderr, "--------");
    fprintf(stderr, "\n%s\t--reverse {-p} <program> {-t} <trace>\n",
	    pname);
    fprintf(stderr, "%s\t--help\t//To See the Options Again.\n\n", pname);
    return;
}

void Check_usage(int argc, gchar * argv[])
{
    gchar *pname;
    guint error_type = 4;

    pname = argv[0];

    if (argc == 2 && (strcmp(argv[1], "--help")) == 0) {
	error_type = HELP;
    } else if (argc == 6) {
	if (!(strcmp(argv[2], "-p")) && !(strcmp(argv[4], "-t"))) {
	    error_type = NO_ERROR;
	} else {
	    error_type = INV_ARGS;
	}
    } else {
	error_type = INV_ARGS;
    }

    if (error_type != NO_ERROR) {
	fprintf(stderr, "\n%s\n", Error_mssgs[error_type]);
	Usage(pname);
	exit(-1);
    }

    return;
}

TempFileWriteContext *TempFileName_Init()
{
    TempFileWriteContext *tempFileWrCtx;
    tempFileWrCtx = malloc(sizeof(TempFileWriteContext));

    tempFileWrCtx->temp_file_name_prefix = "TempFile";
    tempFileWrCtx->temp_file_number = 0;

    return tempFileWrCtx;
}

Bzip2WriteContext *BZ2_WrCntxt_Init()
{
    Bzip2WriteContext *bz2_wrCtx;
    bz2_wrCtx = malloc(sizeof(Bzip2WriteContext));
    bz2_wrCtx->bzerror = BZ_OK;
    bz2_wrCtx->foError = FALSE;

    bz2_wrCtx->fileName = "reversed_trace.bz2";

    bz2_wrCtx->pFile = fopen(bz2_wrCtx->fileName, "w");
    if (!bz2_wrCtx->pFile) {
	bz2_wrCtx->foError = TRUE;
	return bz2_wrCtx;
    }

    bz2_wrCtx->pBZ2File =
	BZ2_bzWriteOpen(&bz2_wrCtx->bzerror, bz2_wrCtx->pFile,
			BZ_BLOCKSIZE100K, BZ_VERBOSITY_SILENT,
			BZ_WORKFACTOR_DEFAULT);
    if (bz2_wrCtx->bzerror == BZ_IO_ERROR) {
	fprintf(stderr, "\n**ERROR**:Opening Bzipfile for Writing!");
	//bzWriteClose ( &bz2_wrCtx->bzerror, bz2_wrCtx->pBZ2File );
	fclose(bz2_wrCtx->pFile);
	return bz2_wrCtx;
    }

    return bz2_wrCtx;
}

gboolean Open_next_temp_file(TempFileWriteContext * ctxt)
{
    gchar *temp_file_fullname;
    gboolean write_error = FALSE;

    temp_file_fullname =
	g_strdup_printf("%s_%d.tmp", ctxt->temp_file_name_prefix,
			ctxt->temp_file_number);
    ctxt->temp_file_number++;
    ctxt->fpTempFile = fopen(temp_file_fullname, "wb+");

    if (!ctxt->fpTempFile) {
	write_error = TRUE;
    }

    g_free(temp_file_fullname);

    return write_error;
}

gboolean Open_prev_temp_file(TempFileWriteContext * ctxt)
{
    gchar *temp_file_fullname;
    gboolean write_error = FALSE;
    fprintf(stderr, "\n Temp File Number = %d", ctxt->temp_file_number);
    temp_file_fullname =
	g_strdup_printf("%s_%d.tmp", ctxt->temp_file_name_prefix,
			ctxt->temp_file_number);
    fprintf(stderr, "\n Temp File Name = %s", temp_file_fullname);

    ctxt->fpTempFile = fopen(temp_file_fullname, "rb+");
    ctxt->temp_file_number--;

    if (!ctxt->fpTempFile) {
	write_error = TRUE;
    }

    g_free(temp_file_fullname);

    return write_error;
}


Reverser_config *Reverser_config_Init()
{
    Reverser_config *reverser_config;

    reverser_config = malloc(sizeof(Reverser_config));
    reverser_config->ctx = malloc(sizeof(TmtReadContext));
    return reverser_config;
}

void TempFileWrContext_Free(TempFileWriteContext * tempFileWrCtx)
{
    free(tempFileWrCtx);
    return;
}

void Reverser_config_Free(Reverser_config * reverser_config)
{
    free(reverser_config->ctx);
    free(reverser_config);
    return;
}

void Terminate_program(TempFileWriteContext * tempfileWriteCtx,
		       Reverser_config * rev_config)
{
    fprintf(stderr, "\n**ERROR Executing Program: Plz DEBUG !!!!\n");
    TempFileWrContext_Free(tempfileWriteCtx);
    Reverser_config_Free(rev_config);

}

gboolean Write_tmp_file(TempFileWriteContext * tf_ctxt,
			Reverser_oper * oper, guint nSize)
{
    gboolean tmp_wr_error;
    size_t write_size = 0;

    tmp_wr_error = FALSE;
    write_size = fwrite(oper, nSize, 1, tf_ctxt->fpTempFile);

    if (write_size != 1) {
	tmp_wr_error = TRUE;
    }

    return tmp_wr_error;
}

guint64 Get_tempFile_size(FILE * fpTempFile)
{
    guint64 file_size;
    int fd;
    struct stat st;

    fd = fileno(fpTempFile);
    fstat(fd, &st);
    file_size = ((guint64) st.st_size);
    return file_size;
}

void Close_tempFile(FILE * fp)
{
    fclose(fp);
    return;
}

gboolean Reverser_read_program(Reverser_config * rev_config,
			       TempFileWriteContext * ctxt)
{
    gint tmterr;
    gboolean program_read_error;
    gboolean tmpFile_open_error, tmpFile_wr_error;
    guint nSize;
    guint writes_since_last_check;
    guint64 total_opcount = 0;
    TmtOper oper1, oper2;
    TmtOper *oper = &oper1, *next_oper = &oper2, *tmp;
    guint64 opcount = 0;
    Reverser_oper rev_oper;
    guint16 index;

    tmpFile_open_error = FALSE;
    tmpFile_wr_error = FALSE;
    program_read_error = FALSE;
    tmterr = TMTIO_OK;
    nSize = sizeof(Reverser_oper);

    fprintf(stderr, "\nWrite SIZE = %d", nSize);
    writes_since_last_check = 0;

    tmt_readcontext_read(&tmterr, rev_config->ctx, next_oper);

    //Open the first tempFile
    tmpFile_open_error = Open_next_temp_file(ctxt);
    if (tmpFile_open_error == TRUE) {
	fprintf(stderr, "\n**ERROR**:Error Opening TempFile Number:%d\n",
		ctxt->temp_file_number);
	program_read_error = TRUE;
	Close_tempFile(ctxt->fpTempFile);
	Terminate_program(ctxt, rev_config);
    }
    fprintf(stderr, "\nWriting into TempFile -- Number: %d ..........",
	    ctxt->temp_file_number - 1);

    //start reading the trace and program files     
    while (tmterr == TMTIO_OK) {

	tmp = oper;
	oper = next_oper;
	next_oper = tmp;

	//Check the filesize of the current TempFile
	// only check trace file size once every 'trace_file_period' writes
	if (writes_since_last_check >= FILESIZE_CHECK_PERIOD) {
	    ctxt->tempFile_size = Get_tempFile_size(ctxt->fpTempFile);
	    // check if trace file size larger than limit
	    if (ctxt->tempFile_size >= TEMP_FILE_MAX) {

		//Close current temp file
		Close_tempFile(ctxt->fpTempFile);

		// open a new trace file and continue
		tmpFile_open_error = Open_next_temp_file(ctxt);
		if (tmpFile_open_error == TRUE) {
		    fprintf(stderr,
			    "\n**ERROR**:Error Opening TempFile Number:%d\n",
			    ctxt->temp_file_number - 1);
		    program_read_error = TRUE;
		    Close_tempFile(ctxt->fpTempFile);
		    Terminate_program(ctxt, rev_config);
		}
		fprintf(stderr,
			"\nWriting into TempFile -- Number: %d ..........",
			ctxt->temp_file_number - 1);
		ctxt->tempFile_size = 0;
	    }
	    writes_since_last_check = 0;
	}
	//Write into the tempfile
	//fprintf(stderr,"\n");
	rev_oper.ip = oper->soper->ip;
	for (index = 0; index < TMT_MAX_REG_SRC; index++) {
	    rev_oper.reg_src[index] = oper->soper->reg_src[index];
	    //fprintf(stderr,"\t%d,%d",rev_oper.reg_src[index],oper->soper->reg_src[index]);
	}
	for (index = 0; index < TMT_MAX_REG_DST; index++) {
	    rev_oper.reg_dst[index] = oper->soper->reg_dst[index];
	}
	rev_oper.instr_category = oper->soper->instr_category;

	//fprintf(stderr,"\nInst Category %d, %d", rev_oper.instr_category, oper->soper->instr_category);
	rev_oper.instr_attr = oper->soper->instr_attr;
	rev_oper.instr_size = oper->soper->instr_size;
	rev_oper.num_reg_src = oper->soper->num_reg_src;
	rev_oper.num_reg_dst = oper->soper->num_reg_dst;
	rev_oper.num_mem_src = oper->soper->num_mem_src;
	rev_oper.num_mem_dst = oper->soper->num_mem_dst;
	//fprintf(stderr,"\nnum of reg src:%d,%d\n",rev_oper.num_reg_src,oper->soper->num_reg_src);
	for (index = 0; index < TMT_MAX_MEM_SRC; index++) {
	    rev_oper.mem_src_size[index] = oper->mem_src_size[index];

	}
	for (index = 0; index < TMT_MAX_MEM_DST; index++) {
	    rev_oper.mem_dst_size[index] = oper->mem_dst_size[index];
	}
	for (index = 0; index < TMT_MAX_MEM_SRC; index++) {
	    rev_oper.mem_src[index] = oper->mem_src[index];
	}
	for (index = 0; index < TMT_MAX_MEM_DST; index++) {
	    rev_oper.mem_dst[index] = oper->mem_dst[index];
	}

	tmpFile_wr_error = Write_tmp_file(ctxt, &rev_oper, nSize);
	if (tmpFile_wr_error == TRUE) {
	    fprintf(stderr,
		    "\n**ERROR**:Error Writing to TempFile Number:%d\n",
		    ctxt->temp_file_number - 1);
	    program_read_error = TRUE;
	    Terminate_program(ctxt, rev_config);
	}
	// increment number of writes since last file size check
	writes_since_last_check++;

	tmt_readcontext_read(&tmterr, rev_config->ctx, next_oper);

	//increment the opcount
	++opcount;
	++total_opcount;	//increment opcount within the while loop

    }				// while loop

    if (tmterr == TMTIO_ERROR) {
	fprintf(stderr,
		"**ERROR**: Trace file ended abnormally, opcount = %lld\n",
		opcount);
	program_read_error = TRUE;
    }
    fprintf(stderr, "\n Total opcounts in read from trace = %lld",
	    total_opcount);
    //Close last temp file
    Close_tempFile(ctxt->fpTempFile);

    return program_read_error;
}

//Writes a single trace line using BZIP2
gboolean Write_Reversed_TmtOper(Reverser_oper * rev_oper,
				Bzip2WriteContext * bz2_wrCtx)
{
    gboolean err;
    err = FALSE;

    BZ2_bzWrite(&bz2_wrCtx->bzerror, bz2_wrCtx->pBZ2File, rev_oper,
		sizeof(Reverser_oper));
    if (bz2_wrCtx->bzerror != BZ_OK) {
	fprintf(stderr,
		"\n**ERROR**:Writing into reverse_trace.bz2 failed!!!");
	err = TRUE;
    }
    return err;
}

gboolean Reverser_write(TempFileWriteContext * ctxt,
			Bzip2WriteContext * bz2_wrCtx)
{
    gboolean write_error = FALSE;

    Reverser_oper *rev_oper;
    int file_position_indicator = SEEK_END;
    long int file_position_offset = 0L;
    size_t object_size = sizeof(Reverser_oper);
    size_t object_count = 1;
    size_t op_return;
    guint64 size_read = 0;
    guint64 read_index;
    guint64 file_size = 0;
    guint64 total_opcount = 0;

    //adjust temp file number
    ctxt->temp_file_number--;


    //allocate memory to the TmtOper structure      
    rev_oper = malloc(sizeof(Reverser_oper));

    while (ctxt->temp_file_number >= 0) {
	write_error = Open_prev_temp_file(ctxt);
	if (write_error == TRUE) {
	    fprintf(stderr,
		    "\n**ERROR**: Failed to open tempfile for reading.\n");
	    free(rev_oper);
	    return write_error;
	}
	//Reading from TempFile

	file_size = Get_tempFile_size(ctxt->fpTempFile);	//get the size of the temp file to be read

	read_index = 1;
	size_read = 0;
	file_position_indicator = SEEK_END;	//fixed :: always seek from end
	file_position_offset = 0L;	// initilally set to zero

	while (size_read < file_size)	//when size_read becomes >= file_size, implies that we have finished reading that tempfile
	{
	    size_read = read_index * object_size;	//at every iteration size_read increases by the size of TmtOper structure

	    read_index++;	//indexes the iteration

	    file_position_offset -= object_size;	//cumulatively sets the offset backwards (negative) by size of TmtOper
	    fseek(ctxt->fpTempFile, file_position_offset, SEEK_END);	//set file position to read in opposite drection

	    op_return =
		fread(rev_oper, object_size, object_count,
		      ctxt->fpTempFile);

	    if (op_return != object_count)	// reading file failed!! Free memory; Close temp Files amd Abort.
	    {
		fprintf(stderr,
			"\n**ERROR**: Failed to read from tempfile.\n");
		free(rev_oper);	//deallocate memory from the Reverser_oper structure
		Close_tempFile(ctxt->fpTempFile);
		system("rm -r *.tmp");
		write_error = TRUE;
		return write_error;
	    }

	    write_error = Write_Reversed_TmtOper(rev_oper, bz2_wrCtx);
	    if (write_error == TRUE) {
		fprintf(stderr,
			"\n**ERROR**:Failed Writing into Bzip file.\n");
		Close_tempFile(ctxt->fpTempFile);
		free(rev_oper);
		system("rm -r *.tmp");
		return write_error;
	    }
	    ++total_opcount;

	}			// finished reading from one tempfile successfully
	fprintf(stderr,
		"\nFinished writing into reverse_trace.bz2 from TempFile Number -- %d.",
		ctxt->temp_file_number + 1);

	//Close the tempfile
	Close_tempFile(ctxt->fpTempFile);
    }				// finished reading from all tempfiles

    fprintf(stderr, "\n SUCCESS : Finished Reading from ALL Temp files.");
    fprintf(stderr, "\nNumber of instructions reversed = %lld",
	    total_opcount);
    //Unlink the tempfiles
    system("rm -rf *.tmp");

    fprintf(stderr, "\n Temp Files have now been erased.");

    return write_error;
}
