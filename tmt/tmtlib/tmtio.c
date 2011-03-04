#ifdef __cplusplus
extern "C" {
#endif
#include "tmtio.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define BZ_BLOCKSIZE100K_MIN 1
#define BZ_BLOCKSIZE100K_MAX 9
#define BZ_BLOCKSIZE100K 5

#define BZ_VERBOSITY_SILENT 0
#define BZ_WORKFACTOR_DEFAULT 30

#define BZ_READ_SLOW 1
#define BZ_READ_FAST 0

static TMTFILE * 
tmt_file_new()
{
  TMTFILE * tmtfp;
  tmtfp = g_new0(TMTFILE,1);
  return (tmtfp);
}

static void
tmt_file_free(TMTFILE * tmtfp)
{
  g_free(tmtfp);
}

TMTFILE * tmt_write_open( gint * tmterror, gchar * filename )
{
  TMTFILE * tmtfp;
  int bzerror;

  tmtfp = tmt_file_new();
  tmtfp->fp = fopen(filename, "w");
  if (!tmtfp->fp) {
    if (tmterror)
      *tmterror = TMTIO_ERROR;
    g_free(tmtfp);
    return (NULL);
  }

  tmtfp->bzfp = BZ2_bzWriteOpen( &bzerror, tmtfp->fp, 
				 BZ_BLOCKSIZE100K, BZ_VERBOSITY_SILENT,
				 BZ_WORKFACTOR_DEFAULT );
  if (!tmtfp->bzfp) {
    if (tmterror)
      *tmterror = TMTIO_ERROR;
    fclose(tmtfp->fp);
    g_free(tmtfp);
    return (NULL);
  }

  if (tmterror)
    *tmterror = TMTIO_OK;
  return (tmtfp);
}

void tmt_write( gint *tmterror, TMTFILE * tmtfp, gpointer buf, gint len )
{
  int bzerror;

  BZ2_bzWrite(&bzerror, tmtfp->bzfp, buf, len);

  if ( bzerror != BZ_OK ) {
    if (tmterror)
      *tmterror = TMTIO_ERROR;
    return;
  }
  if (tmterror)
    *tmterror = TMTIO_OK;
}

void tmt_write_close( gint *tmterror, TMTFILE * tmtfp )
{
  int bzerror;
  unsigned int nbytes_in;
  unsigned int nbytes_out; 

  BZ2_bzWriteClose(&bzerror, tmtfp->bzfp, 0, &nbytes_in, &nbytes_out);
  if (bzerror != BZ_OK ) {
    if (tmterror)
      *tmterror = TMTIO_ERROR;
  }
  fflush(tmtfp->fp);
  system("sync");

  fclose(tmtfp->fp);
  tmt_file_free(tmtfp);
  if (tmterror)
    *tmterror = TMTIO_OK;
}

TMTFILE * tmt_read_open( gint * tmterror, gchar * filename )
{
  TMTFILE * tmtfp;
  int bzerror;

  tmtfp = tmt_file_new();

  tmtfp->fp = fopen(filename, "r");
  if (!tmtfp->fp) {
    if (tmterror)
      *tmterror = TMTIO_ERROR;
    g_free(tmtfp);
    return (NULL);
  }

  tmtfp->read_buffer_remaining=0;
  tmtfp->read_buffer_cur=tmtfp->read_buffer;
  tmtfp->bzip_file_ended=0;

  tmtfp->bzfp = BZ2_bzReadOpen( &bzerror, tmtfp->fp,
				BZ_READ_FAST, BZ_VERBOSITY_SILENT,
				NULL, 0 );
  if (!tmtfp->bzfp) {
    if (tmterror)
      *tmterror = TMTIO_ERROR;
    fclose(tmtfp->fp);
    g_free(tmtfp);
    return (NULL);
  }

  if (tmterror)
    *tmterror = TMTIO_OK;

  return (tmtfp);
}

#define MYMIN(x,y) (((x)<=(y))?(x):(y))
void tmt_read( gint *tmterror, TMTFILE * tmtfp, gpointer buf, gint len )
{
  int bzerror;
  int readfrombuf;
  int locallen=len;
  char *localbuf=(char *)buf;

  if (tmterror)
    *tmterror = TMTIO_OK;

  while(locallen) {
    if(tmtfp->read_buffer_remaining == 0) {
      if(tmtfp->bzip_file_ended) {
	if(tmterror) {
	  printf("Attempt to read past end of file\n");
	  *tmterror=TMTIO_ERROR;
	  return;
	}
      }

      tmtfp->read_buffer_remaining=
	BZ2_bzRead(&bzerror, tmtfp->bzfp, tmtfp->read_buffer,
		   TMT_READ_BUF_SIZE);
      
      tmtfp->read_buffer_cur=tmtfp->read_buffer;
      if ( bzerror != BZ_OK ) {
	if ( bzerror == BZ_STREAM_END ) {
	  tmtfp->bzip_file_ended=1;
	} else {
	  if (tmterror)
	    *tmterror = TMTIO_ERROR;
	  return;
	}
      }
    }
    
    readfrombuf=MYMIN(tmtfp->read_buffer_remaining,locallen);
    if(readfrombuf) {
      memcpy(localbuf,tmtfp->read_buffer_cur,readfrombuf);
    } else {
      printf("WTF, readfrombuf==0!!\n");
    }
    
    tmtfp->read_buffer_remaining -= readfrombuf;
    tmtfp->read_buffer_cur += readfrombuf;
    localbuf+=readfrombuf;
    
    locallen -= readfrombuf;
    if((tmtfp->read_buffer_remaining == 0) &&
       (tmtfp->bzip_file_ended)) {
      if(tmterror) {
	*tmterror=TMTIO_EOF;
      }
    }
  }
}

/* Old tmt_read */
/* void tmt_read( gint *tmterror, TMTFILE * tmtfp, gpointer buf, gint len ) */
/* { */
/*   int bzerror; */

/*   BZ2_bzRead(&bzerror, tmtfp->bzfp, buf, len); */

/*   if ( bzerror != BZ_OK ) { */
/*     if ( bzerror == BZ_STREAM_END ) { */
/*       if (tmterror) */
/* 	*tmterror = TMTIO_EOF; */
/*     } else { */
/*       if (tmterror) */
/* 	*tmterror = TMTIO_ERROR; */
/*     } */
/*     return; */
/*   } */
/*   if (tmterror) */
/*     *tmterror = TMTIO_OK; */
/* } */

void tmt_read_close( gint *tmterror, TMTFILE * tmtfp )
{
  int bzerror;

  BZ2_bzReadClose(&bzerror, tmtfp->bzfp);
  if (bzerror != BZ_OK ) {
    if (tmterror)
      *tmterror = TMTIO_ERROR;
    fclose(tmtfp->fp);
    tmt_file_free(tmtfp);
    return;
  }
  fclose(tmtfp->fp);
  tmt_file_free(tmtfp);
  if (tmterror)
    *tmterror = TMTIO_OK;
}

guint64 tmt_io_get_file_size(TMTFILE * tmtfp)
{
  int fd;
  struct stat st;

  fd = fileno(tmtfp->fp);
  fstat(fd, &st);
  return ((guint64) st.st_size);
}
#ifdef __cplusplus
}
#endif
