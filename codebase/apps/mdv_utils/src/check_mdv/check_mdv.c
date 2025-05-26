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
 *  CHECK_MDV.C
 *
 *  Checks that a given file is a valid MDV file.
 *
 *  Returns 0 on success, -1 on failure
 */

#include <stdio.h>
#include <string.h>

#include <Mdv/mdv/mdv_print.h>
#include <Mdv/mdv/mdv_read.h>
#include <Mdv/mdv/mdv_user.h>
#include <titan/radar.h>


/*
 * Forward function declarations.
 */

void parse_command_line(int argc, char **argv);
void print_usage(void);


/*
 * Global variables from the command line.
 */

char *Prog_name = NULL;
char *In_fname = NULL;

/******************************************************************************
 * MAIN :   Open file and send the output to STDOUT
 *
 */

int main(int argc, char **argv)
{

  int ifield;
  int ichunk;
  int *nlevels;
  
  char **field_names;
  
  void *chunk_data;
  void *volume_data;
  
  MDV_master_header_t master_hdr;
  MDV_field_header_t  field_hdr;
  MDV_vlevel_header_t vlevel_hdr;
  MDV_chunk_header_t  chunk_hdr;
  MDV_dataset_t dataset;
  vol_file_handle_t v_handle;
  
  FILE *infile;

  /*
   * Parse the command line.
   */

  parse_command_line(argc, argv);

  /*
   * check that file is an MDV file
   */

  if (!MDV_verify(In_fname)) {
    fprintf(stderr, "File %s is not MDV format\n", In_fname);
    return (-1);
  }

  /************************************************************
   * Check dataset reading
   */
  
  /*
   * Initialize the dataset.
   */
  
  MDV_init_dataset(&dataset);
  
  /*
   * Read in the MDV dataset
   */
  
  if (MDV_get_dataset(In_fname, &dataset) != MDV_SUCCESS) {
    fprintf(stderr,
	    "Error reading in MDV dataset <%s>\n", In_fname);
    return (-1);
  }
  

  /*
   * Free the dataset.
   */
  
  MDV_free_dataset(&dataset);

  /************************************************************
   * check individual reads
   */
  
  /*
   * Open the input file.
   */
  
  if ((infile = fopen(In_fname, "r")) == NULL) {
    fprintf(stdout,
	    "ERROR:  Cannot open input file <%s>\n",
	    In_fname);
    return (-1);
  }
    
  /*
   * Read and print the master header.
   */
  
  if (MDV_load_master_header(infile, &master_hdr) != MDV_SUCCESS) {
    fprintf(stdout,
	    "ERROR: Error reading master header from input file <%s>\n",
	    In_fname);
    exit(-1);
  }
    
  /*
   * Read and print each field header.  Save the number of levels
   * and name for the field for use in printing the vlevel headers.
   */
  
  if ((nlevels = (int *)malloc(master_hdr.n_fields * sizeof(int)))
      == NULL) {
    fprintf(stdout,
	    "ERROR: Error allocating space for nlevels array\n");
    exit(-1);
  }
	
  if ((field_names = (char **)malloc(master_hdr.n_fields * sizeof(char *)))
      == NULL) {
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
      
    if ((chunk_data = MDV_get_chunk_data(infile, &chunk_hdr)) == NULL) {
      fprintf(stdout,
	      "ERROR: Error reading chunk %d data from input file <%s>\n",
	      ichunk, In_fname);
      exit(-1);
    }
	
    free(chunk_data);

  } /* endfor - ichunk */
    
  /*
   * Read and print the field data.  Reread the field headers
   * so we don't have to keep them hanging around.
   */
  
  for (ifield = 0; ifield < master_hdr.n_fields; ifield++)  {
    if (MDV_load_field_header(infile, &field_hdr, ifield)
	!= MDV_SUCCESS) {
      fprintf(stdout,
	      "ERROR: Error reading field %d header\n",
	      ifield);
      exit(-1);
    }
    
    if ((volume_data = MDV_get_volume(infile, &field_hdr,
				      field_hdr.encoding_type)) == NULL) {
      fprintf(stdout,
	      "ERROR: Error reading field %d data from input file <%s>\n",
	      ifield, In_fname);
      exit(-1);
    }
    
    free(volume_data);
    
  } /* endfor - ifield */

  fclose(infile);

  /***********************************************************
   * check TITAN-based reading
   */
  
  /*
   * initialize volume file index
   */
  
  RfInitVolFileHandle(&v_handle,
		      Prog_name, In_fname,
		      (FILE *) NULL);
  
  /*
   * read in radar volume
   */
  
  if (RfReadVolume(&v_handle, "volume_view"))
    return (-1);

  /*
   * no problems found
   */
  
  return (0);

}

/*****************************************************
 * parse_command_line()
 */

void parse_command_line(int argc, char **argv)
{

  int i;
  int error_flag = FALSE;
  Prog_name = argv[0];
  
  for (i = 1; i < argc; i++) {

    if (!strcmp(argv[i], "-h")) {
      
      print_usage();
      exit(0);

    } else if (!strcmp(argv[i], "-f")) {
	
      if (i < argc - 1) {
	In_fname = argv[i+1];
      } else {
	error_flag = TRUE;
      }
      
    }

  } /* i */

  if (In_fname == NULL || error_flag) {
    print_usage();
    exit(-1);
  }
  
  return;

}

/*****************************************************
 * print_usage()
 */

void print_usage(void)
{
  fprintf(stderr, "Usage: check_mdv [options]\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Options as follows:\n");
  fprintf(stderr, "  [-f file_path] input_file_path - required\n");
  fprintf(stderr, "  [-h] prints help or usage\n");
  fprintf(stderr, "\n");
  
  return;
}
