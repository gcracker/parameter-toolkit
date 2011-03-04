/**CFile***********************************************************************

  FileName    [pdtutil.c]

  PackageName [pdtutil]

  Synopsis    [Utility Functions]

  Description [This package contains general utility functions for the
    pdtrav package.]

  SeeAlso     []  

  Author      [Gianpiero Cabodi and Stefano Quer]

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

/*---------------------------------------------------------------------------*/
/* Nested includes                                                           */
/*---------------------------------------------------------------------------*/

#include "pdtutilInt.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

#define NUM_PER_ROW 5

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

/**Function********************************************************************

  Synopsis    [Check The Pdtutil_Alloc Macro.]

  Description [The Alloc mecanism is actually implemented this way in PdTRAV.
    We call Pdtutil_Alloc that it is a macro that basically maps the call on
    the Alloc function of the Cudd package. To check for a null result we
    use Pdtutil_AllocCheck.]

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

void *
Pdtutil_AllocCheck (
  void * pointer
  )
{
  if (pointer != NULL) {
    return (pointer);
  } else {
    fprintf (stdout, "FATAL ERROR: NULL Alloc Value.\n");
    exit (1);
  }
}

/**Function********************************************************************

  Synopsis    [Print on fp the desired number of charaters on a file.]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

void
Pdtutil_ChrPrint (
  FILE *fp     /* Where to Print */,
  char c       /* Which character to Print */,
  int n        /* How many Times */
  )
{
  int i;

  for (i=0; i<n; i++) {
    fprintf (fp, "%c", c);
  }
  fflush (fp);

  return;
}



/**Function********************************************************************

  Synopsis    [Given a string it Returns an Enumerated type for the verbosity
    type.]

  Description [It receives a string; to facilitate the user that string
    can be an easy-to-remember predefined code or an integer number
    (interpreted as a string).
    It returns the verbosity enumerated type.
    This verbosity mechanism is used all over the PdTRAV package.]

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

Pdtutil_VerbLevel_e
Pdtutil_VerbosityString2Enum (
  char *string     /* String to Analyze */
  )
{
  if (strcmp (string, "same")==0 || strcmp (string, "0")==0) {
    return (Pdtutil_VerbLevelNone_c);
  }

  if (strcmp (string, "none")==0 || strcmp (string, "1")==0) {
    return (Pdtutil_VerbLevelNone_c);
  }

  if (strcmp (string, "usrMin")==0 || strcmp (string, "2")==0) {
    return (Pdtutil_VerbLevelUsrMin_c);
  }

  if (strcmp (string, "usrMed")==0 || strcmp (string, "3")==0) {
    return (Pdtutil_VerbLevelUsrMed_c);
  }

  if (strcmp (string, "usrMax")==0 || strcmp (string, "4")==0) {
    return (Pdtutil_VerbLevelUsrMax_c);
  }

  if (strcmp (string, "appMin")==0 || strcmp (string, "5")==0) {
    return (Pdtutil_VerbLevelAppMin_c);
  }

  if (strcmp (string, "appMed")==0 || strcmp (string, "6")==0) {
    return (Pdtutil_VerbLevelAppMed_c);
  }

  if (strcmp (string, "appMax")==0 || strcmp (string, "7")==0) {
    return (Pdtutil_VerbLevelAppMax_c);
  }

  if (strcmp (string, "devMin")==0 || strcmp (string, "8")==0) {
    return (Pdtutil_VerbLevelDevMin_c);
  }

  if (strcmp (string, "devMed")==0 || strcmp (string, "9")==0) {
    return (Pdtutil_VerbLevelDevMed_c);
  }

  if (strcmp (string, "devMax")==0 || strcmp (string, "10")==0) {
    return (Pdtutil_VerbLevelDevMax_c);
  }

  Pdtutil_Warning (1, "Choice Not Allowed For Verbosity.");
  return (Pdtutil_VerbLevelNone_c);
}

/**Function********************************************************************

 Synopsis    [Given an Enumerated type Returns a string]

 Description []

 SideEffects [none]

 SeeAlso     []

******************************************************************************/

