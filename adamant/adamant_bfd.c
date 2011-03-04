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

// adamant_bfd.c
// 
// More about this class 
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/time.h>

#include "adamant_bfd.h"

//! global variables


//! global function prototypes
void  adamant_bfd_find_address_in_section (bfd *abfd, asection*section, PTR data);


//! \brief Does the BFD file init
//! \author Graham Price
//! \date 08/29/2007
void adamant_bfd_init(GString * bfdFile)
{
    bfd * temp_bfd = NULL;
    char * temp_bfd_target = NULL;

    //! set up the default bfd target

    //! open the bfd file
    temp_bfd = bfd_openr(bfdFile->str, temp_bfd_target);

    
    if(temp_bfd == NULL)
        {
            findAddrSection.filename = NULL;

            g_BFD = NULL;
            g_bfdTarget = NULL;
            g_BFDFile = NULL;
        }
    else
        {
            
            //! save the file name
            findAddrSection.filename = bfdFile->str;

            g_BFDFile = bfdFile;

            //! set the global bfd file to the one just opened
            g_BFD = temp_bfd; 
            g_bfdTarget = temp_bfd_target;
        }
}

/* //! extracts sin values from a list of raw numbers */
/* //! collected from a BDD */
/* //! \brief Return program information from a list */
/* //! \author Graham Price */
/* //! \date 08/30/2008 */
/* void adamant_bfd_PCListInfo(list<guint64> * pc_nums)   */
/* { */
/*     list<guint64>::iterator t_col_iter; */

/*     if(g_BFD != NULL) */
/*         { */
/*             for(t_col_iter = pc_nums->begin(); t_col_iter != pc_nums->end(); t_col_iter++) */
/*                 { */
/*                     //! TODO: need processing work here */
/*                     BfdSinInfo((guint64)(*t_col_iter)); */
/*                 }  */
/*         } */
/* } */



//! \brief Return program information about a SIN
//! \author Graham Price
//! \date 08/30/2008
GString * adamant_bfd_sinInfo(guint64 sin_num)  
{
    GString * temp_string = NULL;
    char * temp_operation = g_new(char, 255); //! /warning bad coding practice!!
    char * temp_args[1];
    temp_args[0] = g_new(char, 255); //! /warning bad coding practice!!

    //! check for a valid BFD and SIN
    if((g_BFD != NULL) && (sin_num != 0))
        {
            //! set up the pc address
            findAddrSection.pc = sin_num;

            //! this call should find the PC (SIN) in the file
            temp_string = adamant_bfd_process_file(g_BFD);
        }
    else
      {
	temp_string = g_string_new("");      
      }
    

    //! CLEANUP
    g_free(temp_operation);
    g_free(temp_args[0]);

    return (temp_string);
}


//! \brief Return program information about a SIN
//! 
//! Derived from the GNU source code for addr2line
//! with some modification
GString * adamant_bfd_process_file (bfd * abfd)
{
    char **matching;
    GString * return_string = NULL;


    if (bfd_check_format (abfd, bfd_archive))
        {
            g_print("Can not get addresses from archive\n");
            return (NULL);
        }

    if (! bfd_check_format_matches (abfd, bfd_object, &matching))
        {
            g_print("The format of this file was not recognized\n");
            return (NULL);
        }

    adamant_bfd_slurp_symtab (abfd);

    return_string = adamant_bfd_translate_addresses (abfd);

    if (findAddrSection.syms != NULL)
        {
            free (findAddrSection.syms);
            findAddrSection.syms = NULL;
        }
    return(return_string);

}


/* Read in the symbol table.  */
//! Derived from the GNU source code for addr2line
//! with some modification
void adamant_bfd_slurp_symtab (bfd * abfd)
{
    long storage;
    long symcount;

    if ((bfd_get_file_flags (abfd) & HAS_SYMS) == 0)
        return;

    storage = bfd_get_symtab_upper_bound (abfd);
    //    if (storage < 0)
    //     bfd_fatal (bfd_get_filename (abfd));

    findAddrSection.syms = (asymbol **) g_malloc (storage);

    symcount = bfd_canonicalize_symtab (abfd, findAddrSection.syms);
    //  if (symcount < 0)
    //   bfd_fatal (bfd_get_filename (abfd));
}


/* Translate program counter into 
   file_name:line_number and 
   optionally function name.  */
//! Derived from the GNU source code for addr2line
//! with some modification
GString * adamant_bfd_translate_addresses (bfd *abfd)
{

    GString * return_string;
    return_string = g_string_new(NULL);

    findAddrSection.found = FALSE;
    bfd_map_over_sections (abfd,  adamant_bfd_find_address_in_section, (PTR)(&findAddrSection));
    
    //! possible need for demangling code, like demangle.h
    
    if (! findAddrSection.found)
        {
            g_string_sprintf(return_string,"??\n");
        }
    else
        {        
            if(findAddrSection.line == 0)
                {
                    g_string_free(return_string, TRUE);
                    return_string = NULL;
                }
            else
                {
                    g_string_sprintf(return_string,"file:%s - function:%s - line:%u\n",
                                     findAddrSection.filename, findAddrSection.functionname, 
                                     findAddrSection.line);
                }
        }
    


    //    if (base_names)
    //         {
    //             char *h;

    //             h = strrchr (filename, '/');
    //             if (h != NULL)
    //                 filename = h + 1;
    //         }

    //    g_string_sprintf (return_string,"line:%u\n", findAddrSection.line);

    return (return_string);
}


/* Look for an address in a section.  This is called via
   bfd_map_over_sections.  */
//! Derived from the GNU source code for addr2line
//! with some modification
void adamant_bfd_find_address_in_section (bfd *abfd, asection*section, PTR data)
{
    bfd_vma vma;
    addrSecStruct* tempData = (addrSecStruct*)(data);

    if (tempData->found)
        return;

    if ((bfd_get_section_flags (abfd, section) & SEC_ALLOC) == 0)
        return;

    vma = bfd_get_section_vma (abfd, section);
    if (tempData->pc < vma)
        return;

    tempData->found = bfd_find_nearest_line (abfd, section, tempData->syms, 
                                   tempData->pc - vma,
                                   &tempData->filename, 
                                   &tempData->functionname, 
                                   &tempData->line);
}

