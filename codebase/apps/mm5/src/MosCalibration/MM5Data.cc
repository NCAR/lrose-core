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
////////////////////////////////////////////////////////////
//
// MM5 data for MOS calibration
//
// $Id: MM5Data.cc,v 1.43 2016/03/07 01:33:50 dixon Exp $
//
//////////////////////////////////////////////////////////////
#include <cfloat>
#include <cmath>
#include <cassert>
#include <dataport/port_types.h>
#include <Spdb/Spdb_typedefs.hh>
#include <rapformats/ComboPt.hh>
#include <rapformats/GenPt.hh>
#include <physics/physics.h>
#include <toolsa/DateTime.hh>

#include "MM5Data.hh"
#include "MM5Point.hh"
#include "DataMgr.hh"
#include "MosCalibration.hh"
using namespace std;

//
// Constants
//
const string MM5Data::RH       = "RH";
const string MM5Data::TEMP     = "Temp";
const string MM5Data::PRS      = "PRESSURE";
const string MM5Data::U        = "U";
const string MM5Data::V        = "V";
const string MM5Data::W        = "W";
const string MM5Data::CLW      = "CLW";
const string MM5Data::ICE      = "ICE";
const string MM5Data::RAIN_CON = "RAIN_CON";
const string MM5Data::RAIN_NON = "RAIN_NON";

const int debug = 1;   // debug level

MM5Data::MM5Data( DataServer& server, DataServer& rServer, 
                  double clwThresh, double icThresh,
                  vector< int >& fcastTimes ) 
    : dataServer( server ),
      rrServer( rServer ),
      forecastTimes( fcastTimes )
{
   cldLiqWaterThresh = clwThresh;
   iceContThresh     = icThresh;
   if (debug >= 1) {
     cout << "MM5Data.const: cldLiqWaterThresh: " << cldLiqWaterThresh
          << endl;
     cout << "MM5Data.const: iceContThresh: " << iceContThresh << endl;
   }
}

MM5Data::~MM5Data()
{
   clear();
}

void
MM5Data::clear() 
{
   map< time_t, MM5Point*, less< time_t > >::iterator it;
   
   for( it = dataPoints.begin(); it != dataPoints.end(); it++ ) {
      delete ((*it).second);
   }
   dataPoints.erase( dataPoints.begin(), dataPoints.end() );
}

