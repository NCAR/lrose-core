// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
/******************************************************************************
**                                                                           **
**  File name:   type_subtype_list.cc                                        **
**                                                                           **
**  Written by:  ?                                                           **
**                                                                           **
**  Contents:    known_type_subtype_list(), known_line_type(),               **
**               SIO_issue_to_valid_extrap()                                 **
**                                                                           **
******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <rapformats/ascii_shapeio.h>
#include <toolsa/mem.h>

#include "shapeio_int.h"
using namespace std;


#define MAXNAMES 200
static char *nameslist[MAXNAMES]={(char*)SIO_PRODTYPE_BOUNDARY_MIGFA,
				  (char*)SIO_PRODTYPE_BOUNDARY_TRUTH,
				  (char*)SIO_PRODTYPE_BOUNDARY_COLIDE,
				  (char*)SIO_PRODTYPE_COMBINED_NC_ISSUE,
				  (char*)SIO_PRODTYPE_COMBINED_NC_VALID,
				  (char*)SIO_PRODTYPE_MINMAX_NC_ISSUE,
				  (char*)SIO_PRODTYPE_MINMAX_NC_VALID,
				  (char*)SIO_PRODTYPE_FIRST_GUESS_ISSUE,
				  (char*)SIO_PRODTYPE_FIRST_GUESS_VALID,
				  (char*)SIO_PRODTYPE_EXTRAP_ISSUE_ANW,
				  (char*)SIO_PRODTYPE_EXTRAP_VALID_ANW,
				  (char*)SIO_SUBTYPE_ALL,
				  (char*)NULL};
static int num=12;

//
// Colide detection line types
//

#define NUM_LINE_TYPES 5

static char *
line_type_list[NUM_LINE_TYPES] = { (char*)"COMBINED_LINE",
				   (char*)"EXTRAPOLATED_LINE",
				   (char*)"SHEAR_LINE",
				   (char*)"THIN_LINE",
				   (char*)"UNKNOWN" };

/*----------------------------------------------------------------*
 *
 * LOCALLY EXPORTED FUNCTIONS
 *
 *----------------------------------------------------------------*/

/*----------------------------------------------------------------*/

char *known_line_type(char *type)
{
  int i;

  // Search through the line type list, looking for a match

  for (i = 0; i < NUM_LINE_TYPES; ++i)
    if (strcmp(type, line_type_list[i]) == 0)
      return line_type_list[i];

  return (char*)"UNKNOWN";
}

/*----------------------------------------------------------------*/
char *known_type_subtype_list(char *type)
{
  int i;

  /*
   * Search through the list to date, looking for a match
   */

  for (i = 0; i < num; ++i)
  {
    if (strcmp(type, nameslist[i]) == 0)
      return nameslist[i];
  }
    
  /*
   * No luck..try to append this in using a malloc.
   */

  if (num >= MAXNAMES-1)
  {
    printf("ERROR nameslist not big enough %d losing '%s'\n",
	   MAXNAMES, type);
    return (char*)SIO_PRODTYPE_BAD;
  }

  nameslist[num] = (char *)umalloc(strlen(type)+1);
  strcpy(nameslist[num], type);
  ++num;

  return(nameslist[num-1]);
}

/*----------------------------------------------------------------*
 *
 * GLOBALLY EXPORTED FUNCTIONS
 *
 *----------------------------------------------------------------*/

/*----------------------------------------------------------------*/

char *SIO_issue_to_valid_extrap(char *type)
{
  char buf[100];
    
  if (!SIO_IS_EXTRAP_ISSUE(type))
  {	
    printf("ERROR expected 'issue' type, got '%s'\n", type);
    return (char*)SIO_PRODTYPE_BAD;
  }

  if (strcmp(type, SIO_PRODTYPE_EXTRAP_ISSUE_ANW) == 0)
    return (char*)SIO_PRODTYPE_EXTRAP_VALID_ANW;
    
  /*
   * Replace "E", "X" with "V" and go looking..
   */

  strcpy(buf, type);
  buf[0] = 'V';

  return (known_type_subtype_list(buf));
}

/*----------------------------------------------------------------*/
