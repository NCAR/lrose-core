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

#include <cstdio>
#include <ctime>

#include <cstdlib>
#include <string.h>
#include <toolsa/umisc.h>

#include <mdv/mdv_write.h>
#include <mdv/mdv_utils.h>
#include <mdv/mdv_user.h>   
#include <mdv/mdv_read.h>
using namespace std;

main(int argc, char *argv[])
{

  if (getenv("CLEAR_SKY_CLIMATOLOGY_DIR")==NULL){
    fprintf(stderr,"Please define CLEAR_SKY_CLIMATOLOGY_DIR\n");
    exit(-1);
  }    

  char ClimateDir[MAX_PATH_LEN];
  sprintf(ClimateDir,"%s",getenv("CLEAR_SKY_CLIMATOLOGY_DIR"));
 
  if (argc < 3){
    fprintf(stderr,"Give me the MDV file and field number on the command line.\n");
    fprintf(stderr,"Note : Field numbers start at 1.\n");
    exit(-1);
  }


  MDV_handle_t Handle;
  MDV_init_handle(&Handle);
  
  // Read the file.
  if(MDV_read_all(&Handle,argv[1],MDV_INT8)){
    fprintf(stderr,"Failed to read %s.\n",argv[1]);
    exit(-1);
  }  


  //
  // Go through all the fields and initialise them.
  //

  MDV_field_header_t *Fhdr = Handle.fld_hdrs;

  int f; // Field number.
  f=atoi(argv[2]);
  if ((f<1) || (f>Handle.master_hdr.n_fields)){
    fprintf(stderr,"There is no field %d in %s\n",
	    f,argv[1]);
    exit(-1);
  }

  f--; // Go to logical indexing.

  int check;
  fprintf(stdout,"Field %d has name %s, units %s\n",
	  f+1, (Fhdr+f)->field_name, (Fhdr+f)->units);
  fprintf(stdout,
	  "Continue and reset this field to missing in %s (0=No,1=Yes) ?",
	  argv[1]);

  fscanf(stdin,"%d",&check);

  if (!(check)) { 
    fprintf(stdout,"Bye.\n");
    exit(0);
  }

  unsigned char *b;
  b=(unsigned char *)Handle.field_plane[f][0];

  for (int k=0; k<(Fhdr+f)->nx * (Fhdr+f)->ny; k++){
    b[k]=(unsigned char)((Fhdr+f)->bad_data_value);
  }


  int i = MDV_write_to_dir(&Handle,ClimateDir, MDV_PLANE_RLE8, TRUE);

  if (i!= MDV_SUCCESS){
    fprintf(stderr,"Failed to write data!\n");
  }

  MDV_free_handle(&Handle);
 
}




