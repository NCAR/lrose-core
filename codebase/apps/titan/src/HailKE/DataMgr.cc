////////////////////////////////////////////////////////////////////////////////
//
//  Working class for HailKE application
//
//  Terri L. Betancourt, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//  October 2001
//
////////////////////////////////////////////////////////////////////////////////

#include <cassert>
#include <iomanip>
#include <toolsa/str.h>
#include <toolsa/pmu.h>
#include <toolsa/DateTime.hh>
#include <didss/DsInputPath.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/Mdvx_enums.hh>

#include "Driver.hh"
#include "DataMgr.hh"
#include "Params.hh"
using namespace std;

//
// Static definitions
//
const float DataMgr::undefinedValue   = FLT_MIN;

const char* DataMgr::hailMassFieldName  = "Hail Mass Flux";
const char* DataMgr::hailMassUnits      = "g/m2s";

const char* DataMgr::hailKeFieldName  = "Hail KE Flux";
const char* DataMgr::hailKeUnits      = "J/m2s";

DataMgr::DataMgr()
{
   massData          = NULL;
   keData            = NULL;
   numValuesPerPlane = 0;
   referenceLevel    = -1;
   lowerThreshold    = FLT_MIN;
   upperThreshold    = FLT_MIN;
}

DataMgr::~DataMgr()
{
}

int
DataMgr::init( Params &params )
{
   //
   // Initialize the input radar data constraints
   //
   radarUrl     = params.radar_url;
   dbzFieldName = params.dbz_field_name;

   referenceLevel = params.freezing_level + 
                    params.reference_height_above_freezing;
   upperThreshold = params.reference_height_threshold;
   lowerThreshold = params.base_height_threshold;

   massA = params.ZM_coefficients.a;
   massB = params.ZM_coefficients.b;

   keA = params.ZE_coefficients.a;
   keB = params.ZE_coefficients.b;

   //
   // Set read characteristics of radar data   
   //
   radarMdv.addReadField( params.dbz_field_name );
   radarMdv.setReadEncodingType( Mdvx::ENCODING_FLOAT32 );
   radarMdv.setReadCompressionType( Mdvx::COMPRESSION_NONE );

   //
   // Initialize the output url
   //
   hailUrl = params.output_url;

   return( 0 );
}

/*------------------------------------------------------------------
 * Temporarily use old DsInputPath until we get ahold of libs/dsdata
 *------------------------------------------------------------------
int
DataMgr::loadInputData( const DateTime& issueTime )
{
   string inputFilePath;

   //
   // Clear out the old stuff
   //
   radarMdv.clearFields();
   radarMdv.clearChunks();

   //
   // Try to read the radar data
   //
   int searchMargin = 0;
   radarMdv.setReadTime( Mdvx::READ_FIRST_BEFORE,
                         radarUrl, searchMargin, issueTime.utime() );

   if ( radarMdv.readVolume() != 0 ) {
      if ( radarMdv.getNoFilesFoundOnRead() ) {
         POSTMSG( ERROR, "No data available at the trigger time." ); 
      }
      else {
         POSTMSG( ERROR, "Cannot load data at trigger time.\n%s",
                          radarMdv.getErrStr().c_str() );
      }
      return( -1 );
   }

   //
   // Got some new radar data
   //
   inputFilePath = radarMdv.getPathInUse();
   radarDataTime = DsInputPath::getDataTime( inputFilePath );
   POSTMSG( DEBUG, "Got data at %s", radarDataTime.dtime() );

   assert ( issueTime == radarDataTime );

   //
   // Fetch the refelectivity field data
   //
   dbzField = radarMdv.getFieldByName( dbzFieldName );

   if ( dbzField == NULL ) {
      POSTMSG( ERROR, "   Field name '%s' is unavailable in file '%s'",
                      dbzFieldName, inputFilePath.c_str() );
      return( -1 );
   }

   return( 0 );
}
--------------------------------------------------------------------*/

int
DataMgr::loadInputPath( const char* inputPath )
{
   string inputFilePath = inputPath;

   //
   // Clear out the old stuff
   //
   radarMdv.clearFields();
   radarMdv.clearChunks();

   //
   // Try to read the radar data
   //
   radarMdv.setReadPath( inputFilePath );

   if ( radarMdv.readVolume() != 0 ) {
      if ( radarMdv.getNoFilesFoundOnRead() ) {
         POSTMSG( ERROR, "No data available at the trigger time." ); 
      }
      else {
         POSTMSG( ERROR, "Cannot load data at trigger time.\n%s",
                          radarMdv.getErrStr().c_str() );
      }
      return( -1 );
   }

   //
   // Got some new radar data
   //
   radarDataTime = DsInputPath::getDataTime( inputFilePath );
   POSTMSG( DEBUG, "Got data at %s", radarDataTime.dtime() );

   //
   // Fetch the refelectivity field data
   //
   dbzField = radarMdv.getFieldByName( dbzFieldName );

   if ( dbzField == NULL ) {
      POSTMSG( ERROR, "   Field name '%s' is unavailable in file '%s'",
                      dbzFieldName, inputFilePath.c_str() );
      return( -1 );
   }

   return( 0 );
}

