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

#include <euclid/Grid.hh>
#include <Mdv/mdv/MdvFile.hh>
#include <Mdv/mdv/MdvField.hh>
#include <toolsa/str.h>
using namespace std;

MdvField::MdvField()
{
   data   = NULL;
   grid   = NULL;
   dataTime = DateTime::NEVER;
   sourceFieldNum = -1;;
   sourceURL = NULL;
   planeNum = GridGeom::UNKNOWN_SIZE;
   integralScaling = false;

   clearBoundingBox();
   _isSearchZRequest = false;
   initHdr();
}

MdvField::MdvField( const string & name, const Grid &g )
{
   data   = NULL;
   grid   = NULL;
   dataTime = DateTime::NEVER;
   sourceFieldNum = -1;;
   sourceURL = NULL;
   planeNum = GridGeom::UNKNOWN_SIZE;
   integralScaling = false;

   clearBoundingBox();
   _isSearchZRequest = false;
   initHdr();
   setData( g );
   setName( name );
}


MdvField::MdvField( const MdvField &source )
{
   data   = NULL;
   grid   = NULL;
   dataTime = DateTime::NEVER;
   sourceFieldNum = -1;;
   sourceURL = NULL;
   planeNum = GridGeom::UNKNOWN_SIZE;
   integralScaling = false;

   copy( source );
}

MdvField::~MdvField()
{
   //
   // We don't own the grid, we just get to look at it
   //
   if ( data )
      delete [] data;

   if (sourceURL) 
      delete sourceURL;
}

MdvField& 
MdvField::operator= ( const MdvField &source )
{
   copy( source );
   return *this;
}

void 
MdvField::initHdr()
{
   MDV_init_field_header( &info );

   info.field_code           = -1;
   info.encoding_type        = MDV_INT8;
   info.data_element_nbytes  = 1;
   info.field_data_offset    = -1;
   info.volume_size          = 0;

   info.forecast_time        = MdvFile::NEVER;
   info.forecast_delta       = MdvFile::NEVER;

   info.nx                   = 0;
   info.ny                   = 0;
   info.nz                   = 0;

   info.grid_dx              = 0.0;
   info.grid_dy              = 0.0;
   info.grid_dz              = 0.0;

   info.grid_minx            = 0.0;
   info.grid_miny            = 0.0;
   info.grid_minz            = 0.0;

   info.scale                = 1;
   info.bias                 = 0.0;
   info.bad_data_value       = 0.0;
   info.missing_data_value   = 0.0;

   info.proj_type            = MDV_PROJ_FLAT;
   info.proj_origin_lat      = 0.0;
   info.proj_origin_lon      = 0.0;
   info.proj_rotation        = 0.0;
   info.vert_reference       = 0.0;

   info.field_name[0]        = '\0';
   info.field_name_long[0]   = '\0';
   info.units[0]             = '\0';
   info.transform[0]         = '\0';
}

void 
MdvField::copy( const MdvField &source )
{

   info = source.info;

/*
   //
   // Copy the header
   // NOTE: user_ and unused_ fields are not copied!
   //
   info.field_code           = source.info.field_code;
   info.encoding_type        = source.info.encoding_type;
   info.data_element_nbytes  = source.info.data_element_nbytes;
   info.field_data_offset    = source.info.field_data_offset;
   info.volume_size          = source.info.volume_size;

   info.forecast_time        = source.info.forecast_time;
   info.forecast_delta       = source.info.forecast_delta;

   info.nx                   = source.info.nx;
   info.ny                   = source.info.ny;
   info.nz                   = source.info.nz;

   info.grid_dx              = source.info.grid_dx;
   info.grid_dy              = source.info.grid_dy;
   info.grid_dz              = source.info.grid_dz;

   info.grid_minx            = source.info.grid_minx;
   info.grid_miny            = source.info.grid_miny;
   info.grid_minz            = source.info.grid_minz;

   info.scale                = source.info.scale;
   info.bias                 = source.info.bias;
   info.bad_data_value       = source.info.bad_data_value;
   info.missing_data_value   = source.info.missing_data_value;

   info.proj_type            = source.info.proj_type;
   info.proj_origin_lat      = source.info.proj_origin_lat;
   info.proj_origin_lon      = source.info.proj_origin_lon;
   info.proj_rotation        = source.info.proj_rotation;
   info.vert_reference       = source.info.vert_reference;

   memcpy( info.proj_param, source.info.proj_param,
           sizeof(fl32)*MDV_MAX_PROJ_PARAMS );

   setDescription( source.info.field_name, source.info.field_name_long,
                   source.info.units, source.info.transform );
*/

   //
   // Copy the grid/data
   //
   dataTime = source.dataTime;
   if ( source.grid ) {
	  if (grid) {
         // Skip the assignment. Don't know the instantiation type.
	     // *grid = *source.grid;
      } 
      else {
         setData( *(source.grid) );
      }
   }

   _isBoundingBoxRequest = source._isBoundingBoxRequest;
   minLat = source.minLat;
   minLon = source.minLon;
   maxLat = source.maxLat;
   maxLon = source.maxLon;

   _isSearchZRequest = source._isSearchZRequest;

   planeNum = source.planeNum;
   integralScaling = source.integralScaling;
   
   sourceFieldNum = source.sourceFieldNum;
   setSourceURL(source.getSourceURL());
}

