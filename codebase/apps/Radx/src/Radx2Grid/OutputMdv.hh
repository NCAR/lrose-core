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
// OutputMdv.hh
//
// OutputMdv class - handles debug output to MDV files
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2012
//
///////////////////////////////////////////////////////////////

#ifndef OutputMdv_HH
#define OutputMdv_HH

#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxRemapLut.hh>
#include <Radx/RadxVol.hh>
#include <Radx/RadxRcalib.hh>
#include <rapformats/DsRadarParams.hh>
#include <rapformats/DsRadarCalib.hh>
#include <radar/ConvStrat.hh>
#include "Params.hh"
using namespace std;

class OutputMdv {
  
public:
  
  // constructor
  
  OutputMdv(const string &prog_name,
	    const Params &params);
  
  // destructor
  
  virtual ~OutputMdv();
  
  // set the master header
  
  void setMasterHeader(const RadxVol &vol);

  // addField()
  
  void addField(const RadxVol &vol,
                MdvxProj &proj,
                const vector<double> &vlevels,
                const string &field_name,
                const string &field_name_long,
		const string &units,
		Radx::DataType_t inputDataType,
                double inputScale,
                double inputOffset,
                fl32 missingVal,
		const fl32 *data);

  void addConvStratBool(const RadxVol &vol,
                        MdvxProj &proj,
                        const string &field_name,
                        const string &field_name_long,
                        ui08 missingVal,
                        const ui08 *data);
  
  void addConvStratFields(const ConvStrat &convStrat,
                          const RadxVol &vol,
                          MdvxProj &proj,
                          const vector<double> &vlevels);
  
  // add the radarParams as a chunk

  void addChunks(const RadxVol &vol, int nFields);

  // write out merged volume
  
  int writeVol();

  // get valid time of MDV data set
  
  inline time_t getValidTime() const { return _mdvx.getValidTime(); }

  // get path in use

  inline string getPathInUse() const { return _mdvx.getPathInUse(); }

protected:
  
private:
  
  const string _progName;
  const Params &_params;
  DsMdvx _mdvx;
  string _outputDir;
  MdvxRemapLut _remapLut;

  int _getDsrParams(const RadxVol &vol, DsRadarParams &rparams);
  int _getDsrCal(const RadxVol &vol, DsRadarCalib &dsrCal);
  int _getDsRadarType(Radx::PlatformType_t ptype);
  int _getDsScanMode(Radx::SweepMode_t mode);
  int _getDsFollowMode(Radx::FollowMode_t mode);
  int _getDsPolarizationMode(Radx::PolarizationMode_t mode);
  int _getDsPrfMode(Radx::PrtMode_t mode, double prtRatio);
  
  int _writeAsCfNetCDF();
  int _writeAsZebraNetCDF();
  int _writeAsMdv();


  string _computeCfNetcdfPath();
  void _writeLdataInfo(const string &outputPath);
  
  int _writeZebraLatLonNetCDF();
  int _writeZebraXYNetCDF();

};

#endif

