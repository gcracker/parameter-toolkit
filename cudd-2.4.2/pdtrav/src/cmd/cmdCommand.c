/**CFile***********************************************************************

  FileName    [cmdCommand.c]

  PackageName [cmd]

  Synopsis    [Main module for a simple traversal of finite state machine]

  Description [External procedures included in this file are:
    <ul>
    <li>Usri_ReadCommand()
    </ul>
    ]

  SeeAlso     []  

  Author    [Gianpiero Cabodi and Stefano Quer]

  Copyright [  Copyright (c) 2001 by Politecnico di Torino.
    All Rights Reserved. This software is for educational purposes only.
    Permission is given to academic institutions to use, copy, and modify
    this software and its documentation provided that this introductory
    message is not removed, that this software and its documentation is
    used for the institutions' internal research and educational purposes,
    and that no monies are exchanged. No guarantee is expressed or implied
    by the distribution of this code.
    Send bug-reports and/or questions to: {cabodi,quer}@polito.it. ]
  
  Revision    []

******************************************************************************/

#include "cmdInt.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Stucture declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/
  
/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/


/**AutomaticEnd***************************************************************/

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Read new options setting]

  Description [Read a new command on a input line and set the number of
    parameters and their values.
    The <b> readline <\b> library is used whenever available.
    Command are read from the standard input or a command file.
    ]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/

int
CmdReadCommand (
  FILE **fin          /* Input stream */,
  int *narg           /* Number of argoments */,
  char **line         /* List of argoments */,
  char **shellLine    /* Shell command */
)
{
  int i, j;
  char *row, *tmp;
  char prompt[9] = "PdTrav> ";

  /*---------------------------- Set String ---------------------------------*/

  row = Pdtutil_Alloc (char, ROW_LEN_MAX);
  tmp = Pdtutil_Alloc (char, ROW_LEN_MAX);

  /*--------------------------- Reading Cycle -------------------------------*/

  /* 
   * Init Reading Cycle
   */

  while (1) {
    /*
     * Reading stdin or filein
     */

    if (*fin == stdin) {
      /* Read From stdin */
#if HAVE_LIBREADLINE
      row = readline (prompt);
      add_history (row);
#else
      fprintf (stdout, "%s", prompt);
      fflush (stdout);
      fgets (row, ROW_LEN_MAX, *fin);
#endif
    } else {
      /* Read From filein */
      if (fgets (row, ROW_LEN_MAX, *fin) == NULL) {
        return (1);
      } else {
        fprintf (stdout, "%s", prompt);
        fflush (stdout);
      }
    }

    /*
     * Continue in case of a NULL command (e.g., return from stdin)
     */

    if (strlen (row) <= 0) {
      continue;      
    }

    /*
     * Continue in case of a return
     */

    if (strcmp (row, "\n") == 0) {
      fprintf (stdout, "\n");
      fflush (stdout);
      continue;      
    }

    /*
     * Deal with the Command, i.e. looking for command, options,
     * and parameters
     */

    *narg = 0;
    i = 0;
    while ((i<strlen(row)) && (row[i]!='\n') ) { /* Cycle Init */
 
      /* Skip Spaces (initial or in between substrings) */
      while ((row[i]==' ') && (i<strlen(row))) {
        i++;
      }

      /* End cycle if end of string */
      if ( i>=strlen (row) ) {
        break;
      } 

      /* If end of line and no string = NULL command = continue */
      if ( (row[i]=='\n') && (*narg==0) ) {
        fprintf (stdout, "\n");
        fflush (stdout);
        continue;      
      }

      /* If first character equals '#' = comment, return what
         there is up to that moment */
      if ( (row[i]=='#') && (*narg==0) ) {
        line[*narg] = Pdtutil_Alloc (char, 2);
        strcpy (line[*narg], "#");
        (*narg)++;

        free (row);
        free (tmp);
        return (0);
      }

      /* If first character equals '!' = shell command line */
      if ( (row[i]=='!')  && (*narg == 0) ) {
        i++;
        *shellLine = Pdtutil_Alloc (char, strlen (row+i)); 
        for (j=0; i<strlen(row) && (row[i]!='\n'); i++, j++) {
          (*shellLine)[j] = row[i];
        }
        (*shellLine)[j] = '\0';
        line[*narg] = Pdtutil_Alloc (char, 2);
        strcpy(line[*narg], "!");
        (*narg)++;
        free (row);
        free(tmp);

        return (0);
      }

      /* Read String inside the Command */
      sscanf (&row[i], "%s", tmp);
      line[*narg] = Pdtutil_Alloc (char, (strlen (tmp) + 1));
      strcpy (line[*narg], tmp);
      i = i + strlen (line[*narg]);
      (*narg)++;

    } /* Cycle End */

    /*
     * End Reading Cycle - Go to the Execution Phase
     */
 
    if (*narg == 0) {
      continue;
    } else {
      break;
    }

  } /* End of Reading Cycle */

  free (row);
  free (tmp);

  return (0);
}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/