char *
Pdtutil_VerbosityEnum2String (
  Pdtutil_VerbLevel_e enumType
  )
{
  switch (enumType) {
    case Pdtutil_VerbLevelSame_c:
      return ("same");
      break;
    case Pdtutil_VerbLevelNone_c:
      return ("none");
      break;
    case Pdtutil_VerbLevelUsrMin_c:
      return ("usrMin");
      break;
    case Pdtutil_VerbLevelUsrMed_c:
      return ("usrMed");
      break;
    case Pdtutil_VerbLevelUsrMax_c:
      return ("usrMax");
      break;
    case Pdtutil_VerbLevelAppMin_c:
      return ("appMin");
      break;
    case Pdtutil_VerbLevelAppMed_c:
      return ("appMed");
      break;
    case Pdtutil_VerbLevelAppMax_c:
      return ("appMax");
      break;
    case Pdtutil_VerbLevelDevMin_c:
      return ("devMin");
      break;
    case Pdtutil_VerbLevelDevMed_c:
      return ("devMed");
      break;
    case Pdtutil_VerbLevelDevMax_c:
      return ("devMax");
      break;
    default:
      Pdtutil_Warning (1, "Choice Not Allowed.");
      return ("none");
      break;
  }
}

/**Function********************************************************************

  Synopsis    [Given an Enumerated type Returns an Integer]

  Description [It receives a string; to facilitate the user that string
    can be an easy-to-remember predefined code or an integer number
    (interpreted as a string).
    It returns the variable file format method type.]

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

Pdtutil_VariableOrderFormat_e
Pdtutil_VariableOrderFormatString2Enum (
  char *string     /* String to Analyze */
)
{
  if (strcmp (string, "default")==0 || strcmp (string, "0")==0) {
    return (Pdtutil_VariableOrderDefault_c);
  }

  if (strcmp (string, "oneRow")==0 || strcmp (string, "1")==0) {
    return (Pdtutil_VariableOrderOneRow_c);
  }

  if (strcmp (string, "ip")==0 || strcmp (string, "2")==0) {
    return (Pdtutil_VariableOrderPiPs_c);
  }

  if (strcmp (string, "ipn")==0 || strcmp (string, "3")==0) {
    return (Pdtutil_VariableOrderPiPsNs_c);
  }

  if (strcmp (string, "index")==0 || strcmp (string, "4")==0) {
    return (Pdtutil_VariableOrderIndex_c);
  }

  if (strcmp (string, "comment")==0 || strcmp (string, "5")==0) {
    return (Pdtutil_VariableOrderComment_c);
  }

  Pdtutil_Warning (1, "Choice Not Allowed For Profile Method.");
  return (Pdtutil_VariableOrderComment_c);
}

/**Function********************************************************************

 Synopsis    [Given an Enumerated type Returns a string]

 Description []

 SideEffects [none]

 SeeAlso     []

******************************************************************************/

char *
Pdtutil_VariableOrderFormatEnum2String (
  Pdtutil_VariableOrderFormat_e enumType
)
{
  switch (enumType) {
    case Pdtutil_VariableOrderDefault_c:
      return ("default");
      break;
    case Pdtutil_VariableOrderOneRow_c:
      return ("oneRow");
      break;
    case Pdtutil_VariableOrderPiPs_c:
      return ("ip");
      break;
    case Pdtutil_VariableOrderPiPsNs_c:
      return ("ipn");
      break;
    case Pdtutil_VariableOrderIndex_c:
      return ("index");
      break;
    case Pdtutil_VariableOrderComment_c:
      return ("comment");
      break;
    default:
      Pdtutil_Warning (1, "Choice Not Allowed.");
      return ("default");
      break;
  }
}

/**Function********************************************************************

 Synopsis    [Returns a complete file name (name, attribute, extension.]

 Description [Takes a file name, an attribute, an extension, and an overwrite
   flag.
   If the name is stdin or stdout return the name as it is.
   Add the attribute to the name.
   Add the extension to the name if it doesn't contains an
   extension already.
   If there is an extension it substitutes it if overwrite = 1.
   Create and returns the new name.]

 SideEffects [none]

 SeeAlso     []

******************************************************************************/

