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
///////////////////////////////////////////////////////////////////////////////
//
// RadarCalib.cc
//
// Radar calibration support
//
// Mike Dixon, EOL, NCAR, Bouler, CO, USA
//
// Oct 2018
//
///////////////////////////////////////////////////////////////////////////////
//
// There are 3 versions of the radar calibration object:
// 
//   libs/rapformats: DsRadarCalib
//   libs/Radx:       RadxRcalib
//   libs/radar:      IwrfCalib
//
// There are good reasons for this setup, but it does make it complicated
// to manage the various classes, and to copy the data from one class to
// another.
//
// This class provides the methods for copying contents from one
// class to another.
//
///////////////////////////////////////////////////////////////////////////////

#include <dataport/bigend.h>
#include <toolsa/DateTime.hh>
#include <toolsa/TaXml.hh>
#include <toolsa/TaArray.hh>
#include <toolsa/TaStr.hh>
#include <toolsa/file_io.h>
#include <toolsa/ugetenv.hh>
#include <radar/RadarCalib.hh>
#include <radar/IwrfCalib.hh>
#include <radar/iwrf_functions.hh>
#include <rapformats/DsRadarCalib.hh>
#include <Radx/RadxRcalib.hh>
#include <iostream>
#include <cmath>
#include <cerrno>
using namespace std;

///////////////
// constructor

RadarCalib::RadarCalib()
{
}

/////////////
// destructor

RadarCalib::~RadarCalib()
{
}

////////////////////////////////////////////////////////////////
// copy from DsRadarCalib to IwrfCalib

void RadarCalib::copyDsRadarToIwrf(const DsRadarCalib &dsCalib,
                                   IwrfCalib &iwrfCalib)
{

  iwrfCalib.setRadarName(dsCalib.getRadarName());
  iwrfCalib.setCalibTime(dsCalib.getCalibTime());
  iwrfCalib.setWavelengthCm(dsCalib.getWavelengthCm());
  iwrfCalib.setBeamWidthDegH(dsCalib.getBeamWidthDegH());
  iwrfCalib.setBeamWidthDegV(dsCalib.getBeamWidthDegV());
  iwrfCalib.setAntGainDbH(dsCalib.getAntGainDbH());
  iwrfCalib.setAntGainDbV(dsCalib.getAntGainDbV());
  iwrfCalib.setPulseWidthUs(dsCalib.getPulseWidthUs());
  iwrfCalib.setXmitPowerDbmH(dsCalib.getXmitPowerDbmH());
  iwrfCalib.setXmitPowerDbmV(dsCalib.getXmitPowerDbmV());
  iwrfCalib.setTwoWayWaveguideLossDbH(dsCalib.getTwoWayWaveguideLossDbH());
  iwrfCalib.setTwoWayWaveguideLossDbV(dsCalib.getTwoWayWaveguideLossDbV());
  iwrfCalib.setTwoWayRadomeLossDbH(dsCalib.getTwoWayRadomeLossDbH());
  iwrfCalib.setTwoWayRadomeLossDbV(dsCalib.getTwoWayRadomeLossDbV());
  iwrfCalib.setReceiverMismatchLossDb(dsCalib.getReceiverMismatchLossDb());
  iwrfCalib.setKSquaredWater(dsCalib.getKSquaredWater());
  iwrfCalib.setRadarConstH(dsCalib.getRadarConstH());
  iwrfCalib.setRadarConstV(dsCalib.getRadarConstV());
  iwrfCalib.setNoiseDbmHc(dsCalib.getNoiseDbmHc());
  iwrfCalib.setNoiseDbmHx(dsCalib.getNoiseDbmHx());
  iwrfCalib.setNoiseDbmVc(dsCalib.getNoiseDbmVc());
  iwrfCalib.setNoiseDbmVx(dsCalib.getNoiseDbmVx());
  iwrfCalib.setI0DbmHc(dsCalib.getI0DbmHc());
  iwrfCalib.setI0DbmHx(dsCalib.getI0DbmHx());
  iwrfCalib.setI0DbmVc(dsCalib.getI0DbmVc());
  iwrfCalib.setI0DbmVx(dsCalib.getI0DbmVx());
  iwrfCalib.setReceiverGainDbHc(dsCalib.getReceiverGainDbHc());
  iwrfCalib.setReceiverGainDbHx(dsCalib.getReceiverGainDbHx());
  iwrfCalib.setReceiverGainDbVc(dsCalib.getReceiverGainDbVc());
  iwrfCalib.setReceiverGainDbVx(dsCalib.getReceiverGainDbVx());
  iwrfCalib.setReceiverSlopeDbHc(dsCalib.getReceiverSlopeDbHc());
  iwrfCalib.setReceiverSlopeDbHx(dsCalib.getReceiverSlopeDbHx());
  iwrfCalib.setReceiverSlopeDbVc(dsCalib.getReceiverSlopeDbVc());
  iwrfCalib.setReceiverSlopeDbVx(dsCalib.getReceiverSlopeDbVx());
  iwrfCalib.setDynamicRangeDbHc(dsCalib.getDynamicRangeDbHc());
  iwrfCalib.setDynamicRangeDbHx(dsCalib.getDynamicRangeDbHx());
  iwrfCalib.setDynamicRangeDbVc(dsCalib.getDynamicRangeDbVc());
  iwrfCalib.setDynamicRangeDbVx(dsCalib.getDynamicRangeDbVx());
  iwrfCalib.setBaseDbz1kmHc(dsCalib.getBaseDbz1kmHc());
  iwrfCalib.setBaseDbz1kmHx(dsCalib.getBaseDbz1kmHx());
  iwrfCalib.setBaseDbz1kmVc(dsCalib.getBaseDbz1kmVc());
  iwrfCalib.setBaseDbz1kmVx(dsCalib.getBaseDbz1kmVx());
  iwrfCalib.setSunPowerDbmHc(dsCalib.getSunPowerDbmHc());
  iwrfCalib.setSunPowerDbmHx(dsCalib.getSunPowerDbmHx());
  iwrfCalib.setSunPowerDbmVc(dsCalib.getSunPowerDbmVc());
  iwrfCalib.setSunPowerDbmVx(dsCalib.getSunPowerDbmVx());
  iwrfCalib.setNoiseSourcePowerDbmH(dsCalib.getNoiseSourcePowerDbmH());
  iwrfCalib.setNoiseSourcePowerDbmV(dsCalib.getNoiseSourcePowerDbmV());
  iwrfCalib.setPowerMeasLossDbH(dsCalib.getPowerMeasLossDbH());
  iwrfCalib.setPowerMeasLossDbV(dsCalib.getPowerMeasLossDbV());
  iwrfCalib.setCouplerForwardLossDbH(dsCalib.getCouplerForwardLossDbH());
  iwrfCalib.setCouplerForwardLossDbV(dsCalib.getCouplerForwardLossDbV());
  iwrfCalib.setTestPowerDbmH(dsCalib.getTestPowerDbmH());
  iwrfCalib.setTestPowerDbmV(dsCalib.getTestPowerDbmV());
  iwrfCalib.setDbzCorrection(dsCalib.getDbzCorrection());
  iwrfCalib.setZdrCorrectionDb(dsCalib.getZdrCorrectionDb());
  iwrfCalib.setLdrCorrectionDbH(dsCalib.getLdrCorrectionDbH());
  iwrfCalib.setLdrCorrectionDbV(dsCalib.getLdrCorrectionDbV());
  iwrfCalib.setSystemPhidpDeg(dsCalib.getSystemPhidpDeg());

}

