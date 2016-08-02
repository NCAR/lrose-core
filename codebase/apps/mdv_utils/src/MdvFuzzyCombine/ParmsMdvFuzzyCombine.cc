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
/**
 * @file ParmsMdvFuzzyCombine.cc
 */
#include "ParmsMdvFuzzyCombine.hh"
#include "Params.hh"
#include "Parm1.hh"
#include <rapmath/FuzzyF.hh>
#include <dsdata/DsUrlTrigger.hh>

//------------------------------------------------------------------
ParmsMdvFuzzyCombine::ParmsMdvFuzzyCombine(int argc, char **argv)
{
  // set things up to read in the parameters specific to this app:
  Params appParms;
  char *ppath;
  if (appParms.loadFromArgs(argc, argv, NULL, &ppath))
  {
    printf("ERROR loading parms");
    exit(1);
  }

  _debug = appParms.debug;
  _instance = appParms.instance;
  _inputUrl = appParms.inputDataUrl;
  _inputMaskUrl = appParms.inputMaskUrl;
  _inputMaskFieldName = appParms.inputMaskFieldName;
  _staticMask = appParms.staticMaskData;
  _outputUrl = appParms.outputDataUrl;
  _isForecastData = appParms.isForecastData;
  _isForecastMaskData = appParms.isForecastMaskData;
  if (!_inputMaskUrl.empty() && _isForecastMaskData && !_isForecastData)
  {
    printf("Cannot have mask input as forecasts with flat file data input\n");
    exit(1);
  }

  for (int i=0; i<appParms.input_n; ++i)
  {
    _addInput(appParms, appParms._input[i]);
  }

  _outputFieldName = appParms.outputFieldName;
  _outputFieldUnits = appParms.outputFieldUnits;

  _erodeMax = appParms.erodeMax;
  _smoothNpt = appParms.smoothNpt;
  _distFillMax = appParms.distFillMax;
  _distFillScale = appParms.distFillScale;

  switch (appParms.fuzzy_dist_to_weight_index)
  {
  case 0:
    _setTaper(appParms.fuzzy0_n, appParms._fuzzy0);
    break;

  case 1:
    _setTaper(appParms.fuzzy1_n, appParms._fuzzy1);
    break;

  case 2:
    _setTaper(appParms.fuzzy2_n, appParms._fuzzy2);
    break;

  case 3:
    _setTaper(appParms.fuzzy3_n, appParms._fuzzy3);
    break;

  case 4:
    _setTaper(appParms.fuzzy4_n, appParms._fuzzy4);
    break;

  case 5:
    _setTaper(appParms.fuzzy5_n, appParms._fuzzy5);
    break;

  case 6:
    _setTaper(appParms.fuzzy6_n, appParms._fuzzy6);
    break;

  case 7:
    _setTaper(appParms.fuzzy7_n, appParms._fuzzy7);
    break;

  case 8:
    _setTaper(appParms.fuzzy8_n, appParms._fuzzy8);
    break;

  case 9:
    _setTaper(appParms.fuzzy9_n, appParms._fuzzy9);
    break;

  default:
    printf("ERROR fuzzy index %d for tapering out of range 0 to 9\n",
	   appParms.fuzzy_dist_to_weight_index);
    exit(1);
  }

  bool error;

  if (!DsUrlTrigger::checkArgs(argc, argv, _archiveTime0, _archiveTime1,
			       _isArchiveMode, error))
  {
    error = true;
  }
  if (error)
  {
    exit(1);
  }
}

//------------------------------------------------------------------
ParmsMdvFuzzyCombine::~ParmsMdvFuzzyCombine()
{
}

//------------------------------------------------------------------
void ParmsMdvFuzzyCombine::_addInput(const Params &p,
				     const Params::Input_t &s)
{
  switch (s.fuzzy_index)
  {
  case 0:
    _addInput(s, p.fuzzy0_n, p._fuzzy0);
    break;
  case 1:
    _addInput(s, p.fuzzy1_n, p._fuzzy1);
    break;
  case 2:
    _addInput(s, p.fuzzy2_n, p._fuzzy2);
    break;
  case 3:
    _addInput(s, p.fuzzy3_n, p._fuzzy3);
    break;
  case 4:
    _addInput(s, p.fuzzy4_n, p._fuzzy4);
    break;
  case 5:
    _addInput(s, p.fuzzy5_n, p._fuzzy5);
    break;
  case 6:
    _addInput(s, p.fuzzy6_n, p._fuzzy6);
    break;
  case 7:
    _addInput(s, p.fuzzy7_n, p._fuzzy7);
    break;
  case 8:
    _addInput(s, p.fuzzy8_n, p._fuzzy8);
    break;
  case 9:
    _addInput(s, p.fuzzy9_n, p._fuzzy9);
    break;
  default:
    printf("ERROR p.fuzzy index %d out of range [0,0]\n", s.fuzzy_index);
    exit(1);
  }
  _fields.push_back(s.name);
}

/*----------------------------------------------------------------*/
void ParmsMdvFuzzyCombine::_addInput(const Params::Input_t &s, const int n,
				     const Params::Fuzzy_t *f)
{
  vector<pair<double,double> > args;

  for (int i=0; i<n; ++i)
  {
    args.push_back(pair<double,double>(f[i].x, f[i].y));
  }
  
  FuzzyF fuzzyf(args);

  _input.push_back(Parm1(s.name, s.weight, fuzzyf));
}


/*----------------------------------------------------------------*/
void ParmsMdvFuzzyCombine::_setTaper(const int n,
				     const Params::Fuzzy_t *f)
{
  vector<pair<double,double> > args;

  for (int i=0; i<n; ++i)
  {
    args.push_back(pair<double,double>(f[i].x, f[i].y));
  }
  _taper = FuzzyF(args);
}
