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
////////////////////////////////////////////////////////////////////////
//
// CapeCinSounding.cc
//
///////////////////////////////////////////////////////////////////////

#include <physics/CapeCinSounding.hh>

const float  CapeCinSounding::CAPE_MISSING = -999.0;
const float  CapeCinSounding::CIN_MISSING  = -999.0;
const float  CapeCinSounding::BAD_DATA   = Sndg::QC_BAD;
const float  CapeCinSounding::MISSING_SNDG_DATA = Sndg::VALUE_UNKNOWN;

CapeCinSounding::CapeCinSounding(const bool debug_flag) :
  isOK(true),
  lookup_table(0),
  pr(0),
  temp(0),
  alt(0),
  mixingr(0),
  rh(0),
  dewpt(0),
  time(0),
  u(0),
  v(0),
  w(0),
  tempC(0),
  windSpeed(0),
  windDir(0),
  ascensionRate(0),
  longitude(0),
  latitude(0),
  nz(0),
  cape(0),
  cin(0),
  lcl(0),
  lfc(0),
  el(0),
  output_nz(0),
  min_calc_index(0),
  max_calc_index(0),
  debug(debug_flag),
  verticalLevelVariable(PRESSURE),
  minVarLevel(0.0),
  maxVarLevel(0.0),
  resampleData(false),
  altIncrement(0.0),
  surfaceLayerAveraging(false),
  surfaceLayerUpperBound(0.0),
  surfaceLayerLowerBound(0.0),
  adiabat_temp_lookup_filename(""),
  override_least_calc_index(false)
{
}

CapeCinSounding::CapeCinSounding(bool debug_flag, 
				 Sndg *sndgPtr,
				 var_t verticalVariable,
				 float minCalcLevel, float maxCalcLevel, bool resample,
				 float altIncr, bool surfAve, float surfLB, 
				 float surfUB, string temp_lookuptable)

{

  if (debug_flag)
    cerr << "CapeCinSounding::CapeCinSounding(): Initializing CapeCinSounding\n";

  //
  // initialize data arrays
  //
  pr = NULL; 

  temp = NULL;

  alt = NULL; 

  mixingr = NULL;

  pr = NULL;
  
  dewpt = NULL;
  
  rh = NULL;
  
  time = NULL;
  
  u = NULL;
  
  v = NULL;
  
  w = NULL;
  
  tempC = NULL;
  
  windSpeed = NULL;
  
  windDir = NULL;
  
  ascensionRate = NULL;
  
  longitude = NULL;
  
  latitude = NULL;

  cape = NULL;

  cin = NULL;

  lcl = NULL;

  lfc = NULL; 

  el = NULL;

  //
  // set parameters
  //
  isOK = true;

  debug = debug_flag;
  
  verticalLevelVariable = verticalVariable;

  minVarLevel = minCalcLevel;

  maxVarLevel = maxCalcLevel;

  resampleData = resample;

  altIncrement = altIncr;

  surfaceLayerAveraging =  surfAve;

  surfaceLayerUpperBound = surfUB;

  surfaceLayerLowerBound =  surfLB;

  lookup_table = NULL;
  
  lookup_table = new AdiabatTempLookupTable(temp_lookuptable.c_str());
  
  if (lookup_table == NULL)
    {
       cerr << "ERROR: CapeCinSounding::CapeCinSounding(): " << endl;
       cerr << "Problem creating adiabatic temp lookup table. Check file path.\n" << endl;
       isOK = false;
    }
  
  extractSoundingData(sndgPtr);

  
}

//
// destructor
//
CapeCinSounding::~CapeCinSounding()
{
  delete(lookup_table);
  
  clearAll();
}      	  

