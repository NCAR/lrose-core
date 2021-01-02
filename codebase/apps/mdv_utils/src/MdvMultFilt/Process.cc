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
#include <toolsa/copyright.h>

/************************************************************************

Module:	Process.cc

Author:	Dave Albo

Date:	Thu Jan 26 16:43:33 2006

Description:   Processing of MDV. Built from Niles Oien Mdv filtering example.

************************************************************************/

/* System include files / Local include files */
#include "Process.hh"
#include <iostream>
#include <vector>
#include <string>
#include <Mdv/MdvxField.hh>
#include <math.h>
#include <toolsa/str.h>
#include <toolsa/pjg_flat.h>
#include <Mdv/MdvxPjg.hh>
#include <Mdv/MdvxChunk.hh>
using namespace std;

static const char * _filter_parm_name(Params::filter_parm_t &f)
{
  switch (f.FilterType)
  {
  case Params::THRESH:
    return "THRESH";
  case Params::SMOOTH:
    return "SMOOTH";
  case Params::COMBINE:
    return "COMBINE";
  case Params::EXPAND:
    return "EXPAND";
  case Params::PASS:
    return "PASS";
  default:
    return "UNKNOWN";
  }
}

/*----------------------------------------------------------------*/
static int _max_parms(Params::filter_parm_t &f, Params *P)
{
  switch (f.FilterType)
  {
  case Params::THRESH:
    return P->MdvThresh_n;
  case Params::SMOOTH:
    return P->MdvSmooth_n;
  case Params::COMBINE:
    return P->MdvCombine_n;
  case Params::EXPAND:
    return P->MdvExpand_n;
  case Params::PASS:
    return 1;
  default:
    printf("ERROR unknown filter type %d\n", (int)f.FilterType);
    return -1;
  }
}

/*----------------------------------------------------------------*/
static bool _initialize_mdvx(DsMdvx &New, Params *P, time_t T)
{
  //New.setDebug( P->Debug);
  New.setDebug( false);
  New.setReadTime(Mdvx::READ_FIRST_BEFORE, P->TriggerUrl, 0, T);
  New.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  New.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  New.setReadScalingType(Mdvx::SCALING_NONE);
  
  if (P->RemapGrid)
  {
    switch ( P->grid_projection)
    {
    case Params::FLAT:
      New.setReadRemapFlat(P->grid_nx, P->grid_ny,
			 P->grid_minx, P->grid_miny,
			 P->grid_dx, P->grid_dy,
			 P->grid_origin_lat, P->grid_origin_lon,
			 P->grid_rotation);
      break;
    case Params::LATLON:
      New.setReadRemapLatlon(P->grid_nx, P->grid_ny,
			   P->grid_minx, P->grid_miny,
			   P->grid_dx, P->grid_dy);
      break;            
    case Params::LAMBERT:
      New.setReadRemapLc2(P->grid_nx, P->grid_ny,
			P->grid_minx, P->grid_miny,
			P->grid_dx, P->grid_dy,
			P->grid_origin_lat, 
			P->grid_origin_lon,
			P->grid_lat1,  P->grid_lat2);
      break;
    default:
      cerr << "Unsupported projection." << endl;
      return false;
      break;
    }               
  }
  return true;
}

/*----------------------------------------------------------------*/
//
// Constructor
//
Process::Process()
{
  OutputUrl = (char *)NULL;
  D = NULL;
}

/*----------------------------------------------------------------*/
//
// Main method - process data at a given time.
//
int Process::Derive(Params *P, time_t t, TempDataState &T)
{
  DsMdvx New;

  if (!_initialize(New, P, t, T))
    return -1;

  // do the filtering here
  int stat = 0;
  string name = P->InFieldName;
  for (int i=0; i<P->FiltParm_n; ++i)
  {
    if (!_filter(name, P->_FiltParm[i], P, T))
    {
      stat = -1;
      break;
    }
  }
  if (P->Debug){
    cerr << "Finished data processing for this time." << endl << endl;
  }
  return stat;
}
    
