#ifndef _REVERSE_H_
#define _REVERSE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "tmt.h"
#include "bzlib.h"

#define FILESIZE_CHECK_PERIOD 1000000
#define TEMP_FILE_MAX (G_MAXUINT32 >> 2)
#define BZ2_READ_BUF_SIZE 1000000

#define TMT_MAX_REG_SRC 9
#define TMT_MAX_REG_DST 16


//List of enumerators Used
enum {
    INV_ARGS = 0,		//Wrong Number or types of argument
    HELP = 1,			//Get Usage help
    NO_PROG = 2,		//Program file Missing
    NO_TRAC = 3,		//Trace File Missing
    NO_ERROR = 4
} Usage_Errors;

//List of structures used

//Reverser Configuration
typedef struct {
    gchar *program;		// program file name
    gchar *trace;		// trace file name
    TmtReadContext *ctx;	// Tmt Read Context

} Reverser_config;

// Context for Temp File Creation and Use
typedef struct {
    gchar *temp_file_name_prefix;	// common tempfile prefix
    gint temp_file_number;	// Index Number of the current tempfile.
    FILE *fpTempFile;		// file pointer to the current temp file
    guint64 tempFile_size;	// size of current temp file 

} TempFileWriteContext;

typedef struct {
    guint64 ip;
    guint16 reg_src[TMT_MAX_REG_SRC];
    guint16 reg_dst[TMT_MAX_REG_DST];
    guint16 instr_category;
    guint16 instr_attr;
    guint8 instr_size;
    guint8 num_reg_src;
    guint8 num_reg_dst;
    guint8 num_mem_src;
    guint8 num_mem_dst;
    guint32 mem_src_size[TMT_MAX_MEM_SRC];
    guint32 mem_dst_size[TMT_MAX_MEM_DST];
    guint64 mem_src[TMT_MAX_MEM_SRC];
    guint64 mem_dst[TMT_MAX_MEM_DST];
} Reverser_oper;

// Bzip2 Write Context
typedef struct {
    FILE *pFile;		//Points to the "reverser_trace.bz2" file
    BZFILE *pBZ2File;		//BZip2 File pointer
    char buf[BZ2_READ_BUF_SIZE + 1];	//Bzip2 Read Buffer
    int nBuf;
    int nWritten;
    char *fileName;		//filename = "reverser_trace.bz2"
    int bzerror;
    gboolean foError;

} Bzip2WriteContext;

//------------------------------------------------------------------------------------------------
//List of Reverser Functions

//Print program usage and exit
void Usage(gchar * pname);

//Check the program usage
void Check_usage(int argc, gchar * argv[]);

//Initialize a TempFileName Context
TempFileWriteContext *TempFileName_Init();

//Intialize Bzip2 File Write Context
Bzip2WriteContext *BZ2_WrCntxt_Init();

//Generate the name of the next tempfile and open
gboolean Open_next_temp_file(TempFileWriteContext * ctxt);

//Close TempFile
void Close_tempFile(FILE * fp);

//Free the TempFileName Context
void TempFileWrContext_Free(TempFileWriteContext * tempFileWrCtx);

//Terminate Program on error and free memory
void Terminate_program(TempFileWriteContext * tempfileWriteCtx,
		       Reverser_config * rev_config);

//Function to initialize a Reverser Configuration for use
Reverser_config *Reverser_config_Init();

//Function to free a Reverser Configuration from Use
void Reverser_config_Free(Reverser_config * reverser_config);

//Write to temp file
gboolean Write_tmp_file(TempFileWriteContext * tf_ctxt,
			Reverser_oper * oper, guint nSize);

//Get the size of the Tempfile
guint64 Get_tempFile_size(FILE * fpTempFile);

//READ FROM TRACE FILE - WRITE TO TEMPFILE
gboolean Reverser_read_program(Reverser_config * rev_config,
			       TempFileWriteContext * ctxt);

//Write into the BZIP2 file
gboolean Write_Reversed_TmtOper(Reverser_oper * rev_oper,
				Bzip2WriteContext * bz2_wrCtx);

//Read from tempfile backward and write into reverse_trace.bz2 file
gboolean Reverser_write(TempFileWriteContext * ctxt,
			Bzip2WriteContext * bz2_wrCtx);


#endif				/* _REVERSE_H_ */
