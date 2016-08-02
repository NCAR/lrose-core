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
//
// One data time becomes a full day of data with this simple
// app which can be used with MdvRepeatDay to play the same data
// again & again. And again.
//
// Niles Oien

#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>  
#include <cstdio>
#include <cstdlib>
#include <math.h>
using namespace std;

int main(int argc, char *argv[])
{

  //
  // Get the input MDV filename from the command line.
  //
  char FileName[1024];

  // See if we got a filename on the input line.
  // if not, ask for one.
  // 
  if (argc > 1){
    sprintf(FileName,"%s", argv[1]);
  } else {
    fprintf(stdout,"Input file name --->");
    fscanf(stdin,"%s",FileName);
  }

  //
  // Did we get an output URL? If not, fake one.
  //
  char outUrl[1024];
  if (argc > 2){
    sprintf(outUrl,"%s", argv[2]);
  } else {
    sprintf(outUrl,"%s", "./mdv/mySpecialDay");
  }

  //
  // Did we get an output interval? If not, fake one.
  //
  int interval = 300; // Seconds
  if (argc > 3){
    interval = atoi(argv[3]);
  }

  //
  // Did we get a time offset? If not, fake one.
  //
  int timeOffset = 0; // Seconds
  if (argc > 4){
    timeOffset = atoi(argv[4]);
  }


  //
  // Instantiate an MDV object and read data.
  //
  DsMdvx In;
  In.setReadPath(FileName);
  if (In.readVolume()){
    fprintf(stderr,"Failed to open %s\n",FileName);
    exit(-1);
  }

  date_time_t T;
  T.year = 2001; T.month = 9; T.day = 11;
  T.hour = 0; T.min = 0; T.sec = 0;
  uconvert_to_utime( &T );

  int lead = 0;

  do {

    time_t tik = T.unix_time + lead + timeOffset;

    Mdvx::master_header_t InMhdr = In.getMasterHeader();     

    InMhdr.time_gen = tik;
    InMhdr.time_begin = tik;
    InMhdr.time_centroid = tik;
    InMhdr.time_end = tik;
    InMhdr.time_expire = tik + interval;

    DsMdvx Out;
    Out.setMasterHeader( InMhdr );


    for (int i=0; i < InMhdr.n_fields; i++){

      MdvxField *InField;

      InField = In.getFieldByNum( i );

      if (InField == NULL){
	fprintf(stderr,"Field %d not there.\n",i);
	exit(-1);
      }

      Mdvx::field_header_t InFhdr = InField->getFieldHeader();   
      Mdvx::vlevel_header_t InVhdr = InField->getVlevelHeader();   
      const void *data = InField->getVol();
      
      MdvxField *field = new MdvxField(InFhdr, InVhdr, data);

      Out.addField(field);

    }

    if ( Out.writeToDir( outUrl ) ){
      fprintf(stderr, "Failed to write to %s at %s\n",
	      outUrl, utimstr(InMhdr.time_centroid));
    }

    lead += interval;

    fprintf(stderr,"Processing for %s\n", utimstr(InMhdr.time_centroid));

  } while (lead + timeOffset < 86400);


  return (0);

}








