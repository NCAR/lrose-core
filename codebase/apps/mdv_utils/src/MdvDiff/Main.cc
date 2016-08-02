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
/*********************************************************************
 * Main.cc:  main routine
 *
 * RAP, NCAR, Boulder CO
 *
 *
 *********************************************************************/

#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxProj.hh>
#include <Mdv/MdvxChunk.hh>
#include <niwot_basic/UrlTrigger.hh>
#include <niwot_basic/CmdArgs.hh>
#include "Parms.hh"
#include "Stats.hh"

static void _process(const time_t &t, const Parms &P);
static void _process_obs(const time_t &t, const Parms &P);
static void _process_obs1(const time_t &t, const Parms &P,
			  DsMdvx &D0, DsMdvx &D1, const MdvxProj &proj);
static void _process_fcst(const time_t &t, const int lts, const Parms &P);
static void _process_fcst2(const time_t &t, const int lts, const Parms &P,
			   DsMdvx &D0, DsMdvx &D1, const MdvxProj &proj);
static void _process_field(const time_t &t, const int lts, const Parms &P,
			   const string &fname, DsMdvx &D0,
			   DsMdvx &D1, const MdvxProj &proj, const bool first);
static void _compare(const Parms &P, const time_t &t, const int lts,
		     const string &name, const Mdvx::field_header_t fh0,
		     const fl32 *data0, const Mdvx::field_header_t fh1, 
		     const fl32 *data1, const MdvxProj &proj);
static bool _fcstinfo_compare(const time_t &t, const int lts,
			      const DsMdvx &D0, const DsMdvx &D1, 
			      const string &fname);
static bool _compare_mhdr(const Mdvx::master_header_t &m0,
			  const Mdvx::master_header_t &m1, const bool print,
			  const bool all_fields);
static bool _compare_fhdr(const Mdvx::field_header_t &f0,
			  const Mdvx::field_header_t &f1, const bool print);
static string _evaluateChunkDiff(const int i, const string &s0,
				 const string &s1);
static bool _loadFile(const string &fname, string &inp);

void tidy_and_exit(int sig);

Stats _stats;
Stats _S;
int _status = -1;

/*********************************************************************
 * main()
 */
int main(int argc, char **argv)
{
  Parms P(argc, argv);
  CmdArgs C(argc, argv);
  bool archive = C.is_archive();
  time_t t0, t1;
    
  _status = 0;
  _stats = Stats(0, "Summary", P.min_diff, P.show_all, P.show_details);

  printf("------Comparing:\n\t%s\n\t%s\n", P.test_url, P.old_url);
  UrlTrigger *ut;
  bool debug = P.debug;
  if (archive)
  {
    t0 = C.get_start();
    t1 = C.get_end();
    if (P.obs)
      ut = new UrlTrigger(t0, t1, P.test_url, UrlTrigger::OBS, debug);
    else
      ut = new UrlTrigger(t0, t1, P.test_url, UrlTrigger::FCST_LEAD, debug);
  }
  else
  {
    if (P.obs)
      ut = new UrlTrigger(P.test_url, UrlTrigger::OBS, debug);
    else
      ut = new UrlTrigger(P.test_url, UrlTrigger::FCST_LEAD, debug);
  }

  if (P.obs)
  {
    time_t t;
    while (ut->next_time(t))
      _process_obs(t, P);
  }
  else
  {
    time_t t;
    int lt;
    while (ut->next_time(t, lt))
      _process_fcst(t, lt, P);
  }

  delete ut;
  tidy_and_exit(0);
}

/*----------------------------------------------------------------*/
void tidy_and_exit(int sig)
{
  printf("FINAL STATS:\n");
  _stats.print_final();
  printf("exiting now\n");
  exit(_status);
}

/*----------------------------------------------------------------*/
void _process_obs(const time_t &t, const Parms &P)
{
  bool got0, got1;
  _S = Stats(t, "Obs", P.min_diff, P.show_all, P.show_details);

  DsMdvx D0;
//   D0.setDebug(true);
  D0.setReadTime(Mdvx::READ_FIRST_BEFORE, P.test_url, 0, t);
  MdvxProj proj(D0);
  if (D0.readVolume())
  {
    _S.addInfo("No data url=%s", P.test_url);
    _S._nread_error++;
    _status = -1;
    _stats._nread_error++;
    got0 = false;
  }
  else
    got0 = true;

  DsMdvx D1;
//   D1.setDebug(true);
  D1.setReadTime(Mdvx::READ_FIRST_BEFORE, P.old_url, 0, t);
  if (D1.readVolume())
  {
    _S.addInfo("No data url=%s", P.old_url);
    _S._nread_error++;
    _status = -1;
    _stats._nread_error++;
    got1 = false;
  }
  else
    got1 = true;

  if (got0 && got1)
  {
    _process_obs1(t, P, D0, D1, proj);
  }
  _S.print_one(proj);
}

