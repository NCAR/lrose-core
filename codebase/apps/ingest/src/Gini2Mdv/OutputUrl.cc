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

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
/*
 *  $Id: OutputUrl.cc,v 1.21 2016/03/07 01:23:01 dixon Exp $
 *
 */

/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/////////////////////////////////////////////////////////////////////////
//
// Class:	OutputUrl
//
// Author:	G M Cunning, Modified for Gini2Mdv by S. Mueller
//
// Date:	Jan 2001 (modified June 2006)
//
// Description: This class manages the output MDV data.
//
//

// C++ include files
#include <cstdio>

// System/RAP include files
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxProj.hh>
#include <toolsa/str.h>
#include <toolsa/toolsa_macros.h>

// Local include files
#include "OutputUrl.hh"
#include "Params.hh"
#include "GiniPDB.hh"
#include "GiniCalibrationCurve.hh"
#include "InputManager.hh"
#include "ProcessGiniFile.hh"

using namespace std;


// define any constants
const string OutputUrl::_className    = "OutputUrl";


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Constructors
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
  
OutputUrl::OutputUrl(Params *params)
   {
   _url = params->output_url;
   _params = params;
   _mdvx = new DsMdvx();
   if(0 == _mdvx) // 0 is NULL
      {
      cerr << "ERROR: OutputUrl Constructor" << endl;
      cerr << "   Could not instantiate DsMdvx object" << endl;
      cerr << "   EXITING" << endl;
      exit(-1);
      }

   _mdvx->setWriteLdataInfo();

   _mdvx->setDebug((int)params->debug);

   } // End of OutputUrl constructor.


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Destructors
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
  
OutputUrl::~OutputUrl()
   {
   delete _mdvx;
   _params = 0;
   } // End of OutputUrl destructor.


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Public Methods
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////
//
// Method Name:	OutputUrl::setMdvMasterHeader
//
// Description:	This method initializes the headers for the ouptu MDV
//		data.
//
// Returns:	
//
// Globals:	
//
// Notes:
//
//

void OutputUrl::setMdvMasterHeader()
   {
   const string methodName = "OutputUrl::setMdvMasterHeader()";

   // Modeled after MdvCombine->OuputUrl::initMasterHeader(...)
   //  Name was changed because the new method takes no arguments and,
   //  for this reason, is substantially different than initMasterHeader().
   // clear the master header
   memset(&_mdvMasterHeader, 0, sizeof(Mdvx::master_header_t));

   // fill the master header
  
   _mdvMasterHeader.num_data_times = 1;

   _mdvMasterHeader.data_dimension = 2;


   _mdvMasterHeader.native_vlevel_type = Mdvx::VERT_TYPE_SURFACE;
   _mdvMasterHeader.vlevel_type        = Mdvx::VERT_TYPE_SURFACE;
   _mdvMasterHeader.vlevel_included = 1;
   _mdvMasterHeader.grid_orientation = Mdvx::ORIENT_SN_WE;
   _mdvMasterHeader.data_ordering = Mdvx::ORDER_XYZ;
   _mdvMasterHeader.n_fields = _params->input_data_array_n;
   _mdvMasterHeader.n_chunks = 0;
   _mdvMasterHeader.field_grids_differ = 0;
   _mdvMasterHeader.sensor_lon = 0.0;
   _mdvMasterHeader.sensor_lat = 0.0;
   _mdvMasterHeader.sensor_alt = 0.0;

   // data set name and source
   STRncopy(_mdvMasterHeader.data_set_name, _params->data_set_name, MDV_NAME_LEN);
   STRncopy(_mdvMasterHeader.data_set_source, _params->data_set_source, MDV_NAME_LEN);
   STRncopy(_mdvMasterHeader.data_set_info, "Combined data set from the following files:\n", MDV_INFO_LEN); 
   _mdvx->setMasterHeader(_mdvMasterHeader);
   } // End of setMdvMasterHeader() method.


/////////////////////////////////////////////////////////////////////////
//
// Method Name:	OutputUrl::setTimes
//
// Description:	
//
// Returns:	
//
// Globals:	
//
// Notes:
//
//

void OutputUrl::setTimes(const time_t& centroid, const time_t& begin, const time_t& end)
   {
   const string methodName = "OutputUrl::setTimes()";

   _mdvMasterHeader.time_gen   = time(0);
   _mdvMasterHeader.time_centroid = centroid; // Used to set date subdirectory and file name.
   if(begin == -1) 
     {
     _mdvMasterHeader.time_begin = centroid;     
     }
   else
     {
     _mdvMasterHeader.time_begin = begin;
     }

   if(end == -1) 
     {
     _mdvMasterHeader.time_end = centroid;
     }
   else
     {
     _mdvMasterHeader.time_end = end;
     }
     
   }