////////////////////////////////////////////////////////////////////////
//
// extractSoundingData():
//
//   1) get sounding data, check for missing or bad data,
//      derive mixing ratio for input to CalcCapeCin3D,
//   2) Perform surface averaging if specified in param file 
//   3) Perform data sampling if specified in param file
//
void CapeCinSounding::extractSoundingData(const Sndg *sndgPtr)
{
  
  //
  // Clear any old data out of class member arrays
  //
  clearAll();	  

  //
  // Get the Sndg header  
  //
  Sndg::header_t oldSndgHeader = sndgPtr->getHeader();  
 
  newSndgHeader.launchTime = oldSndgHeader.launchTime;

  newSndgHeader.nPoints = oldSndgHeader.nPoints;

  newSndgHeader.sourceId = oldSndgHeader.sourceId;

  newSndgHeader.leadSecs = oldSndgHeader.leadSecs;

  newSndgHeader.lat = oldSndgHeader.lat;

  newSndgHeader.lon = oldSndgHeader.lon;

  newSndgHeader.alt = oldSndgHeader.alt;

  sprintf(newSndgHeader.sourceName, "%s", oldSndgHeader.sourceName);

  //
  // Get sounding data
  //
  //
  vector<Sndg::point_t>  pts = sndgPtr->getPoints();

  //
  // Record number of vertical levels of sounding
  //
  nz = (int)pts.size();
 
  //
  // Allocate arrays to hold sounding data  
  //
  pr = new float[nz];
  temp = new float[nz];  
  alt = new float[nz];
  mixingr = new float[nz];
  dewpt = new float[nz];
  rh = new float[nz];
  time= new float[nz];
  u= new float[nz];
  v= new float[nz];
  w= new float[nz];
  tempC = new float[nz];
  windSpeed= new float[nz];
  windDir= new float[nz];
  ascensionRate= new float[nz];
  longitude = new float[nz];
  latitude = new float[nz];

  //
  // Fill data arrays
  //
  int count = 0;

  for( int i = 0; i < nz; i++ ) 
    {
      if( pts[i].pressure !=  MISSING_SNDG_DATA &&
	  pts[i].temp     !=  MISSING_SNDG_DATA &&
	  pts[i].altitude !=  MISSING_SNDG_DATA &&
	  pts[i].rh       !=  MISSING_SNDG_DATA &&
	  pts[i].pressureQC != BAD_DATA &&
	  pts[i].tempQC     != BAD_DATA &&
	  pts[i].humidityQC != BAD_DATA )
	{
	
	  //
	  // pressure in mb
	  //
	  pr[count] = pts[i].pressure;

	  //
	  // altitude in m
	  //
	  alt[count] =  pts[i].altitude;

	  //
	  // relative humidity in %
	  //
	  rh[count] =  pts[i].rh;

	  //
	  // temp in deg C, rh in %, dewpt in deg C
	  //
	  dewpt[count] = PHYrhdp(pts[i].temp, rh[count]);
	  
	  //
	  // pr in mb, dewpt in deg C, mixing r in g/kg
	  //
	  mixingr[count] = PHYmixr (dewpt[count],pr[count]);

	  //
	  // degC -> deg K
	  //
	  temp[count] = pts[i].temp + 273.16;


	  //
	  // record the rest of the sndg variables
	  // to create new sndg with cape and cin vals
	  // added to spareFloats array
	  //
	  time[count]  = pts[i].time;
	  u[count]     = pts[i].u;
          v[count]     = pts[i].v;
	  w[count]     = pts[i].w;
	  tempC[count] = pts[i].temp;
	  windSpeed[count] = pts[i].windSpeed;
	  windDir[count]   = pts[i].windDir;
          ascensionRate[count] = pts[i].ascensionRate;
	  longitude[count] = pts[i].longitude;
	  latitude[count]  = pts[i].latitude;
	  
	  count++;
	}
    }

  if (debug && nz != count )
    {
      cerr << "CapeCinSounding::extractSoundingData(): Removed " << nz - count 
	   << " missing or bad data points from sounding.\n"; 
    } 
  
  //
  // reset number of vertical levels
  //
  nz = count;
  
  //
  // Find surface layer averages of data
  //
  if(surfaceLayerAveraging)
    {
      if (debug)
	cerr << "CapeCinSounding::extractSoundingData(): Performing surface layer averaging of data values.\n";

      int ret = surfaceLayerAve();

      if(ret)
	cerr << "CapeCinSounding::extractSoundingData():No data for specified surface layer\n";
    }
  
  //
  // Resample data
  //
  if (resampleData)
    {
      if (debug)
	cerr << "CapeCinSounding::extractSoundingData(): Performing resampling of data values.\n";

      resampleArrays();
    }
  
}


/////////////////////////////////////////////////////////////////////////
//
// Delete member arrays if necessary, reset variables
//
void CapeCinSounding::clearAll()
{

  //
  // Memory cleanup, reset nz to zero
  //
  if (pr != NULL) 
    delete[] pr;
 
  if (temp  != NULL)
    delete[] temp;
  
  if (alt != NULL) 
    delete[] alt;

  if( mixingr != NULL)
    delete[] mixingr;

  if ( cape != NULL)
    delete[] cape;
  
  if (cin != NULL)
    delete[] cin;
  
  if(lcl != NULL)
    delete[] lcl;
  
  if(lfc != NULL)
    delete[] lfc;
  
  if (el != NULL)
    delete[] el;

  if (time   != NULL)
    delete[] time ;
 
  if ( u  != NULL)
    delete[] u ;
 
  if ( v  != NULL)
    delete[] v ;
 
  if ( w  != NULL)
    delete[] w ;
 
  if ( rh  != NULL)
    delete[] rh ;
 
  if ( tempC  != NULL)
    delete[] tempC  ;
 
  if ( dewpt  != NULL)
    delete[] dewpt ;
 
  if ( windSpeed  != NULL)
    delete[] windSpeed;
 
  if ( ascensionRate  != NULL)
    delete[] ascensionRate ;

  if ( windDir  != NULL)
    delete[] windDir;
  
  if (  longitude != NULL)
    delete[] longitude ;
  
  if ( latitude  != NULL)
    delete[] latitude ;

  nz = 0;

  override_least_calc_index = false;

  min_calc_index = 0;

  output_nz = 0;

  //
  // Free sounding point memory
  //
  vector<Sndg::point_t*>::iterator i ;
  
  for (i = pointsVec.begin(); i != pointsVec.end(); i++)
    delete (*i);

  pointsVec.erase(pointsVec.begin(), pointsVec.end());

}