void MdvField::setInfo( const MDV_field_header_t & header )
{
   info = header;
}

void MdvField::clearBoundingBox()
{
   _isBoundingBoxRequest = false;
   minLat = 0.0;
   minLon = 0.0;
   maxLat = 0.0;
   maxLon = 0.0;
}

void MdvField::setBoundingBox(double newMinLat, double newMinLon,
                                 double newMaxLat, double newMaxLon)
{
   _isBoundingBoxRequest = true;
   minLat = newMinLat;
   minLon = newMinLon;
   maxLat = newMaxLat;
   maxLon = newMaxLon;
}

void MdvField::getBoundingBox(double & outMinLat, double & outMinLon,
                                 double & outMaxLat, double & outMaxLon)
{
   outMinLat = minLat;
   outMinLon = minLon;
   outMaxLat = maxLat;
   outMaxLon = maxLon;
}

// Does the field have special geometry that can only be specified
//   by sending the whole MDV_field_header_t to the server:
//       o Any projection info.
//       o Complete x or y info.
//       o More than one z.
// 
bool MdvField::hasSpecialQueryGeometry() const
{
    if (info.proj_origin_lat != Projection::UNKNOWN_ORIGIN ||
        info.proj_origin_lon != Projection::UNKNOWN_ORIGIN ||
        info.proj_rotation != Projection::UNKNOWN_ROTATION)
    {
        return true;
    }

    if (info.nx != (int) GridGeom::UNKNOWN_SIZE ||
        info.ny != (int) GridGeom::UNKNOWN_SIZE ||
        info.grid_dx != GridGeom::UNKNOWN_RESOLUTION ||
        info.grid_dy != GridGeom::UNKNOWN_RESOLUTION ||
        info.grid_minx != GridGeom::UNKNOWN_LOWER_LEFT ||
        info.grid_miny != GridGeom::UNKNOWN_LOWER_LEFT)
    {
        return true;
    }

    // Todo: Fix unsigned/int comparison.
    if (info.nz != (int) GridGeom::UNKNOWN_SIZE && info.nz > 1) {
        return true;
    }

    return false;
}

bool MdvField::areGeometriesEqual(const MdvField & other) const
{
   if (grid == NULL || other.grid == NULL) {
      return false;
   }

   if (!grid->areGeometriesEqual(*other.getGrid())) {
      return false;
   }

   if (_isBoundingBoxRequest != other._isBoundingBoxRequest ||
       minLat                != other.minLat ||
       minLon                != other.minLon ||
       maxLat                != other.maxLat ||
       maxLon                != other.maxLon ||
       _isSearchZRequest     != other._isSearchZRequest ||
       planeNum              != other.planeNum)
   {
      return false;
   }

   return true;
}

