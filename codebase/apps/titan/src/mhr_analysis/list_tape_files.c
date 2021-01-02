/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/* ** Copyright UCAR (c) 1990 - 2016                                         */
/* ** University Corporation for Atmospheric Research (UCAR)                 */
/* ** National Center for Atmospheric Research (NCAR)                        */
/* ** Boulder, Colorado, USA                                                 */
/* ** BSD licence applies - redistribution and use in source and binary      */
/* ** forms, with or without modification, are permitted provided that       */
/* ** the following conditions are met:                                      */
/* ** 1) If the software is modified to produce derivative works,            */
/* ** such modified software should be clearly marked, so as not             */
/* ** to confuse it with the version available from UCAR.                    */
/* ** 2) Redistributions of source code must retain the above copyright      */
/* ** notice, this list of conditions and the following disclaimer.          */
/* ** 3) Redistributions in binary form must reproduce the above copyright   */
/* ** notice, this list of conditions and the following disclaimer in the    */
/* ** documentation and/or other materials provided with the distribution.   */
/* ** 4) Neither the name of UCAR nor the names of its contributors,         */
/* ** if any, may be used to endorse or promote products derived from        */
/* ** this software without specific prior written permission.               */
/* ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  */
/* ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      */
/* ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    */
/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/***************************************************************************
 * list_tape_files.c
 *
 * lists files on an exabyte tape
 *
 * Mike Dixon RAP NCAR September 1990
 *
 **************************************************************************/

#include "mhr_analysis.h"

void list_tape_files(void)

{
  
  static ui08 buffer[MAX_TAPE_REC_SIZE];
  int tape;
  si32 nread, file_num = 0;
  si32 eof_flag = 0;        /* end of file flag */
  si32 eot_flag = 0;        /* end of tape flag */
  rp7_params_t *rp7p;

  /*
   * open tape
   */

  if((tape = open(Glob->tape_name, O_RDONLY)) < 0) {
    fprintf(stderr, "\nERROR - %s:list_tape_files.\n",
	    Glob->prog_name);
    fprintf(stderr, "Opening tape unit\n");
    perror(Glob->tape_name);
    tidy_and_exit(-1);
  }
  
  /*
   * rewind tape
   */
  
  if (rewind_tape(tape) != 0) {
    fprintf(stderr, "ERROR - %s:list_tape_files - rewinding tape\n",
	    Glob->prog_name);
    perror(Glob->tape_name);
    tidy_and_exit(-1);
  }
  
  while (eot_flag == 0) {
    
  read_another:
    
    nread = read (tape, (char *) buffer, MAX_TAPE_REC_SIZE);
    
    if (nread <= 0) {
      
      if (eof_flag >= 2) {
	printf ("%s : logical end of tape encountered\n",
		Glob->tape_name);
	eot_flag = 1;
      } else {
	file_num++;
	eof_flag++;
	goto read_another;
      }
      
    } else {
      
      eof_flag = 0;
      
      rp7p = (rp7_params_t *) buffer;
      get_tape_status(tape);
      file_num++;
      
      printf("File: %3d  Vol: %4d - %2d/%2d/%2d %2d:%2d:%2d\n",
	     file_num,
	     rp7p->vol_num,
	     rp7p->year,
	     rp7p->month,
	     rp7p->day,
	     rp7p->hour,
	     rp7p->min,
	     rp7p->sec);
      
      fwd_space_file(tape, 1L);
      
    } /* if (nread <= 0) */
    
  } /* while (eot_flag == 0) */
  
}
