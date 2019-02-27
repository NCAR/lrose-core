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

#include <string>

#include <toolsa/str.h>
#include <toolsa/mem.h>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxGrid.hh>
#include <cassert>
using namespace std;

int
MdvxGrid::setMdvxReadFromGrid( DsMdvx& mdvx, const Grid& grid )
{
   Projection projection = grid.getProjection();

   switch ( projection.getType() ) {
      case Projection::FLAT :
           mdvx.setReadRemapFlat( (int)grid.getNx(), 
                                  (int)grid.getNy(),
                                  (double)grid.getMinx(), 
                                  (double)grid.getMiny(),
                                  (double)grid.getDx(), 
                                  (double)grid.getDy(),
                                  (double)grid.getLatOrigin(), 
                                  (double)grid.getLonOrigin(),
                                  (double)grid.getRotation() );
           break;
      case Projection::LAMBERT :
           //
           // How do we get the LcLat1 and LcLat2 from the grid???
           //
           return( -1 );

           /*---------------------------------------------------
           mdvx.setReadRemapLc2( (int)grid.getNx(), 
                                 (int)grid.getNy(),
                                 (double)grid.getMinx(), 
                                 (double)grid.getMiny(),
                                 (double)grid.getDx(), 
                                 (double)grid.getDy(),
                                 (double)grid.getLatOrigin(), 
                                 (double)grid.getLonOrigin(),
                                 (double)grid.getLcLat1(),
                                 (double)grid.getLcLat2() );
           ----------------------------------------------------*/
           break;
      case Projection::LATLON :
           mdvx.setReadRemapLatlon( (int)grid.getNx(), 
                                    (int)grid.getNy(),
                                    (double)grid.getMinx(), 
                                    (double)grid.getMiny(),
                                    (double)grid.getDx(), 
                                    (double)grid.getDy() );
           break;
      default:
           return( -1 );
           break;
   }

   double minVlevel = (double)grid.getMinz();
   double maxVlevel = (double)( grid.getMinz() + grid.getNz() * grid.getDz() );
   mdvx.setReadVlevelLimits( minVlevel, maxVlevel );

   //
   // Set read compositing to the match the grid compositing
   //
   if ( grid.isComposite() ) {
      mdvx.setReadComposite();
   }

   //
   // Set read encoding to the match the grid type
   //
   int gridType = grid.getDataType();
   switch( gridType ) {
      case Grid::CHAR_GRID:
           mdvx.setReadEncodingType( Mdvx::ENCODING_INT8 );
           break;
      case Grid::SHORT_GRID:
           mdvx.setReadEncodingType( Mdvx::ENCODING_INT16 );
           break;
      case Grid::INT_GRID:
           //
           // Unsupported
           //
           return( -1 );
           break;
      case Grid::FLOAT_GRID:
           mdvx.setReadEncodingType( Mdvx::ENCODING_FLOAT32 );
           break;
      case Grid::DOUBLE_GRID:
           //
           // Unsupported
           //
           return( -1 );
           break;
   }

   //
   // Uncompress on the read
   //
   mdvx.setReadCompressionType( Mdvx::COMPRESSION_NONE );
   return( 0 );
}


//
// Overloaded function to use field name rather than number. Niles.
//
int
MdvxGrid::setGridDataFromMdvxField( Grid& grid, 
                                    const DsMdvx& mdvx,
                                    const string& fieldName )
{
   //
   // Make sure the named field exists
   //
   MdvxField *field = mdvx.getFieldByName( fieldName );
   if ( field == NULL ) {
      return( -1 );
   }

   return setGridDataFromMdvxField(grid, *field);
}


int
MdvxGrid::setGridDataFromMdvxField( Grid& grid, 
                                    const DsMdvx& mdvx,
                                    const int& fieldNum )
{
   //
   // Make sure the named field exists
   //
   MdvxField *field = mdvx.getFieldByNum( fieldNum );
   if ( field == NULL ) {
      return( -1 );
   }

   return setGridDataFromMdvxField(grid, *field);
}

//
// Main function uses field reference.
//
int
MdvxGrid::setGridDataFromMdvxField( Grid& grid, 
				    const MdvxField& field )
{
   int status = 0;

   //
   // Fill out any unknown geometry on the grid from the mdv Field
   //
   const Mdvx::field_header_t &fieldHdr = field.getFieldHeader();
   GridGeom geom = grid.getGeometry();

   if ( !geom.isGeometryKnown() ) {
      Projection::ProjId  projType;
      switch ( fieldHdr.proj_type ) {
         case Mdvx::PROJ_FLAT :
              projType = Projection::FLAT;
              break;
         case Mdvx::PROJ_LAMBERT_CONF :
              //
              // This projection doesn't seem to be fully implemented
              // in libs/euclid/projection???
              //
              return( -1 );
              break;
         case Mdvx::PROJ_LATLON :
              projType = Projection::LATLON;
              break;
         default:
              return( -1 );
              break;
      }

      geom.suggest( (size_t)fieldHdr.nx,
                    (size_t)fieldHdr.ny,
                    (size_t)fieldHdr.nz,
                    (float)fieldHdr.grid_dx,
                    (float)fieldHdr.grid_dy,
                    (float)fieldHdr.grid_dz,
                    (float)fieldHdr.grid_minx,
                    (float)fieldHdr.grid_miny,
                    (float)fieldHdr.grid_minz,
                    (double)fieldHdr.proj_origin_lat, 
                    (double)fieldHdr.proj_origin_lon, 
                    projType,
                    (double)fieldHdr.proj_rotation );
      grid.suggestGeometry( geom );
   }

   //
   // By now the grid and field geometry must match
   //
   int numBytesEl = grid.getDataElementNbytes();
   assert (numBytesEl != 0);

//   if ( numBytesEl != 0){
    
       assert( (int) (geom.getNx() * geom.getNy() * geom.getNz()) ==
           (int) (field.getVolLen() / numBytesEl) );
 //  }
   //
   // Set the grid data from a field of the same datatype
   // Because Grid::setFromTArray does not apply the fuzzy function
   // we have to do it ourselves
   //
   status = grid.setFromVoidArray( field.getVol(), geom,
                                   fieldHdr.bad_data_value,
                                   fieldHdr.missing_data_value );
   if ( status == 0 ) {
      grid.applyFuzzyFcn();
   }

   return( status );
}