/*----------------------------------------------------------------*/
void _process_obs1(const time_t &t, const Parms &P, DsMdvx &D0, DsMdvx &D1,
		   const MdvxProj &proj)
{
  const Mdvx::master_header_t m0 = D0.getMasterHeader();
  const Mdvx::master_header_t m1 = D1.getMasterHeader();
  if (!_compare_mhdr(m0, m1, false, P._all_fields))
  {
    _S.addInfo("master headers don't agree");
    _status = -1;
    _compare_mhdr(m0, m1, true, P._all_fields);
    _S._nmhdr_error++;
    _stats._nmhdr_error++;
  }

  vector<MdvxField *> f0, f1;
  vector<string> fieldNames;
  f0 = D0.getFields();
  f1 = D1.getFields();
  for (int i=0; i<(int)f0.size(); ++i)
  {
    string s = f0[i]->getFieldName();
    if (P.wanted_field(s))
    {
      fieldNames.push_back(s);
    }
  }
  for (int i=0; i<(int)f1.size(); ++i)
  {
    string s = f1[i]->getFieldName();
    if (P.wanted_field(s))
    {
      if (find(fieldNames.begin(), fieldNames.end(), s) == fieldNames.end())
	fieldNames.push_back(s);
    }
  }

  // same # of fields based on above, so load each field expect same
  for (int i=0; i<(int)fieldNames.size(); ++i)
  {
    _process_field(t, 0, P, fieldNames[i], D0, D1, proj, i == 0);
  }
}

/*----------------------------------------------------------------*/
void _process_field(const time_t &t, const int lts,
		    const Parms &P, const string &fname,
		    DsMdvx &D0, DsMdvx &D1, const MdvxProj &proj,
		    const bool first)
{
  _S.setField(fname);
  _stats.setField(fname);

  MdvxField *f0 = D0.getFieldByName(fname);
  MdvxField *f1 = D1.getFieldByName(fname);
  if (f0 == 0 || f1 == 0)
  {
    _S.fieldMissingInOne();
    _stats.fieldMissingInOne();
    _status = -1;
    return;
  }

  Mdvx::field_header_t fh0 = f0->getFieldHeader();
  Mdvx::field_header_t fh1 = f1->getFieldHeader();


  if (!_compare_fhdr(fh0, fh1, false))
  {
    _S.addInfo("field headers don't agree, field=%s", fname.c_str());
    _compare_fhdr(fh0, fh1, true);
    _S.fieldHdrError();
    _stats.fieldHdrError();
    _status = -1;
  }
  
  
//   bool got0, got1;
//   MdvxField *f0, *f1;
//   fl32 *data0, *data1;
//   D0.setReadTime(Mdvx::READ_FIRST_BEFORE, P.test_url, 0, t);
//   D0.clearReadFields();
//   D0.addReadField(fname);
//   if (D0.readVolume())
//   {
//     _S.addInfo("No data url=%s field=%s", P.test_url, fname.c_str());
//     _S.fieldReadError();
//     _stats.fieldReadError();
//     _status = -1;
//     got0 = false;
//   }
//   else
//   {
//     got0 = true;
//     f0 = D0.getFieldByName(fname);

  f0->convertType(Mdvx::ENCODING_FLOAT32, Mdvx::COMPRESSION_NONE);
  fl32 *data0 = (fl32 *)f0->getVol();
  f1->convertType(Mdvx::ENCODING_FLOAT32, Mdvx::COMPRESSION_NONE);
  fl32 *data1 = (fl32 *)f1->getVol();

//   D1.setReadTime(Mdvx::READ_FIRST_BEFORE, P.old_url, 0, t);
//   D1.clearReadFields();
//   D1.addReadField(fname);
//   if (D1.readVolume())
//   {
//     _S.addInfo("No data url=%s field=%s", P.old_url, fname.c_str());
//     _S.fieldReadError();
//     _stats.fieldReadError();
//     _status = -1;
//     got1 = false;
//   }
//   else
//   {
//     got1 = true;
//     f1 = D1.getFieldByName(fname);
//     f1->convertType(Mdvx::ENCODING_FLOAT32, Mdvx::COMPRESSION_NONE);
//     data1 = (fl32 *)f1->getVol();
//   }

//   if (!InterfaceLL::loadObs(t, P._proj, P._f1.pUrl, fname, P._f1.pRemap, gold))
//   {
//     _S.addInfo("No data url=%s field=%s", P._f1.pUrl.c_str(), fname.c_str());
//     _S.fieldReadError();
//     _stats.fieldReadError();
//     _status = -1;
//     got1 = false;
//   }
//   else
//     got1 = true;

//   if (got1 && got0)
//   {

  if (first && P.compare_metadata)
  {
    if (!_fcstinfo_compare(t, lts, D0, D1, fname))
    {
      _stats._nmetadata_diff++;
      _S._nmetadata_diff++;
      _status = -1;
      return;
    }
  }

  _compare(P, t, lts, fname, fh0, data0, fh1, data1, proj);
}

