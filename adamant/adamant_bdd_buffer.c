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

#include "adamant_bdd_buffer.h"
#include <gmp.h>
#include <sys/time.h>
#include <assert.h>


/*!
  Function: adamantbdd_buffer_init
  
  This function performs all needed initialization
  for the bdd buffer.

  Called with: adbdd_buffer * buffer - the buffer to init
               char * buffer_name - the name of the file
               char * type - the type of buffer

  Returns: int - 1 if everything went well
  Side effects: clears, initializes all buffer variables

 */
int adamantbdd_buffer_init(adbdd_buffer * buffer, char * buffer_name, char * type)
{
    FILE * fh;
    int bzerror;

    int return_value = 1;

    //http://www.pelicanprogramming.com/tutorials/libbzip2_prog1.html

    //! open the file
    fh = fopen ( buffer_name, "wb" );
    if (!fh)
        {
            /* handle error */
            printf("There was an error opening the output file\n");

            return 1;
        }

    //! set the underlying file handle
    buffer->buffer_bz2_file_ptr = fh;   

    //! try to create a new BDD zlib compressed buffer file
    //    z_stream strm;    

    //! try to create the new BDD bz2 buffer file
    buffer->buffer_bz2_ptr = BZ2_bzWriteOpen ( &bzerror, fh, 9, 0, 30 );
    if (bzerror != BZ_OK)
        {
            printf("There was an error opening the output file\n");

            BZ2_bzWriteClose (&bzerror, buffer->buffer_bz2_ptr, 0, NULL, NULL );

            //! close the underlying file handle            
            //            fclose (buffer->buffer_bz2_file_ptr);

            return 1;
        }

    //! we can now write to the bdd bz2 buffer
    //! using the command:
    //! BZ2_bzwrite(buffer_bz2_ptr,buff,len)

    //! initialize the new buffer
    buffer->type = type;
    buffer->manager = NULL;
    buffer->BDD = NULL;    
    buffer->top = NULL;    
    buffer->bit = 0;    
    buffer->buffer_file_name = buffer_name;

    return(0);
}


/*!
  Function: adamantbdd_buffer_reset
  
  This function closes a buffer and 
  all needed initialization for a new
  bdd buffer.

  Called with: adbdd_buffer * buffer - the buffer to init
               char * buffer_name - the name of the file
               char * type - the type of buffer

  Returns: int - 1 if everything went well
  Side effects: clears, initializes all buffer variables

 */
int adamantbdd_buffer_reset(adbdd_buffer * buffer)
{
    FILE * fh;
    int bzerror;

    //! check the buffer status
    if(buffer == NULL)
        {
            /* handle error */
            printf("There was NULL buffer handed to the\n");
            printf("buffer reset function\n");
	
            return 1;
        }

    //! close the buffer
    else
        {
            adamantbdd_buffer_close(buffer);
        }

    //! open the file
    fh = fopen ( buffer->buffer_file_name, "wb" );
    if (!fh)
        {
            /* handle error */
            printf("There was an error opening the output file\n");
            printf("during a BDD bufffer reset operation\n");

            return 1;
        }

    //! set the underlying file handle
    buffer->buffer_bz2_file_ptr = fh;   

    //! try to create the new BDD bz2 buffer file
    buffer->buffer_bz2_ptr = BZ2_bzWriteOpen ( &bzerror, fh, 9, 0, 30 );
    if (bzerror != BZ_OK)
        {
            printf("There was an error opening the output file\n");

            BZ2_bzWriteClose (&bzerror, buffer->buffer_bz2_ptr, 0, NULL, NULL );

            //! close the underlying file handle
            //            fclose (buffer->buffer_bz2_file_ptr);

            return 1;
        }

    return(0);
}


/*!
  Function: adamantbdd_buffer_readbuffer
  
  This function reads in from a read-only
  compressed buffer.

 */
int adamantbdd_buffer_readbuffer(adbdd_buffer * buffer, adbdd_buffer_member * member)
{
    //    adbdd_buffer_member temp_member;
    int bzerror = 0, return_value = 0;

    //! read in data from the compressed stream
    int nRead = BZ2_bzRead( &bzerror, buffer->buffer_bz2_ptr,
                            member, sizeof(adbdd_buffer_member));

    //! base the return values on the data read in
    if((bzerror == BZ_OK) || (bzerror == BZ_STREAM_END))
        {
            //            *member = temp_member;
            return_value = 1;
        }
    else
        {
            //            member = NULL;
            return_value = 0;
        }
    
    return (return_value);
}


/*!
  Function: adamantbdd_buffer_write2read
  
  This function safely closes all parts of
  a write only BDD buffer, then reopens
  the BDD buffer in read only mode.
  
  Called with; adbdd_buffer * buffer - the buffer to modify
  Returns: void  
  Side effects: void

 */