// // DsMdvSocket access for converting message to a field.
// // 
// int MdvField::setFromMsg( const DsURL & url, int fieldNum,
//                              const DateTime & time,
//                              const MDV_field_header_t & header,
//                              CompositeType compositeType,
//                              int dataLen, const unsigned char * /* data */,
//                              const MDV_vlevel_header_t * vHeader)
// {
//    if (grid == NULL) {
//       // This is a major problem.
//       return -1;
//    }
// 
//    setSourceURL(&url);
//    sourceFieldNum = fieldNum;
//    setDateTime(time);
// 
//    // Copy the name to the grid.
//    setName(header.field_name);
// 
//    // Set the composite type.
//    // Todo: make this more than a boolean.
//    if (compositeType == MDVP_COMPOSITE_NONE) {
//       grid->setComposite(false);
//    }
//    else if (compositeType == MDVP_COMPOSITE_MAX) {
//       grid->setComposite(true);
//    }
//    else {
//       cerr << "Error: Unknown composite type!" << endl;
//       return -1;
//    }
// 
//    // Set the init value on the grid to the missing_data_value from server.
//    // 
//    // Todo: Is this correct? This may be the char value that means invalid?
//    // 
//    grid->setInitValue(header.missing_data_value);
// 
//    // Copy the info into this object's data member.
//    info = header;
// 
//    // Make sure that data is big enough to hold the expected values.
//    calcVolumeSize();
//    if (info.volume_size > dataLen) {
//       cerr << "Error: Not enough data for expected values." << endl;
//       return -1;
//    }
// 
//    // Copy the data into the grid.
//    // 
//    //   If the request grid is an AdaptGrid object, a request was made 
//    //     to the server in which geometry may have been only partially
//    //     specified. The server will respond as follows:
//    // 
//    //       o If a BoundingBox request was made, the server will return
//    //           a grid with native geometry clipped with the bounding box.
//    //           All other geometry is ignored in a BoundingBox request, 
//    //           as the server is responsible for determining the geometry.
//    // 
//    //       o Any specified portions have not changed, except possibly 
//    //           the Z parameters. If the query included a single Z level
//    //           and it was unavailable, the server will find the closest.
//    // 
//    //       o Any unspecified portions have been filled in by the server,
//    //           in the course of getting data from the mdv file.
//    // 
//    //       For AdaptGrid requests, simply replace the geometry in the 
//    //         grid with the returned geometry, then copy in the data.
//    //         Clients using AdaptGrids
//    // 
//    //   If the request grid is not an AdaptGrid object, the returned geometry
//    //     must match that of the request exactly.
//    //   
//    // if (grid->isA(Grid::ADAPTGRID)) {
//    if (0) {
//       // AdaptGrid * ag = (AdaptGrid *) grid;
// // 
//       // // AdaptGrid request may have geometry changes from server.
//       // //   Use the returned geometry without any checks.
//       // // 
//       // ag->setGeometry(info.nx, info.ny, info.nz,
//       //                 info.grid_dx, info.grid_dy, info.grid_dz,
//       //                 info.grid_minx, info.grid_minx, info.grid_minz,
//       //                 info.proj_origin_lat, info.proj_origin_lon,
//       //                 info.proj_rotation);
// 
//       // Set the grid's data with the incoming char[] array.
//       // ag->setData(data, dataLen);
//    }
//    else {
//       // This is a fixed-size grid.
//       //   Make sure returned geometry matches request geometry.
//       // 
//       double dTol = 0.000001;
//       if ( grid->getNx() != (size_t) info.nx ||
//            grid->getNy() != (size_t) info.ny ||
//            grid->getNz() != (size_t) info.nz ||
//            grid->getDx() != info.grid_dx ||
//            grid->getDy() != info.grid_dy ||
//            grid->getDz() != info.grid_dz ||
//            grid->getMinx() != info.grid_minx ||
//            grid->getMiny() != info.grid_miny ||
//            grid->getMinz() != info.grid_minz ||
//            (fabs(grid->getLatOriginFl32() - info.proj_origin_lat) > dTol) ||
//            (fabs(grid->getLonOriginFl32() - info.proj_origin_lon) > dTol) ||
//            (fabs(grid->getRotationFl32() - info.proj_rotation) > dTol) ) {
// 
//          // Dimensions returned from the server do not match request.
//          return -1;
//       }
//    }
//    
//    if (vHeader != NULL) {
//       // Todo: Deal with the vlevel stuff.
//    }
// 
//    return 0;
// }

void 
MdvField::setDescription( const char *name, const char *desc,
                          const char *units, const char *transform )
{
   if ( name )
      STRncopy( info.field_name,      name,      MDV_SHORT_FIELD_LEN );

   if ( desc )
      STRncopy( info.field_name_long, desc,      MDV_LONG_FIELD_LEN );

   if ( units )
      STRncopy( info.units,           units,     MDV_UNITS_LEN );

   if ( transform )
      STRncopy( info.transform,       transform, MDV_TRANSFORM_LEN );
}

void
MdvField::setData( const Grid &g )
{
   double lat, lon, rotation;

   // 
   // Set the encoding type.
   //   Todo: Add method to Grid, which gets corresponding mdv type.
   // 
   int encodingType        = 0;
   int dataElementNbytes   = 0;

   switch (g.getDataType()) {
     case Grid::CHAR_GRID:                                                   
          encodingType = MDV_INT8;
          dataElementNbytes = 1;
          break;                                                                
                                                                                
     case Grid::SHORT_GRID:                                                   
          encodingType = MDV_INT16;
          dataElementNbytes = 2;
          break;                                                                

     case Grid::FLOAT_GRID:
          encodingType = MDV_FLOAT32;
          dataElementNbytes = 4;
          break;

   default: {}

   }
   //
   // I commented this out - it is making for huge log files. Niles.
   //
   // if (info.encoding_type != 0 && info.encoding_type != encodingType) {
   //  cerr << "WARNING: MdvField::setData(...) is changing the header "
   //       << "encoding_type from: " << info.encoding_type << " to: "
   //       << encodingType << " on field: " << getName() << endl;
   // }
   info.encoding_type = encodingType;
   info.data_element_nbytes = dataElementNbytes;

   //                                                                         
   // Update the header info based on grid characteristics
   //
   grid = (Grid *)&g;
   grid->getOrigin( &lat, &lon, &rotation );
   setGeometry( grid->getNx(), grid->getNy(), grid->getNz(),
                grid->getDx(), grid->getDy(), grid->getDz(),
                grid->getMinx(), grid->getMiny(), grid->getMinz(),
                lat, lon, rotation );
   
}