/*----------------------------------------------------------------*/
void _process_fcst(const time_t &t, const int lts, const Parms &P)
{
  bool got0, got1;
  _S = Stats(t, lts, "Fcst", P.min_diff, P.show_all, P.show_details);
  DsMdvx D0;
  D0.setReadTime(Mdvx::READ_SPECIFIED_FORECAST, P.test_url, 0, t, lts);
  MdvxProj proj(D0);
  if (D0.readVolume())
  {
    _S.addInfo("No data url=%s", P.test_url);
    _S._nread_error++;
    _stats._nread_error++;
    _status = -1;
    got0 = false;
  }
  else
    got0 = true;

  DsMdvx D1;
  D1.setReadTime(Mdvx::READ_SPECIFIED_FORECAST, P.old_url, 0, t, lts);
  if (D1.readVolume())
  {
    _S.addInfo("No data url=%s", P.old_url);
    _S._nread_error++;
    _stats._nread_error++;
    _status = -1;
    got1 = false;
  }
  else
    got1 = true;
  if (got0 && got1)
  {
    _process_fcst2(t, lts, P, D0, D1, proj);
  }
  _S.print_one(proj);
}

/*----------------------------------------------------------------*/
void _compare(const Parms &P, const time_t &t, const int lts,
	      const string &name, const Mdvx::field_header_t fh0,
	      const fl32 *data0, const Mdvx::field_header_t fh1, 
	      const fl32 *data1, const MdvxProj &proj)
{
  int nx, ny;
  nx = fh0.nx;
  ny = fh0.ny;

  // already  tested field header, so know nx and ny are same for both 
  // inputs

  _stats._current->_ntotal += nx*ny;
  for (int y=0; y<ny; ++y)
  {
    for (int x=0; x<nx; ++x)
    {
      double v0, v1;
      bool has0, has1;
      v0 = data0[y*nx + x];
      has0 = (v0 != fh0.bad_data_value && v0 != fh0.missing_data_value);
      v1 = data1[y*nx + x];
      has1 = (v1 != fh1.bad_data_value && v0 != fh1.missing_data_value);
      if (has0 && has1)
      {
	if (v0 != v1)
	{
	  if (_S.process_diff(x, y, v0-v1))
	  {
	    _status = -1;
	    _stats.process_diff(x, y, v0-v1);
	    if (P.show_all_data_diff)
	    {
	      double lat, lon;
	      int ind = y*nx + x;
	      string name = _stats._current->_name;
	      proj.xyIndex2latlon(x, y, lat, lon);
	      printf("%s xy[%d,%d] = ind[%d] = latlon[%8.5lf,%8.5lf]:  v0:%8.5lf v1:%8.5lf\n",
		     name.c_str(), x, y, ind, lat, lon, v0, v1);
	    }
	  }
	}
      }
      else if ((has0 && !has1) || (has1 && !has0))
      {
	_S.inc_one_missing();
	_stats.inc_one_missing();
	_status = -1;
      }
    }
  }
}