////////////////////////////////////////////////////////////////////////////////
//
// computeMinMaxAltIndices(): Given the upper and lower bounds of altitude
//                            which define the layer over which cape and cin 
//                            are calculated, we find the corresponding sounding 
//                            data array indices for these bounds.
//
void CapeCinSounding::computeMinMaxAltIndices(float *alt, int nz, int &min_index, 
					     int &max_index)
{
  for (int i = 0; i < nz; i++)
    {
      if (alt[i] < minVarLevel)
	min_index++;
      
      if (alt[i] < maxVarLevel)
	max_index++;
    }
}

/////////////////////////////////////////////////////////////////////////////////
//
// computeMinMaxAltIndices(): Given the upper and lower bounds of pressure
//                            which define the layer over which cape and cin 
//                            are calculated, we find the corresponding sounding 
//                            data array indices for these bounds.
//
void CapeCinSounding::computeMinMaxPressureIndices(float *p, int nz, int &min_index, 
						   int &max_index)
{
 
  for (int i = 0; i < nz; i++)
    {
      if (p[i] >  minVarLevel)
	max_index++; 

      if (p[i] >  maxVarLevel)
	min_index++; 
    }
}

/////////////////////////////////////////////////////////////////////////////////
//
// callCalcCapeCin3D: Execute CalcCapeCin3D.
//
int CapeCinSounding::callCalcCapeCin3D()
{

  //
  // Find the indices in the input arrays to calcCapeCin3D
  // which define the upper and lower bounds of the layer
  // over which cape and cin are calculated.
  //
  min_calc_index = 0;
  
  max_calc_index = 0;
  
  if(verticalLevelVariable == PRESSURE)
    //
    // Use pressure to find min and max level indices
    //
    computeMinMaxPressureIndices(pr, nz, min_calc_index, max_calc_index);
  else
    //
    // Use ALTITUDE to find min and max level indices
    //
    computeMinMaxAltIndices(alt, nz, min_calc_index, max_calc_index);

  //
  // Check to see if we override the minimum index for calculating
  // cape and cin.  (We would do this if we want to insure 
  // the cape and cin are calculated at a specific surface obs)
  //
  if (override_least_calc_index)
    min_calc_index = 0;
  
  //
  // Determine number of vertical levels in result arrays
  //
  output_nz = abs(max_calc_index - min_calc_index) + 1;
   
  //
  // Allocate memory for result arrays
  //
  cape = new float[output_nz];
 
  cin = new float[output_nz];
  
  lcl = new float[output_nz];
  
  lfc = new float[output_nz];
  
  el = new float[output_nz];

  //
  // Call calcCapeCin3D
  //
  int nx, ny, dx, dy;
  
  nx = ny = dx = dy = 1;

  if (debug)
    cerr << "CapeCinSounding::callCalcCapeCin3D()\n"; 

  bool ret = PhysicsLib::calcCapeCin3D(*lookup_table,
				       pr,      MISSING_SNDG_DATA, MISSING_SNDG_DATA,
				       temp,    MISSING_SNDG_DATA, MISSING_SNDG_DATA,
				       mixingr, MISSING_SNDG_DATA, MISSING_SNDG_DATA, 
				       alt,     MISSING_SNDG_DATA, MISSING_SNDG_DATA,
				       cape, CAPE_MISSING,
				       cin,  CIN_MISSING,
				       min_calc_index,
				       max_calc_index,
				       dx, dy, nx, ny, nz,
				       lcl, 0,
				       lfc, 0,
				       el,  0); 
     

  if (ret == false)
    {
      cerr << "CapeCinSounding::callCalcCapeCin3D():Error computing cape and cin.\n";
      return(1);
    }

  //
  // recreate sounding with cape cin info
  //
  createNewSounding();

  return(0);
}

/////////////////////////////////////////////////////////////////////////////
//
// resampleArrays(): Class member arrays pr (pressure), temp (temperature),
//                   mixingr( mixing ratio) and alt (altitude) are 
//                   resampled by increments specified by parameter
//                   altIncrement. The units of this increment are 
//                   the same as parameter verticalLevelVariable( which is either in 
//                   mb for pressure levels or km for alt levels). 
//                   The sampling starts at parameter minVarLevel (if vertical
//                   level is altitude in meters) or maxVarLevel (if vertical
//                   level is pressure in mb) and increments or decrements 
//                   by parameter altIncrement. The assumption is made that 
//                   the data is relatively dense compared to the altIncrement.
//
//