void adamantbdd_buffer_write2read(adbdd_buffer * buffer)
{
    int bzerror;
    int abandon = 0;
    unsigned int nbytes_in_lo32;
    unsigned int nbytes_in_hi32;
    unsigned int nbytes_out_lo32;
    unsigned int nbytes_out_hi32;
    FILE * fh = NULL;
    
    if(buffer != NULL)
        {
            //! flush data out of bz2 buffer
            BZ2_bzflush(buffer->buffer_bz2_ptr);
            
            //! closes the bz2 buffer
            BZ2_bzWriteClose64(&bzerror,
                               buffer->buffer_bz2_ptr,
                               abandon,
                               &nbytes_in_lo32,
                               &nbytes_in_hi32,
                               &nbytes_out_lo32,
                               &nbytes_out_hi32);

            //! now reopen the buffer in read
            //! only mode
            buffer->buffer_bz2_ptr = NULL;
            fh = fopen (buffer->buffer_file_name, "rb");
            if(fh)
                {

                    //! set the underlying file handle
                    buffer->buffer_bz2_file_ptr = fh;   

                    buffer->buffer_bz2_ptr = 
                        BZ2_bzReadOpen (&bzerror, fh, 0, 0, NULL, 0);
                    
                    //! error handling
                    if(bzerror != BZ_OK)
                        {
                            BZ2_bzReadClose(&bzerror,
                                            buffer->buffer_bz2_ptr);

                            //! close the underlying file handle
                            //                            fclose (buffer->buffer_bz2_file_ptr);

                        }
                }
            
            //! Side Note: If we have reached this point
            //! and buffer_bz2_ptr is not NULL
            //! we can read from the buffer             
        }
}


/*!
  Function: adamantbdd_buffer_read2write
  
  This function safely closes a read only BDD
  buffer then reopens the buffer for writing.  Note
  this does not append to the read buffer, but
  destroys the old read buffer.
  
  Called with; adbdd_buffer * buffer - the buffer to modify
  Returns: void  
  Side effects: void

 */
int adamantbdd_buffer_read2write(adbdd_buffer * buffer)
{
    int bzerror;
    int abandon = 0;
    unsigned int nbytes_in_lo32;
    unsigned int nbytes_in_hi32;
    unsigned int nbytes_out_lo32;
    unsigned int nbytes_out_hi32;
    FILE * fh = NULL;
    
    if(buffer != NULL)
        {
            //! close the old read buffer
            BZ2_bzReadClose(&bzerror,
                buffer->buffer_bz2_ptr);

            //! open the file
            fh = fopen ( buffer->buffer_file_name, "wb" );
            if (!fh)
                {
                    /* handle error */
                    printf("There was an error opening the output file\n");
                    
                    return 1;
                }

            //! set the underlying file handle
            buffer->buffer_bz2_file_ptr = fh;   

            //! try to create the new BDD bz2 buffer file
            buffer->buffer_bz2_ptr = BZ2_bzWriteOpen ( &bzerror, fh, 9, 0, 30 );

            if (bzerror != BZ_OK)
                {
                    printf("There was an error opening the output file\n");
                    
                    BZ2_bzWriteClose (&bzerror, buffer->buffer_bz2_ptr, 0, NULL, NULL );
                    
                    //! close the underlying file handle            
                    //            fclose (buffer->buffer_bz2_file_ptr);
                    
                    return 1;
                }
        }
}


/*!
  Function: adamantbdd_buffer_readreset
  
  This function safely closes the buffer and then
  resets for a new read cycle
  
  Called with; adbdd_buffer * buffer - the buffer to read
  Returns: void  
  Side effects: void

 */
void adamantbdd_buffer_readreset(adbdd_buffer * buffer)
{
    int bzerror;
    int abandon = 0;
    FILE * fh = NULL;
    
    if(buffer != NULL)
        {
            //! close the read in buffer
            BZ2_bzReadClose(&bzerror,
                            buffer->buffer_bz2_ptr);
            
            //! close the underlying file handle
            //            fclose (buffer->buffer_bz2_file_ptr);

            //! now reopen the buffer in read
            //! only mode
            buffer->buffer_bz2_ptr = NULL;
            fh = fopen (buffer->buffer_file_name, "rb");

            if(fh)
                {
                    //! set the underlying file handle
                    buffer->buffer_bz2_file_ptr = fh;                    

                    //! open the file in read mode
                    buffer->buffer_bz2_ptr = 
                        BZ2_bzReadOpen (&bzerror, fh, 0, 0, NULL, 0);
                    
                    //! error handling
                    if(bzerror != BZ_OK)
                        {
                            //! close the bz file
                            BZ2_bzReadClose(&bzerror,
                                            buffer->buffer_bz2_ptr);

                            //! close the underlying file handle
                            //                            fclose (buffer->buffer_bz2_file_ptr);
                        }
                }                       
        }
}


/*
  Function: adamantbdd_buffer_close
  
  This function safely closes all parts of
  the bdd buffer.
  
  Called with; adbdd_buffer * buffer - the buffer to close
  Returns: void
  
  Side effects: closes all file pointers in buffer
                clears all memory in buffer
  
 */
void adamantbdd_buffer_close(adbdd_buffer * buffer)
{

    if(buffer != NULL)
        {
            //! flush data out of bz2 buffer
            BZ2_bzflush(buffer->buffer_bz2_ptr);
            
            //! closes the bz2 buffer
            BZ2_bzclose(buffer->buffer_bz2_ptr);

            //! close the bz2 file
            //            fclose (buffer->buffer_bz2_file_ptr);           
        }
}


/*!
  Function: adamantbdd_buffer_addtuple
  
  This function adds the remaining bits of a tuple
  (the bits that were not part of the BDD) to a 
  bz2 compressed buffer.

  Called with: guint64 x - x tuple member
               guint64 y - y tuple member

  Returns: int - status
  Side effects: modifies the current tuple buffer

 */
void adamantbdd_buffer_addtuple(adbdd_buffer * buffer, guint64 x, guint64 y)
{
    adbdd_buffer_member temp_member;
    
    temp_member.data[X_MEM] = x;
    temp_member.data[Y_MEM] = y;

    //! add to the compressed buffer portion of the buffer
    BZ2_bzwrite(buffer->buffer_bz2_ptr, 
                &temp_member, sizeof(adbdd_buffer_member));    

}

