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

#include <stdio.h>

#include <toolsa/os_config.h>

#include <toolsa/file_io.h>

/*************************************************
 * File copy routine
 *
 * Nancy Rehak, Rap, NCAR, Boulder, CO, 80303, USA
 *
 * November 1995
 */

#define BUFSIZE 4096


/********************************************************
 * filecopy()
 *
 * Utility routine to copy the contents of one file
 * to another file.
 *
 * Returns -1 on error, 0 otherwise.
 */

int filecopy(FILE *dest, FILE *source)
{
  register int bytecount;
  char buffer[BUFSIZE];
  
  while ((bytecount = fread(buffer, 1, BUFSIZE, source))
	 > 0)
  {
    if (fwrite(buffer, 1, bytecount, dest) != bytecount)
      return(-1);
    
    if (bytecount < BUFSIZE)
      break;
  }
  
  return(0);
}

/********************************************************
 * filecopy_by_name()
 *
 * Utility routine to copy the contents of one file
 * to another file.
 *
 * Returns -1 on error, 0 otherwise.
 */

int filecopy_by_name(const char *dest_path, const char *source_path)
{

  FILE *dest, *source;

  if ((dest = fopen(dest_path, "wb")) == NULL) {
    fprintf(stderr, "ERROR - filecopy_by_name\n");
    fprintf(stderr, "  Cannot open destination file\n");
    perror(dest_path);
    return -1;
  }
  
  if ((source = fopen(source_path, "rb")) == NULL) {
    fprintf(stderr, "ERROR - filecopy_by_name\n");
    fprintf(stderr, "  Cannot open source file\n");
    perror(source_path);
    fclose(dest);
    return -1;
  }
  
  if (filecopy(dest, source)) {
    fprintf(stderr, "ERROR - filecopy_by_name\n");
    fprintf(stderr, "  Cannot copy source file: %s\n", source_path);
    fprintf(stderr, "    to dest file: %s\n", dest_path);
    perror("");
    fclose(dest);
    fclose(source);
    return -1;
  }

  fclose(dest);
  fclose(source);
  return 0;

}

