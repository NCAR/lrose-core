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
///////////////////////////////////////////////////////////
//  Class used to apply the regression from 
//  MosCalibration to the current combo point
//  from MM5
//
//  $id: $
//////////////////////////////////////////////////////////
#include <string>
#include <cfloat>
#include <physics/physics.h>
#include <toolsa/str.h>
#include <toolsa/Path.hh>
#include <rapformats/ComboPt.hh>
#include <rapformats/GenPt.hh>
#include <toolsa/DateTime.hh>
#include "ApplyRegression.hh"
using namespace std;


void
ApplyRegression::init ( double clrVis, double clrCeil, 
                        const string& regUrl, const string& modelUrl, 
                        int timeWindow, int dLeadTime, bool ckFcast, 
                        bool dbg, bool rtime, const string& dbgDir) 
{
   debug         = dbg;
   realtime      = rtime;
   checkForecast = ckFcast;
   clearVis      = clrVis;
   clearCeiling  = clrCeil;
   regressionUrl = regUrl;
   deltaLeadTime = dLeadTime;
   rainRateUrl   = modelUrl;
   timeMargin    = timeWindow;
   debugDir 	 = dbgDir;

   debugHeadersWritten = "";
   clear();
   clearIntermediate();
}

void
ApplyRegression::clear() 
{
   rhVal               = STATION_NAN;
   uVal                = STATION_NAN;
   vVal                = STATION_NAN;
   tempVal             = STATION_NAN;
   prsVal              = STATION_NAN;
   wspdVal             = STATION_NAN;
   ceilingVal          = STATION_NAN;
   visVal              = STATION_NAN;
}

void
ApplyRegression::clearIntermediate() 
{
   tempDiff_1000_850mb = STATION_NAN;
   tempDiff_900_700mb  = STATION_NAN;
   avgW                = STATION_NAN;
   rainRate            = STATION_NAN;
   liftedIndex         = STATION_NAN;
}


int
ApplyRegression::process( ComboPt& comboPt, station_report_t& S,
                          int leadTime, int stationId ) 
{ 
   //
   // Get the single level and multilevel points from the
   // combo point that was sent in
   //
   const GenPt& singleLevelPt = comboPt.get1DPoint();
   const GenPt& multiLevelPt  = comboPt.get2DPoint();

   //
   // Get the data time from the combo point
   //
   time_t   dataTime = multiLevelPt.getTime();
   DateTime when( dataTime );

   //
   // Get the regression information from the spdb server
   //
   if( getRegData( stationId, leadTime, dataTime ) != 0 ) {
      if( debug ) {
         cerr << "Could not get regression information" << endl;
      }
      
      return( -1 );
   }

   if( debug ) {
      string stationName = Spdb::dehashInt32To4Chars( stationId );
      cerr << "Working on regression equation for " << stationName
           << " for lead time = " << leadTime << endl;
      cerr << " at " << when.dtime() << endl;
   }

   //
   // Get the chunks from the server for the regression data
   //
   int nChunks = regressionSpdbMgr.getNChunks();
   if( nChunks == 0 ) {
      if( debug ) {
         cerr << "No regression data found" << endl;
      }
      return( -1 );
   }
   
   Spdb::chunk_ref_t *regressionChunkRefs = regressionSpdbMgr.getChunkRefs();
   vector<Spdb::chunk_t> regressionChunks = regressionSpdbMgr.getChunks();

   //
   // Find the correct data - if lead time is zero, the server
   // will get all lead times
   //
   GenPt regressionPt;

   bool found = false;
   for( int i = nChunks - 1; i >= 0; i-- ) {
      if( regressionChunkRefs[i].data_type2 == leadTime ) {
         regressionPt.disassemble( regressionChunks[i].data,
                                  regressionChunks[i].len );
	 found = true;
	 break;
      }
   }

   if( !found ) {
      cerr << "No regression data found" << endl;
      return( -1 );
   }

   if ( debug ){
     cerr << "Found regression data"<<endl;
     regressionPt.print(stderr);

   }
    
   //
   // Find model values for simple regressions
   //
   double modelRH   = get2DVal( "RH", multiLevelPt );
   double modelTemp = get2DVal( "Temp", multiLevelPt );
   double modelPrs  = get2DVal( "PRESSURE", multiLevelPt );
   double modelU    = get2DVal( "U", multiLevelPt );
   double modelV    = get2DVal( "V", multiLevelPt );

   //
   // Do the simple regressions
   //
   rhVal     = applySimple( modelRH, "relHum", regressionPt, leadTime);
   tempVal   = applySimple( modelTemp, "temp", regressionPt, leadTime);
   prsVal    = applySimple( modelPrs, "prs", regressionPt, leadTime);
   uVal      = applySimple( modelU, "u", regressionPt, leadTime);
   vVal      = applySimple( modelV, "v", regressionPt, leadTime);

   //
   // Calculate wspd and do the simple regression
   //
   double modelWspd = STATION_NAN;
   if( modelU != STATION_NAN && modelV != STATION_NAN ) {
      modelWspd = sqrt( modelU * modelU + modelV * modelV );
      wspdVal   = applySimple( modelWspd, "wspd", regressionPt, leadTime);
   }

   //
   // Calculate the intermediate values needed for ceiling and
   // visibility calculations
   //
   calcIntermediate( multiLevelPt, singleLevelPt, dataTime, leadTime,
                     stationId );

   //
   // Ceiling
   //
   ceilingVal = calcCeiling( regressionPt, multiLevelPt, modelRH,
                             modelWspd, modelPrs, leadTime );
   if( debug && ceilingVal < 0) {
      cerr << "WARNING: ceiling less than 0:  " << ceilingVal << endl;
   }
   
   //
   // Visibility
   //
   visVal = calcVis( regressionPt, modelRH, modelWspd, modelPrs, leadTime );
   if( debug && visVal < 0 ) {
      cerr << "WARNING: visibility less than 0:  " << visVal << endl;
   }

   // 
   // Fill out the station report
   //
   fillStationReport( S, dataTime, singleLevelPt.getLat(), 
                      singleLevelPt.getLon(), stationId );

   return( 0 );
   
}

