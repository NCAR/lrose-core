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
 * get_vcp.c
 *
 * Get the volume coverage patterns.
 *
 * Mike Dixon RAP NCAR Boulder CO USA
 *
 * May 1997
 *
 **************************************************************************/

#include "ridds2mom.h"
using namespace std;

int get_vcp (char *vcp_path,
	     NEXRAD_vcp_set **vol_cntrl_patterns)

{
  NEXRAD_vcp_set *vcp_set;
  char line [BUFSIZ];
  FILE *fp;
  int i = 0;
  int j = 0;
  int line_no = 1;
  float *fixed_angle;
  NEXRAD_vcp *vcp;
  NEXRAD_vcp *tmp;
  
  if ((fp = fopen (vcp_path, "r")) == NULL) {
    fprintf (stderr, "Error opening vcp file\n");
    perror(vcp_path);
    return (-1);
  }

  vcp_set = (NEXRAD_vcp_set *) umalloc (sizeof (NEXRAD_vcp_set));
  vcp_set->scan_strategy = (NEXRAD_vcp *) umalloc (sizeof (NEXRAD_vcp));
  vcp_set->scan_strategy->fixed_angle = (float **) umalloc (sizeof (float));

  *vol_cntrl_patterns = vcp_set;

  fgets (line, sizeof (line), fp); line_no++; /* skip the comment */
  fgets (line, sizeof (line), fp); line_no++; /* skip the comment */
  fgets (line, sizeof (line), fp); line_no++;
  if (sscanf (line, "%d", &vcp_set->vcp_total) != 1) {
    fprintf(stderr, "ERROR, VCP file %s, line_no %d\n",
	    vcp_path, line_no);
    fclose(fp);
    return (-1);
  }
  
  vcp = vcp_set->scan_strategy;
  for (i = 0; i < vcp_set->vcp_total; i++) {

    fgets (line, sizeof (line), fp); line_no++; /* skip the comment */
    fgets (line, sizeof (line), fp); line_no++;
    if (sscanf (line, "%d", &vcp->pattern_number) != 1) {
      fprintf(stderr, "ERROR, VCP file %s, line_no %d\n",
	      vcp_path, line_no);
      fclose(fp);
      return (-1);
    }
    
    fgets (line, sizeof (line), fp); line_no++; /* skip the comment */
    fgets (line, sizeof (line), fp); line_no++;
    if (sscanf (line, "%d", &vcp->num_of_fixed_angles) != 1) {
      fprintf(stderr, "ERROR, VCP file %s, line_no %d\n",
	      vcp_path, line_no);
      fclose(fp);
      return (-1);
    }
    
    *vcp->fixed_angle = (float *)
      umalloc (vcp->num_of_fixed_angles * sizeof (float));
    fixed_angle = *vcp->fixed_angle;

    fgets (line, sizeof (line), fp); line_no++; /* skip the comment */

    for (j = 0; j < vcp->num_of_fixed_angles; j++) {
      fgets (line, sizeof (line), fp); line_no++;
      if (sscanf (line, "%f", &fixed_angle[j]) != 1) {
	fprintf(stderr, "ERROR, VCP file %s, line_no %d\n",
		vcp_path, line_no);
	fclose(fp);
	return (-1);
      }
    } /* j */

    if (i < vcp_set->vcp_total - 1) {
      vcp->next_pattern = (char *) umalloc (sizeof (NEXRAD_vcp));
      tmp = (NEXRAD_vcp *) vcp->next_pattern;
      vcp = tmp;
      vcp->fixed_angle = (float **) umalloc (sizeof (float));
    } /* if (i < vcp_set->vcp_total - 1) */

  } /* i */

  fclose (fp);

  return (0);

}