int
MM5Data::createList( char* stationId, int leadTime ) 
{
   assert( stationId );
   //
   // Tell the server to get the data
   //
   if( dataServer.readData( stationId, leadTime ) != SUCCESS ) {
      POSTMSG( INFO, "Couldn't read model data for this time period" );
      return( 0 );
   }
   
   //
   // Get the chunks from the server
   //
   int nChunks = dataServer.getNChunks();
   if (debug >= 1) {
     cout << "MM5Data.createList.entry:" << endl;
     cout << "  stationId: " << stationId << endl;
     cout << "  leadTime: " << leadTime << endl;
     cout << "  dataServer startTime: " << utimstr(dataServer.getStartTime()) << endl;
     cout << "  dataServer endTime: " << utimstr(dataServer.getEndTime()) << endl;
     cout << "  dataServer url: " << dataServer.getUrl() << endl;
     cout << "  dataServer nChunks: " << nChunks << endl;
   }

   if( nChunks < 1 ) {
      POSTMSG( INFO, "No model data for this time period" );
      return( 0 );
   }
      
   vector<Spdb::chunk_t> chunks = dataServer.getChunks();

   //
   // Process each chunk
   //
   ComboPt comboPt;
   
   for( int i = 0; i < nChunks; i++ ) {
      if (debug >= 5)
        cout << "MM5Data.createList: start chunk: " << i << endl;

      comboPt.disassemble( chunks[i].data, chunks[i].len );

      const GenPt& singleLevelPt = comboPt.get1DPoint();
      const GenPt& multiLevelPt  = comboPt.get2DPoint();

      //
      // Get the point time
      //
      time_t pointTime = multiLevelPt.getTime();

      //
      // Set up a new MM5 data point
      //
      MM5Point* newPoint = new MM5Point( pointTime, 
                                         singleLevelPt.getLat(),
                                         singleLevelPt.getLon(),
					 leadTime);

      //
      //  Get the data
      //
      double rh    = get2DVal( RH, multiLevelPt, pointTime, 
                               leadTime, stationId );
      double temp  = get2DVal( TEMP, multiLevelPt, pointTime, 
                               leadTime, stationId );
      double prs   = get2DVal( PRS, multiLevelPt, pointTime, 
                               leadTime, stationId );
      double uComp = get2DVal( U, multiLevelPt, pointTime, 
                               leadTime, stationId );
      double vComp = get2DVal( V, multiLevelPt, pointTime, 
                               leadTime, stationId );

      if( rh != GenPt::missingVal )
         newPoint->setRH( rh );
      
      if( temp != GenPt::missingVal )
         newPoint->setTemp( temp );

      if( prs != GenPt::missingVal )
         newPoint->setPrs( prs );

      if( uComp != GenPt::missingVal )
         newPoint->setU( -1 * uComp );

      if( vComp != GenPt::missingVal )
         newPoint->setV( -1 * vComp );

      if( uComp != GenPt::missingVal && vComp != GenPt::missingVal ) {
         double wspd = sqrt( uComp*uComp + vComp*vComp );
         newPoint->setWspd( wspd );
      }

      //
      // Calculate the rain rate
      //
      newPoint->setRainRate( findRainRate( singleLevelPt, pointTime,
                                           leadTime, stationId ) );
      
      //
      // Find the height at which cloud liquid water
      // is greater than the threshold
      //
      int nLevels = multiLevelPt.getNLevels();
      int clNum   = multiLevelPt.getFieldNum( CLW );
      int pNum    = multiLevelPt.getFieldNum( PRS );

      double clwThreshHt = MM5Point::MISSING_VAL;
      double psfc        = MM5Point::MISSING_VAL;
      double pceil       = MM5Point::MISSING_VAL;

      if (debug >= 5) {
        cout << "MM5Data.createList: clNum: " << clNum
             << "  pNum: " << pNum << endl;
      }
      if( clNum != -1 && pNum != -1 ) {
               
         psfc = multiLevelPt.get2DVal( 0, pNum );
         if (debug >= 5) cout << "  psfc: " << psfc << endl;
         if( psfc == GenPt::missingVal )
            break;
         
         if (debug >= 5) cout << "  nLevels: " << nLevels << endl;
         for( int k = 0; k < nLevels; k++ ) {
            double clw = multiLevelPt.get2DVal( k, clNum );
            if (debug >= 10 && clw != 0)
              cout << "    k: " << k << "  clw: " << clw << endl;
            if( clw == GenPt::missingVal )
               continue;
            
            if( clw > cldLiqWaterThresh ) {
               pceil = multiLevelPt.get2DVal( k, pNum );
               if( pceil == GenPt::missingVal )
                  continue;
               
               clwThreshHt = mbToKm( pceil, psfc );
               if (debug >= 10) cout << "      accepted clwThreshHt: "
                 << clwThreshHt << endl;
               break;
            }
         }
      }

      newPoint->setClwht( clwThreshHt );

      //
      // Find the height at which the ice content is
      // greater than the threshold
      //
      int iceNum = multiLevelPt.getFieldNum( ICE );

      double ice       = MM5Point::MISSING_VAL;
      double iceContHt = MM5Point::MISSING_VAL;
      double pice      = MM5Point::MISSING_VAL;
      
      if( iceNum != -1 && pNum != -1 ) {

         for( int j = 0; j < nLevels; j++ ) {
            ice = multiLevelPt.get2DVal( j, iceNum );
            if( ice == GenPt::missingVal )
               continue;
            
            if( ice > iceContThresh ) {
               pice      = multiLevelPt.get2DVal( j, pNum );
               if( pice == GenPt::missingVal )
                  continue;
               
               iceContHt = mbToKm( pice, psfc );
               break;
            }
         }
      }

      newPoint->setIce( iceContHt );

      //
      // Find temperature differences
      //
      int p1000 = findLevelClosestToVal( PRS, multiLevelPt, 1000, 
                                         pointTime, leadTime, stationId );
      int p900  = findLevelClosestToVal( PRS, multiLevelPt, 900, 
                                         pointTime, leadTime, stationId );
      int p850  = findLevelClosestToVal( PRS, multiLevelPt, 850, 
                                         pointTime, leadTime, stationId );
      int p700  = findLevelClosestToVal( PRS, multiLevelPt, 700, 
                                         pointTime, leadTime, stationId );
      
      int tNum = multiLevelPt.getFieldNum( TEMP );
      
      if( tNum != -1 ) {
         if( p1000 != -1 && p850 != -1 ) {
            double temp1 = multiLevelPt.get2DVal( p1000, tNum );
            double temp2 = multiLevelPt.get2DVal( p850, tNum );
            newPoint->setTempDiff_1000_850mb( temp1 - temp2 );
         }
         if( p900 != -1 && p700 != -1 ) {
            double temp1 = multiLevelPt.get2DVal( p900, tNum );
            double temp2 = multiLevelPt.get2DVal( p700, tNum );
            newPoint->setTempDiff_900_700mb( temp1 - temp2 );
         }
      }
            

      //
      // Find the mean vertical velocity over the 1000 to 700mb range
      //
      int wNum = multiLevelPt.getFieldNum( W );
      
      if( wNum != -1 && p1000 != -1 && p700 != -1 && p1000 < p700 ) {

         int    count = 0;
         double wSum  = 0;

         for( int j = p1000; j < p700; j++ ) {
            wSum += multiLevelPt.get2DVal( j, wNum );
            count++;
         }
         newPoint->setAvgW( wSum / count );
      }
            
      //
      // Perform CAPE calculation.
      //
      double myRH;
      
      double *pS  = new double[nLevels];
      double *dpS = new double[nLevels];
      double *tS  = new double[nLevels];

      int rhNum   = multiLevelPt.getFieldNum( RH );

      if( tNum != -1 && rhNum != -1 ) {

         for( int i = 0; i < nLevels; i++ ) {
            tS[i]  = multiLevelPt.get2DVal( i, tNum );
            pS[i]  = multiLevelPt.get2DVal( i, pNum );
            myRH   = multiLevelPt.get2DVal( i, rhNum );

            if( tS[i] == GenPt::missingVal )
               dpS[i] = GenPt::missingVal;
            else
               dpS[i] = PHYrhdp( tS[i], myRH );
         }
      }

      double liftedIndex = findLifted( pS, tS, dpS, nLevels, 500 );
      
      newPoint->setLI( liftedIndex );

      delete[] pS;
      delete[] dpS;
      delete[] tS;

      //
      // Put the data point into the map
      //
      dataPoints[ pointTime ] = newPoint;
      
   }
   
   if (debug >= 1) {
     cout << "MM5Data.createList.exit: stationId: " << stationId
          << "  leadTime: " << leadTime 
          << "  num points: " << dataPoints.size() << endl;
   }
   return( (int) dataPoints.size() );
   
}

