/*
    NASA/TRMM, Code 910.1.
    This is the TRMM Office Radar Software Library.
    Copyright (C) 1996, 1997
            Mike Kolander
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

/******************************************************************
 * Opens a Mcgill format radar data file, and reads and
 * decompresses the contents.
 *
 * All Mcgill data structures referenced herein are 
 * defined in the file "mcgill.h".
 *
 * There exist 3 functions in this file which are
 * accessible to application routines:
 * -----------------------------------------------------
 *   mcgFile_t *mcgFileOpen(int *code, char *filename);
 * Opens and verifies the content of a Mcgill format 
 * data file, reads in the file header record, and
 * initializes the mcgFile_t structure.
 * -------------------------------------------------------
 *   int mcgFileClose(mcgFile_t *file);
 * Closes the Mcgill data file.
 * -------------------------------------------------------
 *   int mcgRayBuild(mcgRay_t *ray, mcgFile_t *file);
 * Returns in the mcgRay_t structure the reflectivity values from 
 * one ray of data from a Mcgill format radar data file. 
 * Successive calls return data from successive rays found 
 * in the file.
 *
 *    Kolander
 *
 *******************************************************************/
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mcgill.h"

#define TRUE 1
#define FALSE 0
#define EMPTY -1.0


/* Fixed elevation angles for Mcgill sweeps. */
static float elev_angle[3][24] = 
   {
   /* PAFB volume scan format types 3 and 4 (12 sweeps) */
	  {
	  0.6, 1.0, 1.6, 2.5, 3.6, 5.2, 7.5, 10.5, 14.7, 20.2, 
	  27.2, 35.6
	  },
	  /* PAFB volume scan format types 1 and 2 (24 sweeps) */
		 {
		 0.6, 0.79, 1.01, 1.27, 1.57, 1.93, 2.35, 2.84, 
		 3.41, 4.08, 4.86, 5.78, 6.85, 8.09, 9.55, 11.23, 
		 13.18, 15.42, 17.98, 20.89, 24.16, 27.78, 31.73, 35.97
		 },
		 /* Sao Paulo radar?? (20 sweeps) */
			{
			1.0, 1.3, 1.6, 2.0, 2.4, 2.9, 3.6, 4.2, 5.1, 
			6.1, 7.3, 8.7, 10.3, 12.2, 14.4, 16.9, 19.8, 
			23.1, 26.7, 30.8
			}
   };

/* Fixed number_of_bins for Mcgill sweeps. */
static int num_bins_normal[24] =
   {
   /* PAFB volume scan formats 1 and 3 (normal mode) */
   240, 180, 180, 180, 180, 180, 180, 180, 180, 180, 
   180, 180, 180, 180, 180, 180, 180, 180, 180, 180,
   180, 180, 180, 180
   };
static int num_bins_compressed[24] =
   {
   /* PAFB volume scan formats 2 and 4 (compressed mode) */
   180, 120, 120, 120, 120, 120, 120, 120, 120, 120,
   120, 120, 120, 120, 120, 120, 120, 120, 120, 120,
   120, 120, 120, 120
   };

FILE *uncompress_pipe (FILE *fp);