/////////////////////////////////////////////////////////////////////////
//
// Method Name:	OutputUrl::buildMdvFieldHeader
//
// Description:	This method initializes the headers for the output MDV data.
//
// Returns:	
//
// Globals:	
//
// Notes:
//
//

Mdvx::field_header_t OutputUrl::buildMdvFieldHeader(GiniPDB *giniPDB, float badDataValue)
   {
   const string methodName = "OutputUrl::buildMdvFieldHeader()";

   Mdvx::field_header_t mdvFieldHeader;
   MEM_zero(mdvFieldHeader);

   mdvFieldHeader.forecast_delta = 0;
   mdvFieldHeader.forecast_time = _mdvMasterHeader.time_centroid;
   mdvFieldHeader.nx = giniPDB->getNx();
   mdvFieldHeader.ny = giniPDB->getNy();
   mdvFieldHeader.nz = 1;
   mdvFieldHeader.grid_dx = giniPDB->getDx()/1000.0;
   mdvFieldHeader.grid_dy = giniPDB->getDy()/1000.0;
   mdvFieldHeader.grid_dz = 0.0;

   MdvxProj proj;
   double x0 = 0.0;
   double y0 = 0.0;
     
   // Projection parameters
   if(giniPDB->getProjection() == GiniLambertProj)
      {
      mdvFieldHeader.proj_type       = Mdvx::PROJ_LAMBERT_CONF;
      mdvFieldHeader.proj_origin_lat = giniPDB->getLatin();
      mdvFieldHeader.proj_origin_lon = giniPDB->getLov();
      mdvFieldHeader.proj_param[0] = giniPDB->getLatin();
      mdvFieldHeader.proj_param[1] = giniPDB->getLatin();

      proj.initLc2(giniPDB->getLatin(), giniPDB->getLov(), giniPDB->getLatin(), giniPDB->getLatin());
      proj.latlon2xy(giniPDB->getLat1(), giniPDB->getLon1(), x0, y0);
      }
   else if(giniPDB->getProjection() == GiniPolarStereoProj)
      {
      double tanLat = 90.0;
      double tanLon = giniPDB->getLov();
      Mdvx::pole_type_t poleType = Mdvx::POLE_NORTH;
      // Gini Polar is secant at 60N instead of the normal tangent at the north pole.
      // As a result, the dx and dy are off by a scale factor which must be adjusted for here.
      // The scale adjustment factor is calculated by Bosowski & Feeman, 1998 as:
      // (1 + cos( PI / 180.0 * ( originLat - secantLat ) ) ) / 2.0
      // with originLat = 90 and secantLat = 60 
      // centralScale = 0.9330127018922193; 

      double secantLat = 60.0;
      double centralScale =
        (1.0 + cos(DEG_TO_RAD * (tanLat - secantLat))) / 2.0;

      mdvFieldHeader.proj_type = Mdvx::PROJ_POLAR_STEREO;
      mdvFieldHeader.proj_origin_lat = tanLat;
      mdvFieldHeader.proj_origin_lon = tanLon;
      mdvFieldHeader.proj_param[0] = tanLon;
      mdvFieldHeader.proj_param[1] = poleType;
      mdvFieldHeader.proj_param[2] = centralScale;

      proj.initPolarStereo(tanLon, poleType, centralScale);
      proj.latlon2xy(giniPDB->getLat1(), giniPDB->getLon1(), x0, y0);
      }
   else if(giniPDB->getProjection() == GiniMercatorProj)
      {
      mdvFieldHeader.proj_type = Mdvx::PROJ_MERCATOR;
      mdvFieldHeader.proj_origin_lat = giniPDB->getLat1();
      mdvFieldHeader.proj_origin_lon = giniPDB->getLon1();
      float dx = fabs(giniPDB->getLon1() - giniPDB->getLon2())/static_cast<float>(giniPDB->getNx());
      float dy = fabs(giniPDB->getLat1() - giniPDB->getLat2())/static_cast<float>(giniPDB->getNy());

      mdvFieldHeader.grid_dx = dx;
      mdvFieldHeader.grid_dy = dy;

      }

   mdvFieldHeader.grid_minx = x0;
   mdvFieldHeader.grid_miny = y0;
   mdvFieldHeader.grid_minz = 0.0;
   mdvFieldHeader.encoding_type = Mdvx::ENCODING_FLOAT32;
   mdvFieldHeader.compression_type = Mdvx::COMPRESSION_NONE;
   mdvFieldHeader.transform_type = Mdvx::DATA_TRANSFORM_NONE;
   mdvFieldHeader.scaling_type = Mdvx::SCALING_NONE;
   mdvFieldHeader.native_vlevel_type = Mdvx::VERT_TYPE_SURFACE;
   mdvFieldHeader.vlevel_type = Mdvx::VERT_TYPE_SURFACE;

   mdvFieldHeader.bad_data_value = badDataValue;
   mdvFieldHeader.missing_data_value = badDataValue;

   mdvFieldHeader.proj_rotation = 0.0;
   mdvFieldHeader.data_element_nbytes = sizeof(fl32);
   mdvFieldHeader.volume_size = (mdvFieldHeader.data_element_nbytes*mdvFieldHeader.nx*mdvFieldHeader.ny*mdvFieldHeader.nz);
   //mdvFieldHeader.

   return mdvFieldHeader;
   } // End of buildMdvFieldHeader() method.