double
MM5Data::getLat() 
{
   if( dataPoints.size() > 0 )
      return( (dataPoints.begin())->second->getLat() );
   else
      return( MM5Point::MISSING_VAL );
}

double
MM5Data::getLon() 
{
   if( dataPoints.size() > 0 )
      return( (dataPoints.begin())->second->getLon() );
   else
      return( MM5Point::MISSING_VAL );
}

double 
MM5Data::mbToKm( double pressure, double psfc )
{
  double HtInKm = 8.5*log(psfc / pressure);
  return( HtInKm );
}

double 
MM5Data::get2DVal( const string& fieldName, const GenPt& multiLevelPt,
                   time_t dataTime, int fcastTime, char* id ) 
{
   DateTime when( dataTime );

   int fieldNum = multiLevelPt.getFieldNum( fieldName );
   if( fieldNum < 0 ) {
      POSTMSG( INFO, "Cannot find field %s at %s for lead time = %d "
                      "and station id = %s", 
               fieldName.c_str(), when.dtime(), fcastTime, id );
      return( MM5Point::MISSING_VAL );
   }
   return( multiLevelPt.get2DVal( 0, fieldNum ));
   
}

int
MM5Data::findLevelClosestToVal( const string& fieldName, const GenPt& multiLevelPt,
                                double value, time_t dataTime, int fcastTime,
                                char* id ) 
{
   DateTime when( dataTime );

   int fieldNum = multiLevelPt.getFieldNum( fieldName );
   if( fieldNum < 0 ) {
      POSTMSG( INFO, "Cannot find field %s at %s for lead time = %d "
                      "and station id = %s", 
               fieldName.c_str(), when.dtime(), fcastTime, id );
      return( -1 );
   }

   int nLevels        = multiLevelPt.getNLevels();
   int closestLevel   = 0;

   double currentDiff = DBL_MAX;
   double currentVal;
   
   for( int j = 0; j < nLevels; j++ ) {
      currentVal = multiLevelPt.get2DVal( j, fieldNum );
      if( fabs( currentVal - value ) < currentDiff ) {
         closestLevel = j;
         currentDiff = fabs( currentVal - value );
      }
   }
   
   return( closestLevel );
}