// Should we allow this? Might be confusing to users of the class.
//   ( This geometry goes away when do updateGeometry() )
// 
void
MdvField::setGeometry( size_t nx, size_t ny, size_t nz,
                       float dx, float dy, float dz,
                       float minx, float miny, float minz,
                       double lat, double lon, double rotation )
{
   info.nx              = (si32)nx;
   info.ny              = (si32)ny;
   info.nz              = (si32)nz;
   info.grid_dx         = (fl32)dx;
   info.grid_dy         = (fl32)dy;
   info.grid_dz         = (fl32)dz;
   info.grid_minx       = (fl32)minx;
   info.grid_miny       = (fl32)miny;
   info.grid_minz       = (fl32)minz;

   calcVolumeSize();

   //
   // Make sure we use the correct data type for the UNKNOWN values
   // In the Projection class, members are doubles (for historical reasons)
   // In the info mdv_field_hdr_t these members are fl32
   //
   if ( Projection::isKnown( lat ))
      info.proj_origin_lat = (fl32)lat;
   else
      info.proj_origin_lat = Projection::UNKNOWN_ORIGIN_FL32;

   if ( Projection::isKnown( lon ))
      info.proj_origin_lon = (fl32)lon;
   else
      info.proj_origin_lon = Projection::UNKNOWN_ORIGIN_FL32;

   if ( Projection::isKnown( rotation ))
      info.proj_rotation   = (fl32)rotation;
   else
      info.proj_rotation   = Projection::UNKNOWN_ROTATION_FL32;
}

void
MdvField::updateGeometry()
{
   //
   // Degenerate case
   //
   if ( !grid )
      return;

   //
   // Get the latest geometry from the field's grid
   //
   info.nx              = (si32)grid->getNx();
   info.ny              = (si32)grid->getNy();
   info.nz              = (si32)grid->getNz();
   info.grid_dx         = (fl32)grid->getDx();
   info.grid_dy         = (fl32)grid->getDy();
   info.grid_dz         = (fl32)grid->getDz();
   info.grid_minx       = (fl32)grid->getMinx();
   info.grid_miny       = (fl32)grid->getMiny();
   info.grid_minz       = (fl32)grid->getMinz();
   info.proj_origin_lat = (fl32) grid->getLatOrigin();
   info.proj_origin_lon = (fl32) grid->getLonOrigin();
   info.proj_rotation   = (fl32) grid->getRotation();
   info.proj_type       = Projection::lookupMdvFileProjId(grid->projectionType());

   info.bad_data_value = grid->getFloatBadValue();;
   info.missing_data_value = grid->getFloatMissingValue();;

   calcVolumeSize();
}

void *
MdvField::getUnscaledData()
{
   if ( !grid ) {
      return NULL;
   }

   return grid->getUnscaledData();
}

unsigned char*
MdvField::getScaledData()
{
   // Initialize valiables with values already in the header.
   // 
   //   Scale and bias get reset when calling Grid::getCharData(...)
   //   unless the instantiated grid type is <unsigned char>.
   //   
   float scale        = info.scale,
         bias         = info.bias,
         badValue     = info.bad_data_value,
         missingValue = info.missing_data_value;

   //
   // Degenerate case
   //
   if ( !grid ) {
      return NULL;
   }

   //
   // Remove the old data
   //
   if ( data ) {
      delete[] data;
   }

   //
   // Scale the grid data
   //   Use the value 0 as the bad data value in the char array.
   //
   unsigned char badOutputChar = 0;
   unsigned char missingOutputChar = 0;
   data = grid->getCharData( &scale, &bias, 
                             badOutputChar, missingOutputChar, 
                             integralScaling );

   //
   // Update the header info
   //
   info.scale                = (fl32)scale;
   info.bias                 = (fl32)bias;
   info.bad_data_value       = (fl32)badValue;
   info.missing_data_value   = (fl32)missingValue;

   return( data );
}

void
MdvField::calcVolumeSize()
{
   if ( grid  &&  grid->isKnown((size_t)info.nx)  &&
                  grid->isKnown((size_t)info.ny)  &&
                  grid->isKnown((size_t)info.nz) ) {
      info.volume_size = info.nx * info.ny * info.nz * info.data_element_nbytes;
   }
   else {
      info.volume_size = 0;
   }
}

int MdvField::getDataSize() const
{
   if (grid == NULL || !grid->isGeometryKnown()) {
      return 0;
   }
   
   return (grid->getNx() * grid->getNy() * grid->getNz());
}