////////////////////////////////////////////////////////////////
// copy from IwrfCalib to DsRadarCalib

void RadarCalib::copyIwrfToDsRadar(const IwrfCalib &iwrfCalib,
                                   DsRadarCalib &dsCalib)
{

  dsCalib.setRadarName(iwrfCalib.getRadarName());
  dsCalib.setCalibTime(iwrfCalib.getCalibTime());
  dsCalib.setWavelengthCm(iwrfCalib.getWavelengthCm());
  dsCalib.setBeamWidthDegH(iwrfCalib.getBeamWidthDegH());
  dsCalib.setBeamWidthDegV(iwrfCalib.getBeamWidthDegV());
  dsCalib.setAntGainDbH(iwrfCalib.getAntGainDbH());
  dsCalib.setAntGainDbV(iwrfCalib.getAntGainDbV());
  dsCalib.setPulseWidthUs(iwrfCalib.getPulseWidthUs());
  dsCalib.setXmitPowerDbmH(iwrfCalib.getXmitPowerDbmH());
  dsCalib.setXmitPowerDbmV(iwrfCalib.getXmitPowerDbmV());
  dsCalib.setTwoWayWaveguideLossDbH(iwrfCalib.getTwoWayWaveguideLossDbH());
  dsCalib.setTwoWayWaveguideLossDbV(iwrfCalib.getTwoWayWaveguideLossDbV());
  dsCalib.setTwoWayRadomeLossDbH(iwrfCalib.getTwoWayRadomeLossDbH());
  dsCalib.setTwoWayRadomeLossDbV(iwrfCalib.getTwoWayRadomeLossDbV());
  dsCalib.setReceiverMismatchLossDb(iwrfCalib.getReceiverMismatchLossDb());
  dsCalib.setKSquaredWater(iwrfCalib.getKSquaredWater());
  dsCalib.setRadarConstH(iwrfCalib.getRadarConstH());
  dsCalib.setRadarConstV(iwrfCalib.getRadarConstV());
  dsCalib.setNoiseDbmHc(iwrfCalib.getNoiseDbmHc());
  dsCalib.setNoiseDbmHx(iwrfCalib.getNoiseDbmHx());
  dsCalib.setNoiseDbmVc(iwrfCalib.getNoiseDbmVc());
  dsCalib.setNoiseDbmVx(iwrfCalib.getNoiseDbmVx());
  dsCalib.setI0DbmHc(iwrfCalib.getI0DbmHc());
  dsCalib.setI0DbmHx(iwrfCalib.getI0DbmHx());
  dsCalib.setI0DbmVc(iwrfCalib.getI0DbmVc());
  dsCalib.setI0DbmVx(iwrfCalib.getI0DbmVx());
  dsCalib.setReceiverGainDbHc(iwrfCalib.getReceiverGainDbHc());
  dsCalib.setReceiverGainDbHx(iwrfCalib.getReceiverGainDbHx());
  dsCalib.setReceiverGainDbVc(iwrfCalib.getReceiverGainDbVc());
  dsCalib.setReceiverGainDbVx(iwrfCalib.getReceiverGainDbVx());
  dsCalib.setReceiverSlopeDbHc(iwrfCalib.getReceiverSlopeDbHc());
  dsCalib.setReceiverSlopeDbHx(iwrfCalib.getReceiverSlopeDbHx());
  dsCalib.setReceiverSlopeDbVc(iwrfCalib.getReceiverSlopeDbVc());
  dsCalib.setReceiverSlopeDbVx(iwrfCalib.getReceiverSlopeDbVx());
  dsCalib.setDynamicRangeDbHc(iwrfCalib.getDynamicRangeDbHc());
  dsCalib.setDynamicRangeDbHx(iwrfCalib.getDynamicRangeDbHx());
  dsCalib.setDynamicRangeDbVc(iwrfCalib.getDynamicRangeDbVc());
  dsCalib.setDynamicRangeDbVx(iwrfCalib.getDynamicRangeDbVx());
  dsCalib.setBaseDbz1kmHc(iwrfCalib.getBaseDbz1kmHc());
  dsCalib.setBaseDbz1kmHx(iwrfCalib.getBaseDbz1kmHx());
  dsCalib.setBaseDbz1kmVc(iwrfCalib.getBaseDbz1kmVc());
  dsCalib.setBaseDbz1kmVx(iwrfCalib.getBaseDbz1kmVx());
  dsCalib.setSunPowerDbmHc(iwrfCalib.getSunPowerDbmHc());
  dsCalib.setSunPowerDbmHx(iwrfCalib.getSunPowerDbmHx());
  dsCalib.setSunPowerDbmVc(iwrfCalib.getSunPowerDbmVc());
  dsCalib.setSunPowerDbmVx(iwrfCalib.getSunPowerDbmVx());
  dsCalib.setNoiseSourcePowerDbmH(iwrfCalib.getNoiseSourcePowerDbmH());
  dsCalib.setNoiseSourcePowerDbmV(iwrfCalib.getNoiseSourcePowerDbmV());
  dsCalib.setPowerMeasLossDbH(iwrfCalib.getPowerMeasLossDbH());
  dsCalib.setPowerMeasLossDbV(iwrfCalib.getPowerMeasLossDbV());
  dsCalib.setCouplerForwardLossDbH(iwrfCalib.getCouplerForwardLossDbH());
  dsCalib.setCouplerForwardLossDbV(iwrfCalib.getCouplerForwardLossDbV());
  dsCalib.setTestPowerDbmH(iwrfCalib.getTestPowerDbmH());
  dsCalib.setTestPowerDbmV(iwrfCalib.getTestPowerDbmV());
  dsCalib.setDbzCorrection(iwrfCalib.getDbzCorrection());
  dsCalib.setZdrCorrectionDb(iwrfCalib.getZdrCorrectionDb());
  dsCalib.setLdrCorrectionDbH(iwrfCalib.getLdrCorrectionDbH());
  dsCalib.setLdrCorrectionDbV(iwrfCalib.getLdrCorrectionDbV());
  dsCalib.setSystemPhidpDeg(iwrfCalib.getSystemPhidpDeg());

}