int 
ApplyRegression::getRegData( int dataType, int dataType2, time_t dataTime ) 
{
   if( realtime ) {

     if( debug ) {
       cerr<< "Getting regression data - REALTIME" << endl;
       cerr << "  url: " << regressionUrl << endl;
       cerr << "  timeMargin: " << timeMargin << endl;
       cerr << "  dataType: " << Spdb::dehashInt32To4Chars(dataType) << endl;
       cerr << "  dataType2: " << dataType2 << endl;
     }

     if ( regressionSpdbMgr.getLatest( regressionUrl, timeMargin, 
				       dataType, dataType2 ) ) {
       cerr << "ERROR - ApplyRegression::getRegData" << endl;
       cerr << regressionSpdbMgr.getErrorStr() << endl;
       return -1;
     } else {
       return 0;
     }

   } else {

     if( debug ) {
       cerr<< "Getting regression data - ARCHIVE" << endl;
       cerr << "  url: " << regressionUrl << endl;
       cerr << "  dataTime: " << DateTime::str(dataTime) << endl;
       cerr << "  timeMargin: " << timeMargin << endl;
       cerr << "  dataType: " << Spdb::dehashInt32To4Chars(dataType) << endl;
       cerr << "  dataType2: " << dataType2 << endl;
     }

     if ( regressionSpdbMgr.getFirstBefore( regressionUrl, dataTime - dataType2, 
					    timeMargin, dataType,
					    dataType2 ) ) {
       cerr << "ERROR - ApplyRegression::getRegData" << endl;
       cerr << regressionSpdbMgr.getErrorStr() << endl;
       return -1;
     } else {
       return 0;
     }

   }
}
      

double 
ApplyRegression::get2DVal( const string& fieldName, 
                           const GenPt& multiLevelPt ) 
{
   int fieldNum = multiLevelPt.getFieldNum( fieldName );
   if( fieldNum < 0 ) {
      if( debug ) {
         cerr << "Cannot find field " << fieldName.c_str() << endl;
      }
      
      return( STATION_NAN );
   }
   return( multiLevelPt.get2DVal( 0, fieldNum ));
   
}

double
ApplyRegression::applySimple( double modelVal, const string& regName,
                              const GenPt& regPt, time_t leadTime )
{
   if( debug ) {
      cerr << "Applying simple regression equation for " << 
         regName.c_str() << endl;
   }
      
   if( modelVal != GenPt::missingVal ) {
      string constantName    = regName + "_VAR_constant";
      string coefficientName = regName + "_VAR_" + regName;

      int    constantIdex    = regPt.getFieldNum( constantName );
      int    coefficientIdex = regPt.getFieldNum( coefficientName );

      if( constantIdex != -1 && coefficientIdex != -1 ) {
         
         double constantTerm = regPt.get1DVal( constantIdex );
         double coefficient  = regPt.get1DVal( coefficientIdex );

         //
         // If the constant or the coeffiecient are missing,
         // return the appropriate missing value
         //   The regression calculation failed upstream
         //
         if( constantTerm == GenPt::missingVal || 
             coefficient == GenPt::missingVal ) {

            if( debug ) {
               cerr << regName.c_str() 
                    << " regression coefficients unavailable" << endl;
            }
               
            return( STATION_NAN );
         }

         if( debug ) {
            cerr << "Constant term = " << constantTerm << 
               ", coefficient = " << coefficient << endl;
         }
      
	 // output debug information
	 if (debugDir != ""){
	   const time_t time_tt = regPt.getTime();
	   struct tm *time_tm = gmtime(&time_tt);
	   char dateString[12];
	   strftime(dateString,12,"%Y%m%d",time_tm);
	   string outputDir = debugDir + "/" + dateString;
	   Path outputPath;
	   outputPath.setDirectory(outputDir.c_str());
	   outputPath.makeDirRecurse();
	   string outputFile = outputDir + "/" + regName + ".txt";
	   cerr << "writing debug output to " << outputFile << endl;
	   FILE *fp;
	   if (debugHeadersWritten.find(regName) == string::npos){
	     debugHeadersWritten += "," + regName;
	     fp = fopen( outputFile.c_str(), "w" );
	     if ( fp == NULL ) {
	       cerr << "Could not open debug output file " <<  outputFile.c_str() << endl;
	     }
	     else{
	       fprintf(fp,"stationID, humanValidTime, unixValidTime, humanInitTime, unixInitTime, forecastSecs, constantTerm, coefficient, modelValue\n");
	     }
	   }
	   else{
	     fp = fopen( outputFile.c_str(), "a" );
	     if ( fp == NULL ) {
	       cerr << "Could not open debug output file " <<  outputFile.c_str() << endl;
	     }			 
	   }
	   if (fp != NULL){
	     fprintf(fp,"%s, %s, %ld, %s, %ld, %ld, %f, %f, %f\n",
		     regPt.getName().c_str(), utimstr(regPt.getTime()), regPt.getTime(), 
		     utimstr(regPt.getTime()-leadTime), regPt.getTime()-leadTime, leadTime,
		     constantTerm, coefficient, modelVal);
	   }
	   fclose(fp);
	 }		 
	
         return( constantTerm + coefficient * modelVal );
      }
   }
   
   return( STATION_NAN );
}