/*----------------------------------------------------------------*/
void _process_fcst2(const time_t &t, const int lts, const Parms &P,
		    DsMdvx &D0, DsMdvx &D1, const MdvxProj &proj)
{
  const Mdvx::master_header_t m0 = D0.getMasterHeader();
  const Mdvx::master_header_t m1 = D1.getMasterHeader();
  if (!_compare_mhdr(m0, m1, false, P._all_fields))
  {
    _S.addInfo("master headers don't agree");
    _compare_mhdr(m0, m1, true, P._all_fields);
    _S._nmhdr_error++;
    _status = -1;
    _stats._nmhdr_error++;
  }

  vector<MdvxField *> f0, f1;
  vector<string> fieldNames;
  f0 = D0.getFields();
  f1 = D1.getFields();
  for (int i=0; i<(int)f0.size(); ++i)
  {
    string s = f0[i]->getFieldName();
    if (P.wanted_field(s))
    {
      fieldNames.push_back(s);
    }
  }
  for (int i=0; i<(int)f1.size(); ++i)
  {
    string s = f1[i]->getFieldName();
    if (P.wanted_field(s))
    {
      if (find(fieldNames.begin(), fieldNames.end(), s) == fieldNames.end())
	fieldNames.push_back(s);
    }
  }

  // same # of fields based on above, so load each field expect same
  for (int i=0; i<(int)fieldNames.size(); ++i)
  {
    _process_field(t, lts, P, fieldNames[i], D0, D1, proj, i == 0);
  }
}  

// /*----------------------------------------------------------------*/
// void _process_field(const time_t &t, const int lts, const Parms &P,
// 		    const string &fname, DsMdvx &D0, DsMdvx &D1,
// 		    const MdvxProj &proj, const bool first)
// {
//   _S.setField(fname);
//   _stats.setField(fname);

//   MdvxField *f0 = D0.getFieldByName(fname);
//   MdvxField *f1 = D1.getFieldByName(fname);
//   if (f0 == 0 || f1 == 0)
//   {
//     _S.fieldMissingInOne();
//     _stats.fieldMissingInOne();
//     _status = -1;
//     return;
//   }

//   Mdvx::field_header_t fh0 = f0->getFieldHeader();
//   Mdvx::field_header_t fh1 = f1->getFieldHeader();

//   if (!_compare_fhdr(fh0, fh1, false))
//   {
//     _S.addInfo("field headers don't agree, field=%s", fname.c_str());
//     _compare_fhdr(fh0, fh1, true);
//     _S.fieldHdrError();
//     _stats.fieldHdrError();
//     _status = -1;
//   }

//   f0->convertType(Mdvx::ENCODING_FLOAT32, Mdvx::COMPRESSION_NONE);
//   fl32 *data0 = (fl32 *)f0->getVol();
//   f1->convertType(Mdvx::ENCODING_FLOAT32, Mdvx::COMPRESSION_NONE);
//   fl32 *data1 = (fl32 *)f1->getVol();

//   bool gotf0, gotf1;
//   FcstGrid gnew, gold;
//   if (!P._f0.loadFcstData(t, lts, P._proj, fname, gnew))
//   {
//     _S.addInfo("No data url=%s field=%s", P._f0.pUrl.c_str(), fname.c_str());
//     _S.fieldReadError();
//     _stats.fieldReadError();
//     _status = -1;
//     gotf0 = false;
//   }
//   else
//     gotf0 = true;
//   if (!P._f1.loadFcstData(t, lts, P._proj, fname, gold))
//   {
//     _S.addInfo("No data url=%s field=%s", P._f1.pUrl.c_str(), fname.c_str());
//     _S.fieldReadError();
//     _stats.fieldReadError();
//     _status = -1;
//     gotf1 = false;
//   }
//   else
//     gotf1 = true;
//   if (gotf1 && gotf0)
//   {
//     if (first && P.compare_metadata)
//     {
//       if (!_fcstinfo_compare(t, lts, gold, gnew, fname))
//       {
// 	_stats._nmetadata_diff++;
// 	_status = -1;
// 	_S._nmetadata_diff++;
// 	return;
//       }
//     }
//     _compare(P, t, lts, fname, gold, gnew);
//   }
// }

/*----------------------------------------------------------------*/
bool _fcstinfo_compare(const time_t &t, const int lts, 
		       const DsMdvx &D0, const DsMdvx &D1,
		       const string &fname)
{
  int nc0 = D0.getNChunks();
  int nc1 = D1.getNChunks();
  if (nc0 != nc1)
  {
    _S.addInfo("Metdata not same\n%d chunks versus %d chunks\n", nc0, nc1);
    return false;
  }
  string msg = "";
  for (int i=0; i<nc0; ++i)
  {
    MdvxChunk *c0, *c1;
    c0 = D0.getChunkByNum(i);
    c1 = D1.getChunkByNum(i);
    if (c0->getId() != c1->getId())
    {
      char buf[1000];
      sprintf(buf, "Chunk[%d], Chunkid %d versus %d\n", i,
	      nc0, nc1);
      msg += buf;
    }
    else
    {
      // this is a little weak for comparisons, to say the least,
      // works for current needs.
      if (c0->getId() != Mdvx::CHUNK_COMMENT)
      {
	printf("Warning, chunk %d not a CHUNK_COMMMENT, ignored\n", i);
	continue;
      }
      int n0 = c0->getSize();
      const char *d0 = (const char *)c0->getData();
      int n1 = c1->getSize();
      const char *d1 = (const char *)c1->getData();
      
      // make sure it is null terminated by a copy
      char *buf0 = new char[n0+1];
      char *buf1 = new char[n1+1];
      strncpy(buf0, d0, n0);
      strncpy(buf1, d1, n1);
      string s0 = buf0;
      string s1 = buf1;
      if (s0 != s1)
      {
	// evaluate which parts are different by writing to temporary
	// files and doing 'diff'
	char buf[1000];
	sprintf(buf, "Chunk[%d],\n", i);
	msg += buf;
	msg += _evaluateChunkDiff(i, s0, s1);
      }
      delete buf0;
      delete buf1;
    }
  }
  if (!msg.empty())
  {
    _S.addInfo("Metadata not same\n%s", msg.c_str());
    return false;
  }
  else
    return true;
}