int
DataMgr::convert2kineticEnergy()
{
   bool    posted = false;
   int     i, upperPlane;
   float   Z, M, E;
   float   inverseMassB = 1.0/massB;
   float   inverseKeB = 1.0/keB;
   float  *lowerData, lowerDbz;
   float  *upperData, upperDbz;
   float   missingValue, badValue;

   //
   // Get a handle to the upper and lower dbz plane data
   //
   dbzField->setPlanePtrs();
   upperPlane = dbzField->computePlaneNum( (double)referenceLevel );
   upperData  = (float*)dbzField->getPlane( upperPlane );
   lowerData  = (float*)dbzField->getPlane( 0 );

   const Mdvx::field_header_t &dbzFieldHdr = dbzField->getFieldHeader();
   missingValue = dbzFieldHdr.missing_data_value;
   badValue     = dbzFieldHdr.bad_data_value;

   //
   // Allocate the data arrays for holding the results (one plane only)
   //
   numValuesPerPlane = dbzFieldHdr.nx * dbzFieldHdr.ny;
   massData          = new float[numValuesPerPlane];
   keData            = new float[numValuesPerPlane];

   //
   // Process each dbz value in the upper and lower levels
   // to determine the final kinetic energy result
   //
   for( i=0; i < numValuesPerPlane; ++i ) {

      //
      // Upper level: set missing/bad/thresholded values to undefined
      //
      upperDbz = upperData[i];
      if ( upperDbz == missingValue || upperDbz == badValue  ||
         ( upperDbz < upperThreshold )) {
         massData[i] = undefinedValue;
         keData[i]   = undefinedValue;
         continue;
      }

      //
      // Lower level: set missing/bad/thresholded values to undefined
      //
      lowerDbz = lowerData[i];
      if ( lowerDbz == missingValue || lowerDbz == badValue  ||
         ( lowerDbz < lowerThreshold )) {
         massData[i] = undefinedValue;
         keData[i]   = undefinedValue;
         continue;
      }

      //
      // Apply ZM and ZE relationships to the lower level reflectivity 
      // to get hail metrics at the ground
      //
      Z = (float) pow( 10.0, lowerDbz/10.0 );
      M = (float) pow( Z/massA, inverseMassB );
      E = (float) pow( Z/keA, inverseKeB );

      massData[i] = M;
      keData[i]   = E;

      if ( !posted ) {
         posted = true;
         POSTMSG( DEBUG, "...Areas of hail mass located" );
      }
   }
   return( 0 );
}

int
DataMgr::writeOutput()
{
   int     status = 0;
   char    buffer[1024];

   //
   // Master Header: Initialize from the input master header
   //                Add relevant changes
   //
   Mdvx::master_header_t masterHdr = radarMdv.getMasterHeader();

   sprintf( buffer, "%s", "Hail Metric:" );
   STRncopy( masterHdr.data_set_name, buffer, MDV_NAME_LEN );

   sprintf( buffer, "%s", radarMdv.getPathInUse().c_str() );
   STRncopy( masterHdr.data_set_source, buffer, MDV_NAME_LEN );

   sprintf( buffer, "ZM: (%4.1fM%4.1f)", massA, massB );
   STRncopy( masterHdr.data_set_info, buffer, MDV_INFO_LEN );

   masterHdr.time_gen = time( NULL );

   hailMdv.setMasterHeader( masterHdr );

   //
   // Clear out old mdv fields and chunks before adding the new ones
   //
   hailMdv.clearFields();
   hailMdv.clearChunks();

   //
   // Output Field Header: Initialize from the input reflectivity field header
   //                      Add relevant changes
   //
   const Mdvx::field_header_t &dbzFieldHdr = dbzField->getFieldHeader();
   Mdvx::field_header_t outputFieldHdr;
   outputFieldHdr = dbzFieldHdr;
   outputFieldHdr.missing_data_value = undefinedValue;
   outputFieldHdr.bad_data_value     = undefinedValue;
   outputFieldHdr.nz                 = 1;
   outputFieldHdr.volume_size        = numValuesPerPlane * sizeof(float);

   //
   // Output Vlevel Header: Create from scratch
   //
   Mdvx::vlevel_header_t hailVlevelHdr;
   hailVlevelHdr.type[0] = Mdvx::VERT_TYPE_Z;
   hailVlevelHdr.level[0] = outputFieldHdr.grid_minz;

   //
   // Mass Field: 
   //
   STRncopy( outputFieldHdr.field_name, hailMassFieldName, 
             MDV_SHORT_FIELD_LEN );
   STRncopy( outputFieldHdr.field_name_long, hailMassFieldName, 
             MDV_LONG_FIELD_LEN );
   STRncopy( outputFieldHdr.units, hailMassUnits,
             MDV_UNITS_LEN );

   MdvxField *hailMassField = new MdvxField( outputFieldHdr,
                                             hailVlevelHdr,
                                             massData );

   hailMassField->convertType( Mdvx::ENCODING_INT8, Mdvx::COMPRESSION_RLE );
   hailMdv.addField( hailMassField );

   //
   // Kinetic Energy Field:
   //
   STRncopy( outputFieldHdr.field_name, hailKeFieldName,
             MDV_SHORT_FIELD_LEN );
   STRncopy( outputFieldHdr.field_name_long, hailKeFieldName,
             MDV_LONG_FIELD_LEN );
   STRncopy( outputFieldHdr.units, hailKeUnits,
             MDV_UNITS_LEN );

   MdvxField *hailKeField = new MdvxField( outputFieldHdr,
                                           hailVlevelHdr,
                                           keData );

   hailKeField->convertType( Mdvx::ENCODING_INT8, Mdvx::COMPRESSION_RLE );
   hailMdv.addField( hailKeField );

   //
   // Do the write
   //
   POSTMSG( DEBUG, "Writing mdv output for %s",
                   DateTime::str( masterHdr.time_centroid ).c_str() );
   status = hailMdv.writeToDir( hailUrl );

   //
   // Free up the output data arrays
   //
   delete []massData;
   delete []keData;

   return( status );
}
