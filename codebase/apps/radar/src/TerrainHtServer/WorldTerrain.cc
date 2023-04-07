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
#include <Mdv/MdvxField.hh>
#include <Mdv/DsMdvx.hh>
#include <toolsa/umisc.h>
#include <toolsa/uusleep.h>
#include <cstdio>
#include <math.h>
#include <xmlrpc-c/girerr.hpp>
#include <xmlrpc-c/base.hpp>
#include <xmlrpc-c/client_simple.hpp>
using namespace std;

static int _getHt(double lat, double lon, 
                  double &htM, bool &isWater);

int main(int argc, char *argv[])
{
  
  // parse args
  
  if (argc < 8){
    fprintf(stderr,
            "USAGE : WorldTerrain LowerLeftLat LowerLeftLon "
            "Nx Ny Dx Dy OutDir\n\n");
    exit(-1);
  } 
  
  double OutLat = atof(argv[1]);
  double OutLon = atof(argv[2]);
  int OutNx = atoi(argv[3]);
  int OutNy = atoi(argv[4]);
  double OutDx = atof(argv[5]);
  double OutDy = atof(argv[6]);
  string outDir(argv[7]);

  //create data arrays

  fl32 *height = (fl32 *) malloc(OutNx * OutNy * sizeof(fl32));
  fl32 *landuse = (fl32 *) malloc(OutNx * OutNy * sizeof(fl32));
  
  memset(height, 0, OutNx * OutNy * sizeof(fl32));
  memset(landuse, 0, OutNx * OutNy * sizeof(fl32));

  // get height for each point in the grid

  int offset = 0;
  for (int iy = 0; iy < OutNy; iy++) {
    
    double Lat = OutLat + iy * OutDy;
    fprintf(stdout,"Latitude : %g\n",Lat);
    
    for (int ix = 0; ix < OutNx; ix++, offset++){

      double Lon = OutLon + ix * OutDx;
      double htM = -9999;
      bool isWater = false;
      if (_getHt(Lat, Lon, htM, isWater)) {
        exit(-1);
      }

      height[offset] = htM;
      if (isWater) {
        landuse[offset] = 1;
      } else {
        landuse[offset] = 2;
      }

      // struct timespec sleepTime;
      // sleepTime.tv_sec = 0;
      // sleepTime.tv_nsec = 2000000;
      // nanosleep(&sleepTime, NULL);
      umsleep(10);
      
    } // ix
  } // iy

  // create Mdvx object
  
  DsMdvx Out;   
  time_t Now = time(NULL);

  // master header

  Mdvx::master_header_t OutMhdr;
  OutMhdr.time_gen = Now;
  OutMhdr.user_time = Now;
  OutMhdr.time_begin = Now;
  OutMhdr.time_end = Now;
  OutMhdr.time_centroid = Now;
  OutMhdr.time_expire = Now;
  OutMhdr.data_collection_type = Mdvx::DATA_MEASURED;
  OutMhdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  OutMhdr.vlevel_included = 1;
  OutMhdr.n_fields = 1;
  OutMhdr.max_nx = OutNx;
  OutMhdr.max_ny = OutNy;
  OutMhdr.max_nz = 1; 
  OutMhdr.sensor_lon = OutLon; 
  OutMhdr.sensor_lat = OutLat;   
  OutMhdr.sensor_alt = 0.0; 
  Out.setMasterHeader(OutMhdr);
  Out.setDataSetInfo("created by WorldTerrain based on SRTM30");

  // fields
  // field header for height

  Mdvx::field_header_t Fhdr;
  MEM_zero(Fhdr);
  
  Fhdr.struct_id = Mdvx::FIELD_HEAD_MAGIC_COOKIE_64;
  Fhdr.field_code = 5;
  Fhdr.user_time1 =     Now;
  Fhdr.forecast_delta = 0; 
  Fhdr.user_time2 =     Now;
  Fhdr.user_time3 =     Now;
  Fhdr.forecast_time =  Now;
  Fhdr.user_time4 =     Now; 
  Fhdr.nx =  OutNx;
  Fhdr.ny =  OutNy;
  Fhdr.nz =  1;
  Fhdr.proj_type = Mdvx::PROJ_LATLON;
  Fhdr.proj_origin_lat =  OutLat;
  Fhdr.proj_origin_lon =  OutLon;
  Fhdr.grid_dx =  OutDx;
  Fhdr.grid_dy =  OutDy;
  Fhdr.grid_dz =  1;
  
  Fhdr.grid_minx =  OutLon;
  Fhdr.grid_miny =  OutLat;
  Fhdr.grid_minz =  0;

  Fhdr.bad_data_value = -9999.0;
  Fhdr.missing_data_value = -9999.0;
  Fhdr.proj_rotation = 0.0;

  Fhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  Fhdr.data_element_nbytes = sizeof(fl32);
  Fhdr.volume_size = Fhdr.nx * Fhdr.ny * Fhdr.nz * sizeof(fl32);
  Fhdr.compression_type = Mdvx::COMPRESSION_NONE;
  Fhdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  Fhdr.scaling_type = Mdvx::SCALING_NONE;     
  
  // vlevel header

  Mdvx::vlevel_header_t Vhdr;
  MEM_zero(Vhdr);               
  for (int iz = 0; iz < Fhdr.nz; iz++) {
    Vhdr.type[iz] = Mdvx::VERT_TYPE_SURFACE;
    Vhdr.level[iz] = Fhdr.grid_minz + iz * Fhdr.grid_dz;
  }
  
  // create height field

  MdvxField *htFld = new MdvxField(Fhdr, Vhdr, (void *)height);
  htFld->setFieldName("Height");
  htFld->setFieldNameLong("Height MSL");
  htFld->setUnits("m");
  
  if (htFld->compress(Mdvx::COMPRESSION_GZIP)){
    cerr << "ERROR - compress failed - height field" << endl;
    return -1;
  }  
  Out.addField(htFld);

  // now landuse field

  Fhdr.bad_data_value = 0;
  Fhdr.missing_data_value = 0;
  Fhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  Fhdr.data_element_nbytes = sizeof(fl32);
  Fhdr.volume_size = Fhdr.nx * Fhdr.ny * Fhdr.nz * sizeof(fl32);
  Fhdr.compression_type = Mdvx::COMPRESSION_NONE;
  Fhdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  Fhdr.scaling_type = Mdvx::SCALING_NONE;     
  
  MdvxField *landuseFld = new MdvxField(Fhdr, Vhdr, (void *)landuse);
  landuseFld->setFieldName("LandUse");
  landuseFld->setFieldNameLong("Land use - land or water");
  landuseFld->setUnits("water=1,land=2");
  
  if (landuseFld->compress(Mdvx::COMPRESSION_GZIP)){
    cerr << "ERROR - compress failed - landuse field" << endl;
    return -1;
  }  
  Out.addField(landuseFld);

  // write output
  
  if (Out.writeToDir(outDir)) {
    cerr << "Cannot write file to dir: " << outDir << endl;
  } else {
    cerr << "Wrote file: " << Out.getPathInUse() << endl;
  }

  free(height);
  free(landuse);

  return 0;

}