char *
Pdtutil_FileName (
  char *filename     /* file name */,
  char *attribute    /* attribute */,
  char *extension    /* extension */,
  int overwrite      /* overwrite the extension if 1 */
)
{
  char *name, *attr, *ext, *newfilename=NULL;
  int i, lname, lattr, lext, pointPos;

  /*------------------- Deal with NULL parameters ----------------------*/

  if (filename != NULL) {
    lname = strlen (filename);
    name =  Pdtutil_StrDup (filename);
  } else {
    lname = 0;
    name =  Pdtutil_StrDup ("");
  }
  if (attribute != NULL) {
    lattr = strlen (attribute);
    attr =  Pdtutil_StrDup (attribute);
  } else {
    lattr = 0;
    attr =  Pdtutil_StrDup ("");
  }
  if (extension != NULL) {
    lext = strlen (extension);
    ext =  Pdtutil_StrDup (extension);
  } else {
    lext = 0;
    ext =  Pdtutil_StrDup ("");
  }

  /*----------------------- Check For stdin-stdout --------------------------*/

  if (strcmp (name, "stdin")==0 || strcmp (name, "stdout") == 0) {
    return (name);
  }

  /*----------------------- Check For Name Extension ------------------------*/

  pointPos = (-1);
  for (i=0; i<lname; i++) {
    if (name[i] == '.') {
      pointPos = i;
      break;
    }
  }

  /*--------------- Add Attribute and Extension if Necessary ----------------*/

  if (pointPos < 0) {
    /*
     * There is no . in the name
     */

    /* Add both Attribute and Extension at the end of the name */
    newfilename = Pdtutil_Alloc (char, (lname+lattr+lext+1+1));
    sprintf (newfilename, "%s%s.%s", name, attr, ext);
  } else {
    /*
     * There is a . in the name
     * (strcpy, strncpy, strcat seem to give some problem here ...)
     */

    newfilename = Pdtutil_Alloc (char, (lname+lattr+1));

    /* Copy the Name */
    for (i=0; i<pointPos; i++) {
      newfilename[i] = name[i];
    }
    
    /* Add Attribute in the Middle */
    for (i=0; i<lattr; i++) {
      newfilename[pointPos+i] = attr[i];
    }
    
    if (overwrite==0) {
      /* Add old extension */
      for (i=pointPos; i<lname; i++) {
        newfilename[i+lattr] = name[i];
      }
      newfilename[lname+lattr] = '\0';
    } else {
      /* Add new extension */
      newfilename[pointPos+lattr] = '.';
      for (i=0; i<lext; i++) {
        newfilename[pointPos+lattr+1+i] = ext[i];
      }
      newfilename[pointPos+lattr+lext+1] = '\0';
    }
  }

  return (newfilename);
}


/**Function********************************************************************

 Synopsis    [Opens a file checking for files already opened or impossible
   to open.]

 Description [It receives a file pointer, a file name, and a mode.
   If the file pointer is not null, the file has been already opened
   at this pointer is returned.
   If the filename is "stdin" or "stdout" it considers that as a synonym
   for standard input/output.
   Open mode can be "r" for read or "w" for write.
   In particularly, "rt" open for read a text file and "rb" open
   for read a binary file; "wt" write a text file and "wb" write 
   a binary file.<BR>
   Returns a pointer to the file and an integer flag that is 1 if a file
   has been opened.]

 SideEffects [none]

 SeeAlso     [Pdtutil_FileClose]

******************************************************************************/

FILE *
Pdtutil_FileOpen (
  FILE *fp           /* IN: File Pointer */,
  char *filename     /* IN: File Name */,
  char *mode         /* IN: Open Mode */,
  int *flag          /* OUT: File Opened (if 1) */
  )
{
  *flag = 0;

  if (fp != NULL) {
    return (fp);
  }

  /*----------------------- Check for NULL filename -------------------------*/

  if (filename == NULL) {
    if (strcmp (mode, "r")==0 || strcmp (mode, "rt")==0 ||
       strcmp (mode, "rb")==0) {
      fp = stdin;
    } else {
      fp = stdout;
    }
    return (fp);
  }

  /*--------------------------- Check for stdin -----------------------------*/

  if (strcmp (filename, "stdin") == 0 && 
    (strcmp (mode, "r") || strcmp (mode, "rt") || strcmp (mode, "rb")) ) {
      fp = stdin;
      return (fp);
  }

  /*--------------------------- Check for stdout ----------------------------*/

  if (strcmp (filename, "stdout") == 0 && 
    (strcmp (mode, "w") || strcmp (mode, "wt") || strcmp (mode, "wb")) ) {
      fp = stdout;
      return (fp);
  }

  /*----------------------------- Open File ---------------------------------*/

  *flag = 1;
  fp = fopen (filename, mode);
  if (fp == NULL) {
    fprintf (stderr, "Error : Cannot open file %s.\n", filename);
    return NULL;
  }

  return (fp);
}



