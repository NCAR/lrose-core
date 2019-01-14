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

#include <toolsa/umisc.h>
#include <cstdio>
#include <Mdv/mdv/mdv_handle.h>
#include <Mdv/mdv/mdv_read.h>
#include <Mdv/mdv/mdv_write.h>
#include <dsserver/DmapAccess.hh>
using namespace std;

void process_file(char *InDir, char *OutDir, date_time_t FileTime){


  //
  // Put together the input MDV filename.
  //


  char Filename[MAX_PATH_LEN];

  if (InDir[strlen(InDir) -1] != '/'){
    sprintf(Filename,"%s/%02d%02d%02d.mdv",InDir,
            FileTime.hour,FileTime.min,FileTime.sec);
  } else {
    sprintf(Filename,"%s%02d%02d%02d.mdv",InDir,
            FileTime.hour,FileTime.min,FileTime.sec);

  }

  MDV_handle_t M;
  MDV_init_handle(&M);
  if (MDV_read_all(&M,Filename,MDV_INT8)){
    fprintf(stderr,"Failed to read %s\n",Filename);
    exit(-1);
  }

  // Cludge the times in the input MDV file.
  time_t offset;
  offset= FileTime.unix_time-M.master_hdr.time_centroid;

  M.master_hdr.time_gen += offset;
  M.master_hdr.time_begin += offset;
  M.master_hdr.time_end += offset;
  M.master_hdr.time_centroid += offset;
  M.master_hdr.time_expire += offset;

  for (int i = 0; i < M.master_hdr.n_fields; i++){
    M.fld_hdrs[i].forecast_time += offset;
  }

  // Write out the new file
  
  if (MDV_write_to_dir(&M,OutDir,MDV_PLANE_RLE8,1) != MDV_SUCCESS){
    fprintf(stderr, "Failed to write MDV file to %s\n",OutDir);
    return;
  }

  DmapAccess dmapAccess;
  dmapAccess.regLatestInfo(M.master_hdr.time_centroid, OutDir);

  // Let go.

  MDV_free_handle(&M);

}