/*----------------------------------------------------------------*/
//
// Destructor
//
Process::~Process(){
  //
  // Only do the write if fields were added.
  //

  Mdvx::master_header_t Mhdr = Out.getMasterHeader();

  if (Mhdr.n_fields > 0){
    if (Out.writeToDir(OutputUrl)) {
      cerr << "Failed to wite to " << OutputUrl << endl;
      exit(-1);
    }      
  }
  free(OutputUrl);
  if (D != NULL)
    delete D;
}

/*----------------------------------------------------------------*/
bool Process::_filter(string &name, Params::filter_parm_t &f, Params *P,
		      TempDataState &T)
{
  int i = _filter_init(name, f, P, T);
  if (i < 0)
    return false;

  switch (f.FilterType)
  {
  case Params::THRESH:
    D->filter_thresh(P->_MdvThresh[i]);
    break;
  case Params::SMOOTH:
    D->filter_smooth(P->_MdvSmooth[i]);
    break;
  case Params::COMBINE:
    _filter_combine(P->_MdvCombine[i], T);
    break;
  case Params::EXPAND:
    D->filter_expand_value(P->_MdvExpand[i]);
    break;
  case Params::PASS:
    D->filter_passthrough();
    break;
  default:
    printf("ERROR unknown filter type %d\n", (int)f.FilterType);
    return false;
  }

  return _filter_finish(f, P);
}

/*----------------------------------------------------------------*/
void Process::_filter_combine(Params::mdv_combine_parm_t &p,
			      TempDataState &T)
{
  fl32 *f2, missing2, bad2, newbad;
  int num2;

  // find the other input and get the data.
  string name = p.SecondInpFieldName;
  if ((f2 = T.getVals(name, missing2, bad2, num2)) == NULL)
  {
    cerr << "Fatal error" << endl;
    exit(-1);
  }
  // do synchronize if need to.
  if (D->synchronize_data(f2, num2, missing2, bad2, newbad))
  {
    // if so, adjust temporary data missing/bad.
    T.putVals(name, newbad, newbad);

    // re-get the data.
    f2 = T.getVals(name, missing2, bad2, num2);

    // put to here for later.
    InFhdr.missing_data_value = newbad;
    InFhdr.bad_data_value = newbad;
  }

  // finally do the filter.
  D->filter_combine(f2, missing2, bad2, num2, p);
}

/*----------------------------------------------------------------*/
bool Process::_initialize(DsMdvx &New, Params *P, time_t t, 
			  TempDataState &T)
{
  if (P->Debug)
    cerr << "Data at " << utimstr(t) << endl;

  // init the New object.
  if (!_initialize_mdvx(New, P, t))
    return false;

  // read in data.
  if (New.readVolume())
  {
    cerr << "Read failed at " << utimstr(t) << " from ";
    cerr << P->TriggerUrl  << endl;
    return false;
  }     

  // initialize the output from the input.
  _initialize_out(New, P);

  // Get the desired field from New into D.
  if (!_extract_field(New, P))
    return false;

  // re-init the temp data
  T.clear();

  return true;
}