/*----------------------------------------------------------------*/
bool _compare_mhdr(const Mdvx::master_header_t &m0,
		   const Mdvx::master_header_t &m1, const bool print,
		   const bool all_fields)
{
  bool stat = true;

  if (m0.time_gen != m1.time_gen)
  {
    if (print)
      _S.addInfo("\tMaster_hdr:time_gen %d %d", m0.time_gen, m1.time_gen);
    stat = false;
  }
  if (m0.time_begin != m1.time_begin)
  {
    if (print)
      _S.addInfo("\tMaster_hdr:time_begin %d %d", m0.time_begin, m1.time_begin);
    stat = false;
  }
  if (m0.time_end != m1.time_end)
  {
    if (print)
      _S.addInfo("\tMaster_hdr:time_end %d %d", m0.time_end, m1.time_end);
    stat = false;
  }
  if (m0.time_centroid != m1.time_centroid)
  {
    if (print)
      _S.addInfo("\tMaster_hdr:time_centroid %d %d", m0.time_centroid, m1.time_centroid);
    stat = false;
  }
  if (m0.time_expire != m1.time_expire)
  {
    if (print)
      _S.addInfo("\tMaster_hdr:time_expire %d %d", m0.time_expire, m1.time_expire);
    stat = false;
  }
  if (m0.num_data_times != m1.num_data_times)
  {
    if (print)
      _S.addInfo("\tMaster_hdr:num_data_times %d %d", m0.num_data_times,
	       m1.num_data_times);
    stat = false;
  }

  if (m0.data_dimension != m1.data_dimension)
  {
    if (print)
      _S.addInfo("\tMaster_hdr:data_dimension %d %d", m0.data_dimension,
	       m1.data_dimension);
    stat = false;
  }
  if (m0.data_collection_type != m1.data_collection_type)
  {
    if (print)
      _S.addInfo("\tMaster_hdr:data_collection_type %d %d",
	       m0.data_collection_type, m1.data_collection_type);
    stat = false;
  }
  if (m0.native_vlevel_type != m1.native_vlevel_type)
  {
    if (print)
      _S.addInfo("\tMaster_hdr:native_vlevel_type %d %d",
	       m0.native_vlevel_type, m1.native_vlevel_type);
    stat = false;
  }
  if (m0.vlevel_type != m1.vlevel_type)
  {
    if (print)
      _S.addInfo("\tMaster_hdr:vlevel_type %d %d", m0.vlevel_type, m1.vlevel_type);
    stat = false;
  }
  if (m0.vlevel_included != m1.vlevel_included)
  {
    if (print)
      _S.addInfo("\tMaster_hdr:vlevel_included %d %d",
	       m0.vlevel_included, m1.vlevel_included);
    stat = false;
  }
  if (m0.grid_orientation != m1.grid_orientation)
  {
    if (print)
      _S.addInfo("\tMaster_hdr:grid_orientation %d %d",
	       m0.grid_orientation, m1.grid_orientation);
    stat = false;
  }
  if (m0.data_ordering != m1.data_ordering)
  {
    if (print)
      _S.addInfo("\tMaster_hdr:data_ordering %d %d", m0.data_ordering, m1.data_ordering);
    stat = false;
  }
  if (all_fields)
  {
    if (m0.n_fields != m1.n_fields)
    {
      if (print)
	_S.addInfo("\tMaster_hdr:n_fields %d %d", m0.n_fields, m1.n_fields);
      stat = false;
    }
  }
  if (m0.max_nx != m1.max_nx)
  {
    if (print)
      _S.addInfo("\tMaster_hdr:max_nx %d %d", m0.max_nx, m1.max_nx);
    stat = false;
  }
  if (m0.max_ny != m1.max_ny)
  {
    if (print)
      _S.addInfo("\tMaster_hdr:max_ny %d %d", m0.max_ny, m1.max_ny);
    stat = false;
  }
  if (m0.max_nz != m1.max_nz)
  {
    if (print)
      _S.addInfo("\tMaster_hdr:max_nz %d %d", m0.max_nz, m1.max_nz);
    stat = false;
  }
  if (m0.n_chunks != m1.n_chunks)
  {
    if (print)
      _S.addInfo("\tMaster_hdr:n_chunks %d %d", m0.n_chunks, m1.n_chunks);
    stat = false;
  }
  if (m0.forecast_time != m1.forecast_time)
  {
    if (print)
      _S.addInfo("\tMaster_hdr:forecast_time %d %d", m0.forecast_time, m1.forecast_time);
    stat = false;
  }
  if (m0.forecast_delta != m1.forecast_delta)
  {
    if (print)
      _S.addInfo("\tMaster_hdr:forecast_delta %d %d", m0.forecast_delta,
	       m1.forecast_delta);
    stat = false;
  }
  if (m0.sensor_lon != m1.sensor_lon)
  {
    if (print)
      _S.addInfo("\tMaster_hdr:sensor_lon %f %f", m0.sensor_lon, m1.sensor_lon);
    stat = false;
  }
  if (m0.sensor_lat != m1.sensor_lat)
  {
    if (print)
      _S.addInfo("\tMaster_hdr:sensor_lat %f %f", m0.sensor_lat, m1.sensor_lat);
    stat = false;
  }
  if (m0.sensor_alt != m1.sensor_alt)
  {
    if (print)
      _S.addInfo("\tMaster_hdr:sensor_alt %f %f", m0.sensor_alt, m1.sensor_alt);
    stat = false;
  }

//   if (strcmp(m0.data_set_info, m1.data_set_info) != 0)
//   {
//     if (print)
//       _S.addInfo("\tMaster_hdr:data_set_info %s %s", m0.data_set_info, m1.data_set_info);
//     stat = false;
//   }
//   if (strcmp(m0.data_set_name, m1.data_set_name) != 0)
//   {
//     if (print)
//       _S.addInfo("\tMaster_hdr:data_set_info %s %s", m0.data_set_info, m1.data_set_info);
//     stat = false;
//   }
//   if (strcmp(m0.data_set_source, m1.data_set_source) != 0)
//   {
//     if (print)
//       _S.addInfo("\tMaster_hdr:data_set_info %s %s", m0.data_set_info, m1.data_set_info);
//     stat = false;
//   }
  return stat;
}

