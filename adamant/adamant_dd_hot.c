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

#include <stdio.h>
#include "adamant_dd_hot.h"
#include "adamant.h"


#define BZ_BLOCKSIZE100K_MIN 1
#define BZ_BLOCKSIZE100K_MAX 9
#define BZ_BLOCKSIZE100K 9

#define BZ_VERBOSITY_SILENT 0
#define BZ_WORKFACTOR_DEFAULT 30

#define BZ_READ_SLOW 1
#define BZ_READ_FAST 0


adamantHotManager * adamant_hot_init()
{
  adamantHotManager * manager = g_try_new0(adamantHotManager, 1);

  if (manager != NULL)
    {
      manager->hot_code_hash_ = g_hash_table_new_full(g_int_hash, EqualUINT64, g_free, NULL);
      
      manager->top_hot_value_ = 0;
      manager->bottom_hot_value_ = 99999;
      
      manager->top_hot_sin_ = 0;
      manager->bottom_hot_sin_ = 0;
    }

  return (manager);
}

void adamant_hot_free(adamantHotManager * manager)
{
  g_free(manager->hot_code_hash_);
  g_free(manager);
}


guint64 adamant_hot_sinLookupInc(adamantHotManager * manager, guint64 sin)
{
  guint64 * count = g_hash_table_lookup(manager->hot_code_hash_, &sin);
  
  if(count == NULL) 
    {
      guint64 *sin_p = g_new0(guint64, 1);
      *sin_p = sin;
      count = g_new0(guint64, 1);
      *count = (guint64)1;
      g_hash_table_insert(manager->hot_code_hash_, sin_p, count);

      //! DEBUG
      count = g_hash_table_lookup(manager->hot_code_hash_, &sin);
    }
  else
    {
      *count = *count + 1;
    }

  adamant_hot_setTop(manager, sin, *count);
  adamant_hot_setBottom(manager, sin, *count);

  return(*count);
}


guint64 adamant_hot_sinLookup(adamantHotManager * manager, guint64 sin)
{
  guint64 * count = g_hash_table_lookup(manager->hot_code_hash_, &sin);
  
  if(count == NULL) 
    {			
      return(0);
    }
  else
    {
      return(*count);
    }
}


void adamant_hot_sinInc(adamantHotManager * manager, guint64 sin)
{
  guint64 * count = g_hash_table_lookup(manager->hot_code_hash_, &sin);
  
  if(count == NULL) 
    {			
      guint64 *sin_p = g_new0(guint64, 1);
      *sin_p = sin;

      count = g_new0(guint64, 1);
      *count = (guint64)1;
      g_hash_table_insert(manager->hot_code_hash_, sin_p, count);
    }
  else
    {
      *count = *count + 1;
    }

  adamant_hot_setTop(manager, sin, *count);
  adamant_hot_setBottom(manager, sin, *count);
}

inline void adamant_hot_setTop(adamantHotManager * manager, guint64 sin, guint64 value)
{
  if (value > manager->top_hot_value_)
    {
      manager->top_hot_value_ = value;
      manager->top_hot_sin_ = sin;
    }
}

inline guint64 adamant_hot_getTopValue(adamantHotManager * manager)
{
  return manager->top_hot_value_;
}

inline guint64 adamant_hot_getTopSin(adamantHotManager * manager)
{
  return manager->top_hot_sin_;
}

inline void adamant_hot_setBottom(adamantHotManager * manager, guint64 sin, guint64 value)
{
  if (value < manager->bottom_hot_value_)
    {
      manager->bottom_hot_value_ = value;
      manager->bottom_hot_sin_ = sin;
    }
}

inline guint64 adamant_hot_getBottomValue(adamantHotManager * manager)
{
  return manager->bottom_hot_value_;
}

inline guint64 adamant_hot_getBottomSin(adamantHotManager * manager)
{
  return manager->bottom_hot_sin_;
}