void CapeCinSounding::resampleArrays()
{
  //
  // Allocate memory for temp storage of array data
  //
  float *temp_pr = new float[nz];

  float *temp_temp = new float[nz];

  float *temp_alt = new float[nz];

  float *temp_mixingr = new float[nz];

  float *temp_rh = new float[nz];
  float *temp_dewpt = new float[nz];
  float *temp_time = new float[nz];
  float *temp_u = new float[nz];
  float *temp_v = new float[nz];
  float *temp_w = new float[nz];
  float *temp_tempC = new float[nz];
  float *temp_windSpeed = new float[nz];
  float *temp_windDir = new float[nz];
  float *temp_ascensionRate = new float[nz];
  float *temp_longitude = new float[nz];
  float *temp_latitude = new float[nz];
	  

  int j = 0;
  if (verticalLevelVariable == PRESSURE)
    {

      //
      // resampling will start with the first element 
      // of each array as long as the pr[0] is less than 
      // maxVarLevel
      //
      if ( pr[0] < maxVarLevel)
	{
	  //
	  // record the first elements of the arrays
	  //
	  temp_pr[0] = pr[0];
	      
	  temp_temp[0] = temp[0];
	  
	  temp_alt[0] = alt[0];
	  
	  temp_mixingr[0] = mixingr[0];
	  
	  temp_rh[0] = rh[0];
	  temp_dewpt[0] = dewpt[0];
	  temp_time[0] = time[0];
	  temp_u[0] = u[0];
	  temp_v[0] = v[0];
	  temp_w[0] = w[0];
	  temp_tempC[0] = tempC[0];
	  temp_windSpeed[0] = windSpeed[0];
	  temp_windDir[0] = windDir[0];
	  temp_ascensionRate[0] = ascensionRate[0];
	  temp_longitude[0] = longitude[0];
	  temp_latitude[0] = latitude[0];

	  j = 1;
	}
      
      //
      // Given a starting level of maxVarLevel and
      // an increment of altIncrement we find 
      // the level closest to but less than the first pressure element 
      // and start sampling from there.
      //
      float pressure_level = maxVarLevel;

      while ( pressure_level > pr[0] )
	   pressure_level = pressure_level - altIncrement;

      //
      // Sample data at increments of altIncrement
      //
      for (int i = 1; i < nz ; i++)
	{
	  if ( pr[i] <=  pressure_level)
	    {
	      temp_pr[j] = pr[i];
	      
	      temp_temp[j] = temp[i];
	      
	      temp_alt[j] = alt[i];
	      
	      temp_mixingr[j] = mixingr[i];
	      
	      temp_rh[j] = rh[i];
	      temp_dewpt[j] = dewpt[i];
	      temp_time[j] = time[i];
	      temp_u[j] = u[i];
	      temp_v[j] = v[i];
	      temp_w[j] = w[i];
	      temp_tempC[j] = tempC[i];
	      temp_windSpeed[j] = windSpeed[i];
	      temp_windDir[j] = windDir[i];
	      temp_ascensionRate[j] = ascensionRate[i];
	      temp_longitude[j] = longitude[i];
	      temp_latitude[j] = latitude[i];
	      
	      j++;
	      
	      pressure_level = pressure_level - altIncrement;
	    } 
	 
	}
    }
  else
    {
      //
      // resampling will start with the first element 
      // of each array as long as the alt[0] is greater than  
      // bounds minVarLevel.
      //
      if ( alt[0] > minVarLevel)
	{
	  //
	  // record the first elements of the arrays
	  //
	  temp_pr[0] = pr[0];
	      
	  temp_temp[0] = temp[0];
	  
	  temp_alt[0] = alt[0];
	  
	  temp_mixingr[0] = mixingr[0];
	  
	  temp_rh[0] = rh[0];
	  temp_dewpt[0] = dewpt[0];
	  temp_time[0] = time[0];
	  temp_u[0] = u[0];
	  temp_v[0] = v[0];
	  temp_w[0] = w[0];
	  temp_tempC[0] = tempC[0];
	  temp_windSpeed[0] = windSpeed[0];
	  temp_windDir[0] = windDir[0];
	  temp_ascensionRate[0] = ascensionRate[0];
	  temp_longitude[0] = longitude[0];
	  temp_latitude[0] = latitude[0];

	  j = 1;
	}
     
      //
      // Given a starting level of minVarLevel and
      // an increment of altIncrement we find 
      // the level closest to but greater than the  first altitude element 
      // 
      float alt_level = minVarLevel;

      while ( alt_level < alt[0])
	alt_level = alt_level +  altIncrement;
      
      //
      // Sample the data at increments of altIncrement
      //
      for (int i = 1; i < nz ; i++)
	{
	  if ( alt[i] >=  alt_level)
	    {
	      temp_pr[j] = pr[i];
	      
	      temp_temp[j] = temp[i];
	      
	      temp_alt[j] = alt[i];
	      
	      temp_mixingr[j] = mixingr[i];
	      
	      temp_rh[j] = rh[i];
	      temp_dewpt[j] = dewpt[i];
	      temp_time[j] = time[i];
	      temp_u[j] = u[i];
	      temp_v[j] = v[i];
	      temp_w[j] = w[i];
	      temp_tempC[j] = tempC[i];
	      temp_windSpeed[j] = windSpeed[i];
	      temp_windDir[j] = windDir[i];
	      temp_ascensionRate[j] = ascensionRate[i];
	      temp_longitude[j] = longitude[i];
	      temp_latitude[j] = latitude[i];

	      j++;
	      
	      alt_level = alt_level + altIncrement;
	    }  
	}
    }

  //
  // Modify nz to new resampled data array size:
  //
  nz = j;

  //
  // Copy temp data to class member arrays( overwriting data)
  //
  for (int k = 0; k < nz; k++)
    {
      pr[k] = temp_pr[k];

      temp[k] = temp_temp[k];

      alt[k] = temp_alt[k];

      mixingr[k] = temp_mixingr[k];

      rh[k] = temp_rh[k];
      dewpt[k] = temp_dewpt[k];
      time[k] = temp_time[k];
      u[k] = temp_u[k];
      v[k] = temp_v[k];
      w[k] = temp_w[k];
      tempC[k] = temp_tempC[k];
      windSpeed[k] = temp_windSpeed[k];
      windDir[k] = temp_windDir[k];
      ascensionRate[k] = temp_ascensionRate[k];
      longitude[k] = temp_longitude[k];
      latitude[k] = temp_latitude[k];
    } 

  if (debug)
    {
      cerr << "CapeCinSounding::resampleArrays():alt increment: " <<  altIncrement << endl;
      cerr << "  Showing the first few elements of the resampled data arrays." << endl;

      int N;

      10 < nz ? N = 10: N = nz;

      for (int k = 0; k < N; k++)
	{
	  cerr << "pr["  << k << "] = " << pr[k] << " ";

	  cerr << "temp[" << k << "] = " << temp[k] << " ";

	  cerr << "alt["  << k << "] = " << alt[k] << " ";

	  cerr << "mixingr[" <<k << "] = " << mixingr[k] << endl;
	}
      cerr << "\n";
    }
  
  //
  // Cleanup
  //
  delete[] temp_pr;

  delete[] temp_temp;

  delete[] temp_alt;

  delete[] temp_mixingr;  

  delete[] temp_rh;
  delete[] temp_dewpt;
  delete[] temp_time;
  delete[] temp_u;
  delete[] temp_v;
  delete[] temp_w;
  delete[] temp_tempC;
  delete[] temp_windSpeed;
  delete[] temp_windDir;
  delete[] temp_ascensionRate;
  delete[] temp_longitude;
  delete[] temp_latitude;


}