////////////////////////////////////////////////////////////////
// copy from RadxRcalib to IwrfCalib

void RadarCalib::copyRadxToIwrf(const RadxRcalib &rCalib,
                                IwrfCalib &iwrfCalib)
{

  iwrfCalib.setRadarName(rCalib.getRadarName());
  iwrfCalib.setCalibTime(rCalib.getCalibTime());
  iwrfCalib.setWavelengthCm(rCalib.getWavelengthCm());
  iwrfCalib.setBeamWidthDegH(rCalib.getBeamWidthDegH());
  iwrfCalib.setBeamWidthDegV(rCalib.getBeamWidthDegV());
  iwrfCalib.setAntGainDbH(rCalib.getAntennaGainDbH());
  iwrfCalib.setAntGainDbV(rCalib.getAntennaGainDbV());
  iwrfCalib.setPulseWidthUs(rCalib.getPulseWidthUsec());
  iwrfCalib.setXmitPowerDbmH(rCalib.getXmitPowerDbmH());
  iwrfCalib.setXmitPowerDbmV(rCalib.getXmitPowerDbmV());
  iwrfCalib.setTwoWayWaveguideLossDbH(rCalib.getTwoWayWaveguideLossDbH());
  iwrfCalib.setTwoWayWaveguideLossDbV(rCalib.getTwoWayWaveguideLossDbV());
  iwrfCalib.setTwoWayRadomeLossDbH(rCalib.getTwoWayRadomeLossDbH());
  iwrfCalib.setTwoWayRadomeLossDbV(rCalib.getTwoWayRadomeLossDbV());
  iwrfCalib.setReceiverMismatchLossDb(rCalib.getReceiverMismatchLossDb());
  iwrfCalib.setKSquaredWater(rCalib.getKSquaredWater());
  iwrfCalib.setRadarConstH(rCalib.getRadarConstantH());
  iwrfCalib.setRadarConstV(rCalib.getRadarConstantV());
  iwrfCalib.setNoiseDbmHc(rCalib.getNoiseDbmHc());
  iwrfCalib.setNoiseDbmHx(rCalib.getNoiseDbmHx());
  iwrfCalib.setNoiseDbmVc(rCalib.getNoiseDbmVc());
  iwrfCalib.setNoiseDbmVx(rCalib.getNoiseDbmVx());
  iwrfCalib.setI0DbmHc(rCalib.getI0DbmHc());
  iwrfCalib.setI0DbmHx(rCalib.getI0DbmHx());
  iwrfCalib.setI0DbmVc(rCalib.getI0DbmVc());
  iwrfCalib.setI0DbmVx(rCalib.getI0DbmVx());
  iwrfCalib.setReceiverGainDbHc(rCalib.getReceiverGainDbHc());
  iwrfCalib.setReceiverGainDbHx(rCalib.getReceiverGainDbHx());
  iwrfCalib.setReceiverGainDbVc(rCalib.getReceiverGainDbVc());
  iwrfCalib.setReceiverGainDbVx(rCalib.getReceiverGainDbVx());
  iwrfCalib.setReceiverSlopeDbHc(rCalib.getReceiverSlopeDbHc());
  iwrfCalib.setReceiverSlopeDbHx(rCalib.getReceiverSlopeDbHx());
  iwrfCalib.setReceiverSlopeDbVc(rCalib.getReceiverSlopeDbVc());
  iwrfCalib.setReceiverSlopeDbVx(rCalib.getReceiverSlopeDbVx());
  iwrfCalib.setDynamicRangeDbHc(rCalib.getDynamicRangeDbHc());
  iwrfCalib.setDynamicRangeDbHx(rCalib.getDynamicRangeDbHx());
  iwrfCalib.setDynamicRangeDbVc(rCalib.getDynamicRangeDbVc());
  iwrfCalib.setDynamicRangeDbVx(rCalib.getDynamicRangeDbVx());
  iwrfCalib.setBaseDbz1kmHc(rCalib.getBaseDbz1kmHc());
  iwrfCalib.setBaseDbz1kmHx(rCalib.getBaseDbz1kmHx());
  iwrfCalib.setBaseDbz1kmVc(rCalib.getBaseDbz1kmVc());
  iwrfCalib.setBaseDbz1kmVx(rCalib.getBaseDbz1kmVx());
  iwrfCalib.setSunPowerDbmHc(rCalib.getSunPowerDbmHc());
  iwrfCalib.setSunPowerDbmHx(rCalib.getSunPowerDbmHx());
  iwrfCalib.setSunPowerDbmVc(rCalib.getSunPowerDbmVc());
  iwrfCalib.setSunPowerDbmVx(rCalib.getSunPowerDbmVx());
  iwrfCalib.setNoiseSourcePowerDbmH(rCalib.getNoiseSourcePowerDbmH());
  iwrfCalib.setNoiseSourcePowerDbmV(rCalib.getNoiseSourcePowerDbmV());
  iwrfCalib.setPowerMeasLossDbH(rCalib.getPowerMeasLossDbH());
  iwrfCalib.setPowerMeasLossDbV(rCalib.getPowerMeasLossDbV());
  iwrfCalib.setCouplerForwardLossDbH(rCalib.getCouplerForwardLossDbH());
  iwrfCalib.setCouplerForwardLossDbV(rCalib.getCouplerForwardLossDbV());
  iwrfCalib.setTestPowerDbmH(rCalib.getTestPowerDbmH());
  iwrfCalib.setTestPowerDbmV(rCalib.getTestPowerDbmV());
  iwrfCalib.setDbzCorrection(rCalib.getDbzCorrection());
  iwrfCalib.setZdrCorrectionDb(rCalib.getZdrCorrectionDb());
  iwrfCalib.setLdrCorrectionDbH(rCalib.getLdrCorrectionDbH());
  iwrfCalib.setLdrCorrectionDbV(rCalib.getLdrCorrectionDbV());
  iwrfCalib.setSystemPhidpDeg(rCalib.getSystemPhidpDeg());

}

