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
// MdvMgr.cc
//
// MdvMgr object
//
// January 2004
//
///////////////////////////////////////////////////////////////

#include "MdvMgr.hh"
using namespace std;


/////////////////////////////////////////////////////////////// 
//
// constructor. Sets up DsMdvx object.
//
MdvMgr::MdvMgr (Params &params, time_t start, time_t end)
{
  //
  // Make a copy of the TDRP parameters and start and end times.
  //
  _params = params;

  _start = start;

  _end = end;

}

int MdvMgr::init()
{
  
  int ret = _startMdvObject();
  
  return( ret);
  
}

///////////////////////////////////////////////////////////////
//
// addPolygon(): Adds Polygon to the output grid.
//
int MdvMgr::addPolygon(GenPoly &genPoly){

  PMU_auto_register("GenPoly2Mdv:Adding polygon to grid.");
  
  //
  // Get Mdv master header, field, field header, and data
  //          
  Mdvx::master_header_t mhdr = _Mdv->getMasterHeader();
              
  MdvxField *field = _Mdv->getFieldByName( "Polygons" );      

  Mdvx::field_header_t fhdr = field->getFieldHeader();        
 
  fl32 *mdvData = (fl32 *) field->getVol(); 

  //
  // Get grid dimensions 
  //
  int nx = fhdr.nx;

  int ny = fhdr.ny;

  int nz = fhdr.nz;

  //
  // Make a copy of the data to work with.
  //
  fl32 *fieldData = new float[nx * ny * nz];

  if (fieldData == NULL)
    {
      cerr << "MdvMgr::addPolygon():Memory allocation failed." << endl;
      exit(1);
    }

  for (int i=0; i < nx * ny * nz; i++)
    {
      fieldData[i] = mdvData[i];
    }
  
  MdvxProj proj(mhdr, fhdr);  

  //
  // Initialize min and max x and y coordinates of 
  // vertices of polygon. (We keep track of the min 
  // and max x and y coordinates that correspond to the vertices
  // so we only have to analyze a subset of the grid.) 
  //
  float max_x = 0;
  
  float max_y = 0; 
  
  float min_x = nx - 1;
  
  float min_y = ny - 1;

  //
  // Get the vertices. If the vertices are outside
  // of the grid, assign them the lat and lon of the nearest grid
  // point. keep track of min and max x and y coordiantes
  //
  int numVertices = genPoly.getNumVertices();

  double *lats = new double[numVertices];

  double *lons = new double[numVertices];

  for (int i = 0; i < numVertices; i++)
    {
      GenPoly::vertex_t vertex = genPoly.getVertex(i);
      
      lats[i] = vertex.lat;
      
      lons[i] = vertex.lon;

      int x_index, y_index;
      
      int ret = proj.latlon2xyIndex(lats[i], lons[i], x_index, y_index);
 
      //
      // Vertices are out of the grid. Set the coordinates to grid boundary
      // if out of the grid.
      //
      if (ret)
	{
	  if (x_index > nx - 1 )
	    x_index = nx -1;
	  
	  if (x_index < 0 )
	    min_x = 0;
	  
	  if (y_index > ny - 1)
	    y_index = ny - 1;
	  
	  if (y_index < 0)
	    y_index = 0;
	}

      //
      // Keep track of max x and min y for determnining subgrid
      // 
      if (x_index > max_x )
	max_x = x_index;

      if (x_index < min_x)
	min_x = x_index;

      if (y_index > max_y)
	max_y = y_index;
      
       if (y_index < min_y)
	min_y = y_index;
    }


  //
  // Create polyline. Args are:  origin_lat, origin_lon, rotation, numVertices,
  // lats of vertices, lons of vertices, x coord of center of polygon, y coord 
  // of center of polygon, time, shape. We set x coord of center of polygon = 
  // y coord of center of polygon = time = 0 since  these args are irrelevant 
  // for our needs and use of the Polyline class. Note that 'Shape' defines 
  // whether the polyline is open or closed and we make it closed.
  //
  double min_lat, min_lon;
  proj.xyIndex2latlon(0, 0, min_lat, min_lon);

  Polyline *polyline = new Polyline( min_lat, min_lon, 0, numVertices,
				     lats, lons, 0, 0, 0, Polyline::CLOSED);
  
  for (int i = (int)floor(min_x); i <= (int)ceil(max_x); i++)
    {
      for (int j = (int)floor(min_y); j <= (int)ceil(max_y); j++)
	{
	  
	  //
	  // If (i,j) in polygon 
	  // tag with GenPoly value
	  //
	  double point_lat, point_lon;
	  
	  proj.xyIndex2latlon(i, j, point_lat, point_lon);
	  
	  
	  if (polyline->inPolyline(point_lat, point_lon))
	    fieldData[i + nx * j] = genPoly.get1DVal(0);
      }
  }

  field->setVolData( fieldData, nx * ny * nz * sizeof(fl32),
		     Mdvx::ENCODING_FLOAT32,
		     Mdvx::SCALING_NONE);

  //
  // cleanup
  //
  delete[] fieldData;
  
  delete[] lats;
  
  delete[] lons;

  delete polyline;

  return 0;
}
///////////////////////////////////////////////////////////////  
//
// Write Mdv data out. 
//
int MdvMgr::write(){
  
  
  PMU_auto_register("GenPoly2Mdv:Writing output.");

  //
  // Set the time on the output data.
  //
  Mdvx::master_header_t master_hdr = _Mdv->getMasterHeader();

  time_t Now; 
  
  Now=time(NULL);
  
  master_hdr.time_gen = Now;
  master_hdr.time_begin = _start;
  master_hdr.time_end = _end;

  switch ( _params.time_stamp_mode) {
     case Params::TIMESTAMP_START :
        master_hdr.time_centroid = _start;
        break;

     case Params::TIMESTAMP_MIDDLE :
        master_hdr.time_centroid = _start + (_end - _start) / 2;
        break;

     case Params::TIMESTAMP_END :
        master_hdr.time_centroid = _end;
        break;

     default :
        cerr << "Unrecognized timestamp option." << endl;
        return 1;
        break;

  }

  master_hdr.time_expire = _end + (_end - _start);

  _Mdv->setMasterHeader( master_hdr );

  if (_params.debug){
    cerr << "MdvMgr::write(): Writing MDV output to " << _params.mdv_url;
    cerr << " for time " << utimstr( master_hdr.time_centroid ) << endl;
  }

  //
  // Set the times in the field headers.
  //
  MdvxField *field = _Mdv->getFieldByName( "Polygons" );      
  
  Mdvx::field_header_t fhdr = field->getFieldHeader();        
  
  fhdr.forecast_delta = 0;
  
  fhdr.forecast_time =  master_hdr.time_centroid;
  
  field->setFieldHeader( fhdr );

  //
  // Compress the fields appropriately.
  //
  if (field->convertRounded(Mdvx::ENCODING_INT8,
				 Mdvx::COMPRESSION_ZLIB)){
    cerr << "convertRounded failed." << endl;

    return 1;
  }

  //
  // Write output.
  //
  if (_Mdv->writeToDir( _params.mdv_url )) {

    cerr << "MdvMgr::write(): Failed to write to " << _params.mdv_url << endl;

    return 1;
  }
  
  return 0;

}