/**********************************************************************/
mcgFile_t *mcgFileOpen(int *code, char *filename)
/**********************************************************************/
   {
   /* Open a Mcgill format radar data file, and read and verify 
      the content of the first (header) record from the file.
      This function returns one of the following coded integer values:
		 MCG_OK: Normal return.
         MCG_OPEN_FILE_ERR: Error occurred while attempting to open
		                    the data file.
         MCG_EOF: Unexpected End_of_File occurred reading from file.
         MCG_READ_ERR: Error occurred while reading from file.
         MCG_FORMAT_ERR: Format error encountered reading from file.
         SYS_NO_SPACE: Error attempting to allocate memory space.
   */
   mcgFile_t *file;
   char buffer[MCG_RECORD];
   char *csp_buffer;
   
   /* Allocate space for the mcgFile structure. */
   if ((file = (mcgFile_t *)malloc(sizeof(mcgFile_t))) == NULL)
	  {
	  *code = SYS_NO_SPACE;
      return(NULL);
	  }
   
   /* Open Mcgill data file for reading */
   if ((file->fp = fopen(filename, "r")) == NULL)
	  {
	  *code = MCG_OPEN_FILE_ERR;
	  return(NULL);
	  }
   file->fp = uncompress_pipe(file->fp); /* Transparently, use gunzip. */
   /* Read first (header) record from data file into buffer */
   if (fread(buffer, sizeof(char), MCG_RECORD, file->fp) < MCG_RECORD)
	  {
	  *code = MCG_FORMAT_ERR;
	  return(NULL);
	  }

   /* Move header values from buffer to the mcgHeader structure.*/
   memcpy(&file->head, buffer, sizeof(mcgHeader_t));

   /* Check the leading header characters for the site ID string. */
   if ((strncmp((char *)&file->head, "PAFB RADAR VOLUME SCAN AT", 25) == 0) ||
	   (strncmp((char *)&file->head, "P A B  RADAR VOLUME SCAN AT", 27) == 0))
	  {
	  file->site = MCG_PAFB_1_2;
	  }
   /* Check if file from Sao Paulo.
   else if (strncmp((char *)&file->head, SAOP_ID_STRING) == 0)
	  {
	  file->site = MCG_SAOP;
	  }
   */
   else  /* Can't identify site/format. */
	  {
	  *code = MCG_FORMAT_ERR;
	  return(NULL);
	  }
   
   /* Check validity of some file header values. If
	  not valid, we assume the file is garbled beyond legibility. */
   if ((file->head.vol_scan_format < 1) || (file->head.vol_scan_format > 4)
	   || (file->head.day < 1) || (file->head.day > 31)
	   || (file->head.month < 1) || (file->head.month > 12))
	  {
	  /* Invalid/unexpected file format */
	  *code = MCG_FORMAT_ERR;
	  return(NULL);
	  }

   /* Set the proper number_of_bins, depending on scan format. */
   switch (file->head.vol_scan_format)
	  {
	case 1: 
	case 3:
	  file->num_bins = num_bins_normal;
	  break;
	case 2:
	case 4:
	  file->num_bins = num_bins_compressed;
	  break;
	default:
	  break;
	  }  /* end switch */

   /* I've never seen scan_formats 3 or 4 from PAFB, but I make
	  allowance for them here. */
   if (file->site == MCG_PAFB_1_2)
	  {
      if ((file->head.vol_scan_format == 3) || 
		  (file->head.vol_scan_format == 4))
		 {
		 file->site = MCG_PAFB_3_4;
		 }
	  }
   
   /* If a Mcgill Csp block is contained in this file, it's next.
	  Skip past it. It's useless for our purposes. */
   if (file->head.csp_rec == 1)
	  {
	  /* if (fseek(file->fp, MCG_CSP, SEEK_CUR) != 0)
		 {
		 *code = MCG_READ_ERR;
		 return(NULL);
		 }
	  */
	  /* Allocate buffer space for the csp block, read the block
		 from the disk file into the buffer, then free the buffer.
		 An ugly way to discard the block, but the fseek solution
		 above won't work with stdin capabilities.
	  */
	  if ((csp_buffer=(char *)malloc(MCG_CSP)) == NULL)
		 {
		 *code = SYS_NO_SPACE;
		 return(NULL);
		 }
	  if (fread(csp_buffer, sizeof(char), MCG_CSP, file->fp) < MCG_CSP)
		 {
		 *code = MCG_READ_ERR;
		 return(NULL);
		 }
	  free(csp_buffer);
	  }
   
   /* File is open and properly initialized. */
   *code = MCG_OK;
   return(file);
   }


int rsl_pclose( FILE *stream);
/**********************************************************************/
int mcgFileClose(mcgFile_t *file)
/**********************************************************************/
   {
   /* Close a Mcgill format radar data file. Returns one of the
	  following coded integer values:
	     MCG_OK: File closed successfully.
		 MCG_CLOSE_FILE_ERR: System error occurred during file close 
		                     operation.
   */


   if (rsl_pclose(file->fp) == 0)
	  {
	  file->fp = NULL;
	  return(MCG_OK);
	  }
   else
	  return(MCG_CLOSE_FILE_ERR);
   }



/************************************************************************/
int mcgRecordRead(mcgRecord_t *record, mcgFile_t *file)
/************************************************************************/
   {
   /* Reads a Mcgill logical record (2048 bytes) from the data file
	  into the mcgRecord_t structure. Returns one of the following coded
	  integer values:
	     MCG_OK: Successfully read a record.
		 MCG_EOF: End_of_File condition occurred while reading a record.
		 MCG_READ_ERR: Read error occurred.
   */

   /* Read data from file directly into the record structure. */
   if (fread(record, sizeof(char), MCG_RECORD, file->fp) < MCG_RECORD)
	  {
	  if (feof(file->fp))
	     return(MCG_EOF);
	  else
	     return(MCG_READ_ERR);
	  }

   return(MCG_OK);
   }



