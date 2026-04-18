////////////////////////////////////////////////////////////////////////////////
//
//  Working class for HailKE application
//
//  Terri L. Betancourt, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//  October 2001
//
////////////////////////////////////////////////////////////////////////////////
#ifndef _HAILKE_DATAMGR_INC_
#define _HAILKE_DATAMGR_INC_

#include <vector>
#include <Mdv/DsMdvx.hh>
using namespace std;

//
// Forward class declarations
//
class Params;


class DataMgr
{
public:
   DataMgr();
  ~DataMgr();

   //
   // Processing steps invoked by application Driver
   // Return 0 upon success, -1 upon failure
   //
   int              init( Params &params );
   int              convert2kineticEnergy();
   int              writeOutput();

/*------------------------------------------------------------------
 * Temporarily use old DsInputPath until we get ahold of libs/dsdata
 *------------------------------------------------------------------
   int              loadInputData( const DateTime& issueTime );
--------------------------------------------------------------------*/
   int              loadInputPath( const char* inputPath );


   //
   // Static constants
   //
   const static float undefinedValue;
   const static char* hailMassFieldName;
   const static char* hailMassUnits;
   const static char* hailKeFieldName;
   const static char* hailKeUnits;

private:

   //
   // User-defined thresholds
   //
   float            referenceLevel;  // H(r) in Km.
   float            upperThreshold;  // dBZ at H(r)
   float            lowerThreshold;  // dBZ at H(min)

   //
   // Coefficients (a) and exponents (b)
   // for Z=aM**b  and  Z=aE**b relationships
   //
   float            massA;
   float            massB;
   float            keA;
   float            keB;

   //
   // Input reflectivity data
   //
   char            *radarUrl;
   DsMdvx           radarMdv;
   DateTime         radarDataTime;

   MdvxField       *dbzField;
   char            *dbzFieldName;

   //
   // Output data
   //
   int              numValuesPerPlane;
   float           *massData, *keData;

   char            *hailUrl;
   DsMdvx           hailMdv;

};

#endif
