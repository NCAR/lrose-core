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
 *  PRINT_SPDB_INDEX.C Print Utility Program for SPDB index files 
 */

#include <stdio.h>
#include <string.h>

#include <dataport/bigend.h>
#include <dataport/port_types.h>
#include <symprod/spdb.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

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
int Chunks_flag = FALSE;
int Min_posn_flag = FALSE;


/******************************************************************************
 * MAIN :   Open file and send the output to STDOUT
 *
 */

int main(int argc, char **argv)
{
  FILE *input_file;
  
  spdb_hdr_t spdb_hdr;
  
  /*
   * Parse the command line.
   */

  parse_command_line(argc, argv);

  /*
   * Open the input file
   */

  if ((input_file = fopen(In_fname, "r")) == NULL)
  {
    fprintf(stderr, "Error opening file <%s> for reading\n",
	    In_fname);
    exit(-1);
  }
  
  /*
   * Read in the header
   */

  if (fread(&spdb_hdr, sizeof(spdb_hdr_t), 1, input_file) != 1)
  {
    fprintf(stderr, "Error reading SPDB header from file <%s>\n",
	    In_fname);
    fclose(input_file);
    exit(-1);
  }
  
  /*
   * Swap the header
   */

  BE_to_array_32((ui32 *) ((char *) &spdb_hdr + SPDB_LABEL_MAX),
		 sizeof(spdb_hdr_t) - SPDB_LABEL_MAX);

  /*
   * Print out the header values
   */

  SPDB_print_index_header(&spdb_hdr, stdout, Min_posn_flag);
  
  if (Chunks_flag)
  {
    int chunk;
    spdb_chunk_ref_t chunk_hdr;
    
    for (chunk = 0; chunk < spdb_hdr.n_chunks; chunk++)
    {
      if (fread(&chunk_hdr, sizeof(chunk_hdr), 1, input_file) != 1)
      {
	fprintf(stderr,
		"Error reading chunk %d from <%s>\n",
		chunk, In_fname);
	
	fclose(input_file);
	exit(01);
      }
      
      BE_to_array_32((ui32 *)&chunk_hdr,
		     sizeof(spdb_chunk_ref_t));

      SPDB_print_chunk_ref(&chunk_hdr, stdout);
      
    } /* endfor - chunk */
    
  } /* endif - Chunks_flag */
  
  fclose(input_file);
  
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
    if (STRequal_exact(argv[i], "-h") ||
	STRequal_exact(argv[i], "-help"))
    {
      print_usage();
      exit(0);
    }
    else if (STRequal_exact(argv[i], "-chunks"))
    {
      Chunks_flag = TRUE;
    }
    else if (STRequal_exact(argv[i], "-min_posn"))
    {
      Min_posn_flag = TRUE;
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
  fprintf(stderr, "Usage: print_spdb_index [options] inputfile\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Options as follows:\n");
  fprintf(stderr, "  [-h] prints help or usage\n");
  fprintf(stderr, "  [-chunks] print out the chunk headers\n");
  fprintf(stderr, "  [-handle] use handle routines\n");
  fprintf(stderr, "  [-min_posn] prints the minute position indices\n");
  fprintf(stderr, "\n");
  
  return;
}