MdvMgr::~MdvMgr()
{
  delete _Mdv;
}

///////////////////////////////////////////////////////////////  
//
// Start an MDV object from scratch.
//
int MdvMgr::_startMdvObject(){

  //
  // Set up the master header.
  //
  Mdvx::master_header_t master_hdr;

  time_t Now = time(NULL);

  master_hdr.time_gen = Now;

  master_hdr.time_begin = _start;

  master_hdr.time_end = _end;

  switch ( _params.time_stamp_mode) {

     case Params::TIMESTAMP_START :
        master_hdr.time_centroid = _start;
        break;
 
     case Params::TIMESTAMP_MIDDLE :
        master_hdr.time_centroid = _start + (_end - _start) / 2;
        break;
 
     case Params::TIMESTAMP_END :
        master_hdr.time_centroid = _end;
        break;
 
     default :
        cerr << "Unrecognized timestamp option." << endl;
        exit(-1);
        break;
 
  }

  master_hdr.time_expire = _end + (_end - _start);

  sprintf(master_hdr.data_set_info,"%s","Polygons");

  sprintf(master_hdr.data_set_name,"%s","Polygons");

  sprintf(master_hdr.data_set_source,"%s", _params.spdb_url );

  _Mdv = new DsMdvx();
  _Mdv->setAppName("GenPoly2Mdv");

  _Mdv->setMasterHeader(master_hdr);

  _Mdv->clearFields();    

  Mdvx::field_header_t fhdr;

  MEM_zero(fhdr);

  fhdr.nx =  _params.output_grid.nx;

  fhdr.ny =  _params.output_grid.ny;

  fhdr.nz =  1; 

  switch ( _params.output_projection){

  case Params::OUTPUT_PROJ_LATLON :
    fhdr.proj_type = Mdvx::PROJ_LATLON;
    break;

  case Params::OUTPUT_PROJ_FLAT :
    fhdr.proj_type = Mdvx::PROJ_FLAT;
    break;

  case Params::OUTPUT_PROJ_LAMBERT :
    fhdr.proj_type = Mdvx::PROJ_LAMBERT_CONF;
    break;

  }

  fhdr.proj_origin_lat =   _params.output_origin.lat;
  fhdr.proj_origin_lon =   _params.output_origin.lon;
  fhdr.grid_dx =  _params.output_grid.dx;
  fhdr.grid_dy =  _params.output_grid.dy;
  fhdr.grid_dz =  0.0;
 
  fhdr.grid_minx =  _params.output_grid.minx;
  fhdr.grid_miny =  _params.output_grid.miny;
  fhdr.grid_minz =  0.0;

  fhdr.bad_data_value = -999;
  fhdr.missing_data_value = fhdr.bad_data_value;
  fhdr.proj_rotation = _params.output_rotation;

  sprintf(fhdr.field_name_long,"%s", "Polygons");
  sprintf(fhdr.field_name,"%s", "Polygons");
  sprintf(fhdr.units,"%s", "None");
  sprintf(fhdr.transform,"%s"," ");

  fhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  fhdr.data_element_nbytes = sizeof(fl32);
  fhdr.volume_size = fhdr.nx * fhdr.ny * fhdr.nz * sizeof(fl32);
  fhdr.compression_type = Mdvx::COMPRESSION_NONE;
  fhdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  fhdr.scaling_type = Mdvx::SCALING_NONE;

  Mdvx::vlevel_header_t vhdr;
  MEM_zero(vhdr);

  for (int iz = 0; iz < fhdr.nz; iz++) {
    vhdr.type[iz] = Mdvx::VERT_TYPE_SURFACE;
    vhdr.level[iz] = fhdr.grid_minz + iz * fhdr.grid_dz;
  }

  //
  // Create data, all missing.
  //
  fl32 *data = new float [fhdr.nx*fhdr.ny*fhdr.nz];
  if (data == NULL ){
    cerr << "New failed." << endl;
    exit(1);
  }
 
  for (int i = 0 ; i < fhdr.nx*fhdr.ny*fhdr.nz; i++)
    data[i] = fhdr.missing_data_value;

  // create field
 
  MdvxField *field = new MdvxField(fhdr, vhdr, data);
 
  // add field to mdvx object
 
  _Mdv->addField(field);

  delete[] data;

  return 0;

} 