/*----------------------------------------------------------------*/
void Process::_initialize_out(DsMdvx &New, Params *P)
{
  OutputUrl = STRdup(P->OutUrl);

  Mdvx::master_header_t InMhdr = New.getMasterHeader();
  Mdvx::master_header_t OutMhdr = InMhdr;
  Out.setMasterHeader(OutMhdr);
  Out.clearFields();     
  Out.clearChunks();
  Out.setAppName("MdvMultFilt");

  //
  // Copy and MdvChunks that were in the input to the output.
  // These chunks are added to an MDV file to carry excess
  // information, such as the setup parameters for a radar.
  // Here, we just pass them on from input to output.
  //
  for (size_t ii = 0; ii < New.getNChunks(); ii++) {
    MdvxChunk *chunk = new MdvxChunk(*New.getChunkByNum(ii));
    Out.addChunk(chunk);
  }
}
/*----------------------------------------------------------------*/
bool Process::_extract_field(DsMdvx &New, Params *P)
{
  MdvxField *InField;

  if (!(strncmp(P->InFieldName,"#",1))){
    int Fnum = atoi(P->InFieldName + 1);
    InField = New.getFieldByNum( Fnum ); 
  } else {
    InField = New.getFieldByName( P->InFieldName );
  }
  if (InField == NULL){
    cerr << "New field " << P->InFieldName << " not found." << endl;
    return false;
  }

  InFhdr = InField->getFieldHeader();
  InVhdr = InField->getVlevelHeader();
  D = new Data(InField);
  return true;
}


/*----------------------------------------------------------------*/
int Process::_filter_init(string &name, Params::filter_parm_t &f,
			  Params *P, TempDataState &T)
{
  // name is the default input from upstream (= contents of D).

  // see if input is needed downstream and copy to it if so.
  T.store_if_needed(name, *D);

  // if the input name is not the one we actually want, need to pull
  // it out of T.
  string s = f.InFieldName;
  if (name != s)
  {
    if (!T.extract(s, *D))
    {
      printf("ERROR extracting data %s from store..\n", s.c_str());
      return -1;
    }
  }

  // set up new name for return 
  name = f.OutFieldName;

  // set index
  int i = f.FilterIndex;
  int max = _max_parms(f, P);
  if (i < 0 || i >= max)
  {
    printf("ERRROR %s index %d out of range [0,%d]\n", _filter_parm_name(f),
	   i, max-1);
    return -1;
  }
  else
    return i;
}
    
/*----------------------------------------------------------------*/
bool Process::_filter_finish(Params::filter_parm_t &f, Params *P)
{
  // the filters all change the contents of D.
  // fold those changes back into mdv state.
  _evaluate(P->Debug);
  return _set_fld(f, P);
}

/*----------------------------------------------------------------*/
void Process::_evaluate(bool debug)
{
  double min, max, mean;
  
  if (D->evaluate(min, max, mean))
  {
    D->change(max+1.0);
    InFhdr.missing_data_value = max + 1.0;
    InFhdr.bad_data_value = max + 1.0;
    if (debug){
      cerr << "Data range from " << min << " to " << max;
      cerr << " with mean " << mean << endl;
    }
  }
  else
  {
    if (debug)
      cerr << "All data are missing." << endl;
  }
}


/*----------------------------------------------------------------*/
bool Process::_set_fld(Params::filter_parm_t &f, Params *P)
{
  if (!f.OutToUrl)
    return true;

  MdvxField *fld;
  fl32 *data;

  data = D->get_data();
  fld = new MdvxField(InFhdr, InVhdr, (void *)data);
  fld->setFieldName(f.OutFieldName);
  fld->setUnits(f.OutFieldUnits);
  fld->setFieldNameLong(f.OutFieldName);
  switch (P->output_encoding_type)
  {
  case Params::ENCODING_INT8 :
    if (fld->convertRounded(Mdvx::ENCODING_INT8,
			    Mdvx::COMPRESSION_GZIP) != 0)
    {
      cerr << "convertRounded failed - I cannot go on." << endl;
      delete fld;
      return false;
    }  
    break;
    
  case Params::ENCODING_INT16 :
    if (fld->convertRounded(Mdvx::ENCODING_INT16,
			    Mdvx::COMPRESSION_GZIP) != 0)
    {
      cerr << "convertRounded failed - I cannot go on." << endl;
      delete fld;
      return false;
    }
    break;
  
  case Params::ENCODING_FLOAT32 :
    fld->requestCompression(Mdvx::COMPRESSION_GZIP);
    break;
  }


  Out.addField(fld);
  return true;
}

