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
/******************************************************************************
 *  PRINT_MDV.C Print Utility Program for MDV files 
 */

#include <stdio.h>
#include <string.h>

#include <toolsa/umisc.h>
#include <Mdv/mdv/mdv_print.h>
#include <Mdv/mdv/mdv_read.h>
#include <Mdv/mdv/mdv_user.h>
#include <Mdv/mdv/mdv_field_handle.h>
#include <Mdv/mdv/mdv_handle.h>

/*
 * Forward function declarations.
 */

void parse_command_line(int argc, char **argv);
void print_usage(void);
void print_using_handle(void);
void print_lowlevel(void);

/*
 * Global variables from the command line.
 */

char *Prog_name = NULL;
char *In_fname = NULL;
int Full_flag = FALSE;
int Data_flag = FALSE;
int Lowlevel_flag = FALSE;
int Float_flag = FALSE;

/******************************************************************************
 * MAIN :   Open file and send the output to STDOUT
 *
 */

int main(int argc, char **argv)
{

  /*
   * Parse the command line.
   */

  parse_command_line(argc, argv);

  /*
   * check that file is an MDV file
   */

  if (!MDV_verify(In_fname)) {
    fprintf(stderr, "ERROR - %s\n", Prog_name);
    fprintf(stderr, "File %s is not MDV format\n", In_fname);
    exit (-1);
  }
    
  if (Lowlevel_flag)
  {
    print_lowlevel();
  }
  else
  {
    print_using_handle();
  }
  
  exit(0);

}

/*****************************************************
 * parse_command_line()
 */

void parse_command_line(int argc, char **argv)
{
  int i;
  int infile_found = FALSE;
  
  Prog_name = argv[0];
  
  for (i = 1; i < argc; i++)
  {
    if (strcmp(argv[i], "-h") == 0 ||
	strcmp(argv[i], "-help") == 0)
    {
      print_usage();
      exit(0);
    }
    if (strcmp(argv[i], "-f") == 0 ||
	strcmp(argv[i], "-full") == 0)
    {
      Full_flag = TRUE;
    }
    else if (strcmp(argv[i], "-d") == 0 ||
	     strcmp(argv[i], "-data") == 0)
    {
      Data_flag = TRUE;
      Full_flag = TRUE;
    }
    else if (strcmp(argv[i], "-low") == 0)
    {
      Lowlevel_flag = TRUE;
    }
    else if (strcmp(argv[i], "-float") == 0)
    {
      Full_flag = TRUE;
      Data_flag = TRUE;
      Float_flag = TRUE;
      Lowlevel_flag = FALSE;
    }
    else if (strcmp(argv[i], "-native") == 0)
    {
      Full_flag = TRUE;
      Data_flag = TRUE;
      Float_flag = FALSE;
      Lowlevel_flag = FALSE;
    }
    else
    {
      if (infile_found)
      {
	print_usage();
	exit(1);
      }
      
      infile_found = TRUE;
      
      In_fname = argv[i];
    }
  }

  if (!infile_found)
  {
    print_usage();
    exit(1);
  }
  
  return;
}

/*****************************************************
 * print_usage()
 */

void print_usage(void)
{
  fprintf(stderr, "Usage: print_mdv [options] inputfile\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Options as follows:\n");
  fprintf(stderr, "  [-data, -d] prints out the data values\n");
  fprintf(stderr, "  [-full, -f] prints out every field in every header\n");
  fprintf(stderr, "  [-float] print data as floats - converts from stored type\n");
  fprintf(stderr, "    Forces -d. Overrides -low.\n");
  fprintf(stderr, "  [-low] utilizes lowlevel code\n");
  fprintf(stderr, "  [-h, -help] prints help or usage\n");
  fprintf(stderr, "  [-native] print data native\n");
  fprintf(stderr, "    Forces -d. Overrides -low.\n");
  fprintf(stderr, "\n");
  
  return;
}

/**********************
 * print_using_handle()
 */

void print_using_handle(void)

