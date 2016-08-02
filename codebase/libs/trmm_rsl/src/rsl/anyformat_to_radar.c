/*
    NASA/TRMM, Code 910.1.
    This is the TRMM Office Radar Software Library.
    Copyright (C) 1996, 1997
            John H. Merritt
            Space Applications Corporation
            Vienna, Virginia

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
/*
 * By John H. Merritt
 * Science Applications Corporation, Vienna, VA
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <trmm_rsl/rsl.h> 
void rsl_readflush(FILE *fp);
/*********************************************************************/
/*                                                                   */
/*                   RSL_filetype                                    */
/*                                                                   */
/*********************************************************************/
enum File_type RSL_filetype(char *infile)
{
  /* Open the input file and peek at the first few bytes to determine
   * the type of file.
   * 
   * UF     - First two bytes 'UF'
   *        - or 3,4 bytes 'UF'
   *        - or 5,6 bytes 'UF'. This is the most common.
   *
   * WSR88D - First 8 bytes: 'ARCHIVE2' or 'AR2V0001'
   *
   * TOGA   - ??
   * NSIG   - ??
   * LASSEN - SUNRISE
   * RSL    - RSL
   * MCGILL - P A B
   * RAPIC  - /IMAGE:
   * RADTEC - 320      (decimal, in first two bytes)
   * RAINBOW - First two bytes: decimal 1, followed by 'H'
   */
  FILE *fp;
  char magic[11];

  if ((fp = fopen(infile, "r")) == NULL) {
	perror(infile);
	return UNKNOWN;
  }

  /* Read the magic bytes. */
  /* fp = uncompress_pipe(fp); This caused issues at UCAR */
  if (fread(magic, sizeof(magic), 1, fp) != 1) {
	char *magic_str = (char *)calloc(sizeof(magic)+1, sizeof(char));
	memcpy(magic_str, magic, sizeof(magic));
	fprintf(stderr,"Error fread: Magic is %s\n", magic_str);
	free (magic_str);
	perror("RSL_filetype");
	fclose(fp);
	return UNKNOWN;
  }

  /* rsl_readflush(fp); This caused issues at UCAR - just close the file. */
  fclose(fp);

  if (strncmp("ARCHIVE2.", magic, 9) == 0) return WSR88D_FILE;
  if (strncmp("AR2V000", magic, 7) == 0) return WSR88D_FILE;
  if (strncmp("UF", magic, 2) == 0) return UF_FILE;
  if (strncmp("UF", &magic[2], 2) == 0) return UF_FILE;
  if (strncmp("UF", &magic[4], 2) == 0) return UF_FILE;
  if ((int)magic[0] == 0x0e &&
	  (int)magic[1] == 0x03 &&
	  (int)magic[2] == 0x13 &&
	  (int)magic[3] == 0x01
	  ) return HDF_FILE;
  if (strncmp("RSL", magic, 3) == 0) return RSL_FILE;
  if ((int)magic[0] == 7) return NSIG_FILE_V1;
  if ((int)magic[1] == 7) return NSIG_FILE_V1;
  if ((int)magic[0] == 27) return NSIG_FILE_V2;
  if ((int)magic[1] == 27) return NSIG_FILE_V2;
  if (strncmp("/IMAGE:", magic, 7) == 0) return RAPIC_FILE;
  if ((int)magic[0] == 0x40 &&
	  (int)magic[1] == 0x01
	  ) return RADTEC_FILE;
  if ((int)magic[0] == 0x01 && magic[1] == 'H') return RAINBOW_FILE;

  if (strncmp("SUNRISE", &magic[4], 7) == 0) return LASSEN_FILE;
/* The 'P A B' is just too specific to be a true magic number, but that's all
 * I've got.
 */
  if (strncmp("P A B ", magic, 6) == 0) return MCGILL_FILE;
  /* Byte swapped ? */
  if (strncmp(" P A B", magic, 6) == 0) return MCGILL_FILE;
  if (strncmp("Volume", magic, 6) == 0) return EDGE_FILE;
  if (strncmp("SSWB", magic, 4) == 0) return DORADE_FILE;
  if (strncmp("VOLD", magic, 4) == 0) return DORADE_FILE;

  return UNKNOWN;
}
  



  


/*********************************************************************/
/*                                                                   */
/*                   RSL_anyformat_to_radar                          */
/*                                                                   */
/*********************************************************************/

Radar *RSL_anyformat_to_radar(char *infile, ...)
{
  va_list ap;
  char *callid_or_file;
  Radar *radar;

/* If it is detected that the input file is WSR88D, use the second argument
 * as the call id of the site, or the file name of the tape header file.
 *
 * Assumption: Input files are seekable.
 */
  radar = NULL;
  switch (RSL_filetype(infile)) {
  case WSR88D_FILE:
	callid_or_file = NULL;
	va_start(ap, infile);
	callid_or_file = va_arg(ap, char *);
	va_end(ap);
	radar = RSL_wsr88d_to_radar(infile, callid_or_file);
	break;
  case      UF_FILE: radar = RSL_uf_to_radar(infile);     break;
  case    TOGA_FILE: radar = RSL_toga_to_radar(infile);   break;
  case NSIG_FILE_V1: radar = RSL_nsig_to_radar(infile);	  break;
  case NSIG_FILE_V2: radar = RSL_nsig2_to_radar(infile);  break;
  case   RAPIC_FILE: radar = RSL_rapic_to_radar(infile);  break;
  case  RADTEC_FILE: radar = RSL_radtec_to_radar(infile); break;
  case     RSL_FILE: radar = RSL_read_radar(infile);      break;
#ifdef HAVE_LIBTSDISTK
  case     HDF_FILE: radar = RSL_hdf_to_radar(infile);    break;
#endif
  case RAINBOW_FILE: radar = RSL_rainbow_to_radar(infile); break;
  case  MCGILL_FILE: radar = RSL_mcgill_to_radar(infile); break;
  case    EDGE_FILE: radar = RSL_EDGE_to_radar(infile);   break;
  case  LASSEN_FILE: radar = RSL_lassen_to_radar(infile); break;
  case  DORADE_FILE: radar = RSL_dorade_to_radar(infile); break;

  default:
	fprintf(stderr, "Unknown input file type.  File <%s> is not recognized by RSL.\n", infile);
	return NULL;
  }

  return radar;
}