////////////////////////////////////////////////////////////////
// copy from IwrfCalib to RadxRcalib

void RadarCalib::copyIwrfToRadx(const IwrfCalib &iwrfCalib,
                                RadxRcalib &rCalib)
{

  rCalib.setRadarName(iwrfCalib.getRadarName());
  rCalib.setCalibTime(iwrfCalib.getCalibTime());
  rCalib.setWavelengthCm(iwrfCalib.getWavelengthCm());
  rCalib.setBeamWidthDegH(iwrfCalib.getBeamWidthDegH());
  rCalib.setBeamWidthDegV(iwrfCalib.getBeamWidthDegV());
  rCalib.setAntennaGainDbH(iwrfCalib.getAntGainDbH());
  rCalib.setAntennaGainDbV(iwrfCalib.getAntGainDbV());
  rCalib.setPulseWidthUsec(iwrfCalib.getPulseWidthUs());
  rCalib.setXmitPowerDbmH(iwrfCalib.getXmitPowerDbmH());
  rCalib.setXmitPowerDbmV(iwrfCalib.getXmitPowerDbmV());
  rCalib.setTwoWayWaveguideLossDbH(iwrfCalib.getTwoWayWaveguideLossDbH());
  rCalib.setTwoWayWaveguideLossDbV(iwrfCalib.getTwoWayWaveguideLossDbV());
  rCalib.setTwoWayRadomeLossDbH(iwrfCalib.getTwoWayRadomeLossDbH());
  rCalib.setTwoWayRadomeLossDbV(iwrfCalib.getTwoWayRadomeLossDbV());
  rCalib.setReceiverMismatchLossDb(iwrfCalib.getReceiverMismatchLossDb());
  rCalib.setKSquaredWater(iwrfCalib.getKSquaredWater());
  rCalib.setRadarConstantH(iwrfCalib.getRadarConstH());
  rCalib.setRadarConstantV(iwrfCalib.getRadarConstV());
  rCalib.setNoiseDbmHc(iwrfCalib.getNoiseDbmHc());
  rCalib.setNoiseDbmHx(iwrfCalib.getNoiseDbmHx());
  rCalib.setNoiseDbmVc(iwrfCalib.getNoiseDbmVc());
  rCalib.setNoiseDbmVx(iwrfCalib.getNoiseDbmVx());
  rCalib.setI0DbmHc(iwrfCalib.getI0DbmHc());
  rCalib.setI0DbmHx(iwrfCalib.getI0DbmHx());
  rCalib.setI0DbmVc(iwrfCalib.getI0DbmVc());
  rCalib.setI0DbmVx(iwrfCalib.getI0DbmVx());
  rCalib.setReceiverGainDbHc(iwrfCalib.getReceiverGainDbHc());
  rCalib.setReceiverGainDbHx(iwrfCalib.getReceiverGainDbHx());
  rCalib.setReceiverGainDbVc(iwrfCalib.getReceiverGainDbVc());
  rCalib.setReceiverGainDbVx(iwrfCalib.getReceiverGainDbVx());
  rCalib.setReceiverSlopeDbHc(iwrfCalib.getReceiverSlopeDbHc());
  rCalib.setReceiverSlopeDbHx(iwrfCalib.getReceiverSlopeDbHx());
  rCalib.setReceiverSlopeDbVc(iwrfCalib.getReceiverSlopeDbVc());
  rCalib.setReceiverSlopeDbVx(iwrfCalib.getReceiverSlopeDbVx());
  rCalib.setDynamicRangeDbHc(iwrfCalib.getDynamicRangeDbHc());
  rCalib.setDynamicRangeDbHx(iwrfCalib.getDynamicRangeDbHx());
  rCalib.setDynamicRangeDbVc(iwrfCalib.getDynamicRangeDbVc());
  rCalib.setDynamicRangeDbVx(iwrfCalib.getDynamicRangeDbVx());
  rCalib.setBaseDbz1kmHc(iwrfCalib.getBaseDbz1kmHc());
  rCalib.setBaseDbz1kmHx(iwrfCalib.getBaseDbz1kmHx());
  rCalib.setBaseDbz1kmVc(iwrfCalib.getBaseDbz1kmVc());
  rCalib.setBaseDbz1kmVx(iwrfCalib.getBaseDbz1kmVx());
  rCalib.setSunPowerDbmHc(iwrfCalib.getSunPowerDbmHc());
  rCalib.setSunPowerDbmHx(iwrfCalib.getSunPowerDbmHx());
  rCalib.setSunPowerDbmVc(iwrfCalib.getSunPowerDbmVc());
  rCalib.setSunPowerDbmVx(iwrfCalib.getSunPowerDbmVx());
  rCalib.setNoiseSourcePowerDbmH(iwrfCalib.getNoiseSourcePowerDbmH());
  rCalib.setNoiseSourcePowerDbmV(iwrfCalib.getNoiseSourcePowerDbmV());
  rCalib.setPowerMeasLossDbH(iwrfCalib.getPowerMeasLossDbH());
  rCalib.setPowerMeasLossDbV(iwrfCalib.getPowerMeasLossDbV());
  rCalib.setCouplerForwardLossDbH(iwrfCalib.getCouplerForwardLossDbH());
  rCalib.setCouplerForwardLossDbV(iwrfCalib.getCouplerForwardLossDbV());
  rCalib.setTestPowerDbmH(iwrfCalib.getTestPowerDbmH());
  rCalib.setTestPowerDbmV(iwrfCalib.getTestPowerDbmV());
  rCalib.setDbzCorrection(iwrfCalib.getDbzCorrection());
  rCalib.setZdrCorrectionDb(iwrfCalib.getZdrCorrectionDb());
  rCalib.setLdrCorrectionDbH(iwrfCalib.getLdrCorrectionDbH());
  rCalib.setLdrCorrectionDbV(iwrfCalib.getLdrCorrectionDbV());
  rCalib.setSystemPhidpDeg(iwrfCalib.getSystemPhidpDeg());

}