double
MM5Data::findRainRate( const GenPt& singleLevelPt, time_t dataTime,
                       int leadTime, char* stationId )
{
   DateTime when( dataTime );

   int    pLeadTime = -1;
   double rainRate  = 0.0;

   //
   // Find the previous lead time
   //
   if( leadTime == forecastTimes[0] )
      return( rainRate );
      
   for( int i = 1; i < (int) forecastTimes.size(); i++ ) {
      if( forecastTimes[i] == leadTime ) {
         pLeadTime = forecastTimes[i-1];
         break;
      }
   }

   if( pLeadTime != -1 ) {

      //
      // Find the current rain accumulation
      //
      double rainCon   = MM5Point::MISSING_VAL;
      double rainNon   = MM5Point::MISSING_VAL;
      double rainAccum = MM5Point::MISSING_VAL;

      int rcNum = singleLevelPt.getFieldNum( RAIN_CON );
      int rnNum = singleLevelPt.getFieldNum( RAIN_NON );
      
      if( rcNum != -1 && rnNum != -1 ) {
         
         rainCon   = singleLevelPt.get1DVal( rcNum );
         rainNon   = singleLevelPt.get1DVal( rnNum );
         rainAccum = rainCon + rainNon;

      }

      //
      // Get the data with the same generate time, but the
      // previous lead time.  
      //
      int status = rrServer.readExact( dataTime, stationId, pLeadTime );
      if( status != SUCCESS ) {
            
         POSTMSG( INFO, "Could not compute rain rate for %s at %s "
                  "for a lead time = %d", stationId, when.dtime(),
                  leadTime );
         return( 0.0 );

      }

      double pRainCon   = MM5Point::MISSING_VAL;
      double pRainNon   = MM5Point::MISSING_VAL;
      double pRainAccum = MM5Point::MISSING_VAL;

      vector<Spdb::chunk_t> rrChunks = rrServer.getChunks();

      if( rrServer.getNChunks() != 1 ) {
         return( 0 );
      }

      //
      // Get the single level point out of this data
      //
      ComboPt rrComboPt;

      rrComboPt.disassemble( rrChunks[0].data, rrChunks[0].len );

      const GenPt& rrSinglePt = rrComboPt.get1DPoint();

      //
      // Get the previous rain accumulation
      //
      rcNum = rrSinglePt.getFieldNum( RAIN_CON );
      rnNum = rrSinglePt.getFieldNum( RAIN_NON );

      if( rcNum != -1 && rnNum != -1 ) {

         pRainCon   = rrSinglePt.get1DVal( rcNum );
         pRainNon   = rrSinglePt.get1DVal( rnNum );
         pRainAccum = pRainCon + pRainNon;
      }
      
      //
      //    Calculate the rain rate
      //
      if( rainAccum != MM5Point::MISSING_VAL && 
          pRainAccum != MM5Point::MISSING_VAL && pLeadTime != -1 ) {
         rainRate = ( rainAccum - pRainAccum ) / 
            ( leadTime - pLeadTime );
      }
      
   }
   
   return( rainRate );
}