void
ApplyRegression::calcIntermediate( const GenPt& multiLevelPt, 
                                   const GenPt& singleLevelPt,
                                   time_t dataTime, int leadTime,
                                   int stationId )
{
   //
   // Clear values
   //
   clearIntermediate();

   //
   // Find the number of levels
   //
   int nLevels = multiLevelPt.getNLevels();
   
   //
   // Find temperature differences
   //
   int p1000 = findLevelClosestToVal( "PRESSURE", multiLevelPt, 1000 );
   int p900  = findLevelClosestToVal( "PRESSURE", multiLevelPt, 900 );
   int p850  = findLevelClosestToVal( "PRESSURE", multiLevelPt, 850 );
   int p700  = findLevelClosestToVal( "PRESSURE", multiLevelPt, 700 );
      
   int tNum  = multiLevelPt.getFieldNum( "Temp" );
      
   if( tNum != -1 ) {
      if( p1000 != -1 && p850 != -1 ) {
         double temp1 = multiLevelPt.get2DVal( p1000, tNum );
         double temp2 = multiLevelPt.get2DVal( p850, tNum );
         tempDiff_1000_850mb = temp1 - temp2;
      }
      if( p900 != -1 && p700 != -1 ) {
         double temp1 = multiLevelPt.get2DVal( p900, tNum );
         double temp2 = multiLevelPt.get2DVal( p700, tNum );
         tempDiff_900_700mb = temp1 - temp2;
      }
   }

   //
   // Find the mean vertical velocity over the 1000 to 700mb range
   //
   int wNum = multiLevelPt.getFieldNum( "W" );
      
   if( wNum != -1 && p1000 != -1 && p700 != -1 && p1000 < p700 ) {

      int    count = 0;
      double wSum  = 0;

      for( int j = p1000; j < p700; j++ ) {
         wSum += multiLevelPt.get2DVal( j, wNum );
         count++;
      }
      avgW = wSum / count;
   }

   //
   // Calculate the rain rate
   //
   rainRate = findRainRate( singleLevelPt, dataTime, leadTime, 
                            stationId );
      
   //
   // Perform CAPE calculation.
   //
   double myRH;
      
   double *pS  = new double[nLevels];
   double *dpS = new double[nLevels];
   double *tS  = new double[nLevels];

   int rhNum   = multiLevelPt.getFieldNum( "RH" );
   int pNum    = multiLevelPt.getFieldNum( "PRESSURE" );

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

   liftedIndex = findLifted( pS, tS, dpS, nLevels, 500 );

   delete[] pS;
   delete[] dpS;
   delete[] tS;

}