/////////////////////////////////////////////////////////////////////////
//
// Method Name:	OutputUrl::buildMdvVlevelHeader
//
// Description:	
// Returns:	
//
// Globals:	
//
// Notes:
//
//

Mdvx::vlevel_header_t OutputUrl::buildMdvVlevelHeader()
   {
   const string methodName = "OutputUrl::buildMdvVlevelHeader()";

   Mdvx::vlevel_header_t mdvVlevelHeader;
   MEM_zero(mdvVlevelHeader);
   mdvVlevelHeader.type[0]  = Mdvx::VERT_TYPE_SURFACE;
   mdvVlevelHeader.level[0] = 0.0;

   return mdvVlevelHeader;
   } // End of buildMdvVlevelHeader() method.


/////////////////////////////////////////////////////////////////////////
//
// Method Name:	OutputUrl::buildMdvField
//
// Description:	
//
// Returns:	
//
// Globals:	
//
// Notes:
//
//

MdvxField* OutputUrl::buildMdvField(ProcessGiniFile *processGiniFile,
                                    InputManager *inputManager,
                                    GiniCalibrationCurve *calibrationCurve,
                                    Params *params)
   {
   const string methodName = "OutputUrl::buildMdvField()";

   // Construct field header
   Mdvx::field_header_t mdvFieldHeader = buildMdvFieldHeader(processGiniFile->getDecodedProductDefinitionBlock(), calibrationCurve->getBadDataValue());

   // Construct vlevel header
   Mdvx::vlevel_header_t mdvVlevelHeader = buildMdvVlevelHeader();

   // Check for incompatible projection

   // Check for invalid input scans
   if(!processGiniFile->getScanLeftToRight())
      {
      cerr << "ERROR: " << methodName << endl;
      cerr << "   Gini image not scanned left to right. Aborting." << endl;
      return(0);
      }
   if(!processGiniFile->getScanXInFirst())
      {
      cerr << "ERROR: " << methodName << endl;
      cerr << "   Gini image does not vary most rapidly in X. Aborting." << endl;
      return(0);
      }

   // If original scan was ordered top-to-bottom (north-to-south), flip the data "vertically."
   if(processGiniFile->getScanTopToBottom())
      {
      fl32 *calibratedDataBuffer;
      calibratedDataBuffer = processGiniFile->getCalibratedDataBuffer();
      fl32 *tmpDataBuffer;
      tmpDataBuffer = new fl32[processGiniFile->getNx()*processGiniFile->getNy()];
      if(0 == tmpDataBuffer) // 0 is NULL
         {
         cerr << "ERROR: " << methodName << endl;
         cerr << "   Could not allocate memory for inversion of raw data" << endl;
         return(0);
         }

      // Store reorganized data in temporary buffer.
      for(int indexX=0; indexX<processGiniFile->getNx(); indexX++)
         {
         for(int indexY=0; indexY<processGiniFile->getNy(); indexY++)
            {
            int oldIndex = indexY*processGiniFile->getNx() + indexX;
            int newIndex = (processGiniFile->getNy() - 1 - indexY)*processGiniFile->getNx() + indexX;
            tmpDataBuffer[newIndex] = calibratedDataBuffer[oldIndex];
            }
         }

      // Copy contents of temporary buffer to _rawDataBuffer.
      for(int indexGrid=0; indexGrid < processGiniFile->getNx()*processGiniFile->getNy(); indexGrid++)
         {
         calibratedDataBuffer[indexGrid] = tmpDataBuffer[indexGrid];
         }
      processGiniFile->setCalibratedDataBuffer(calibratedDataBuffer);

      // Cleanup memory
      delete [] tmpDataBuffer;
      }

   // Create Mdv field
   MdvxField *mdvField = new MdvxField(mdvFieldHeader, mdvVlevelHeader, processGiniFile->getCalibratedDataBuffer());
   if(0 == mdvField) // 0 is NULL
      {
      cerr << "ERROR: " << methodName << endl;
      cerr << "   Could not instantiate MdvxField object" << endl;
      cerr << "   EXITING" << endl;
      exit(-1);
      }
   mdvField->setFieldName(inputManager->getShortMdvFieldName().c_str());
   mdvField->setFieldNameLong(inputManager->getLongMdvFieldName().c_str());
   mdvField->setUnits(calibrationCurve->getUnits().c_str());

   if(params->GridRemap)
      {
      MdvxRemapLut lut;

      switch(params->GridProjection)
         {
         case Params::PROJ_FLAT :
            if(mdvField->remap2Flat(lut,
                                    params->GridParams.nx,
                                    params->GridParams.ny,
                                    params->GridParams.minx,
                                    params->GridParams.miny,
                                    params->GridParams.dx,
                                    params->GridParams.dy,
                                    params->GridOrigin.lat,
                                    params->GridOrigin.lon,
                                    0.0))
               {
               cerr << "ERROR: " << methodName << endl;
               cerr << "   Error on remap to new grid" << endl;
               return 0; // 0 is NULL
               }
            break;

         case Params::PROJ_LATLON :
            if(mdvField->remap2Latlon(lut,
                                    params->GridParams.nx,
                                    params->GridParams.ny,
                                    params->GridParams.minx,
                                    params->GridParams.miny,
                                    params->GridParams.dx,
                                    params->GridParams.dy))
               {
               cerr << "ERROR: " << methodName << endl;
               cerr << "   Error on remap to new grid" << endl;
               return 0; // 0 is NULL
               }
            break;

         case Params::PROJ_LAMBERT :
            if(mdvField->remap2Lc2(lut,
                                    params->GridParams.nx,
                                    params->GridParams.ny,
                                    params->GridParams.minx,
                                    params->GridParams.miny,
                                    params->GridParams.dx,
                                    params->GridParams.dy,
                                    params->GridOrigin.lat,
                                    params->GridOrigin.lon,
				    params->GridLambert_lat1,
				    params->GridLambert_lat2))
               {
               cerr << "ERROR: " << methodName << endl;
               cerr << "   Error on remap to new grid" << endl;
               return 0; // 0 is NULL
               }
            break;

         default:
            cerr << "ERROR: " << methodName << endl;
            cerr << "   Unsupported grid remapping - default to native grid" << endl;
            return 0; // 0 is NULL
         }
      }

   if(mdvField->convertType(Mdvx::ENCODING_INT8, Mdvx::COMPRESSION_ZLIB, Mdvx::SCALING_ROUNDED))
      {
      cerr << "ERROR: " << methodName << endl;
      cerr << "   Unsupported grid remapping - default to native grid" << endl;
      return 0; // 0 is NULL
      }

   return mdvField;
   } // End of buildMdvField() method.


