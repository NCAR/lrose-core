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

#include <iostream>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxProj.hh>

int main(int argc, char *argv[]){

  typedef struct {
    char *label;
    double lat;
    double lon;
  } point_t;

  static const int numPoints = 10;

  static const point_t points[] = {
   { "5605 N 63rd Street" ,40.07726, -105.20699 },
   { "6235 Lookout Road",  40.07018, -105.20820 },
   { "6101 Lookout Road",  40.06798, -105.21086 },
   { "6000 Spine Road",    40.06487, -105.21568 },
   { "6055 Longbow",       40.06415, -105.21261 },
   { "5920 Longbow",       40.06426, -105.21448 },
   { "4600 Sleepytime Drive",40.05931,-105.21753 },
   { "5445 Westridge",     40.06357, -105.22677 },
   { "5100 N 51st Marina", 40.07072,-105.22135 },
   { "Road Near Training Site",40.07110, -105.21886}
  };


  for (int month=4; month < 11; month++){
    for (int hour = 14; hour < 24; hour +=3){

      cout << endl << "Month : " << month << " Hour : " << hour << endl;

      //
      // Put together the URL and read the data.
      //
      char url[1024];
      sprintf(url,"mdvp::prob_max_%02d%02d//localhost::/home/oien/rap/projects/CS/mdv/prob_max_%02d%02d",
	      month, hour, month, hour);

      DsMdvx New;
      New.setReadTime(Mdvx::READ_FIRST_BEFORE, url, 0, time(NULL));
      New.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
      New.setReadCompressionType(Mdvx::COMPRESSION_NONE);

      if (New.readVolume()){
	cerr << "Read failed at " << utimstr(time(NULL)) << " from ";
	cerr << url  << endl;
	return -1;
      }     

      MdvxField *Field = New.getFieldByName( "probability" );
      if (Field == NULL){
	cerr << "Something is terribly wrong!" << endl;
	exit(-1);
      }

      Mdvx::field_header_t Fhdr = Field->getFieldHeader();
      Mdvx::master_header_t Mhdr = New.getMasterHeader();

      MdvxProj Proj(Mhdr, Fhdr);
      
      fl32 *Data = (fl32 *) Field->getVol();

      for (int ip=0; ip < numPoints; ip++){
	cout << points[ip].label << " at " << points[ip].lat << ", " << points[ip].lon << " : ";

	int ix, iy;
	if (Proj.latlon2xyIndex(points[ip].lat, points[ip].lon, ix, iy)){
	  cerr << "Outside grid." << endl;
	  continue;
	}

	int index = iy * Fhdr.nx + ix;

	if (
	    (Data[index] == Fhdr.bad_data_value) ||
	    (Data[index] == Fhdr.missing_data_value)
	    ) {
	  cout << "0%";
	} else {
	  cout << Data[index] << "%";
	}
	cout << endl;
      }


    }
  }



  return 0;

}