double
ApplyRegression::calcCeiling( const GenPt& regressionPt, 
                              const GenPt& multiLevelPt,
                              double modelRH, double modelWspd,
                              double modelPrs, time_t leadTime ) 
{
   //
   // Tell the user what we are doing
   //
   if( debug ) {
      cerr << "Beginning ceiling calculation" << endl;
   }
   
   //
   // Get the indeces of the coefficients in the regression point
   //
   int ceilConstIdex = regressionPt.getFieldNum( "ceiling_VAR_constant" );
   int clwCoeffIdex  = regressionPt.getFieldNum( "ceiling_VAR_clwht" );
   int iceCoeffIdex  = regressionPt.getFieldNum( "ceiling_VAR_ice" );
   int wspdIdex      = regressionPt.getFieldNum( "ceiling_VAR_wspd" );
   int avgWIdex      = regressionPt.getFieldNum( "ceiling_VAR_avgW" );
   int rainRateIdex  = regressionPt.getFieldNum( "ceiling_VAR_rainRate" );
   int rhIdex        = regressionPt.getFieldNum( "ceiling_VAR_rh" );
   int prsIdex       = regressionPt.getFieldNum( "ceiling_VAR_prs" );
   int tempDiff1Idex = 
      regressionPt.getFieldNum( "ceiling_VAR_tempDiff_1000_850mb" );
   int tempDiff2Idex = 
      regressionPt.getFieldNum( "ceiling_VAR_tempDiff_900_700mb" );
   
   //
   // If any of the coefficients are not present in the regression
   // point, we cannot do the regression calculation below.  Return 
   // the missing value in this case.
   //
   if( ceilConstIdex == -1 || clwCoeffIdex == -1 || iceCoeffIdex == -1 ||
        wspdIdex == -1 || avgWIdex == -1 || rainRateIdex == -1 ||
        rhIdex == -1 || prsIdex == -1 || tempDiff1Idex == -1 ||
        tempDiff2Idex == -1 ) {

      if( debug ) {
         cerr << "Ceiling regression coefficients unavailable" << endl;
      }

      return( STATION_NAN );
   }
   
   //
   // Get the values of the coefficients
   //
   double ceilingConstant = regressionPt.get1DVal( ceilConstIdex );
   double clwCoefficient  = regressionPt.get1DVal( clwCoeffIdex );
   double iceCoefficient  = regressionPt.get1DVal( iceCoeffIdex );
   double wspdCoefficient = regressionPt.get1DVal( wspdIdex );
   double avgWCoefficient = regressionPt.get1DVal( avgWIdex );
   double rrCoefficient   = regressionPt.get1DVal( rainRateIdex );
   double rhCoefficient   = regressionPt.get1DVal( rhIdex );
   double prsCoefficient  = regressionPt.get1DVal( prsIdex );
   double tempDiff1Coeff  = regressionPt.get1DVal( tempDiff1Idex );
   double tempDiff2Coeff  = regressionPt.get1DVal( tempDiff2Idex );

   //
   // If any of the coefficients are set to the missing value, 
   // the regression calculation upstream failed.  Return missing 
   // value in this case.
   //
   if( ceilingConstant == GenPt::missingVal ||
       clwCoefficient == GenPt::missingVal ||
       iceCoefficient == GenPt::missingVal ||
       wspdCoefficient == GenPt::missingVal ||
       avgWCoefficient == GenPt::missingVal ||
       rrCoefficient == GenPt::missingVal ||
       rhCoefficient == GenPt::missingVal ||
       prsCoefficient == GenPt::missingVal ||
       tempDiff1Coeff == GenPt::missingVal ||
       tempDiff2Coeff == GenPt::missingVal ) {

      if( debug ) {
         cerr << "Ceiling regression coefficients unavailable" << endl;
      }

      return( STATION_NAN );
   }

   //
   // Initialize for clw and ice calculations
   //   
   int    nLevels       = multiLevelPt.getNLevels();
   int    pNum          = multiLevelPt.getFieldNum( "PRESSURE" );
   double clwThreshHt   = STATION_NAN;
   double iceContHt     = STATION_NAN;
   double psfc          = multiLevelPt.get2DVal( 0, pNum );

   //
   // If the clw coefficient is zero, we don't care what the value
   // is, so only do the calculation if the coefficient is not zero.
   //
   if( clwCoefficient != 0 ) {

      //
      // First find the threshold used for cloud liquid water.  Then
      // apply it to find the height at which the cloud liquid water
      // is greater than the threshold  
      //
      bool threshExceeded = false;
      int  clwThreshIdex  = regressionPt.getFieldNum( "clwThresh" );

      //
      // If the clw threshold is not present in the regression point
      // or we could not get the value of pressure at the surface from
      // the mm5 point, return the missing value for ceiling
      //
      if( clwThreshIdex == -1 || psfc == GenPt::missingVal ) {

         if( debug ) {
            cerr << "One or both of the following was missing: "
                 << "cloud liquid water threshold or pressure at "
                 << "the surface"
                 << endl;
         }
         
         return( STATION_NAN );
      }
      
      //
      // Get the threshold value and initilize
      //
      int    clNum        = multiLevelPt.getFieldNum( "CLW" );
      double clwThreshold = regressionPt.get1DVal( clwThreshIdex );
      double pceil        = STATION_NAN;

      //
      // If the clw or the pressure data are not available in the mm5
      // point, return the missing value for ceiling
      //
      if( clNum == -1 || pNum == -1 ) {

         if( debug ) {
            cerr << "One or both of the following was missing: " 
                 << "cloud liquid water data or pressure data"
                 << endl;
         }
         
         return( STATION_NAN );
      }

      for( int k = 0; k < nLevels; k++ ) {

         //
         // Get the cloud liquid water value from the mm5 point
         //
         double clw = multiLevelPt.get2DVal( k, clNum );
         if( clw == GenPt::missingVal )
           continue;
            
         //
         // If the threshold is exceeded, compute the height
         // at which this took place
         //
         if( clw > clwThreshold ) {

            threshExceeded = true;
                  
            pceil = multiLevelPt.get2DVal( k, pNum );
            if( pceil == GenPt::missingVal )
               continue;

            if( psfc >= pceil ) {
               clwThreshHt = mbToKm( pceil, psfc );
            }
            else {

               if( debug ) {
                  cerr << "Warning: pressure at surface = " << psfc
                       << ", pressure at altitude = " << pceil << endl;
               }

               //
               // If the pressure values are bad, we are not
               // going to be able to compute clwThreshHt, so
               // return the missing value for ceiling
               //
               return( STATION_NAN );
            }
            
            break;
         }
      }
      
      //
      // If the threshold was never exceeded return the clear
      // value if the user requested this kind of behavior.
      // Otherwise, if we couldn't compute clwThreshHt for any 
      // reason, return the missing value for ceiling.
      //
      if( !threshExceeded && checkForecast ) {

         if( debug ) {
            cerr << "Cloud liquid water never exceeded threshold" 
                 << endl;
            cerr << "Setting ceiling to clear value"
                 << endl;
         }
         
         return( clearCeiling );
      }
      else if( clwThreshHt == STATION_NAN ) {

         if( debug ) {
            cerr << "Cloud liquid water threshold height missing" << endl;
         }
         
         return( STATION_NAN );
      }

   }

   //
   // If the ice coefficient is zero, we don't care what the value
   // is, so only do the calculation if the coefficient is not zero.
   //
   if( iceCoefficient != 0 ) {
   
      //
      //  Find the ice content threshold and then apply it to find the
      //  height at which the ice content exceeds the given threshold
      // 
      bool threshExceeded = false;
      int  iceThreshIdex  = regressionPt.getFieldNum( "iceThresh" );

      // 
      // If the ice threshold is not present or we couldn't get the
      // value of the pressure at the surface from the mm5 point
      // return the missing value for ceiling
      //
      if( iceThreshIdex == -1 || psfc == GenPt::missingVal ) {

         if( debug ) {
            cerr << "One or both of the following is missing: "
                 << "ice content threshold or pressure at the surface"
                 << endl;
         }
         
         return( STATION_NAN );
      }
      
      //
      // Get the threshold value
      //
      double iceThresh = regressionPt.get1DVal( iceThreshIdex );
      
      //
      // Get the index for the ice variable in the mm5 point
      //
      int    iceNum = multiLevelPt.getFieldNum( "ICE" );
      double ice    = STATION_NAN;
      double pice   = STATION_NAN;

      //
      // If the ice content or the pressure data are not available
      // from the mm5 point, return the missing value for ceiling
      //
      if( iceNum == -1 || pNum == -1 ) {

         if( debug ) {
            cerr << "One or both of the following is missing: "
                 << "ice content or pressure data" 
                 << endl;
         }
         
         return( STATION_NAN );
      }

      for( int j = 0; j < nLevels; j++ ) {

         //
         // Get the ice content value from the mm5 point
         //
	 ice = multiLevelPt.get2DVal( j, iceNum );
         if( ice == GenPt::missingVal )
            continue;

         //
         // If the threshold has been exceeded, calculate the
         // height at which this took place
         //            
	 if( ice > iceThresh ) {

            threshExceeded = true;
            
	    pice = multiLevelPt.get2DVal( j, pNum );
            if( pice == GenPt::missingVal )
               continue;

            if( psfc >= pice ) {
               iceContHt = mbToKm( pice, psfc );
            }
            else {

               if( debug ) {
                  cerr << "Warning:  pressure at surface = " << psfc
                       << ", pressure at altitude = " << pice << endl;
               }

               //
               // If the pressure values are bad, we are not going
               // to be able to compute the iceContHt, so return
               // the missing value for ceiling
               //
               return( STATION_NAN );
            }
               
	    break;
         }
      }

      //
      // If the threshold was never exceeded return the clear
      // value if the user requested this kind of behavior.
      // Otherwise, if we couldn't compute iceContHt for any 
      // reason, return the missing value for ceiling.
      //
      if( !threshExceeded && checkForecast ) {

         if( debug ) {
            cerr << "Ice content threshold was never exceeded"
                 << endl;
            cerr << "Setting ceiling to the clear value"
                 << endl;
         }
         
         return( clearCeiling );
      }
      else if( iceContHt == STATION_NAN ) {

         if( debug ) {
            cerr << "Ice content threshold height missing" << endl;
         }
         
         return( STATION_NAN );
      }
   }

   //
   // Check the other variable values
   //   We do not care what the value is of any of these variables
   //   if the corresponding coefficient is zero.  So only return
   //   the missing value for ceiling if a given variable is missing
   //   and the corresponding coefficient is non-zero.
   //   
   if( wspdCoefficient != 0 && modelWspd == STATION_NAN ) {

      if( debug ) {
         cerr << "wind speed missing" << endl;
      }
      
      return( STATION_NAN );
   }
      
   if( avgWCoefficient != 0 && avgW == STATION_NAN ) {

      if( debug ) {
         cerr << "average W missing" << endl;
      }
      
      return( STATION_NAN );
   }
      
   if( rrCoefficient != 0 && rainRate == STATION_NAN ) {

      if( debug ) {
         cerr << "rain rate missing" << endl;
      }
      
      return( STATION_NAN );
   }
      
   if( rhCoefficient != 0 && modelRH == STATION_NAN ) {

      if( debug ) {
         cerr << "relative humidity missing" << endl;
      }
      
      return( STATION_NAN );
   }
      
   if( prsCoefficient != 0 && modelPrs == STATION_NAN ) {

      if( debug ) {
         cerr << "pressure missing" << endl;
      }
      
      return( STATION_NAN );
   }
      
   if( tempDiff1Coeff != 0 && tempDiff_1000_850mb == STATION_NAN ) {

      if( debug ) {
         cerr << "temperature difference between 1000mb and 850mb missing"
              << endl;
      }
      
      return( STATION_NAN );
   }
    
   if( tempDiff2Coeff != 0 && tempDiff_900_700mb == STATION_NAN ) {

      if( debug ) {
         cerr << "temperature difference between 900mb and 700mb missing"
              << endl;
      }
      
      return( STATION_NAN );
   }

   //
   // Tell the user what we are doing
   //
   if( debug ) {
      cerr << "Applying regression equation for ceiling" << endl;
   }
      


   // output debug information
   if (debugDir != ""){     
     const time_t time_tt = regressionPt.getTime();
     struct tm *time_tm = gmtime(&time_tt);
     char dateString[12];
     strftime(dateString,12,"%Y%m%d",time_tm);
     string outputDir = debugDir + "/" + dateString;
     Path outputPath;
     outputPath.setDirectory(outputDir.c_str());
     outputPath.makeDirRecurse();
     string outputFile = outputDir  + "/ceiling.txt";
     FILE *fp;
     if (debugHeadersWritten.find("ceiling") == string::npos){
       debugHeadersWritten += ",ceiling";
       fp = fopen( outputFile.c_str(), "w" );
       if ( fp == NULL ) {
	 fprintf(stderr, "Could not open debug output file %s", outputFile.c_str() );
       }
       else{
	 fprintf(fp,"stationID, humanValidTime, unixValidTime, humanInitTime, unixInitTime, forecastSecs, ceilingConstant, clwCoefficient, clwThreshHt, " 
		 "iceCoefficient, iceContHt, wspdCoefficient, modelWspd, avgWCoefficient, avgW, rrCoefficient, rainRate, "
		 "rhCoefficient, modelRH, prsCoefficient, modelPrs, tempDiff1Coeff, tempDiff_1000_850mb, tempDiff2Coeff, tempDiff_900_700mb\n");
       }
     }
     else{
       fp = fopen( outputFile.c_str(), "a" );
       if ( fp == NULL ) {
	 fprintf(stderr, "Could not open debug output file %s", outputFile.c_str() );
       }			 
     }
     if (fp != NULL){
       fprintf(fp," %s, %s, %ld, %s, %ld, %ld, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f\n",
	       regressionPt.getName().c_str(), utimstr(regressionPt.getTime()),regressionPt.getTime(), 
	       utimstr(regressionPt.getTime()-leadTime), regressionPt.getTime()-leadTime, leadTime, ceilingConstant, clwCoefficient, clwThreshHt,
	       iceCoefficient, iceContHt, wspdCoefficient, modelWspd, avgWCoefficient, avgW, rrCoefficient, rainRate, 
	       rhCoefficient, modelRH, prsCoefficient, modelPrs, tempDiff1Coeff, tempDiff_1000_850mb, tempDiff2Coeff, tempDiff_900_700mb);			
     }
     fclose(fp);
   }		 
  


   //
   // Calculate the ceiling value
   //   We know that all of the pertinant values are non-missing
   //   at this point.
   //
   double ceiling = ceilingConstant + clwCoefficient * clwThreshHt + 
      iceCoefficient * iceContHt + wspdCoefficient * modelWspd +
      avgWCoefficient * avgW + rrCoefficient * rainRate +
      rhCoefficient * modelRH + prsCoefficient * modelPrs +
      tempDiff1Coeff * tempDiff_1000_850mb + 
      tempDiff2Coeff * tempDiff_900_700mb;
      
   return( ceiling );
}



