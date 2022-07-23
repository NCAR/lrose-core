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
/////////////////////////////////////////////////////////////
// StationLoc.cc
//
// StationLoc object
//
// James Yu, III, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2000
//
///////////////////////////////////////////////////////////////
//
// StationLoc finds out the closest point from the mouse click point
//


#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <cerrno>
#include <iostream>
#include <toolsa/toolsa_macros.h>
#include <toolsa/pjg_flat.h>
#include <toolsa/http.h>
#include <Spdb/StationLoc.hh>

using namespace std;

// constructor
StationLoc::StationLoc()
{

}


// destructor
StationLoc::~StationLoc()
{
}

// clear out info

void StationLoc::clear() {
  _info.clear();
}

//////////////////////////////////////////////////////////////////
/// PrintAll()
/// PrintAll function can print all content in the file which was 
/// specified in the Main function out.
//////////////////////////////////////////////////////////////////

void StationLoc::PrintAll()
{
  
  for (size_t j=0;j<_info.size();j++)
    {
      cout << "lat= " << _info[j].ilat << endl;
      cout << "iname= " << _info[j].iname <<endl;
      cout << "ilon= "  << _info[j].ilon <<endl;
      cout << "itype= "  << _info[j].itype << endl;
      cout << "ialt= "  << _info[j].ialt << endl;
    }
}

//////////////////////////////////////////////////////////////////
/// FindClosest()
///
/// Parameters:
///   getlat is the search latitude
///   getlon is the search longtitude
///   max_distance, in km, of the closest point.
///     If no station within max_distance, returns empty string
//////////////////////////////////////////////////////////////////

string StationLoc::FindClosest(double getlat,
			       double getlon,
			       double max_distance /* = 100*/ ) const
{
 
  //compute var define here
  double now_distant,min_distant=10000000;
  string min_name,min_type;
  double min_lat=0,min_lon=0;     
  double dlat,dlon;   //just for computing efficiency
  int counter=0;
  double lon_margin,lat_margin; //find out the lon(lat) margin from km to degree 
  string str;
  //  string temps;
  
  lon_margin=((max_distance*DEG_PER_KM_AT_EQ)/cos(getlat*DEG_TO_RAD))*2.0;
  lat_margin=(max_distance*DEG_PER_KM_AT_EQ)*2.0;
  // cout << "degarea is " << degarea << " and from" << max_distance;
  
  //call PJGLatLon2RTheta function to get distance, r, and angle, theta. 
  double *r,*theta;
  r=new double;
  theta= new double;
  double max_lat_margin=getlat+lat_margin,
    max_lon_margin=getlon+lon_margin,
    min_lat_margin=getlat-lat_margin,
    min_lon_margin=getlon-lon_margin;  // just for computing efficiency
  
  for (size_t j=0;j<_info.size();j++)
    {
      if (((_info[j].ilat<max_lat_margin) &&
	   (_info[j].ilat>min_lat_margin)) && 
	  ((_info[j].ilon<max_lon_margin) &&
	   (_info[j].ilon>min_lon_margin)))
	{
	  counter++;
	  dlon=getlon-_info[j].ilon;
	  dlat=getlat-_info[j].ilat;
	  
	  now_distant=(dlon*dlon)+(dlat*dlat);
	  
	  if ((min_distant>now_distant) || (counter==1))
	    {
	      min_distant=now_distant;
	      min_name=_info[j].iname;
	      min_type=_info[j].itype;
	      min_lat=_info[j].ilat;
	      min_lon=_info[j].ilon;
	    }
	}
    }
  
  PJGLatLon2RTheta(getlat,getlon,min_lat,min_lon,r,theta);

  if ((max_distance<*r) || (counter==0)) {
    min_name="";
  }
  
  return(min_name);  
}

//////////////////////////////////////////////////////////////////
/// FindPosition()
///
/// Given station name, fill out lat, lon, alt and type
//
// Returns 0 on success, -1 on failure.
//
//////////////////////////////////////////////////////////////////

int StationLoc::FindPosition(const string &name,
			     double &lat,
			     double &lon,
			     double &alt,
			     string &type)

{

  for (size_t j=0;j<_info.size();j++)
  {

     if (name==_info[j].iname)
      {
         type=_info[j].itype;
         lat=_info[j].ilat;
         lon=_info[j].ilon;
         alt=_info[j].ialt;
	 return 0;
      }

  }
  return -1;
}  