////////////////////////////////////////////////////////////////////////////
//
// surfaceLayerAve(): Given the parameters surfaceLayerLowerBound and
//                    surfaceLayerUpperBound, we find the average of the
//                    variables pressure, temperature, altitude, and mixing 
//                    ratio in this layer. We determine the place
//                    in the pressure array where the average pressure of the surface
//                    layer falls and we keep only the data from     
//                    this index forward and rewrite all of the arrays ( pr, temp, mixingr,
//                    and alt) to start with the surface layer average of the variable 
//                    followed by the rest of the data.
//
int CapeCinSounding::surfaceLayerAve()
{
  //
  // Allocate memory for temp storage of array data
  //
  float *temp_pr = new float[nz];
  
  float *temp_temp = new float[nz];
  
  float *temp_alt = new float[nz];
  
  float *temp_mixingr = new float[nz];

  float *temp_rh = new float[nz];
  float *temp_dewpt = new float[nz];
  float *temp_time = new float[nz];
  float *temp_u = new float[nz];
  float *temp_v = new float[nz];
  float *temp_w = new float[nz];
  float *temp_tempC = new float[nz];
  float *temp_windSpeed = new float[nz];
  float *temp_windDir = new float[nz];
  float *temp_ascensionRate = new float[nz];
  float *temp_longitude = new float[nz];
  float *temp_latitude = new float[nz];

  //
  // Initialize variables
  //
  float p_total = 0;

  float t_total = 0;

  float alt_total = 0;
  
  float mixingr_total = 0;
  
  float rh_total = 0;
  float dewpt_total = 0;
  float time_total = 0;
  float u_total = 0;
  float v_total = 0;
  float w_total = 0;
  float tempC_total = 0;
  float windSpeed_total = 0;
  float windDir_total = 0;
  float ascensionRate_total = 0;
  float longitude_total = 0;
  float latitude_total = 0;


  int count = 0;
  
  //
  // Find averages of variables in specified surface layer.
  //
  if (verticalLevelVariable == PRESSURE)
    {
      for (int i = 0; i < nz; i++)
	{
	  if (  surfaceLayerLowerBound <= pr[i]  &&  
		pr[i] <= surfaceLayerUpperBound)
	     {
	       p_total += pr[i];

	       t_total += temp[i];

	       alt_total += alt[i];

	       mixingr_total += mixingr[i];

	       rh_total += rh[i];
	       dewpt_total += dewpt[i];
	       time_total += time[i];
	       u_total  += u[i];
	       v_total  += v[i];
	       w_total  += w[i];
	       tempC_total += tempC[i];
	       windSpeed_total += windSpeed[i];
	       windDir_total   += windDir[i];
	       ascensionRate_total += ascensionRate[i];
	       longitude_total += longitude[i];
	       latitude_total += latitude[i];

	       count++;
	     }
	  if ( pr[i] <= surfaceLayerLowerBound)
	    i = nz;
	}
    }
  else
    {
      //
      // The vertical level variable is ALTITUDE with units of m.
      //
      for (int i = 0; i < nz; i++)
	{
	  if (  surfaceLayerLowerBound <= alt[i]  &&  
		alt[i] <= surfaceLayerUpperBound)
	     {
	       p_total += pr[i];

	       t_total += temp[i];

	       alt_total += alt[i];

	       mixingr_total += mixingr[i];

	       rh_total += rh[i];
	       dewpt_total += dewpt[i];
	       time_total += time[i];
	       u_total  += u[i];
	       v_total  += v[i];
	       w_total  += w[i];
	       tempC_total += tempC[i];
	       windSpeed_total += windSpeed[i];
	       windDir_total   += windDir[i];
	       ascensionRate_total += ascensionRate[i];
	       longitude_total += longitude[i];
	       latitude_total += latitude[i];

	       count++;
	     }
	  if ( alt[i] >=  surfaceLayerUpperBound)
	    i = nz;
	}

    }

  if (count == 0)
    //
    // There isnt any data for specified surface layer
    //
    return(1);

  //
  // Determine variable averages
  //
  float p_ave = p_total/count;
  
  float t_ave = t_total/count;
  
  float alt_ave = alt_total/count;
  
  float mixingr_ave = mixingr_total/count;
  
  float rh_ave = rh_total/count ;
  float dewpt_ave = dewpt_total/count;
  float time_ave = time_total/count;
  float u_ave = u_total/count;
  float v_ave = v_total/count;
  float w_ave = w_total/count;
  float tempC_ave = tempC_total/count;
  float windSpeed_ave = windSpeed_total/count;
  float windDir_ave = windDir_total/count;
  float ascensionRate_ave = ascensionRate_total/count;
  float longitude_ave = longitude_total/count;
  float latitude_ave = latitude_total/count;

  //
  // The first element of the pressure, temp, mixing ration and alt arrays 
  // will be the averages of the surface layer.
  //
  temp_pr[0] = p_ave;

  temp_temp[0] = t_ave;

  temp_alt[0] = alt_ave;

  temp_mixingr[0] = mixingr_ave;
  
  temp_rh[0] = rh_ave;
  temp_dewpt[0] = dewpt_ave;
  temp_time[0] = time_ave;
  temp_u[0] = u_ave;
  temp_v[0] = v_ave;
  temp_w[0] = w_ave;
  temp_tempC[0] = tempC_ave;
  temp_windSpeed[0] = windSpeed_ave;
  temp_windDir[0] = windDir_ave;
  temp_ascensionRate[0] = ascensionRate_ave;
  temp_longitude[0] = longitude_ave;
  temp_latitude[0] = latitude_ave;

  count = 1;

  //
  // Fill temp arrays with the subset of data that we want
  // from the  pressure, temp, mixing ration and alt arrays.
  //
  for (int i = 0; i < nz; i ++)
    {
      if (p_ave > pr[i])
	{
	  temp_pr[count] = pr[i];
	  
	  temp_temp[count] = temp[i];
	  
	  temp_alt[count] = alt[i];
	  
	  temp_mixingr[count] = mixingr[i];
	  
	  temp_rh[count] = rh[i];
	  temp_dewpt[count] = dewpt[i];
	  temp_time[count] = time[i];
	  temp_u[count] = u[i];
	  temp_v[count] = v[i];
	  temp_w[count] = w[i];
	  temp_tempC[count] = tempC[i];
	  temp_windSpeed[count] = windSpeed[i];
	  temp_windDir[count] = windDir[i];
	  temp_ascensionRate[count] = ascensionRate[i];
	  temp_longitude[count] = longitude[i];
	  temp_latitude[count] = latitude[i];

	  count++;
	}
    }
  
  //
  // Modify the total number of elements in the array.
  //
  nz = count;
  
  //
  // Overwrite class arrays (pressure, temp, alt, and mixing ratio)
  // with the temp array data.
  // 
  for (int i = 0; i < nz ; i++)
    {
      pr[i] =  temp_pr[i];
      
      temp[i] = temp_temp[i];
      
      alt[i] = temp_alt[i];
      
      mixingr[i] = temp_mixingr[i];

      rh[i] = temp_rh[i];
      dewpt[i] = temp_dewpt[i];
      time[i] = temp_time[i];
      u[i] = temp_u[i];
      v[i] = temp_v[i];
      w[i] = temp_w[i];
      tempC[i] = temp_tempC[i];
      windSpeed[i] = temp_windSpeed[i];
      windDir[i] = temp_windDir[i];
      ascensionRate[i] = temp_ascensionRate[i];
      longitude[i] = temp_longitude[i];
      latitude[i] = temp_latitude[i];
    }
  
  
  if (debug)
    {
      cerr << "CapeCinSounding::surfaceLayerAve(): \n"
	   << "  Surface layer lower bound : " << surfaceLayerLowerBound << endl
	   << "  Surface layer upper bound : " << surfaceLayerUpperBound << endl
	   << "  Showing the first few levels after specified surface layer averaging." << endl;
      
      int N;

      10 < nz ? N = 10: N = nz;

      for (int k = 0; k < N; k++)
	{
	  cerr << "pr["  << k << "] = " << pr[k] << " ";
	  
	  cerr << "temp[" << k << "] = " << temp[k] << " ";
	  
	  cerr << "alt["  << k << "] = " << alt[k] << " ";
	  
	  cerr << "mixingr[" << k << "] = " << mixingr[k] << endl;
	 
	}
    }

  //
  // Cleanup
  //
  delete[] temp_pr;
  
  delete[] temp_temp;
  
  delete[] temp_alt;
  
  delete[] temp_mixingr;  
  
  delete[] temp_rh;
  delete[] temp_dewpt;
  delete[] temp_time;
  delete[] temp_u;
  delete[] temp_v;
  delete[] temp_w;
  delete[] temp_tempC;
  delete[] temp_windSpeed;
  delete[] temp_windDir;
  delete[] temp_ascensionRate;
  delete[] temp_longitude;
  delete[] temp_latitude;


  return 0;
}