/**Function********************************************************************

 Synopsis    [Closes a file.]

 Description [It closes a file if it is not the standard input or output
   and flag is 1.
   This flag has to be used with the one returned by Pdtutil_FileOpen: If
   a file has been opened is closed too.]

 SideEffects [none]

 SeeAlso     [Pdtutil_FileOpen]

******************************************************************************/

void
Pdtutil_FileClose (
  FILE *fp    /* IN: File Pointer */,
  int *flag   /* IN-OUT: File to be Closed */ 
)
{
  if ( (fp!=stdin) && (fp!=stdout) && (*flag==1) ) {
    *flag = 0;
    fclose (fp);
  }

  return;
}

/**Function********************************************************************

 Synopsis    [Parses hierarchical names separated by '.']

 Description [It receives a string. It returns two strings: The first one
   is the content of the original string before the first character "."
   found in the string. The second one is the remaining part of the
   string.
   If the initial string is NULL two NULL pointers are returned.
   If the initial string doen't contain a sharater '.' the first string is
   equal to the original one and the second is NULL.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/

void
Pdtutil_ReadSubstring (
  char *stringIn           /* String to be parsed */,
  char **stringFirstP      /* First Sub-String Pointer */,
  char **stringSecondP     /* Second Sub-String Pointer */
)
{
  int i, pointPos;

  *stringFirstP = NULL;
  *stringSecondP = NULL;

  if (stringIn == NULL) {
    return;
  }

  pointPos = (-1);
  for (i=0; i<strlen (stringIn); i++) {
    if (stringIn[i]=='.') {
      pointPos = i;
      break;
    }
  }

  /*--------------- Add Attribute and Extension if Necessary ----------------*/

  if (pointPos < 0) {
    /*
     * There is no . in the name
     */

    *stringFirstP = Pdtutil_StrDup (stringIn);
  } else {
    /*
     * There is a . in the name
     */

    *stringFirstP = Pdtutil_Alloc (char, pointPos+1);
    for (i=0; i<pointPos; i++) {
      (*stringFirstP)[i] = stringIn[i];
    }
    (*stringFirstP)[i] = '\0';
    *stringSecondP = Pdtutil_StrDup (&stringIn[pointPos+1]);
  }

  return;
}

/**Function********************************************************************

  Synopsis    [parses hierarchical names separated by '.']

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/

void
Pdtutil_ReadName(
  char *extName,     /* String to be parsed */
  int *nstr,         /* Number of names extracted */
  char **names,      /* Array of names */
  int maxNstr        /* Maximum number of names */
)
{
  char *string;
  char *token;

  *nstr = 0;

  if (extName[0] == '\0') {
    return;
  }

  /*Duplicate extName, because strtok writes the string*/
  string = Pdtutil_StrDup(extName);
  /*Extract first name*/
  if((token = strtok(string, "."))==NULL) {
    return;
  } else {
    names[*nstr] = Pdtutil_StrDup(token);
    (*nstr)++;
  }
  /*extract remaining names*/
  while((token=strtok(NULL, "."))!=NULL) {
    if(*nstr > maxNstr) {
      fprintf(stderr,"Error: Name Too Long.\n");
      break;
    } 
    names[*nstr] = Pdtutil_StrDup(token);
    (*nstr)++;
  };
  free(string);

  return;
}



/**Function*******************************************************************

  Synopsis    [Duplicates a string]

  Description [Duplicates a string and returns it.]

  SideEffects [none]

  SeeAlso     []

*****************************************************************************/

char *
Pdtutil_StrDup (
  char *str
)
{
  char *str2;

  if (str == NULL) {
    return (NULL);
  }

  str2 = Pdtutil_Alloc (char, strlen (str)+1);
  if (str2 != NULL) {
    strcpy (str2, str);
  }

  return (str2);
}