double
ApplyRegression::calcVis( const GenPt& regressionPt, 
                          double modelRH, double modelWspd, 
                          double modelPrs, time_t leadtime ) 
{
   if( debug ) {
      cerr << "Beginning visibility calculation" << endl;
   }
   
   //
   // Get the indeces for the coefficients in the regression point
   //
   int visConstIdex        = regressionPt.getFieldNum( "vis_VAR_constant" );
   int wspdCoeffIdex       = regressionPt.getFieldNum( "vis_VAR_wspd" );
   int avgWCoeffIdex       = regressionPt.getFieldNum( "vis_VAR_avgW" );
   int rainRateCoeffIdex   = regressionPt.getFieldNum( "vis_VAR_rainRate" );
   int rhVisCoeffIdex      = regressionPt.getFieldNum( "vis_VAR_rh" );
   int prsVisCoeffIdex     = regressionPt.getFieldNum( "vis_VAR_prs" );
   int liCoeffIdex         = regressionPt.getFieldNum( "vis_VAR_li" );
   
   int tempDiff_1000_850mbCoeffIdex = 
      regressionPt.getFieldNum( "vis_VAR_tempDiff_1000_850mb" );
   int tempDiff_900_700mbCoeffIdex  = 
      regressionPt.getFieldNum( "vis_VAR_tempDiff_900_700mb" );
   
   //
   // We cannot do the regression calculation if any of the
   // coefficients are not present in the file.  In this case
   // return the missing value.
   //
   if( visConstIdex == -1 || wspdCoeffIdex == -1 || 
       avgWCoeffIdex == -1 || rainRateCoeffIdex == -1 || 
       rhVisCoeffIdex == -1 || prsVisCoeffIdex == -1 || 
       liCoeffIdex == -1 || tempDiff_1000_850mbCoeffIdex == -1 || 
       tempDiff_900_700mbCoeffIdex == -1 ) {

      if( debug ) {
         cerr << "Could not compute regression for vis because "
              << "one or more of the coefficients is missing"
              << endl;
      }
      
      return( STATION_NAN );
   }
      
   //
   // Get the values of the coefficients
   //
   double visConstant         = regressionPt.get1DVal( visConstIdex );
   double wspdCoefficient     = regressionPt.get1DVal( wspdCoeffIdex );
   double avgWCoefficient     = regressionPt.get1DVal( avgWCoeffIdex );
   double rainRateCoefficient = regressionPt.get1DVal( avgWCoeffIdex );
   double rhVisCoefficient    = regressionPt.get1DVal( rhVisCoeffIdex );
   double prsVisCoefficient   = regressionPt.get1DVal( prsVisCoeffIdex );
   double liCoefficient       = regressionPt.get1DVal( liCoeffIdex );
      
   double tempDiff_1000_850mbCoeff = 
      regressionPt.get1DVal( tempDiff_1000_850mbCoeffIdex );
   double tempDiff_900_700mbCoeff = 
      regressionPt.get1DVal( tempDiff_900_700mbCoeffIdex );

   //
   // If any of the coefficients are missing, the regression
   // calculation upstream failed.  In this case, return the
   // missing value.
   //
   if( visConstant == GenPt::missingVal ||
       wspdCoefficient == GenPt::missingVal ||
       avgWCoefficient == GenPt::missingVal ||
       rainRateCoefficient == GenPt::missingVal ||
       rhVisCoefficient == GenPt::missingVal ||
       prsVisCoefficient == GenPt::missingVal ||
       prsVisCoefficient == GenPt::missingVal ||
       liCoefficient == GenPt::missingVal ) {

      if( debug ) {
         cerr << "Vis regression coefficients unavailable" << endl;
      }
         
      return( STATION_NAN );
   }

   //
   // Check the value of each variable
   //   If the coefficient is zero, we don't care if we couldn't
   //   compute the corresponding value.  So only return the missing
   //   value if the coefficient is non-zero and we couldn't compute
   //   the corresponding value.
   //
   if( wspdCoefficient != 0 && modelWspd == STATION_NAN ) {

      if( debug ) {
         cerr << "wind speed missing" << endl;
      }
      
      return( STATION_NAN );
   }
      
   if( avgWCoefficient != 0 && avgW == STATION_NAN ) {

      if( debug ) {
         cerr << "average W missing" << endl;
      }
      
      return( STATION_NAN );
   }
      
   if( rainRateCoefficient != 0 && rainRate == STATION_NAN ) {

      if( debug ) {
         cerr << "rain rate missing" << endl;
      }
      
      return( STATION_NAN );
   }
      
   if( rhVisCoefficient != 0 && modelRH == STATION_NAN ) {

      if( debug ) {
         cerr << "relative humidity missing" << endl;
      }
      
      return( STATION_NAN );
   }
      
   if( prsVisCoefficient != 0 && modelPrs == STATION_NAN ) {

      if( debug ) {
         cerr << "pressure missing" << endl;
      }
      
      return( STATION_NAN );
   }
      
   if( liCoefficient != 0 && liftedIndex == STATION_NAN ) {

      if( debug ) {
         cerr << "lifted index missing" << endl;
      }
      
      return( STATION_NAN );
   }
      
   if( tempDiff_1000_850mbCoeff != 0 && 
       tempDiff_1000_850mb == STATION_NAN ) {

      if( debug ) {
         cerr << "temperature difference between 1000mb and 850mb missing"
              << endl;
      }
      
      return( STATION_NAN );
   }
      
   if( tempDiff_900_700mbCoeff != 0 &&
       tempDiff_900_700mb == STATION_NAN ) {

      if( debug ) {
         cerr << "temperature difference between 900mb and 700mb missing"
              << endl;
      }
      
      return( STATION_NAN );
   }

   //
   // Tell the user what we are doing
   //
   if( debug ) {
      cerr << "Applying regression equation for visibility" << endl;
   }



   // output debug information
   if (debugDir != ""){
     const time_t time_tt = regressionPt.getTime();
     struct tm *time_tm = gmtime(&time_tt);
     char dateString[12];
     strftime(dateString,12,"%Y%m%d",time_tm);
     string outputDir = debugDir + "/" + dateString;
     Path outputPath;
     outputPath.setDirectory(outputDir.c_str());
     outputPath.makeDirRecurse();
     string outputFile = outputDir  + "/visibility.txt";
     FILE *fp;
     if (debugHeadersWritten.find("visibility") == string::npos){
       debugHeadersWritten += ",visibility";
       fp = fopen( outputFile.c_str(), "w" );
       if ( fp == NULL ) {
	 fprintf(stderr, "Could not open debug output file %s", outputFile.c_str() );
       }
       else{
	 fprintf(fp,"stationID, humanValidTime, unixValidTime, humanInitTime, unixInitTime, forecastSecs,"
		 "visConstant, wspdCoefficient, modelWspd, avgWCoefficient, avgW, rainRateCoefficient, rainRate, "
		 "rhVisCoefficient, modelRH, prsVisCoefficient, modelPrs, liCoefficient, liftedIndex, "
		 "tempDiff_1000_850mbCoeff, tempDiff_1000_850mb, tempDiff_900_700mbCoeff, tempDiff_900_700mb\n");
       }
     }
     else{
       fp = fopen( outputFile.c_str(), "a" );
       if ( fp == NULL ) {
	 fprintf(stderr, "Could not open debug output file %s", outputFile.c_str() );
       }			 
     }
     if ( fp != NULL ) { 
       fprintf(fp,"%s, %s, %ld, %s, %ld, %ld, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f\n",
	       regressionPt.getName().c_str(), utimstr(regressionPt.getTime()),regressionPt.getTime(), 
	       utimstr(regressionPt.getTime()-leadtime), regressionPt.getTime()-leadtime, leadtime, visConstant, wspdCoefficient, modelWspd, avgWCoefficient, avgW, rainRateCoefficient, rainRate,
	       rhVisCoefficient, modelRH, prsVisCoefficient, modelPrs, liCoefficient, liftedIndex, 
	       tempDiff_1000_850mbCoeff, tempDiff_1000_850mb, tempDiff_900_700mbCoeff, tempDiff_900_700mb);			
     }
     fclose(fp);
   }		 

   //
   // Calculate visibility
   //   We know that all of the pertinent values are non-missing
   //
   double visibility = visConstant + wspdCoefficient * modelWspd + 
      avgWCoefficient * avgW + rainRateCoefficient * rainRate +
      rhVisCoefficient * modelRH + prsVisCoefficient * modelPrs +
      liCoefficient * liftedIndex +
      tempDiff_1000_850mbCoeff * tempDiff_1000_850mb +
      tempDiff_900_700mbCoeff * tempDiff_900_700mb;
   
   return( visibility );
}