/************************************************************************/
mcgSegmentID mcgSegmentKeyIdentify(int key)
/************************************************************************/
   {
   /* Decodes the first byte from a Mcgill file segment.
	  The first byte of the segment identifies the type of the segment.
	  Mcgill segment types are data, elevation, and end_of_data.
   */
   if ((key > 0) && (key < 16))
      return(MCG_DATA_SEG);
   else if ((key > 31) && (key < 63))
      return(MCG_ELEV_SEG);
   else if (key == 63)
      return(MCG_EOD_SEG);
   else 
      return(MCG_UNDEFINED_SEG);
   }
   


/*************************************************************************/
int mcgRayBuild(mcgRay_t *ray, mcgFile_t *file)
/*************************************************************************/
   {
   /* Returns in the ray structure the reflectivity values from one
	  ray of data from a Mcgill format radar data file. Successive calls 
	  to this routine return data from successive rays found in the file.
	  This function returns the following coded integer values:
	     MCG_OK : Ray structure successfully filled with data values.
		 MCG_EOD: Empty ray structure, No more data in file.
		 MCG_EOF: Premature End_of_File condition encountered.
		 MCG_READ_ERR: System error occurred while reading from file.
		 MCG_FORMAT_ERR: Format error encountered while reading from file.
   */

   /* This function is typically called about 9000 times while reading a
	  Mcgill data file. The following static variables must retain their
	  values between successive calls. */
   static int seg_num=MCG_MAX_SEG_NUM;
   static int eod_found = FALSE;
   static int sweep_num=0;
   static float elev=0.0;
   static float azm;
   int base, j, n;
   mcgSegmentID seg_type;
   static mcgRecord_t record;


   /* If we've previously found the end_of_data (eod) data segment in 
	  the last Mcgill record, we're done. Do this for robustness, in case
	  the calling application routine screws up. */
   if (eod_found)
	  {
	  return(MCG_EOD);
	  }
   
   /* Initialize the ray data values. */
   ray->elev = elev;
   ray->azm = EMPTY;
   ray->sweep_num = sweep_num;
   memset(ray->data, 0, 240);
   ray->num_bins = file->num_bins[sweep_num];
   
   /* Loop to fill the ray bins with data values from consecutive 
	  Mcgill data segments. */
   while (1)
	  {
	  /* Read in from the Mcgill file a new record, if necessary. */
	  if (seg_num == MCG_MAX_SEG_NUM)
		 {
		 if ((n=mcgRecordRead(&record, file)) < MCG_OK)
		    return(n);
		 else
			{
			  /* record_empty = FALSE; */
			seg_num = 0;
			}
		 }

	  /* The first byte of segment is used to identify segment type. */
	  seg_type = mcgSegmentKeyIdentify(record.segment[seg_num][0]);
	  switch (seg_type)
		 {
	   case MCG_DATA_SEG:
		 /* Determine the azimuth angle for this data segment */
		 azm = record.segment[seg_num][1] * 64.0 + 
		       record.segment[seg_num][2] - 1.0;
		 /* Check if this data segment contains data belonging to
		    the current ray. If so, add the segment bin values to
			the ray structure. */
		 if ((azm == ray->azm) || (ray->azm == EMPTY))
			{
			ray->azm = azm;
			/* Compute the ray bin base_address for the 16 bin values
			   from this data segment. */
			base = (record.segment[seg_num][0] - 1)*16;
			/* Move the 16 bin values from segment to ray structure */
			for (j=0; j<16; j++)
			   {
			   ray->data[base + j] = record.segment[seg_num][3+j];
			   }
			}
		 else  /* This data segment belongs to the next ray. Return the
				  current ray. We'll start with this segment the next
				  time mcgRayBuild() is called. */
			{
			return(MCG_OK);
			}
		 break;

	   case MCG_ELEV_SEG:
		 sweep_num = record.segment[seg_num][0] - 31 - 1;
		 elev = elev_angle[file->site][sweep_num];
		 /* If ray structure is not empty, return it, since 
			we've found a Mcgill elevation segment, which initiates 
			a new sweep. */
		 if (ray->azm != EMPTY)
			{
			seg_num += 1;
			if (seg_num == MCG_MAX_SEG_NUM)
			  /*			   record_empty = TRUE; */
			return(MCG_OK);
			}
		 else   /* Ray structure is empty. Continue on to read the
				   next Mcgill segment.*/
			{
			ray->elev = elev;
			ray->sweep_num = sweep_num;
			}
		 break;

	   case MCG_EOD_SEG:
		 eod_found = TRUE;
		 if (ray->azm == EMPTY)
			return(MCG_EOD);
		 else  /* Return the ray structure we've filled. */
			return(MCG_OK);
		 break;

	   default:
		 return(MCG_FORMAT_ERR);
		 break;
		 }  /* end switch */

	  seg_num += 1;
	  }  /* end while (1) */
   }