//////////////////////////////////////////
// get ht from server 
// for given lat/lon

static int _getHt(double lat, double lon, 
                  double &htM, bool &isWater)
  
{
  
  try {
    
    // make a call to the server, sending lat/lon as double args
    
    string const serverUrl("http://localhost:9090/RPC2");
    string const methodName("get.height");
    xmlrpc_c::clientSimple myClient;
    xmlrpc_c::value result;
    myClient.call(serverUrl, methodName, "dd", &result, lat, lon);
    
    // decode response struct { double ht, bool isWater }
    
    xmlrpc_c::value_struct reply(result);
    map<string, xmlrpc_c::value> const replyData =
      static_cast<map<string, xmlrpc_c::value> >(reply);
    map<string, xmlrpc_c::value>::const_iterator htP = replyData.find("heightM");
    map<string, xmlrpc_c::value>::const_iterator waterP = replyData.find("isWater");
    map<string, xmlrpc_c::value>::const_iterator errorP = replyData.find("isError");
    
    htM = -9999.0;
    isWater = false;
    bool isError = false;
    
    if (htP != replyData.end()) {
      htM = (xmlrpc_c::value_double) htP->second;
    }

    if (waterP != replyData.end()) {
      isWater = (xmlrpc_c::value_boolean) waterP->second;
    }
    
    if (errorP != replyData.end()) {
      isError = (xmlrpc_c::value_boolean) errorP->second;
    }
    
    cerr << "==>> lat, lon, ht, water, error: "
         << lat << ", "
         << lon << ", "
         << htM << ", "
         << (isWater?"Y":"N") << ", "
         << (isError?"Y":"N") << endl;
    
  } catch (exception const& e) {

    cerr << "Client threw error: " << e.what() << endl;
    return -1;

  } catch (...) {

    cerr << "Client threw unexpected error." << endl;
    return -1;

  }
  
  return 0;

}