double 
ApplyRegression::mbToKm( double pressure, double psfc )
{
  double HtInKm = 8.5*log(psfc / pressure);
  return( HtInKm );
}

int
ApplyRegression::findLevelClosestToVal( const string& fieldName, 
                                        const GenPt& multiLevelPt,
                                        double value ) 
{
   int fieldNum = multiLevelPt.getFieldNum( fieldName );
   if( fieldNum < 0 ) {
      if( debug ) {
         cerr << "Cannot find field " << fieldName.c_str() << endl;
      }
      
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
ApplyRegression::findRainRate( const GenPt& singleLevelPt, 
                               time_t dataTime, int leadTime, 
                               int stationId )
{
   DateTime when( dataTime );

   double rainRate  = 0.0;
   
   if( leadTime != 0 ) {

      //
      // Find the current rain accumulation
      //
      double rainCon   = STATION_NAN;
      double rainNon   = STATION_NAN;
      double rainAccum = STATION_NAN;

      int rcNum = singleLevelPt.getFieldNum( "RAIN_CON" );
      int rnNum = singleLevelPt.getFieldNum( "RAIN_NON" );
      
      if( rcNum != -1 && rnNum != -1 ) {
         
         rainCon   = singleLevelPt.get1DVal( rcNum );
         rainNon   = singleLevelPt.get1DVal( rnNum );
         rainAccum = rainCon + rainNon;

      }
      
      //
      // Get the data with the same generate time, but the
      // previous lead time
      //
      int prevLeadTime = MAX( leadTime - deltaLeadTime, 0 );
      int status = rainRateSpdbMgr.getExact( rainRateUrl, 
                                             dataTime, 
                                             stationId, 
                                             prevLeadTime );
      if( status != 0 ) {
         if( debug ) {
            cerr << "Warning: Could not compute rain rate" << endl;
         }
         
         return( 0.0 );
      }

      Spdb::chunk_ref_t    *rainRateChunkRefs = rainRateSpdbMgr.getChunkRefs();
      vector<Spdb::chunk_t> rainRateChunks    = rainRateSpdbMgr.getChunks();

      ComboPt rrComboPt;

      bool found = false;
      for( int i = rainRateSpdbMgr.getNChunks() - 1; i >= 0; i-- ) {
         if( rainRateChunkRefs[i].data_type2 == prevLeadTime ) {
            rrComboPt.disassemble( rainRateChunks[i].data, 
                                   rainRateChunks[i].len );
            found = true;
	    break;
         }
      }

      if( !found ) {
	 return( 0.0 );
      }
      
      // 
      // Get the single level point out of this data
      //
      const GenPt& rrSinglePt = rrComboPt.get1DPoint();

      //
      // Get the previous rain accumulation
      //
      double pRainCon   = STATION_NAN;
      double pRainNon   = STATION_NAN;
      double pRainAccum = STATION_NAN;

      rcNum = rrSinglePt.getFieldNum( "RAIN_CON" );
      rnNum = rrSinglePt.getFieldNum( "RAIN_NON" );

      if( rcNum != -1 && rnNum != -1 ) {

         pRainCon   = rrSinglePt.get1DVal( rcNum );
         pRainNon   = rrSinglePt.get1DVal( rnNum );
         pRainAccum = pRainCon + pRainNon;

      }
      
      //
      //    Calculate the rain rate
      //
      if( rainAccum != STATION_NAN && pRainAccum != STATION_NAN ) {
         rainRate = ( rainAccum - pRainAccum ) / 
            ( leadTime - prevLeadTime );
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
ApplyRegression::reverseThetaE( double thte, double p, double tguess )
{
  if ( thte == STATION_NAN || 
       p == STATION_NAN || 
       tguess == STATION_NAN ) 
    return( STATION_NAN );
	
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
	return( STATION_NAN );
     
     double cor = (thte-tenu) / (tenup-tenu);

     tgnu = tgnu + cor;
     if ( fabs(cor) <= .01 ) {
	return( tgnu + 273.15 );
     }
   }

   return( STATION_NAN );
  
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
ApplyRegression::thetaE( double pressure, double temperature, double dewPoint )
{
   if ( pressure == STATION_NAN ||
        temperature == STATION_NAN ||
        dewPoint == STATION_NAN ) 
      return( STATION_NAN );

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
ApplyRegression::prTlcl( double t, double td )
{
  if ( t == STATION_NAN || td == STATION_NAN )
     return( STATION_NAN );

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
ApplyRegression::prMixr( double td, double p )
{
  if ( p == STATION_NAN || td == STATION_NAN )
     return( STATION_NAN );

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
ApplyRegression::prVapr( double t )
{
  if( t == STATION_NAN ) 
     return( STATION_NAN );

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
ApplyRegression::findLifted( double *pres, double *temp, double *dp, 
                             int nl, double pli )
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
   if ( (nl-isrf) < 3 || (pres[0] != GenPt::missingVal && 
        pres[0] < (pli+50.0)) )
      return( STATION_NAN );

   if( pres[isrf] == GenPt::missingVal || temp[isrf] == GenPt::missingVal ||
       dp[isrf] == GenPt::missingVal )
      return( STATION_NAN );
   
   double thte1 = thetaE( pres[isrf], temp[isrf], dp[isrf] );

   if ( thte1 == STATION_NAN ) 
      return( STATION_NAN );
   
   double ptop = pres[isrf] - 50.0;

   int i = isrf;
   do {
      i++;
   } while ( (pres[i] >= ptop || pres[i] == GenPt::missingVal) && i <= nl );

   if ( i >= nl ) 
      return( STATION_NAN );

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
   } while ( (pres[liftp] > pli || pres[liftp] == GenPt::missingVal) && 
             liftp < nl );

   if ( liftp >= nl ) 
      return( STATION_NAN );

   double fac, tpli;
  
   if( temp[liftp] == GenPt::missingVal )
      return( STATION_NAN );
   
   if ( pres[liftp] < pli ) {
      fac = (pres[liftp] - pli) / (pres[liftp] - pres[liftp-1]);

      if( temp[liftp-1] == GenPt::missingVal )
         return( STATION_NAN );
      
      tpli = temp[liftp] + (temp[liftp-1] - temp[liftp])*fac;
   }
   else {
      tpli = temp[liftp];
   }
   
   //
   // Lift the parcel and compute the li.
   //
   double tlift;
  
   if ( tpli != STATION_NAN ) {
      tlift = reverseThetaE( thte, pli, tpli+273.0 );

      if ( tlift == STATION_NAN ) 
	 return( STATION_NAN );

      tlift = tlift - 273.15;

      return( tpli - tlift );
   }

   return( STATION_NAN );

}

void
ApplyRegression::fillStationReport( station_report_t& report, 
                                    time_t dataTime, double lat,
                                    double lon, int stationId ) 
{
   //
   // Check values if necessary
   //
   if( checkForecast ) {

      //
      // Relative Humidity
      //
      if( rhVal != STATION_NAN ) {
         if( rhVal > 100.0 ) {
            if( debug ) {
               cerr << "Warning:  relative humidity = " << rhVal << endl;
               cerr << "Setting to 100" << endl;
            }
            rhVal = 100.0;
         }

         if( rhVal < 0.0 ) {

            if( debug ) {
               cerr << "Warning:  relatvie humidity = " << rhVal << endl;
               cerr << "Setting to 0" << endl;
            }
            
            rhVal = 0.0;
         }
         
      }

      //
      // Pressure
      //
      if( prsVal != STATION_NAN ) {
         if( prsVal < 0.0 ) {

            if( debug ) {
               cerr << "Warning:  pressure = " << prsVal << endl;
               cerr << "Setting to 0" << endl;
            }
            
            prsVal = 0.0;
         }
      }
      
      //
      // Visibility
      //
      if( visVal != STATION_NAN ) {
         if( visVal < 0.0 ) {

            if( debug ) {
               cerr << "Warning:  visibility = " << visVal << endl;
               cerr << "Setting to 0" << endl;
            }
            
            visVal = 0.0;
         }
         
         if( visVal > clearVis ) {

            if( debug ) {
               cerr << "Warning:  visibility = " << visVal << endl;
               cerr << "Setting to " << clearVis << endl;
            }

            visVal = clearVis;
         }
      }
      
      //
      // Ceiling
      //
      if( ceilingVal != STATION_NAN ) {
         if( ceilingVal < 0.0 ) {

            if( debug ) {
               cerr << "Warning:  ceiling = " << ceilingVal << endl;
               cerr << "Setting to 0" << endl;
            }

            ceilingVal = 0.0;
         }

         if( ceilingVal > clearCeiling ) {

            if( debug ) {
               cerr << "Warning:  ceiling = " << ceilingVal << endl;
               cerr << "Setting to " << clearCeiling << endl;
            }

            ceilingVal = clearCeiling;
         }
      }
   }
      
   report.msg_id           = METAR_REPORT;
   report.time             = dataTime;
   report.accum_start_time = 0;
   report.weather_type     = 0;
   report.lat              = lat;
   report.lon              = lon;
   report.alt              = 0.0;
   report.temp             = tempVal;
   report.relhum           = rhVal;
   report.windspd          = wspdVal;
   report.winddir          = PHYwind_dir( uVal, vVal );
   report.windgust         = STATION_NAN;
   report.pres             = prsVal;
   report.liquid_accum     = STATION_NAN;
   report.precip_rate      = STATION_NAN;
   report.rvr              = STATION_NAN;
   report.visibility       = visVal;
   report.ceiling          = ceilingVal;

   if( tempVal == STATION_NAN || rhVal == STATION_NAN ) 
      report.dew_point = STATION_NAN;
   else
      report.dew_point = PHYrhdp( tempVal, rhVal );

   STRcopy( report.station_label, 
            Spdb::dehashInt32To4Chars( stationId ).c_str(),
            5 );
   
}

   
   
   
