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
/*************************************************************************
 *
 * RfLabels.c
 *
 * part of the rfutil library - radar file access
 *
 * Label utilities
 *
 * Mike Dixon
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * December 1990
 *
 **************************************************************************/

#include <toolsa/umisc.h>
#include <toolsa/str.h>
#include <titan/file_io.h>

/*************************************************************************
 *
 * RfReadFileLabel()
 *
 * part of the rfutil library - radar file access
 *
 * reads in the header label string from one
 * of the radar family of files
 *
 * returns R_SUCCESS on success, R_FAILURE on failure
 *
 **************************************************************************/

int RfReadFileLabel(const char *file_name, char **file_label_p)
{

  static char file_label[R_FILE_LABEL_LEN];
  FILE *radar_file;

  *file_label_p = file_label;

  /*
   * open file
   */
  
  if ((radar_file = Rf_fopen_uncompress((char *) file_name, "r")) == NULL) {

    fprintf(stderr, "ERROR - RfReadFileLabel\n");
    fprintf(stderr, "Cannot open file.\n");
    perror(file_name);
    return (R_FAILURE);

  }
  
  /*
   * read label
   */

  if (ufread(file_label,
	     (int) sizeof(char),
	     (int) R_FILE_LABEL_LEN,
	     radar_file) != R_FILE_LABEL_LEN) {
    
    fprintf(stderr, "ERROR - RfReadFileLabel\n");
    perror(file_name);
    return (R_FAILURE);

  }
  
  /*
   * close the file
   */

  fclose(radar_file);

  return(R_SUCCESS);

}
