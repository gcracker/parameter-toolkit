#ifdef __cplusplus
extern "C" {
#endif
#ifndef TMTIO_H
#define TMTIO_H

/*
 * Header dependencies
 */
#include <glib.h>
#include <stdio.h>
#include "bzlib.h"

G_BEGIN_DECLS

/*
 * Type declarations
 */
typedef struct _TMTFILE TMTFILE;

#define TMT_READ_BUF_SIZE (1000000)

/*
 * Type definitions
 */
struct _TMTFILE
{
  FILE   * fp;
  BZFILE * bzfp; 
  char read_buffer[TMT_READ_BUF_SIZE+1];
  int read_buffer_remaining;
  char *read_buffer_cur;
  int bzip_file_ended;
};

/*
 * Defines
 */
#define TMTIO_ERROR  0
#define TMTIO_OK     1
#define TMTIO_EOF   -1

/*
 * Method declarations
 */
TMTFILE * tmt_write_open( gint * tmterror, gchar * filename );

void tmt_write( gint *tmterror, TMTFILE * tmtfp, gpointer buf, gint len );

void tmt_write_close( gint *tmterror, TMTFILE * tmtfp );

TMTFILE * tmt_read_open( gint * tmterror, gchar * filename );

void tmt_read( gint *tmterror, TMTFILE * tmtfp, gpointer buf, gint len );

void tmt_read_close( gint *tmterror, TMTFILE * tmtfp );

guint64 tmt_io_get_file_size(TMTFILE * tmtfp);

G_END_DECLS

#endif /* TMTIO_H */
#ifdef __cplusplus
}
#endif