/*----------------------------------------------------------------*/
bool _compare_fhdr(const Mdvx::field_header_t &f0,
		   const Mdvx::field_header_t &f1, const bool print)
{
  bool stat = true;
  if (f0.forecast_delta != f1.forecast_delta)
  {
    if (print)
      _S._current->addInfo("\tField_Hdr:forecast_delta %d %d", f0.forecast_delta, f1.forecast_delta);
    stat = false;
  }
  if (f0.forecast_time != f1.forecast_time)
  {
    if (print)
      _S._current->addInfo("\tField_Hdr:forecast_time %d %d", f0.forecast_time, f1.forecast_time);
    stat = false;
  }
  if (f0.nx != f1.nx)
  {
    if (print)
      _S._current->addInfo("\tField_Hdr:nx %d %d", f0.nx, f1.nx);
    stat = false;
  }
  if (f0.ny != f1.ny)
  {
    if (print)
      _S._current->addInfo("\tField_Hdr:ny %d %d", f0.ny, f1.ny);
    stat = false;
  }
  if (f0.nz != f1.nz)
  {
    if (print)
      _S._current->addInfo("\tField_Hdr:nz %d %d", f0.nz, f1.nz);
    stat = false;
  }
  if (f0.proj_type != f1.proj_type)
  {
    if (print)
      _S._current->addInfo("\tField_Hdr:proj_type %d %d", f0.proj_type, f1.proj_type);
    stat = false;
  }
  if (f0.encoding_type != f1.encoding_type)
  {
    if (print)
      _S._current->addInfo("\tField_Hdr:encoding_type %d %d", f0.encoding_type, f1.encoding_type);
    stat = false;
  }
  if (f0.data_element_nbytes != f1.data_element_nbytes)
  {
    if (print)
      _S._current->addInfo("\tField_Hdr:data_element_nbytes %d %d", f0.data_element_nbytes, f1.data_element_nbytes);
    stat = false;
  }
  if (f0.compression_type != f1.compression_type)
  {
    if (print)
      _S._current->addInfo("\tField_Hdr:compression_type %d %d", f0.compression_type, f1.compression_type);
    stat = false;
  }
  if (f0.transform_type != f1.transform_type)
  {
    if (print)
      _S._current->addInfo("\tField_Hdr:transform_type %d %d", f0.transform_type, f1.transform_type);
    stat = false;
  }
  if (f0.scaling_type != f1.scaling_type)
  {
    if (print)
      _S._current->addInfo("\tField_Hdr:scaling_type %d %d", f0.scaling_type, f1.scaling_type);
    stat = false;
  }
  if (f0.native_vlevel_type != f1.native_vlevel_type)
  {
    if (print)
      _S._current->addInfo("\tField_Hdr:native_vlevel_type %d %d", f0.native_vlevel_type, f1.native_vlevel_type);
    stat = false;
  }
  if (f0.vlevel_type != f1.vlevel_type)
  {
    if (print)
      _S._current->addInfo("\tField_Hdr:vlevel_type %d %d", f0.vlevel_type, f1.vlevel_type);
    stat = false;
  }
  if (f0.dz_constant != f1.dz_constant)
  {
    if (print)
      _S._current->addInfo("\tField_Hdr:dz_constant %d %d", f0.dz_constant, f1.dz_constant);
    stat = false;
  }
  if (f0.data_dimension != f1.data_dimension)
  {
    if (print)
      _S._current->addInfo("\tField_Hdr:data_dimension %d %d", f0.data_dimension, f1.data_dimension);
    stat = false;
  }
  if (f0.zoom_clipped != f1.zoom_clipped)
  {
    if (print)
      _S._current->addInfo("\tField_Hdr:zoom_clipped %d %d", f0.zoom_clipped, f1.zoom_clipped);
    stat = false;
  }
  if (f0.zoom_no_overlap != f1.zoom_no_overlap)
  {
    if (print)
      _S._current->addInfo("\tField_Hdr:zoom_no_overlap %d %d", f0.zoom_no_overlap, f1.zoom_no_overlap);
    stat = false;
  }
  if (f0.proj_origin_lat != f1.proj_origin_lat)
  {
    if (print)
      _S._current->addInfo("\tField_Hdr:proj_origin_lat %f %f", f0.proj_origin_lat, f1.proj_origin_lat);
    stat = false;
  }

  if (f0.proj_origin_lon != f1.proj_origin_lon)
  {
    if (print)
      _S._current->addInfo("\tField_Hdr:proj_origin_lon %f %f", f0.proj_origin_lon, f1.proj_origin_lon);
    stat = false;
  }
  if (f0.vert_reference != f1.vert_reference)
  {
    if (print)
      _S._current->addInfo("\tField_Hdr:vert_reference %f %f", f0.vert_reference, f1.vert_reference);
    stat = false;
  }
  if (f0.grid_dx != f1.grid_dx)
  {
    if (print)
      _S._current->addInfo("\tField_Hdr:grid_dx %f %f", f0.grid_dx, f1.grid_dx);
    stat = false;
  }
  if (f0.grid_dy != f1.grid_dy)
  {
    if (print)
      _S._current->addInfo("\tField_Hdr:grid_dy %f %f", f0.grid_dy, f1.grid_dy);
    stat = false;
  }
  if (f0.grid_dz != f1.grid_dz)
  {
    if (print)
      _S._current->addInfo("\tField_Hdr:grid_dz %f %f", f0.grid_dz, f1.grid_dz);
    stat = false;
  }
  if (f0.grid_minx != f1.grid_minx)
  {
    if (print)
      _S._current->addInfo("\tField_Hdr:grid_minx %f %f", f0.grid_minx, f1.grid_minx);
    stat = false;
  }
  if (f0.grid_miny != f1.grid_miny)
  {
    if (print)
      _S._current->addInfo("\tField_Hdr:grid_miny %f %f", f0.grid_miny, f1.grid_miny);
    stat = false;
  }
  if (f0.grid_minz != f1.grid_minz)
  {
    if (print)
      _S._current->addInfo("\tField_Hdr:grid_minz %f %f", f0.grid_minz, f1.grid_minz);
    stat = false;
  }
//   if (f0.scale != f1.scale)
//   {
//     if (print)
//       _S._current->addInfo("\tField_Hdr:scale %f %f", f0.scale, f1.scale);
//     stat = false;
//   }
//   if (f0.bias != f1.bias)
//   {
//     if (print)
//       _S._current->addInfo("\tField_Hdr:bias %f %f", f0.bias, f1.bias);
//     stat = false;
//   }
//   if (f0.bad_data_value != f1.bad_data_value)
//   {
//     if (print)
//       _S._current->addInfo("\tField_Hdr:bad_data_value %f %f", f0.bad_data_value, f1.bad_data_value);
//     stat = false;
//   }
//   if (f0.missing_data_value != f1.missing_data_value)
//   {
//     if (print)
//       _S._current->addInfo("\tField_Hdr:missing_data_value %f %f", f0.missing_data_value, f1.missing_data_value);
//     stat = false;
//   }
  if (f0.proj_rotation != f1.proj_rotation)
  {
    if (print)
      _S._current->addInfo("\tField_Hdr:proj_rotation %f %f", f0.proj_rotation, f1.proj_rotation);
    stat = false;
  }
//   if (f0.min_value != f1.min_value)
//   {
//     if (print)
//       _S._current->addInfo("\tField_Hdr:min_value %f %f", f0.min_value, f1.min_value);
//     stat = false;
//   }
//   if (f0.max_value != f1.max_value)
//   {
//     if (print)
//       _S._current->addInfo("\tField_Hdr:max_value %f %f", f0.max_value, f1.max_value);
//     stat = false;
//   }
//   if (f0.min_value_orig_vol != f1.min_value_orig_vol)
//   {
//     if (print)
//       _S._current->addInfo("\tField_Hdr:min_value_orig_vol %f %f", f0.min_value_orig_vol, f1.min_value_orig_vol);
//     stat = false;
//   }
//   if (f0.max_value_orig_vol != f1.max_value_orig_vol)
//   {
//     if (print)
//       _S._current->addInfo("\tField_Hdr:max_value_orig_vol %f %f", f0.max_value_orig_vol, f1.max_value_orig_vol);
//     stat = false;
//   }
  if (strcmp(f0.field_name_long, f1.field_name_long) != 0)
  {
    if (print)
      _S._current->addInfo("\tField_Hdr:field_name_long %s %s", f0.field_name_long, f1.field_name_long);
    stat = false;
  }
  if (strcmp(f0.field_name, f1.field_name) != 0)
  {
    if (print)
      _S._current->addInfo("\tField_Hdr:field_name %s %s", f0.field_name, f1.field_name);
    stat = false;
  }
  if (strcmp(f0.units, f1.units) != 0)
  {
    if (print)
      _S._current->addInfo("\tField_Hdr:units %s %s", f0.units, f1.units);
    stat = false;
  }
  if (strcmp(f0.transform, f1.transform) != 0)
  {
    if (print)
      _S._current->addInfo("\tField_Hdr:transform %s %s", f0.transform, f1.transform);
    stat = false;
  }
  return stat;
}

