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
/***************************************************************************
 * read_tape_record.c
 *
 * reads a physical record from the tape
 *
 * On success, returns the size of the record read, and loads the data
 * into buffer
 *
 * On failure returns -1: this is either a read failure, or logical
 * end of tape.
 *
 * Gary Blackburn / Mike Dixon RAP NCAR September 1990
 *
 **************************************************************************/

#include "polar2gate.h"

si32 read_tape_record (char *tape_buffer)

{

  static int tape;
  static int first_call = TRUE;
  si32 eof_flag = 0;              /* end of file flag */
  si32 nread;
  int errcnt;

  /*
   * on first call, open tape device
   */

  if (first_call) {

    if((tape = open(Glob->device_name, O_RDONLY)) < 0) {
      fprintf(stderr, "\nERROR - %s:read_tape_record.\n",
	      Glob->prog_name);
      fprintf(stderr, "Opening tape unit\n");
      perror(Glob->device_name);
      tidy_and_exit(-1);
    }

    first_call = FALSE;

  }

  errcnt = 0;

 do_read:

  nread = read (tape, tape_buffer, MAX_TAPE_REC_SIZE);

  if (nread < 0) {

    fprintf(stderr, "ERROR - %s:read_tape_record\n",
	    Glob->prog_name);
    perror(Glob->device_name);
    if(++errcnt > 20)
      return (-1L);  /* quit if many errors */
    goto do_read;

  } else if (nread == 0) {

    /*
     * no bytes returned from read
     */
    
    if (eof_flag >= 2) {

      /*
       * logical end of tape 
       */
      
      if (Glob->summary_print ||
	  Glob->header_print) {
	printf ("%s: logical end of tape encountered\n",
		Glob->device_name);
      }

      return (-1L);

    } else {

      /*
       * end of file
       */

      eof_flag++;

      if (Glob->summary_print ||
	  Glob->header_print) {
	printf("%s: end of file\n", Glob->device_name);
      }
      
      goto do_read;

    }

  } else {

    return (nread);

  }

}