/**Function********************************************************************

  Synopsis           [Reads the order in the <em>.ord</em> file.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/

int 
Pdtutil_OrdRead (
  char ***pvarnames                             /* Varnames Array */,
  int **pvarauxids                              /* Varauxids Array */,
  char *filename                                /* File Name */,  
  FILE *fp                                      /* File Pointer */,
  Pdtutil_VariableOrderFormat_e ordFileFormat   /* File Format */
)
{
  int flagFile;
  int nvars, size, id, i, continueFlag;
  char buf[PDTUTIL_MAX_STR_LEN+1], name[PDTUTIL_MAX_STR_LEN+1];
  char **varnames=NULL;
  int *varauxids=NULL;

  fp = Pdtutil_FileOpen (fp, filename, "r", &flagFile);
  if (fp == NULL) {
    return (-1);
  }

  size = PDTUTIL_INITIAL_ARRAY_SIZE;
  varnames = Pdtutil_Alloc (char *,size);
  varauxids = Pdtutil_Alloc (int,size);

  i = 0;
  continueFlag = 1;
  while (continueFlag) {
    /* Check for Read Format */
    if (ordFileFormat == Pdtutil_VariableOrderOneRow_c) {
      if (fscanf (fp, "%s", name) == EOF) {
        break;
      } else {
        id = i;
      }
    } else {
      if (fgets (buf, PDTUTIL_MAX_STR_LEN, fp) == NULL) {
        break;
      } else {
        if (buf[0] == '#') {
          continue;
        }  

        if (sscanf (buf, "%s %d", name, &id) < 2) {
          id = i;
          if (sscanf (buf, "%s", name) < 1) {
            continue;
          }
        }
      }
    }

    if (i >= size) {
      /* No more room in arrays: resize */
      size *= 2;
      varnames = Pdtutil_Realloc (char *, varnames, size);
      varauxids = Pdtutil_Realloc (int, varauxids, size);
      if ((varnames == NULL) || (varauxids == NULL)) {
        return (-1);
      }
    }

    varnames[i] = Pdtutil_StrDup (name);
    varauxids[i] = id;
    i++;
  }

  nvars = i;

  if (nvars<size) {
    /* Too much memory allocated: resize */
    varnames = Pdtutil_Realloc (char *, varnames, nvars);
    varauxids = Pdtutil_Realloc (int, varauxids, nvars);
    if ((varnames == NULL) || (varauxids == NULL)) {
      return (-1);
    }
  }

  *pvarnames = varnames;
  *pvarauxids = varauxids;

  Pdtutil_FileClose (fp, &flagFile);

  return (nvars);
}


/**Function********************************************************************

  Synopsis           [Write the order in the <em>.ord</em> file.]

  Description        [Write the order to file, using variable names or
    auxids (or both of them). Array of sorted ids is used if the sortedIds
    parameter is not NULL. Produce different slightly different format
    depending on the format parameter:
    Pdtutil_VariableOrderPiPs_c
      only variables names for Primary Input and Present State Variables are
      stored (one for each row)
    Pdtutil_VariableOrderPiPsNs_c
      store previous information + Next State Variables Names
    Pdtutil_VariableOrderIndex_c
      store previous information + Variable Auxiliary Index
    Pdtutil_VariableOrderComment_c
      store previous information + Comments (row starting for #)
    ]

  SideEffects        []

  SeeAlso            []

******************************************************************************/