int CapeCinSounding::surfaceObsCapeCin(float p, float t, float a, float q)
{

  if (pr == NULL || alt == NULL || mixingr == NULL || temp == NULL)
    {
      cerr << "CapeCinSounding::calcCapeCin() -- Error. Cannot compute "
	   << "cape and cin. NULL input array(s) found.\n";
      return 1;
    }

  //
  // Allocate memory for temp storage of array data
  //
  float *temp_pr = new float[nz + 1];
  
  float *temp_temp = new float[nz + 1];
  
  float *temp_alt = new float[nz + 1];
  
  float *temp_mixingr = new float[nz + 1];

  float *temp_rh = new float[nz + 1];
  float *temp_dewpt = new float[nz + 1];
  float *temp_time = new float[nz + 1];
  float *temp_u = new float[nz +1];
  float *temp_v = new float[nz +1];
  float *temp_w = new float[nz +1];
  float *temp_tempC = new float[nz +1];
  float *temp_windSpeed = new float[nz +1];
  float *temp_windDir = new float[nz +1];
  float *temp_ascensionRate = new float[nz +1];
  float *temp_longitude = new float[nz +1];
  float *temp_latitude = new float[nz +1];

  //
  // The first element of the pressure, temp, mixing ratio and alt arrays 
  // will be the input args.
  //
  temp_pr[0] = p;

  temp_temp[0] = t;

  temp_alt[0] = a;

  temp_mixingr[0] = q;
  
  temp_rh[0] = Sndg::VALUE_UNKNOWN;
  temp_dewpt[0] = Sndg::VALUE_UNKNOWN;
  temp_time[0] = Sndg::VALUE_UNKNOWN;
  temp_u[0] = Sndg::VALUE_UNKNOWN;
  temp_v[0] = Sndg::VALUE_UNKNOWN;
  temp_w[0] = Sndg::VALUE_UNKNOWN;
  temp_tempC[0] =  Sndg::VALUE_UNKNOWN;
  temp_windSpeed[0] =  Sndg::VALUE_UNKNOWN;
  temp_windDir[0] =  Sndg::VALUE_UNKNOWN;
  temp_ascensionRate[0] = Sndg::VALUE_UNKNOWN;
  temp_longitude[0] =  Sndg::VALUE_UNKNOWN;
  temp_latitude[0] =  Sndg::VALUE_UNKNOWN;

  //
  // Fill temp arrays with the subset of data that we want
  // from the  pressure, temp, mixing ratio and alt arrays.
  // We determine where the input pressure falls 
  // in the pressure array and keep all data after that index.
  //

  int count = 1;

  for (int i = 0; i < nz; i ++)
    {
      if (p > pr[i])
	{
	  temp_pr[count] = pr[i];
	  
	  temp_temp[count] = temp[i];
	  
	  temp_alt[count] = alt[i];
	  
	  temp_mixingr[count] = mixingr[i];
	  
	  temp_rh[count] = rh[i];
	  temp_dewpt[count] = dewpt[i];
	  temp_time[count] = time[i];
	  temp_u[count] = u[i];
	  temp_v[count] = v[i];
	  temp_w[count] = w[i];
	  temp_tempC[count] = tempC[i];
	  temp_windSpeed[count] = windSpeed[i];
	  temp_windDir[count] = windDir[i];
	  temp_ascensionRate[count] = ascensionRate[i];
	  temp_longitude[count] = longitude[i];
	  temp_latitude[count] = latitude[i];

	  count++;
	}
    }
  
  //
  // Modify the total number of elements in the array.
  //
  if (count > nz)
    {
       
      //
      // reallocate memory if the data count is greater than nz
      //
      delete[] pr;
      delete[] temp;
      delete[] alt;
      delete[] mixingr;
      delete[] rh;
      delete[] dewpt;
      delete[] time;
      delete[] u;
      delete[] v;
      delete[] w;
      delete[] tempC;
      delete[] windSpeed;
      delete[] windDir;
      delete[] ascensionRate;
      delete[] longitude;
      delete[] latitude;

      pr = new float[count];
      temp = new float[count];
      alt = new float[count];
      mixingr = new float[count];
      rh = new float[count];
      dewpt = new float[count];
      time = new float[count];
      u = new float[count];
      v = new float[count];
      w = new float[count];
      tempC = new float[count];
      windSpeed = new float[count];
      windDir = new float[count];
      ascensionRate = new float[count];
      longitude = new float[count];
      latitude = new float[count];
 
    }

  nz = count;
  
  //
  // Overwrite class arrays (pressure, temp, alt, and mixing ratio)
  // with the temp array data.
  // 
  for (int i = 0; i < nz ; i++)
    {
      pr[i] =  temp_pr[i];
      
      temp[i] = temp_temp[i];
      
      alt[i] = temp_alt[i];
      
      mixingr[i] = temp_mixingr[i];
      
      rh[i] = temp_rh[i];
      dewpt[i] = temp_dewpt[i];
      time[i] = temp_time[i];
      u[i] = temp_u[i];
      v[i] = temp_v[i];
      w[i] = temp_w[i];
      tempC[i] = temp_tempC[i];
      windSpeed[i] = temp_windSpeed[i];
      windDir[i] = temp_windDir[i];
      ascensionRate[i] = temp_ascensionRate[i];
      longitude[i] = temp_longitude[i];
      latitude[i] = temp_latitude[i];

    }
  
  
  if (debug)
    {
      cerr << "CapeCinSounding::surfaceObsCapeCin(): " 
	   << "Showing the first few levels after specified surface input." << endl;
      
      int N;

      10 < nz ? N = 10: N = nz;

      for (int k = 0; k < N; k++)
	{
	  cerr << "pr["  << k << "] = " << pr[k] << " ";
	  
	  cerr << "temp[" << k << "] = " << temp[k] << " ";
	  
	  cerr << "alt["  << k << "] = " << alt[k] << " ";
	  
	  cerr << "mixingr[" << k << "] = " << mixingr[k] << endl;
	 
	}
    }

  //
  // We want to calc cape and cin starting from input pressure.
  // Setting this flag insures we will calculate cape and cin starting
  // from index 0;
  //
  override_least_calc_index = true;

  int ret = callCalcCapeCin3D();
  
  if (ret)
    {
      cerr << "Error calling CapeCin3D";
      return 1;
    }
  
  //
  // Cleanup
  //
  delete[] temp_pr;
  
  delete[] temp_temp;
  
  delete[] temp_alt;
  
  delete[] temp_mixingr;  
  
  delete[] temp_rh;
  delete[] temp_dewpt;
  delete[] temp_time;
  delete[] temp_u;
  delete[] temp_v;
  delete[] temp_w;
  delete[] temp_tempC;
  delete[] temp_windSpeed;
  delete[] temp_windDir;
  delete[] temp_ascensionRate;
  delete[] temp_longitude;
  delete[] temp_latitude;

  return 0; 

}

