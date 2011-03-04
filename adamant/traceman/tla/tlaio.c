/*#*STARTLICENCE*#
Copyright (c) 2005-2009, Regents of the University of Colorado

All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.

Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.

Neither the name of the University of Colorado nor the names of its
contributors may be used to endorse or promote products derived from
this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
#*ENDLICENCE*#*/


#include "tlaio.h"

#define BZ_BLOCKSIZE100K_MIN 1
#define BZ_BLOCKSIZE100K_MAX 9
#define BZ_BLOCKSIZE100K 5

#define BZ_VERBOSITY_SILENT 0
#define BZ_WORKFACTOR_DEFAULT 30

#define BZ_READ_SLOW 1
#define BZ_READ_FAST 0

//Print usage of tla onto screen
void Usage(gchar* pname)
{
        fprintf(stderr,"\nUSAGE");
        fprintf(stderr,"\n-----------------------------------------");
        fprintf(stderr,"\n%s\t--full\t{-r}\t<reversed_trace.bz2\t{-b}\t<bin_width>",pname);
        fprintf(stderr,"\n%s\t--reg\t{-r}\t<reversed_trace.bz2\t{-b}\t<bin_width>",pname);
        fprintf(stderr,"\n%s\t--mem\t{-r}\t<reversed_trace.bz2\t{-b}\t<bin_width>",pname);
	fprintf(stderr,"\n%s\t--sample\t{-r}\t<reversed_trace.bz2\t{-s}\t<sample_width>\t{-o}\t<sample_offset>",pname);
        fprintf(stderr,"%s\t--help\t//To See the Options Again.\n\n",pname);
        return;
}

//Check to see that the command line arguments were proper
guint Check_usage(int argc, gchar * argv[])
{
        gchar* pname;
        gboolean usage_error = FALSE;
	guint mode = ERROR;
		
        pname = argv[0];

        if(argc == 2 && (strcmp(argv[1],"--help"))==0) 
	{
		Usage(pname);
		exit(-1);
	}
        else if(argc == 6)
        {
           if(!(strcmp(argv[1],"--full")) && !(strcmp(argv[2],"-r")) && !(strcmp(argv[4],"-b")))
	   {
		mode = FULL;
		usage_error = FALSE;
	   }
	   else	if(!(strcmp(argv[1],"--reg")) && !(strcmp(argv[2],"-r")) && !(strcmp(argv[4],"-b")))
	   {
           	mode = REG;
		usage_error = FALSE;
           }
	   else	if(!(strcmp(argv[1],"--mem")) && !(strcmp(argv[2],"-r")) && !(strcmp(argv[4],"-b")))
	   {
		mode = MEM;
		usage_error = FALSE;
	   }
	   else 
	   {
		usage_error = TRUE;
	   }
        }
	else if(argc == 8)
	{
           if(!(strcmp(argv[1],"--sample")) && !(strcmp(argv[2],"-r")) && !(strcmp(argv[4],"-s")) && !(strcmp(argv[6],"-o")))
           {
		mode = SAMPLE;
		usage_error = FALSE;
	   }
	   else
	   {
		usage_error = TRUE;
	   }
	}
        else
        {
             usage_error = TRUE;
        }

        if(usage_error != FALSE)
        {
           fprintf(stderr,"\n** Usage Error**: Wrong number or types of argument");
           Usage(pname);
           exit(-1);
        }

        return mode;
}

//Initialize the Tla runtime configuration
Tla_config * Tla_Init(gchar * fileName, TlaOper * oper)
{
	Tla_config * tla_config;
	tla_config = malloc(sizeof(Tla_config));
	tla_config->config_error = FALSE;
	tla_config->pFile = fopen(fileName,"rb");
	
	//check to see whether the file was opened without error
	if(!(tla_config->pFile))
	{
		fprintf(stderr,"\n**ERROR**: Could not open file %s !!",fileName);
		tla_config->config_error = TRUE;
		return tla_config;
	}
	
	tla_config->pBZ2File = BZ2_bzReadOpen( &tla_config->bz2error, tla_config->pFile, BZ_READ_FAST, BZ_VERBOSITY_SILENT, NULL, 0 );
	if(tla_config->bz2error != BZ_OK)
	{
		fprintf(stderr,"\n**ERROR**: Failed to open Bzip2 File for read!!");
		tla_config->config_error = TRUE;
		return tla_config;
	}

	tla_config->buffer_size = sizeof(TlaOper);
	//fprintf(stderr,"\nRead Buffer Size = %d",tla_config->buffer_size);
	
	return tla_config;
}

//Free Tla Runtime Configuration
void Tla_free(Tla_config * tla_config)
{
	fclose(tla_config->pFile);
	BZ2_bzReadClose(&tla_config->bz2error,tla_config->pBZ2File);
	free(tla_config);
return;
}

void Tla_Read(Tla_config * tla_config, TlaOper * oper)
{
	//Reading into oper from reversed_trace.bz2 file one TlaOper at a time.
	BZ2_bzRead(&tla_config->bz2error, tla_config->pBZ2File, oper,tla_config->buffer_size);
return;
	
}

MemInfo * memInfo_new()
{
	MemInfo * memInfo;
	memInfo = g_new(MemInfo,1);
	memInfo->isActive = 1;
	return memInfo;
}



