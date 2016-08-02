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
 
  if (argc < 2){
    fprintf(stderr,"Give me the initial MDV file on command line.\n");
    exit(-1);
  }


  MDV_handle_t Handle;
  MDV_init_handle(&Handle);
  
  // Read the file.
  if(MDV_read_all(&Handle,argv[1],MDV_INT8)){
    fprintf(stderr,"Failed to read %s.\n",argv[1]);
    exit(-1);
  }  

  date_time_t DataTime;

  DataTime.year=1990;
  DataTime.day=15;
  DataTime.min=0; DataTime.sec=0;

  //
  // Go through all the fields and initialise them.
  //

  MDV_field_header_t *Fhdr = Handle.fld_hdrs;

  for (int f=0; f<Handle.master_hdr.n_fields; f++){

      switch ((Fhdr+f)->field_code){

      case 11 : // Temperature
	(Fhdr+f)->scale=0.5; 
	(Fhdr+f)->bias=-63.5;
	break;

      case 84 : // Albedo
	(Fhdr+f)->scale=0.4; 
	(Fhdr+f)->bias=0.0;
	break;

      default :
	fprintf(stderr,"Unrecognised field code %d\n", 
		(Fhdr+f)->field_code );
	exit(-1);
	break;


      }


    unsigned char *b;
    b=(unsigned char *)Handle.field_plane[f][0];

    (Fhdr+f)->bad_data_value=255.0;
    (Fhdr+f)->missing_data_value=255.0;

    for (int k=0; k< (Fhdr+f)->nx * (Fhdr+f)->ny; k++){
      b[k]=(unsigned char)((Fhdr+f)->bad_data_value);
    }

  }

  //
  // Then go and munge the times.
  //


  for (DataTime.month=1; DataTime.month < 13; DataTime.month++){
    for (DataTime.hour=0; DataTime.hour < 24; DataTime.hour++){

      fprintf(stdout,"Month %d Hour %d\n",
	      DataTime.month,DataTime.hour);

      DataTime.unix_time=uconvert_to_utime(&DataTime); 

      Handle.master_hdr.time_gen = time(NULL);
      Handle.master_hdr.time_begin = DataTime.unix_time;
      Handle.master_hdr.time_end = DataTime.unix_time+3600;
      Handle.master_hdr.time_centroid = DataTime.unix_time+1800;
      Handle.master_hdr.time_expire = time(NULL)+20*365*24*3600;

      int i = MDV_write_to_dir(&Handle,ClimateDir, MDV_PLANE_RLE8, TRUE);

      if (i!= MDV_SUCCESS){
	fprintf(stderr,"Failed to write data!\n");
      }
    }
  }

  MDV_free_handle(&Handle);
 

}