int
MdvxGrid::addMdvxFieldFromGrid( 
             DsMdvx & mdvx,
             const string & fieldName,
             const Grid & grid, 
             Mdvx::encoding_type_t encoding       /* = Mdvx::ENCODING_INT8*/,
             Mdvx::compression_type_t compression /* = Mdvx::COMPRESSION_GZIP*/,
             Mdvx::scaling_type_t scaling         /* = Mdvx::SCALING_DYNAMIC*/,
             const double scale                   /* = 1.0*/,
             const double bias                    /* = 0.0 */ )
{
   Mdvx::scaling_type_t outputScaling = scaling;
   
   int nbytes = grid.getDataElementNbytes();

   //
   // Set the field header from the grid
   //
   Mdvx::field_header_t   fieldHdr;
   MEM_zero( fieldHdr );

   fieldHdr.grid_minx           = grid.getMinx();
   fieldHdr.grid_miny           = grid.getMiny();
   fieldHdr.grid_minz           = grid.getMinz();
   fieldHdr.grid_dx             = grid.getDx();
   fieldHdr.grid_dy             = grid.getDy();
   fieldHdr.grid_dz             = grid.getDz();
   fieldHdr.nx                  = grid.getNx();
   fieldHdr.ny                  = grid.getNy();
   fieldHdr.nz                  = grid.getNz();
   fieldHdr.volume_size         = grid.getNumValues() * nbytes;
   fieldHdr.data_element_nbytes = nbytes;
   if (nbytes == 4) {
     fieldHdr.encoding_type = Mdvx::ENCODING_FLOAT32;
   } else if (nbytes == 2) {
     fieldHdr.encoding_type = Mdvx::ENCODING_INT16;
   } else {
     fieldHdr.encoding_type = Mdvx::ENCODING_INT8;
   }
   fieldHdr.bad_data_value      = grid.getFloatBadValue();
   fieldHdr.missing_data_value  = grid.getFloatMissingValue();

   STRncopy( fieldHdr.field_name, fieldName.c_str(), MDV_SHORT_FIELD_LEN );

   //
   // Convert from grid-projection type to mdvx-projection type
   //
   Projection projection = grid.getProjection();
   switch ( projection.getType() ) {
      case Projection::FLAT :
           fieldHdr.proj_type = Mdvx::PROJ_FLAT;
           break;
      case Projection::LAMBERT :
           //
           // This projection doesn't seem to be fully implemented
           // in libs/euclid/projection???
           //
           //fieldHdr.proj_type = Mdvx::PROJ_LAMBERT_CONF;
           //fieldHdr.proj_param[0] = projection.getLC2Lat1();
           //fieldHdr.proj_param[0] = projection.getLC2Lat2();
           // 
           return( -1 );
           break;
      case Projection::LATLON :
           fieldHdr.proj_type = Mdvx::PROJ_LATLON;
           break;
      default:
           return( -1 );
           break;
   }
   fieldHdr.proj_origin_lat = (fl32) projection.getLatOrigin();
   fieldHdr.proj_origin_lon = (fl32) projection.getLonOrigin();
   fieldHdr.proj_rotation   = (fl32) projection.getRotation();

   //
   // Set scale and bias if necessary
   //
   int outputElementSize = Mdvx::dataElementSize( encoding );
   if( nbytes == outputElementSize ) {
      fieldHdr.scale = 1.0;
      fieldHdr.bias  = 0.0;
      outputScaling  = Mdvx::SCALING_SPECIFIED;
   }

   //
   // For now, we aren't going to worry about vlevels since
   // the grid class doesn't have a good interface for vlevels
   // When we add this capability, don't forget to set the
   // masterHeader.vlevel_included
   //
   Mdvx::vlevel_header_t  vlevelHdr;
   MEM_zero( vlevelHdr );

   //
   // Add the field-related info to the mdvx dataset
   // Mdvx takes ownership of the field which must be 'newed'
   //
   MdvxField *field = new MdvxField( fieldHdr, vlevelHdr, 
                                     grid.getVoidData() );

   //
   // Set the encoding, compression, and scaling
   //
   if ( field->convertType( encoding, compression, 
                            outputScaling, scale, bias ) != 0 ) {
      return( -1 );
   }

   mdvx.addField( field );
   return( 0 );
}