/////////////////////////////////////////////////////////////////////////
//
// Method Name:	OutputUrl::addToInfo
//
// Description:	concatenates string to data_set_info field of master header.
//
// Returns:	
//
// Globals:	
//
// Notes:
//
//

void OutputUrl::addToInfo(const char *info_str)
   {
   const string methodName = "OutputUrl::addToInfo()";

   STRconcat(_mdvMasterHeader.data_set_info, info_str, MDV_INFO_LEN);
   _mdvx->setMasterHeader(_mdvMasterHeader);
   } // End of addToInfo() method.


void OutputUrl::addToInfo(const string &info_str)
   {
   addToInfo(info_str.c_str());
   } // End of addToInfo() method.


/////////////////////////////////////////////////////////////////////////
//
// Method Name:	 OutputUrl::clear
//
// Description:	clear out the current information in the output URL
//
// Returns:	
//
// Globals:	
//
// Notes:
//
//

void OutputUrl::clear(void)
   {
   const string methodName = "OutputUrl::clear()";

   _mdvx->clearFields();
   _mdvx->clearErrStr();
   } // End of clear() method.


/////////////////////////////////////////////////////////////////////////
//
// Method Name:	 OutputUrl::addField
//
// Description:	~description
//
// Returns:	
//
// Globals:	
//
// Notes:
//
//

void OutputUrl::addField(MdvxField* in_field)
   {
   const string methodName = "OutputUrl::addField()";

   _mdvx->addField(in_field);
   } // End of addField() method.


/////////////////////////////////////////////////////////////////////////
//
// Method Name:	 OutputUrl::writeVol
//
// Description:	this method writes the new output volume.
//
// Returns:	
//
// Globals:	
//
// Notes:
//
//

bool OutputUrl::writeVol()
   {
   const string methodName = "OutputUrl::writeVol()";

   if (_mdvx->writeToDir(_url) != 0)
      {
      cerr << "ERROR: " << methodName << endl;
      cerr << "   WARNING: " << endl;
      cerr << "   Error writing data for time ";
      cerr << utimstr(_mdvMasterHeader.time_centroid);
      cerr << " to URL " << _url << endl;
      cerr << "   EXITING" << endl << endl;
      return false;
      }
   else
     {
       cout << "Wrote file to " << _mdvx->getPathInUse() << endl;
     }

   return true;
   } // End of writeVol() method.