{

  int iz;
  int ifield;
  int ichunk;
    
  MDV_field_header_t  *fhdr;
  MDV_handle_t mdv;
  
  MDV_init_handle(&mdv);

  /*
   * read input mdv file
   */
  
  if (MDV_handle_read_all(&mdv, In_fname, MDV_ASIS, MDV_COMPRESSION_ASIS,
			  MDV_SCALING_ROUNDED, 0.0, 0.0)) {
    fprintf(stderr, "ERROR - print_mdv\n");
    fprintf(stderr, "Cannot read mdv file %s\n", In_fname);
    exit (-1);
  }
  
  if (Full_flag) {
    MDV_print_master_header_full(&mdv.master_hdr, stdout);
  } else {
    MDV_print_master_header(&mdv.master_hdr, stdout);
  }
  
  for (ifield = 0; ifield < mdv.master_hdr.n_fields; ifield++) {
    fprintf(stdout, "Field %d\n", ifield);
    if (Full_flag) {
      MDV_print_field_header_full(mdv.fld_hdrs + ifield, stdout);
      if (ifield == 0) {
	fprintf(stdout, "  Grid parameters for field 0\n");
	fprintf(stdout, "  ---------------------------\n");
	MDV_print_grid(stdout, "  ", &mdv.grid);
      }
    } else {
      MDV_print_field_header(mdv.fld_hdrs + ifield, stdout);
    }
  }
    
  if (Full_flag) {
  }

  if (mdv.master_hdr.vlevel_included) {
    
    for (ifield = 0; ifield < mdv.master_hdr.n_fields; ifield++) {

      fhdr = mdv.fld_hdrs + ifield;

      fprintf(stdout, "Field %d\n", ifield);
      if (Full_flag) {
	MDV_print_vlevel_header_full(mdv.vlv_hdrs + ifield,
				     fhdr->nz,
				     fhdr->field_name,
				     stdout);
      } else {
	MDV_print_vlevel_header(mdv.vlv_hdrs + ifield,
				fhdr->nz,
				fhdr->field_name,
				stdout);
      }
      
    } /* endfor - ifield */
    
  } /* endif - vlevel_included */
  
  /*
   * Print each chunk header and the associated data.
   */
  
  for (ichunk = 0; ichunk < mdv.master_hdr.n_chunks; ichunk++) {

    if (Full_flag) {
      MDV_print_chunk_header_full(mdv.chunk_hdrs + ichunk, stdout);
      MDV_print_chunk_data_full(mdv.chunk_data[ichunk],
				mdv.chunk_hdrs[ichunk].chunk_id,
				mdv.chunk_hdrs[ichunk].size, stdout);
    } else {
      MDV_print_chunk_header(mdv.chunk_hdrs + ichunk, stdout);
    }
    
  } /* endfor - ichunk */

  /*
   * Print the field data.
   */
  
  if (Data_flag) {

    /*
     * read in data uncompressed
     */
    
    if (MDV_handle_read_all(&mdv, In_fname, MDV_ASIS, MDV_COMPRESSION_NONE,
			    MDV_SCALING_ROUNDED, 0.0, 0.0)) {
      fprintf(stderr, "ERROR - print_mdv\n");
      fprintf(stderr, "Cannot read mdv file %s\n", In_fname);
      exit (-1);
    }
  
    for (ifield = 0; ifield < mdv.master_hdr.n_fields; ifield++) {

      fprintf(stdout, "------>Field: %d\n", ifield);
      fhdr = mdv.fld_hdrs + ifield;
      
      for (iz = 0; iz < fhdr->nz; iz++) {

	int plane_size;
	MDV_field_handle_t *fhand;

	/*
	 * create field handle just for this plane
	 */

	plane_size = fhdr->nx * fhdr->ny * fhdr->data_element_nbytes;
	fhand =
	  MDV_fhand_create_plane_from_parts(fhdr, iz,
					    mdv.field_plane[ifield][iz],
					    plane_size);

	/*
	 * print out
	 */
	
	fprintf(stdout, "------->Plane: %d\n", iz);
	MDV_fhand_print_voldata(fhand, stdout, !Float_flag, FALSE, TRUE);

	/*
	 * free up
	 */

	MDV_fhand_delete(fhand);
	
      } /* iz */

    } /* ifield */

  } /* if (Data_flag) */

}

/******************
 * print_lowlevel()
 */

void print_lowlevel(void)