/*----------------------------------------------------------------*/
string _evaluateChunkDiff(const int i, const string &s0, const string &s1)
{
  // probably a better way!!
  string ret;

  FILE *fp = fopen("./temp.0.txt", "w");
  fprintf(fp, s0.c_str());
  fclose(fp);
  fp = fopen("./temp.1.txt", "w");
  fprintf(fp, s1.c_str());
  fclose(fp);
  system("diff ./temp.0.txt ./temp.1.txt > ./temp.diff.txt");
  if (!_loadFile("./temp.diff.txt", ret))
    ret = "ERROR";
  return ret;
}

//------------------------------------------------------------------
bool _loadFile(const string &fname, string &inp)
{
  stat_struct_t fileStat;

  if (ta_stat(fname.c_str(), &fileStat))
  {
    printf("Cannot stat file %s\n",fname.c_str());
    return false;
  }
  int fileLen = static_cast<int>(fileStat.st_size);
  
  // open file
  FILE *in;
  if ((in = fopen(fname.c_str(), "r")) == NULL)
  {
    printf("Cannot open file %s\n",fname.c_str());
    return false;
  }

  // read in buffer

  char *fileBuf = new char[fileLen + 1];
  if (static_cast<int>(fread(fileBuf, 1, fileLen, in)) != fileLen)
  {
    printf("Cannot read file %s\n", fname.c_str());
    fclose(in);
    delete[] fileBuf;
    return false;
  }

  // ensure null termination
  fileBuf[fileLen] = '\0';

  // close file
  fclose(in);

  // put into the string
  inp = fileBuf;
  delete[] fileBuf;
  return true;
}