////////////////////////////////////////////////////////////////
// copy from DsRadarCalib to RadxRcalib

void RadarCalib::copyDsRadarToRadx(const DsRadarCalib &dsCalib,
                                   RadxRcalib &rCalib)
{

  rCalib.setRadarName(dsCalib.getRadarName());
  rCalib.setCalibTime(dsCalib.getCalibTime());
  rCalib.setWavelengthCm(dsCalib.getWavelengthCm());
  rCalib.setBeamWidthDegH(dsCalib.getBeamWidthDegH());
  rCalib.setBeamWidthDegV(dsCalib.getBeamWidthDegV());
  rCalib.setAntennaGainDbH(dsCalib.getAntGainDbH());
  rCalib.setAntennaGainDbV(dsCalib.getAntGainDbV());
  rCalib.setPulseWidthUsec(dsCalib.getPulseWidthUs());
  rCalib.setXmitPowerDbmH(dsCalib.getXmitPowerDbmH());
  rCalib.setXmitPowerDbmV(dsCalib.getXmitPowerDbmV());
  rCalib.setTwoWayWaveguideLossDbH(dsCalib.getTwoWayWaveguideLossDbH());
  rCalib.setTwoWayWaveguideLossDbV(dsCalib.getTwoWayWaveguideLossDbV());
  rCalib.setTwoWayRadomeLossDbH(dsCalib.getTwoWayRadomeLossDbH());
  rCalib.setTwoWayRadomeLossDbV(dsCalib.getTwoWayRadomeLossDbV());
  rCalib.setReceiverMismatchLossDb(dsCalib.getReceiverMismatchLossDb());
  rCalib.setKSquaredWater(dsCalib.getKSquaredWater());
  rCalib.setRadarConstantH(dsCalib.getRadarConstH());
  rCalib.setRadarConstantV(dsCalib.getRadarConstV());
  rCalib.setNoiseDbmHc(dsCalib.getNoiseDbmHc());
  rCalib.setNoiseDbmHx(dsCalib.getNoiseDbmHx());
  rCalib.setNoiseDbmVc(dsCalib.getNoiseDbmVc());
  rCalib.setNoiseDbmVx(dsCalib.getNoiseDbmVx());
  rCalib.setI0DbmHc(dsCalib.getI0DbmHc());
  rCalib.setI0DbmHx(dsCalib.getI0DbmHx());
  rCalib.setI0DbmVc(dsCalib.getI0DbmVc());
  rCalib.setI0DbmVx(dsCalib.getI0DbmVx());
  rCalib.setReceiverGainDbHc(dsCalib.getReceiverGainDbHc());
  rCalib.setReceiverGainDbHx(dsCalib.getReceiverGainDbHx());
  rCalib.setReceiverGainDbVc(dsCalib.getReceiverGainDbVc());
  rCalib.setReceiverGainDbVx(dsCalib.getReceiverGainDbVx());
  rCalib.setReceiverSlopeDbHc(dsCalib.getReceiverSlopeDbHc());
  rCalib.setReceiverSlopeDbHx(dsCalib.getReceiverSlopeDbHx());
  rCalib.setReceiverSlopeDbVc(dsCalib.getReceiverSlopeDbVc());
  rCalib.setReceiverSlopeDbVx(dsCalib.getReceiverSlopeDbVx());
  rCalib.setDynamicRangeDbHc(dsCalib.getDynamicRangeDbHc());
  rCalib.setDynamicRangeDbHx(dsCalib.getDynamicRangeDbHx());
  rCalib.setDynamicRangeDbVc(dsCalib.getDynamicRangeDbVc());
  rCalib.setDynamicRangeDbVx(dsCalib.getDynamicRangeDbVx());
  rCalib.setBaseDbz1kmHc(dsCalib.getBaseDbz1kmHc());
  rCalib.setBaseDbz1kmHx(dsCalib.getBaseDbz1kmHx());
  rCalib.setBaseDbz1kmVc(dsCalib.getBaseDbz1kmVc());
  rCalib.setBaseDbz1kmVx(dsCalib.getBaseDbz1kmVx());
  rCalib.setSunPowerDbmHc(dsCalib.getSunPowerDbmHc());
  rCalib.setSunPowerDbmHx(dsCalib.getSunPowerDbmHx());
  rCalib.setSunPowerDbmVc(dsCalib.getSunPowerDbmVc());
  rCalib.setSunPowerDbmVx(dsCalib.getSunPowerDbmVx());
  rCalib.setNoiseSourcePowerDbmH(dsCalib.getNoiseSourcePowerDbmH());
  rCalib.setNoiseSourcePowerDbmV(dsCalib.getNoiseSourcePowerDbmV());
  rCalib.setPowerMeasLossDbH(dsCalib.getPowerMeasLossDbH());
  rCalib.setPowerMeasLossDbV(dsCalib.getPowerMeasLossDbV());
  rCalib.setCouplerForwardLossDbH(dsCalib.getCouplerForwardLossDbH());
  rCalib.setCouplerForwardLossDbV(dsCalib.getCouplerForwardLossDbV());
  rCalib.setTestPowerDbmH(dsCalib.getTestPowerDbmH());
  rCalib.setTestPowerDbmV(dsCalib.getTestPowerDbmV());
  rCalib.setDbzCorrection(dsCalib.getDbzCorrection());
  rCalib.setZdrCorrectionDb(dsCalib.getZdrCorrectionDb());
  rCalib.setLdrCorrectionDbH(dsCalib.getLdrCorrectionDbH());
  rCalib.setLdrCorrectionDbV(dsCalib.getLdrCorrectionDbV());
  rCalib.setSystemPhidpDeg(dsCalib.getSystemPhidpDeg());

}