//////////////////////////////////////////////////////////////
//
//  reverseThetaE -  This routine returns 
//  the temerature of a parcel of air with 
//  a given eqiv. pot. temp (thte) lifted 
//  to level p.  Tguess is a first guess at
//  the value (or set=0, for routine determined 
//  first guess).  Tguess in KELVIN, return is 
//  in KELVIN.
//
//  Niles Oien
//
/////////////////////////////////////////////////////////////
double 
MM5Data::reverseThetaE( double thte, double p, double tguess )
{
  if ( thte == MM5Point::MISSING_VAL || 
       p == MM5Point::MISSING_VAL || 
       tguess == MM5Point::MISSING_VAL ) 
    return( MM5Point::MISSING_VAL );
        
  double tg = tguess;

  //
  // Determine first guess
  //
  if ( tg == 0.0 ) { 
    
    double tc = thte - 270.0;

    if( tc < 0.0 ) 
       tc = 0.0;

    tg = ( thte - 0.5*pow(tc,1.05 ) ) * pow( (p/1000.0),0.2 );

   }

   double tgnu = tg - 273.15;

   for ( int i = 0; i < 100; i++ ) {

     double tgnup = tgnu + 1.0;
     double tenu  = thetaE ( p, tgnu, tgnu );
     double tenup = thetaE ( p, tgnup, tgnup );

     if ( tenu <= 0.0 || tenup <= 0.0 ) 
        return( MM5Point::MISSING_VAL );
     
     double cor = (thte-tenu) / (tenup-tenu);

     tgnu = tgnu + cor;
     if ( fabs(cor) <= .01 ) {
        return( tgnu + 273.15 );
     }
   }

   return( MM5Point::MISSING_VAL );
  
}

//////////////////////////////////////////////////////////////
//
// Returns equivalent potential temperature, 
// given pressure, temperature and dewpoint
//
// Niles Oien
//
//////////////////////////////////////////////////////////////
double 
MM5Data::thetaE( double pressure, double temperature, double dewPoint )
{
   if ( pressure == MM5Point::MISSING_VAL ||
        temperature == MM5Point::MISSING_VAL ||
        dewPoint == MM5Point::MISSING_VAL ) 
      return( MM5Point::MISSING_VAL );

  double rmix  = prMixr( dewPoint, pressure );
  double e     = (2.0/7.0) * ( 1.0 - (0.001*0.28*rmix) );
  double thtam = (temperature+273.15) * pow((1000.0/pressure),e);

  double rVal  = thtam * 
                 exp( (3.376/prTlcl( temperature, dewPoint ) - 0.00254 ) *
                      ( rmix * (1.0 + 0.81*0.0001*rmix) ) );

  return( rVal );

}

//////////////////////////////////////////////////////////
//
// Returns Lifted Condensation Level Temperature, 
// given temperature and dewpoint
//
// Niles Oien
//
//////////////////////////////////////////////////////////
double 
MM5Data::prTlcl( double t, double td )
{
  if ( t == MM5Point::MISSING_VAL || td == MM5Point::MISSING_VAL )
     return( MM5Point::MISSING_VAL );

  double tk = t + 273.15;
  double dk = td + 273.15;        

  double rVal = ( 1.0 / ( 1/(dk-56.0) + log( tk/dk ) / 800.0 )) + 56.0;

  return( rVal );
}

//////////////////////////////////////////////////////////
//
//  Returns mixing ratio given td and pressure.  
//  If temperture is passed instead of td it returns 
//  saturation m.r. (g/kg).
//
//  Niles Oien
//
//////////////////////////////////////////////////////////
double 
MM5Data::prMixr( double td, double p )
{
  if ( p == MM5Point::MISSING_VAL || td == MM5Point::MISSING_VAL )
     return( MM5Point::MISSING_VAL );

  double corr = 1.001 + ( ( p - 100.0) / 900.0)  * 0.0034;
  double e    = prVapr( td ) * corr;

  double rVal =  0.62197 * (e / (p-e)) * 1000.0;
  
  return( rVal );
}


