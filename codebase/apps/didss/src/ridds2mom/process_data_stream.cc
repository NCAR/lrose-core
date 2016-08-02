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
 * process_data_stream.c
 *
 * reads in a beam data, reformats it and writes it out.
 *
 * Mike Dixon RAP NCAR Boulder CO USA
 *
 * May 1997
 *
 **************************************************************************/

#include "ridds2mom.h"
using namespace std;

int process_data_stream(void)

{

  ui08 *read_buffer;
  ui08 *write_buffer;
  int iret;
  int nread;
  int nwrite;
  int forever = TRUE;
  NEXRAD_vcp_set *vcp_set;

  /*
   * read in vol coverage patterns
   */
  
  if (get_vcp (Glob->params.vol_coverage_path,
	       &vcp_set)) {
    fprintf(stderr, "ERROR reading vcp file %s\n",
	    Glob->params.vol_coverage_path);
    return(-1);
  }

  while (forever) {

    /*
     * read in record
     */

    if (read_stream(&read_buffer, &nread)) {
      return (-1);
    }

    /*
     * reformat buffer
     */
    
    if (Glob->params.output_format == LL_FORMAT) {
      iret = reformat2ll(read_buffer, nread,
			 vcp_set, &write_buffer,
			 &nwrite);
    } else if (Glob->params.output_format == DS_FORMAT) {
      iret = reformat2ds(read_buffer, nread,
 			 vcp_set, &write_buffer,
			 &nwrite);
    } else {
      /* output native RIDDS */
      write_buffer = read_buffer;
      nwrite = nread;
      iret = 0;
    }
    
    /*
     * write output
     */
    
    if (iret == 0 && nwrite > 0) {

      if (Glob->params.print_header) {
	print_header(write_buffer);
      } else if (Glob->params.print_summary) {
	print_summary(write_buffer);
      }

      write_stream(write_buffer, nwrite);
      
    }
    
    /*
     * archive input ridds stream
     */
    
    if (iret == 0 && Glob->params.write_archive_fmq) {
      if (write_archive_fmq(read_buffer, nread)) {
	iret = -1;
      }      
    }
    
  } /* while (forever) */

  return (0);
  
}