int 
Pdtutil_OrdWrite (
  char **varnames                               /* Varnames Array */,
  int *varauxids                                /* Varauxids Array */,
  int *sortedIds                                /* Variable Permutations */,
  int nvars                                     /* Number of Variables */,
  char *filename                                /* File Name */,  
  FILE *fp                                      /* File Pointer */,
  Pdtutil_VariableOrderFormat_e ordFileFormat   /* File Format */
  )
{
  int flagFile;
  int i, j, k;

  fp = Pdtutil_FileOpen (fp, filename, "w", &flagFile);
  if (fp == NULL) {
    return (1);
  } 

  /*
   *  Print out initial comment
   */

  if (ordFileFormat == Pdtutil_VariableOrderComment_c) {
    fprintf (fp, "# ");
    if (varnames != NULL) {
      fprintf (fp, "%-38s", "names");
    }
    if (varauxids != NULL) {
      fprintf (fp, "auxids");
    }
  fprintf (fp, "\n");
  }

  /*
   *  Print out element
   */

  for (k=0, i=0; i<nvars; i++) {
    if (sortedIds != NULL) {
      j = sortedIds[i];
    }  else  {
      j = i;
    } 

    Pdtutil_Assert (j<nvars, "Wrong permutation in WriteOrder.");

    /* Skip Next State Variables */
    if ( ((ordFileFormat==Pdtutil_VariableOrderOneRow_c) ||
      (ordFileFormat==Pdtutil_VariableOrderPiPs_c)) &&
      varnames!=NULL && varnames[j]!=NULL) {
      if (strstr (varnames[j], "$NS")!=NULL) {
        continue;
      }
    }

    /* Store Variable Names For One Row Case */
    if ((ordFileFormat==Pdtutil_VariableOrderOneRow_c) && (varnames!=NULL)) {
      Pdtutil_Assert (varnames[j] != NULL,
        "Not supported NULL varname while writing ord.");
      k++;
      if ((k%NUM_PER_ROW) == 0) {
        fprintf (fp, "%s\n", varnames[j]);
      } else {
        fprintf (fp, "%s ", varnames[j]);
      }
      continue;
    }

    /* Store Variable Name */
    if (varnames != NULL) {
      Pdtutil_Assert (varnames[j] != NULL,
        "Not supported NULL varname while writing ord");
      fprintf (fp, "%-40s", varnames[j]);
    }

    /* Store Variable Auxiliary Index */
    if (ordFileFormat >= Pdtutil_VariableOrderIndex_c) {
      if (varauxids != NULL) {
        fprintf (fp, " %d", varauxids[j]);
      } 
    }

    /* Next Row */
    fprintf (fp, "\n");
  } /* End of for i ... */

  Pdtutil_FileClose (fp, &flagFile);

  return (0);
}

/**Function********************************************************************

  Synopsis    [Duplicates an array of strings]

  Description [Allocates memory and copies source array] 

  SideEffects [None]

  SeeAlso     []

******************************************************************************/

char **
Pdtutil_StrArrayDup (
  char **array       /* array of strings to be duplicated */,
  int n              /* size of the array */
)
{
  char **array2;
  int i;

  if (array == NULL) {
    return (NULL);
  }

  array2 = Pdtutil_Alloc(char *, n);
  if (array2 == NULL) {
    Pdtutil_Warning (1, "Error allocating memory.");
    return (NULL);
  }

  /*
   *  Initialize all slots to NULL for fair FREEing in case of failure
   */

  for (i=0; i<n; i++) {
    array2[i] = NULL;
  }


  for (i=0; i<n; i++) { 
    if (array[i] != NULL) {
      if ((array2[i]=Pdtutil_StrDup(array[i]))==NULL) {
        Pdtutil_StrArrayFree(array2,n);
        return (NULL);
      }
    }
  }

  return (array2);
}


/**Function********************************************************************

  Synopsis    [Inputs an array of strings]

  Description [Allocates memory and inputs source array. Skips anything from
    '#' to the end-of-line (a comment).
    ] 

  SideEffects [None]

  SeeAlso     []

******************************************************************************/

int 
Pdtutil_StrArrayRead (
  char ***parray     /* array of strings (by reference) */,
  FILE *fp           /* file pointer */
)
{
  int i, nvars, size;
  char buf[PDTUTIL_MAX_STR_LEN+1];
  char **array;

  Pdtutil_Assert(fp!=NULL,"reading from a NULL file");

  size = PDTUTIL_INITIAL_ARRAY_SIZE ;
  array = Pdtutil_Alloc(char *,size);

  for (i=0; (fscanf (fp, "%s", buf)!=EOF);) {
    if (buf[0] == '#') { /* comment: skip to end-of-line */
      fgets (buf, PDTUTIL_MAX_STR_LEN, fp);
      continue;
    }
    if (i>=size) { /* no more room in array: resize */
      size *= 2;
      array = Pdtutil_Realloc(char *, array, size);
      if (array == NULL)
        return (-1);
    }
    array[i++] = Pdtutil_StrDup(buf);
  }

  nvars = i;

  if (nvars<size) { /* too much memory allocated: resize */
    array = Pdtutil_Realloc(char *, array, size);
    if (array == NULL)
      return (-1);
  }

  *parray = array;

  return (nvars);
}