//////////////////////////////////////////////////////////////////
/// FindAllPosition()
///
/// Given station name, fill out lat, lon, alt and type
//
// Returns number of station matches on success, -1 on failure.
//
//////////////////////////////////////////////////////////////////

int StationLoc::FindAllPosition(const string &name,
				vector <double> &lats,
				vector <double> &lons,
				vector <double> &alts,
				vector <string> &types)
{

  int counter=0;

  for (size_t j=0;j<_info.size();j++)
  {

     if (name==_info[j].iname)
      {
	types.push_back(_info[j].itype);
	lats.push_back(_info[j].ilat);
	lons.push_back(_info[j].ilon);
	alts.push_back(_info[j].ialt);
	counter++;
      }
  }

  if (counter <= 0) {
    return -1;
  } else {
    return counter;
  }
}  

//////////////////////////////////////////////////////////////////
/// ReadData() 
///
/// ReadData function needs one const character parameter, which 
/// gives the file name this program should open to read. If this
/// file can not be opened or opened error, it returns -1. 
/// Otherwise, it returns 0.
//////////////////////////////////////////////////////////////////

int StationLoc::ReadData(const char *FilePath)
{
 
  // Handle a http URL
  if(strncasecmp(FilePath,"http:",5) == 0) { return ReadDataHttp(FilePath); }

  FILE *IN;
  // File must be local
  if((IN=fopen(FilePath,"r"))==NULL) {
    cout << "StationLoc::ReadData open failed for file:" << FilePath; 
    return(-1);
  }
 
  char line[1024];
  char name[1024],type[1024];
  double lat,lon,alt;
  Info info;
      
  while (!feof(IN)) {

    if(fgets(line, 1024, IN)) {

      // skip over comments
      
      if (line[0] == '#') {
	continue;
      }
      
      for(size_t ii = 0; ii < strlen(line); ii++) {

	// eliminate the comma in the data source 	
	if(line[ii] == ',') {
	  line[ii] =' ';
	}
	
      } 
      
      if(sscanf(line,"%s %lf %lf %lf %s",name,&lat,&lon,&alt,type)==5) {
        info.ilat=lat;
        info.ilon=lon;
        info.ialt=alt;
        info.itype=type;
        info.iname=name;    
        _info.push_back(info);   //push four data fields into info class 
      } else if (sscanf(line,"%s %lf %lf %lf",name,&lat,&lon,&alt)==4) {
        info.ilat=lat;
        info.ilon=lon;
        info.ialt=alt;
        info.itype="unknown";
        info.iname=name;    
        _info.push_back(info);   //push four data fields into info class 
      } else if (sscanf(line,"%s %lf %lf",name,&lat,&lon)==3) {
        info.ilat=lat;
        info.ilon=lon;
        info.ialt=-9999.0;
        info.itype="unknown";
        info.iname=name;    
        _info.push_back(info);   //push four data fields into info class 
      } 

    }  

  }
  fclose(IN);

  return(0);
} 

//////////////////////////////////////////////////////////////////
/// ReadDataBuffer() 
///
/// On Error, it returns -1. 
/// Otherwise, it returns 0.
//////////////////////////////////////////////////////////////////

int StationLoc::ReadDataHttp(const char *url)
{
 
  char *buffer = NULL;
  char *str_ptr,*local_ptr,*tmp_ptr;
  int buf_len,ret_stat;
  char name[10],type[10];
  double lat,lon,alt;
  Info info;
 
   // allow 5 seconds to pull in station location data
   ret_stat =  HTTPgetURL((char *)url,5000, &buffer, &buf_len);
   if(ret_stat <= 0 || buf_len <= 0) {
    cout << "StationLoc::ReadDataHttp get failed for URL:" << url; 
    return(-1);
  }

   // Prime strtok_r;
  str_ptr = strtok_r(buffer,"\n",&local_ptr); 

  while (str_ptr != NULL ) {
     // Replace commas with spaces
     while((tmp_ptr = strchr(str_ptr,',')) != NULL) { *tmp_ptr = ' ';}

     if(sscanf(str_ptr,"%s %lf %lf %lf %s",name,&lat,&lon,&alt,type)==5) {
        info.ilat=lat;
        info.ilon=lon;
        info.ialt=alt;
        info.itype=type;
        info.iname=name;    
        _info.push_back(info);   //push four data fields into info class 
    }  

    // Get next line
    str_ptr = strtok_r(NULL,"\n",&local_ptr);
  }

  if(buffer != NULL) free(buffer);

  return(0);
} 