void adamant_hot_bufferInit(adamantHotManager * manager)
{
  FILE * fh = NULL;
  gchar * tmpFileName = NULL;
  int bzerror;
  
  //! open the temp file
  int fd = g_file_open_tmp (NULL, &tmpFileName, NULL);

  if(NULL == tmpFileName)
    {
      fprintf(stderr, "There was an error opening the output file\n");
      return;
    }

  manager->hot_buffer_fd = fd;
  manager->hot_buffer_file_name = tmpFileName;

  //  char fTemplate[] = "/tmp/fileXXXXXX";
  //  int fd = mkstemp(fTemplate);
  fh = fdopen(fd, "wb+" );

  //  fh = fopen ( manager->hot_buffer_file_name->str, "wb+" );
  if (!fh)
    {
      /* handle error */
      fprintf(stderr, "There was an error opening the output file\n");
      return;
    }

  manager->hotbuffer_bz2_file_ptr_ = fh;

  //! try to create the new BDD bz2 buffer file
  manager->hotbuffer_bz2_ptr_ = BZ2_bzWriteOpen ( &bzerror, fh, 
						  BZ_BLOCKSIZE100K, BZ_VERBOSITY_SILENT,
						  BZ_WORKFACTOR_DEFAULT );
  if (bzerror != BZ_OK)
    {
      fprintf(stderr, "There was an error opening the output file\n");      
      BZ2_bzWriteClose (&bzerror, manager->hotbuffer_bz2_ptr_, 0, NULL, NULL );
    }
}


void adamant_hot_buffer_write2read(adamantHotManager * manager)
{
    int bzerror;
    int abandon = 0;
    unsigned int nbytes_in;
    unsigned int nbytes_out;
    FILE * fh = NULL;
    
    if(manager != NULL)
        {
            //! flush data out of bz2 buffer
            BZ2_bzflush(manager->hotbuffer_bz2_ptr_);
            
            //! closes the bz2 buffer
	    BZ2_bzWriteClose(&bzerror, manager->hotbuffer_bz2_ptr_, 0, &nbytes_in, &nbytes_out);

	    fflush(manager->hotbuffer_bz2_file_ptr_);
	    //	    fclose(manager->hotbuffer_bz2_file_ptr_);
	    
	    //	    manager->hotbuffer_bz2_file_ptr_ = NULL;
            manager->hotbuffer_bz2_ptr_ = NULL;

	    rewind(manager->hotbuffer_bz2_file_ptr_);
	    fh = manager->hotbuffer_bz2_file_ptr_;

	    //	    fh = fopen ( manager->hot_buffer_file_name->str, "rb");

	    if(fh)
	      {
		manager->hotbuffer_bz2_file_ptr_ = fh;
		manager->hotbuffer_bz2_ptr_ = 
		  BZ2_bzReadOpen (&bzerror, manager->hotbuffer_bz2_file_ptr_, 
				  BZ_READ_FAST, BZ_VERBOSITY_SILENT,
				  NULL, 0 );
	    
		//! error handling
		if(bzerror != BZ_OK)
		  {
		    BZ2_bzReadClose(&bzerror,
				    manager->hotbuffer_bz2_ptr_);
		
		  }
	      }
            
            //! Side Note: If we have reached this point
            //! and hotbuffer_bz2_ptr_ is not NULL
            //! we can read from the buffer             
        }
}

int adamant_hot_buffer_readbuffer(adamantHotManager * manager, adamantHotBuffer * member)
{
  int bzerror = 0;

  //! read in data from the compressed stream
  int nRead = BZ2_bzRead( &bzerror, manager->hotbuffer_bz2_ptr_,
			  member, sizeof(adamantHotBuffer));
  
  return (nRead);
}

void adamant_hot_buffer_close(adamantHotManager * manager)
{

  if(manager->hotbuffer_bz2_ptr_ != NULL)
    {
      //! flush data out of bz2 buffer
      BZ2_bzflush(manager->hotbuffer_bz2_ptr_);
      
      //! closes the bz2 buffer
      BZ2_bzclose(manager->hotbuffer_bz2_ptr_);
    }

  if(NULL != manager->hot_buffer_file_name)
    {
      g_remove (manager->hot_buffer_file_name);
    }

  g_free(manager->hot_buffer_file_name);
  g_free(manager);
}

void adamant_hot_buffer_setfile(adamantHotManager * manager, const gchar * filename)
{
  if(manager != NULL)
    {
      manager->hot_buffer_file_name = g_strdup(filename);
    }
}

void adamant_hot_buffer_writebuffer(adamantHotManager * manager, adamantHotBuffer * member)
{
    //! add to the compressed buffer portion of the buffer
  BZ2_bzwrite(manager->hotbuffer_bz2_ptr_, 
	      member, sizeof(adamantHotBuffer));    

}

void adamant_hot_buffer_writetuple(adamantHotManager * manager, guint64 x, guint64 y)
{
  adamantHotBuffer member;
  member.din_ = x;
  member.sin_ = y;

  adamant_hot_buffer_writebuffer(manager, &member);
}