{

  int ifield;
  int iplane;
  int ichunk;
  
  FILE *infile;
    
  MDV_master_header_t master_hdr;
  MDV_field_header_t  field_hdr;
  MDV_vlevel_header_t vlevel_hdr;
  MDV_chunk_header_t  chunk_hdr;
  
  int *nlevels;
  char **field_names;
  
  void *chunk_data;
  void *volume_data;
  void *volume_ptr;
  
  fprintf(stderr, "Printing using lowlevel\n");

  /*
   * Open the input file.
   */
  
  if ((infile = fopen(In_fname, "r")) == NULL)
    {
      fprintf(stdout,
	      "ERROR:  Cannot open input file <%s>\n",
	      In_fname);
      exit(-1);
    }
  
  /*
   * Read and print the master header.
   */
  
  if (MDV_load_master_header(infile, &master_hdr) != MDV_SUCCESS)
    {
      fprintf(stdout,
	      "ERROR: Error reading master header from input file <%s>\n",
	      In_fname);
      exit(-1);
    }
  
  if (Full_flag)
    MDV_print_master_header_full(&master_hdr, stdout);
  else
    MDV_print_master_header(&master_hdr, stdout);
  
  /*
   * Read and print each field header.  Save the number of levels
   * and name for the field for use in printing the vlevel headers.
   */
  
  if ((nlevels = (int *)malloc(master_hdr.n_fields * sizeof(int)))
      == NULL)
    {
      fprintf(stdout,
	      "ERROR: Error allocating space for nlevels array\n");
      exit(-1);
    }
  
  if ((field_names = (char **)malloc(master_hdr.n_fields * sizeof(char *)))
      == NULL)
    {
      fprintf(stdout,
	      "ERROR: Error allocating space for field_names array\n");
      exit(-1);
    }
  
  for (ifield = 0; ifield < master_hdr.n_fields; ifield++) {
    if (MDV_load_field_header(infile, &field_hdr, ifield)
	!= MDV_SUCCESS) {
      fprintf(stdout,
	      "ERROR: Error reading field %d header from input file <%s>\n",
	      ifield, In_fname);
      exit(-1);
    }
    
    fprintf(stdout, "Field %d\n", ifield);
    if (Full_flag)
      MDV_print_field_header_full(&field_hdr, stdout);
    else
      MDV_print_field_header(&field_hdr, stdout);
    
    nlevels[ifield] = field_hdr.nz;
    
    field_names[ifield] = malloc(strlen(field_hdr.field_name_long) + 1);
    strcpy(field_names[ifield], field_hdr.field_name_long);
    
  } /* endfor - ifield */
    
  /*
   * Read and print each vlevel header.
   */
  
  if (master_hdr.vlevel_included) {
    for (ifield = 0; ifield < master_hdr.n_fields; ifield++) {
      if (MDV_load_vlevel_header(infile, &vlevel_hdr,
				 &master_hdr, ifield) != MDV_SUCCESS) {
	fprintf(stdout,
		"ERROR: Error reading vlevel %d header from input file <%s>\n",
		ifield, In_fname);
	exit(-1);
      }
      
      fprintf(stdout, "Field %d\n", ifield);
      if (Full_flag)
	MDV_print_vlevel_header_full(&vlevel_hdr, nlevels[ifield],
				     field_names[ifield], stdout);
      else
	MDV_print_vlevel_header(&vlevel_hdr, nlevels[ifield],
				field_names[ifield], stdout);
    } /* endfor - ifield */
    
  } /* endif - vlevel_included */
  
  /*
   * Read and print each chunk header and the associated data.
   */
  
  for (ichunk = 0; ichunk < master_hdr.n_chunks; ichunk++) {
    if (MDV_load_chunk_header(infile, &chunk_hdr, &master_hdr,
			      ichunk) != MDV_SUCCESS) {
      fprintf(stdout,
	      "ERROR: Error loading chunk %d header from input file <%s>\n",
	      ichunk, In_fname);
      exit(-1);
    }
    
    if (Full_flag) {
      MDV_print_chunk_header_full(&chunk_hdr, stdout);
      
      if ((chunk_data = MDV_get_chunk_data(infile, &chunk_hdr)) == NULL) {
	fprintf(stdout,
		"ERROR: Error reading chunk %d data from input file <%s>\n",
		ichunk, In_fname);
	exit(-1);
      }
      
      MDV_print_chunk_data_full(chunk_data, chunk_hdr.chunk_id,
				chunk_hdr.size, stdout);
      
      free(chunk_data);
    }
    else
    {
      MDV_print_chunk_header(&chunk_hdr, stdout);
    }
      
  } /* endfor - ichunk */
  
  /*
   * Read and print the field data.  Reread the field headers
   * so we don't have to keep them hanging around.
   */
  
  if (Data_flag) {

    int volume_len;
    
    for (ifield = 0; ifield < master_hdr.n_fields; ifield++) {

      if (MDV_load_field_header(infile, &field_hdr, ifield) != MDV_SUCCESS) {
	fprintf(stdout,
		"ERROR: Error reading field %d header for "
		"field data printing\n", ifield);
	exit(-1);
      }
      
      if ((volume_data =
	   MDV_read_field_volume(infile, &field_hdr,
				 MDV_ASIS,
				 MDV_COMPRESSION_NONE,
				 MDV_SCALING_ROUNDED,
				 0.0, 0.0, &volume_len)) == NULL) {
	fprintf(stdout,
		"ERROR: Error reading field %d volume from input file <%s>\n",
		ifield, In_fname);
	exit(-1);
      }
      
      volume_ptr = volume_data;
      
      for (iplane = 0; iplane < field_hdr.nz; iplane++)	{

	void *plane_ptr;
	  
	plane_ptr = (void *)((char *)volume_ptr +
			     (iplane* field_hdr.nx * field_hdr.ny *
			      MDV_data_element_size(field_hdr.encoding_type)));

	MDV_print_field_plane_full(&field_hdr, plane_ptr,
				   ifield, iplane, stdout);
	
      } /* endfor - iplane */
      
      free(volume_data);
      
    } /* endfor - ifield */

  } /* endif - Data_flag */
  
}