//
// createNewSounding(): 
// Takes the modified sounding variable 
// arrays plus the cape and cin arrays and constructs a new
// sounding. The cape and the cin occupy the first two elements
// of the Sndg::point_t.spareFloats.
// NOTE: IF CAPE == 0, CIN IS SET TO Sndg::VALUE_UNKNOWN;
//
void CapeCinSounding::createNewSounding()

{
  sprintf(newSndgHeader.sourceName,"Created by CapeCinSounding");
  
  newSndgHeader.nPoints = nz; 
  
  newSndg.setHeader(newSndgHeader);

  newSndg.clearPoints();

  for(int i = 0; i < nz; i++)
    {
      //
      // create new sounding point
      //
      Sndg::point_t *sndgPt = new Sndg::point_t;

      sndgPt->time = time[i];
      sndgPt->pressure = pr[i];
      sndgPt->altitude = alt[i];
      sndgPt->u = u[i];
      sndgPt->v = v[i];
      sndgPt->w = w[i];
      sndgPt->rh = rh[i];
      sndgPt->temp = tempC[i];
      sndgPt->dewpt = dewpt[i];
      sndgPt->windDir = windDir[i];
      sndgPt->ascensionRate = ascensionRate[i];
      sndgPt->longitude = longitude[i];
      sndgPt->latitude = latitude[i];
      sndgPt->pressureQC = Sndg::QC_MISSING;
      sndgPt->tempQC = Sndg::QC_MISSING;
      sndgPt->humidityQC = Sndg::QC_MISSING;
      sndgPt->uwindQC = Sndg::QC_MISSING;
      sndgPt->vwindQC = Sndg::QC_MISSING;
      sndgPt->ascensionRateQC = Sndg::QC_MISSING;
      
      if ( min_calc_index <=  i  && i <= max_calc_index )
	{
	  //
	  // Note that indices for which cape and cin are computed correspond
	  // with the indices between min_calc_index and max_calc_index.
	  //
	  sndgPt->spareFloats[0] = cape[i - min_calc_index];
	  
	  //
	  // If cape is zero set cin to unknown value
	  //
	  if (fabs (cape[i - min_calc_index ]) < .001)
	    sndgPt->spareFloats[1] = Sndg::VALUE_UNKNOWN;
	  else
	    sndgPt->spareFloats[1] = cin[i - min_calc_index];
	}
      else
	{
	  sndgPt->spareFloats[0] = Sndg::VALUE_UNKNOWN;
          sndgPt->spareFloats[1] = Sndg::VALUE_UNKNOWN;
	}
	
      for (int i = 2; i < 10; i++)
	sndgPt->spareFloats[i] = Sndg::VALUE_UNKNOWN;      
      
      newSndg.addPoint(*sndgPt);

      pointsVec.push_back(sndgPt);
    } 

}