////////////////////////////////////////////////////////////////
// copy from RadxRcalib to DsRadarCalib

void RadarCalib::copyRadxToDsRadar(const RadxRcalib &rCalib,
                                   DsRadarCalib &dsCalib)

{

  dsCalib.setRadarName(rCalib.getRadarName());
  dsCalib.setCalibTime(rCalib.getCalibTime());
  dsCalib.setWavelengthCm(rCalib.getWavelengthCm());
  dsCalib.setBeamWidthDegH(rCalib.getBeamWidthDegH());
  dsCalib.setBeamWidthDegV(rCalib.getBeamWidthDegV());
  dsCalib.setAntGainDbH(rCalib.getAntennaGainDbH());
  dsCalib.setAntGainDbV(rCalib.getAntennaGainDbV());
  dsCalib.setPulseWidthUs(rCalib.getPulseWidthUsec());
  dsCalib.setXmitPowerDbmH(rCalib.getXmitPowerDbmH());
  dsCalib.setXmitPowerDbmV(rCalib.getXmitPowerDbmV());
  dsCalib.setTwoWayWaveguideLossDbH(rCalib.getTwoWayWaveguideLossDbH());
  dsCalib.setTwoWayWaveguideLossDbV(rCalib.getTwoWayWaveguideLossDbV());
  dsCalib.setTwoWayRadomeLossDbH(rCalib.getTwoWayRadomeLossDbH());
  dsCalib.setTwoWayRadomeLossDbV(rCalib.getTwoWayRadomeLossDbV());
  dsCalib.setReceiverMismatchLossDb(rCalib.getReceiverMismatchLossDb());
  dsCalib.setKSquaredWater(rCalib.getKSquaredWater());
  dsCalib.setRadarConstH(rCalib.getRadarConstantH());
  dsCalib.setRadarConstV(rCalib.getRadarConstantV());
  dsCalib.setNoiseDbmHc(rCalib.getNoiseDbmHc());
  dsCalib.setNoiseDbmHx(rCalib.getNoiseDbmHx());
  dsCalib.setNoiseDbmVc(rCalib.getNoiseDbmVc());
  dsCalib.setNoiseDbmVx(rCalib.getNoiseDbmVx());
  dsCalib.setI0DbmHc(rCalib.getI0DbmHc());
  dsCalib.setI0DbmHx(rCalib.getI0DbmHx());
  dsCalib.setI0DbmVc(rCalib.getI0DbmVc());
  dsCalib.setI0DbmVx(rCalib.getI0DbmVx());
  dsCalib.setReceiverGainDbHc(rCalib.getReceiverGainDbHc());
  dsCalib.setReceiverGainDbHx(rCalib.getReceiverGainDbHx());
  dsCalib.setReceiverGainDbVc(rCalib.getReceiverGainDbVc());
  dsCalib.setReceiverGainDbVx(rCalib.getReceiverGainDbVx());
  dsCalib.setReceiverSlopeDbHc(rCalib.getReceiverSlopeDbHc());
  dsCalib.setReceiverSlopeDbHx(rCalib.getReceiverSlopeDbHx());
  dsCalib.setReceiverSlopeDbVc(rCalib.getReceiverSlopeDbVc());
  dsCalib.setReceiverSlopeDbVx(rCalib.getReceiverSlopeDbVx());
  dsCalib.setDynamicRangeDbHc(rCalib.getDynamicRangeDbHc());
  dsCalib.setDynamicRangeDbHx(rCalib.getDynamicRangeDbHx());
  dsCalib.setDynamicRangeDbVc(rCalib.getDynamicRangeDbVc());
  dsCalib.setDynamicRangeDbVx(rCalib.getDynamicRangeDbVx());
  dsCalib.setBaseDbz1kmHc(rCalib.getBaseDbz1kmHc());
  dsCalib.setBaseDbz1kmHx(rCalib.getBaseDbz1kmHx());
  dsCalib.setBaseDbz1kmVc(rCalib.getBaseDbz1kmVc());
  dsCalib.setBaseDbz1kmVx(rCalib.getBaseDbz1kmVx());
  dsCalib.setSunPowerDbmHc(rCalib.getSunPowerDbmHc());
  dsCalib.setSunPowerDbmHx(rCalib.getSunPowerDbmHx());
  dsCalib.setSunPowerDbmVc(rCalib.getSunPowerDbmVc());
  dsCalib.setSunPowerDbmVx(rCalib.getSunPowerDbmVx());
  dsCalib.setNoiseSourcePowerDbmH(rCalib.getNoiseSourcePowerDbmH());
  dsCalib.setNoiseSourcePowerDbmV(rCalib.getNoiseSourcePowerDbmV());
  dsCalib.setPowerMeasLossDbH(rCalib.getPowerMeasLossDbH());
  dsCalib.setPowerMeasLossDbV(rCalib.getPowerMeasLossDbV());
  dsCalib.setCouplerForwardLossDbH(rCalib.getCouplerForwardLossDbH());
  dsCalib.setCouplerForwardLossDbV(rCalib.getCouplerForwardLossDbV());
  dsCalib.setTestPowerDbmH(rCalib.getTestPowerDbmH());
  dsCalib.setTestPowerDbmV(rCalib.getTestPowerDbmV());
  dsCalib.setDbzCorrection(rCalib.getDbzCorrection());
  dsCalib.setZdrCorrectionDb(rCalib.getZdrCorrectionDb());
  dsCalib.setLdrCorrectionDbH(rCalib.getLdrCorrectionDbH());
  dsCalib.setLdrCorrectionDbV(rCalib.getLdrCorrectionDbV());
  dsCalib.setSystemPhidpDeg(rCalib.getSystemPhidpDeg());

}