/**Function********************************************************************

  Synopsis    [Outputs an array of strings]

  Description [Allocates memory and inputs source array] 

  SideEffects [None]

  SeeAlso     []

******************************************************************************/

int
Pdtutil_StrArrayWrite (
  FILE *fp           /* output file */,
  char **array       /* array of strings */,
  int n              /* size of the array */
)
{
  int i;

  Pdtutil_Assert(fp!=NULL,"writing to a NULL file");

  for (i=0; i<n; i++) { 
    if (fprintf(fp,"%s\n", array[i]) == EOF) {
      (void) fprintf (stderr,"Pdtutil_StrArrayWrite: Error writing to file\n");
      return EOF;
    }
  }

  return n;
}


/**Function********************************************************************

  Synopsis    [Frees an array of strings]

  Description [Frees memory for strings and the array of pointers] 

  SideEffects [None]

  SeeAlso     []

******************************************************************************/

void
Pdtutil_StrArrayFree (
  char **array       /* array of strings */,
  int n              /* size of the array */
)
{
  int i;

  if (array == NULL) {
    return;
  }

  for (i=0; i<n; i++) {
    Pdtutil_Free (array[i]);
  }

  Pdtutil_Free(array);

  return;
}


/**Function********************************************************************

  Synopsis    [Duplicates an array of ints]

  Description [Allocates memory and copies source array] 

  SideEffects [None]

  SeeAlso     []

******************************************************************************/

int *
Pdtutil_IntArrayDup (
  int *array         /* Array of Integers to be Duplicated */,
  int n              /* size of the array */
  )
{
  int i, *array2;

  if (array == NULL) {
    return (NULL);
  }

  array2 = Pdtutil_Alloc (int, n);
  if (array2 == NULL) {
    (void) fprintf (stdout,"Pdtutil_IntArrayDup: Error allocating memory\n");
    return NULL;
  }

  for (i=0; i<n; i++) { 
    array2[i] = array[i];
  }

  return (array2);
}

/**Function********************************************************************

  Synopsis    [Inputs an array of ints]

  Description [Allocates memory and inputs source array. Skips anything from
    '#' to the end-of-line (a comment).
    ] 

  SideEffects [None]

  SeeAlso     []

******************************************************************************/

int 
Pdtutil_IntArrayRead (
  int **parray     /* array of strings (by reference) */,
  FILE *fp           /* file pointer */
)
{
  int i, nvars, size;
  char buf[PDTUTIL_MAX_STR_LEN+1];
  int *array;

  Pdtutil_Assert(fp!=NULL,"reading from a NULL file");

  size = PDTUTIL_INITIAL_ARRAY_SIZE;
  array = Pdtutil_Alloc (int, size);

  for (i=0; (fscanf (fp, "%s", buf)!=EOF);) {
    if (buf[0] == '#') { /* comment: skip to end-of-line */
      fgets (buf, PDTUTIL_MAX_STR_LEN, fp);
      continue;
    }
    if (i>=size) { /* no more room in array: resize */
      size *= 2;
      array = Pdtutil_Realloc(int, array, size);
      if (array == NULL)
        return (-1);
    }
    sscanf(buf, "%d", &array[i++]);
  }

  nvars = i;

  if (nvars<size) { /* too much memory allocated: resize */
    array = Pdtutil_Realloc(int, array, size);
    if (array == NULL)
      return (-1);
  }

  *parray = array;

  return (nvars);
}


/**Function********************************************************************

  Synopsis    [Outputs an array of ints]

  Description [Outputs an array of ints] 

  SideEffects [None]

  SeeAlso     []

******************************************************************************/

int
Pdtutil_IntArrayWrite (
  FILE *fp           /* output file */,
  int  *array        /* array of ints */,
  int n              /* size of the array */
)
{
  int i;

  Pdtutil_Assert(fp!=NULL,"writing to a NULL file");

  for (i=0; i<n; i++) { 
    if (fprintf(fp,"%d\n", array[i]) == EOF) {
      (void) fprintf (stderr,"Pdtutil_IntArrayWrite: Error writing to file\n");
      return EOF;
    }
  }

  return n;
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/