////////////////////////////////////////////////////////////////////////
//
// Returns vapor pressure (mb) given temperature.  If t is actual 
// temperature, the result is saturation vapor pressure.  If t is 
// dewpoint, the result is actual vapor pressure.
//
// Niles Oien
//
///////////////////////////////////////////////////////////////////////
double 
MM5Data::prVapr( double t )
{
  if( t == MM5Point::MISSING_VAL ) 
     return( MM5Point::MISSING_VAL );

  double rVal =  6.112 * exp( (17.67*t)/(t+243.5) );
  
  return( rVal );
}

///////////////////////////////////////////////////////////////////////
// 
// Written by Niles Oien
//
// Checks for missingVal added by Jaimi Yee.  Most likely these will
// not be necessary, but if they are, they *may* need to be revisited.  
//
//////////////////////////////////////////////////////////////////////
double 
MM5Data::findLifted( double *pres, double *temp, double *dp, int nl, double pli )
{
   //
   // First find surface point in sounding.  
   // The first few points may be fictious
   // points below the ground.  Assume ground 
   // point is first point with T and Td.
   //
   int isrf = 0;
  
   do {
      isrf++;
   } while ( isrf < nl && 
             ( temp[isrf] == GenPt::missingVal || 
               dp[isrf] == GenPt::missingVal ) );

   //
   // Then find parcle 50 mb higher.
   //
   if ( (nl-isrf) < 3 || (pres[0] != GenPt::missingVal && pres[0] < (pli+50.0)) )
      return( MM5Point::MISSING_VAL );

   if( pres[isrf] == GenPt::missingVal || temp[isrf] == GenPt::missingVal ||
       dp[isrf] == GenPt::missingVal )
      return( MM5Point::MISSING_VAL );
   
   double thte1 = thetaE( pres[isrf], temp[isrf], dp[isrf] );

   if ( thte1 == MM5Point::MISSING_VAL ) 
      return( MM5Point::MISSING_VAL );
   
   double ptop = pres[isrf] - 50.0;

   int i = isrf;
   do {
      i++;
   } while ( (pres[i] >= ptop || pres[i] == GenPt::missingVal) && i <= nl );

   if ( i >= nl ) 
      return( MM5Point::MISSING_VAL );

   i--;

   double thte, thte2;

   if ( temp[i] != GenPt::missingVal && 
        dp[i] != GenPt::missingVal &&
        pres[i] != GenPt::missingVal ) {
      thte2 = thetaE( pres[i], temp[i], dp[i] );
      thte  = (thte1+thte2) / 2.0;
   }
   else {
      thte = thte1;
   }

   //
   // Find the temperature at the level to be lifted to.
   //
   int liftp = i;
   do {
      liftp ++;
   } while ( (pres[liftp] > pli || pres[liftp] == GenPt::missingVal) && liftp < nl );

   if ( liftp >= nl ) 
      return( MM5Point::MISSING_VAL );

   double fac, tpli;
  
   if( temp[liftp] == GenPt::missingVal )
      return( MM5Point::MISSING_VAL );
   
   if ( pres[liftp] < pli ) {
      fac = (pres[liftp] - pli) / (pres[liftp] - pres[liftp-1]);

      if( temp[liftp-1] == GenPt::missingVal )
         return( MM5Point::MISSING_VAL );
      
      tpli = temp[liftp] + (temp[liftp-1] - temp[liftp])*fac;
   }
   else {
      tpli = temp[liftp];
   }
   
   //
   // Lift the parcel and compute the li.
   //
   double tlift;
  
   if ( tpli != MM5Point::MISSING_VAL ) {
      tlift = reverseThetaE( thte, pli, tpli+273.0 );

      if ( tlift == MM5Point::MISSING_VAL ) 
         return( MM5Point::MISSING_VAL );

      tlift = tlift - 273.15;

      return( tpli - tlift );
  }

   return( MM5Point::MISSING_VAL );

}








   
   
