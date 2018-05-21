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
// TrConsolidate.cc
//
// TrConsolidate class
//
// Mike Dixon, RAP, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 1999
//
///////////////////////////////////////////////////////////////

#include "TrConsolidate.hh"
#include <rapmath/math_macros.h>
using namespace std;

//////////////
// constructor
//

TrConsolidate::TrConsolidate(const string &prog_name, const Params &params) :
  Worker(prog_name, params)

{

}

/////////////
// destructor
//

TrConsolidate::~TrConsolidate()

{

}

/****************************************************************************
 * run
 *
 * Storms which merge and/or split may have different complex track numbers.
 * These complex tracks must be combined.
 *
 * This routine searches for groups of storms which are part of a
 * common merger/split. The complex tracks in this group are
 * consolidated.
 *
 * Returns 0 on success, -1 on failure.
 *
 ****************************************************************************/

int TrConsolidate::run(TitanTrackFile &tfile,
		       vector<TrStorm*> &storms1,
		       vector<TrStorm*> &storms2,
		       vector<track_utime_t> &track_utime)
  
{

  /*
   * initialize storms2 array for checking
   */
  
  for (size_t jstorm = 0; jstorm < storms2.size(); jstorm++) {
    storms2[jstorm]->status.checked = false;
  }
  
  /*
   * loop through each storm2 entry
   */
  
  for (size_t jstorm = 0; jstorm < storms2.size(); jstorm++) {
    
    TrStorm &storm = *storms2[jstorm];
    
    if (!storm.status.checked &&
	(storm.status.has_merger || storm.status.has_split)) {
      
      storm.status.checked = true;
      _complexNums.clear();
      _storm1Nums.clear();

      for (int kstorm = 0; kstorm < storm.status.n_match; kstorm++) {
	
	/*
	 * search recursively to find all complex tracks in the
	 * merger / split
	 */
	
	if (_params.debug >= Params::DEBUG_VERBOSE) {
	  fprintf(stderr, "STARTING_SEARCH ....\n");
	}

	_process_storms1_entry(storms1, storms2,
			       storm.match_array[kstorm].storm_num, jstorm);
      } /* kstorm */
      
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	fprintf(stderr, "#### COMMON COMPLEX NUMS - Storm2 array %ld ####\n",
		(long) jstorm);
	for (size_t i = 0; i < _complexNums.size(); i++) {
	  fprintf(stderr, "%d/%d", _storm1Nums[i], _complexNums[i]);
	  if (i == _complexNums.size() - 1) {
	    fprintf(stderr, "\n");
	  } else {
	    fprintf(stderr, " ");
	  }
	} /* i */
      }
      
      /*
       * find min complex num
       */

      int min_complex = 999999999;
      for (size_t i = 0; i < _complexNums.size(); i++) {
	min_complex = MIN(min_complex, _complexNums[i]);
      }

      /*
       * consolidate the tracks, using the min complex number
       */
      
      for (size_t i = 0; i < _complexNums.size(); i++) {
	if (_complexNums[i] != min_complex) {
	  if (_consolidate(tfile, storms1, min_complex,
			   _complexNums[i], track_utime)) {
	    return (-1);
	  }
	}
      } /* i */
      
    } /*  if (!storm.status.checked) */

  } /* jstorm */
  
 return (0);

}

/****************************************************************************
 * _consolidate()
 *
 * Consolidates the file entries for two complex storm tracks - this
 * is required when two complex tracks merge, forming one complex
 * track. The consolidation is done in favor of the first track, which
 * is the older of the two.
 *
 * The complex tracks (if any) with a higher number than the vacated
 * position are moved down to fill the gap. One entry for a complex
 * track (the highest one) will be left vacant in the file until an
 * additional complex track is required.
 *
 * Returns 0 on success, -1 on failure.
 *
 ****************************************************************************/

int TrConsolidate::_consolidate(TitanTrackFile &tfile,
				vector<TrStorm*> &storms1,
				int lower_track_num,
				int higher_track_num,
				vector<track_utime_t> &track_utime)

{

  int simple_num;
  int ntracks1, ntracks2, n_simple_tracks;
  int nentries;
  int start_scan, end_scan;
  int start_time, end_time;
  int *simples1, *simples2;
  complex_track_params_t ctparams1, ctparams2;

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    fprintf(stderr, "\nConsolidating complex tracks %d and %d\n",
	    lower_track_num, higher_track_num);
  }

  /*
   * check that first track is older
   */

  if (lower_track_num >= higher_track_num) {

    fprintf(stderr, "ERROR - %s:TrConsolidate.\n",
	    _progName.c_str());
    fprintf(stderr, "Tracks given in wrong order.\n");
    fprintf(stderr, "First track num %ld\n", (long) lower_track_num);
    fprintf(stderr, "Second track num %ld\n", (long) higher_track_num);
    return (-1);
    
  }

  /*
   * read in the two track structs
   */

  if (tfile.ReadComplexParams(lower_track_num, false)) {
    cerr << "ERROR - " << _progName << "TrConsolidate::_consolidate" << endl;
    cerr << tfile.getErrStr() << endl;
    return -1;
  }

  ctparams1 = tfile._complex_params;
  ntracks1 = ctparams1.n_simple_tracks;

  if (tfile.ReadComplexParams(higher_track_num, false)) {
    cerr << "ERROR - " << _progName << "TrConsolidate::_consolidate" << endl;
    cerr << tfile.getErrStr() << endl;
    return -1;
  }

  ctparams2 = tfile._complex_params;
  ntracks2 = ctparams2.n_simple_tracks;
  
  n_simple_tracks = ntracks1 + ntracks2;

  /*
   * realloc the simples_per_complex array
   */
  
  tfile._simples_per_complex[lower_track_num] = (si32 *) urealloc
    (tfile._simples_per_complex[lower_track_num],
     ((ntracks1 + ntracks2) * sizeof(si32)));
  
  /*
   * compute scan and time values for the consolidated track
   */

  if (ctparams1.start_scan < ctparams2.start_scan) {

    start_scan = ctparams1.start_scan;
    start_time = ctparams1.start_time;

  } else {

    start_scan = ctparams2.start_scan;
    start_time = ctparams2.start_time;

  } /* if (ctparams1.start_scan < ctparams2.start_scan) */

  if (ctparams1.end_scan > ctparams2.end_scan) {

    end_scan = ctparams1.end_scan;
    end_time = ctparams1.end_time;

  } else {

    end_scan = ctparams2.end_scan;
    end_time = ctparams2.end_time;

  } /* if (ctparams1.end_scan > ctparams2.end_scan) */

  /*
   * put consolidated values into track number 1
   */
  
  ctparams1.start_scan = start_scan;
  ctparams1.duration_in_scans = end_scan - start_scan + 1;
  ctparams1.start_time = start_time;
  ctparams1.end_time = end_time;

  if (ctparams1.duration_in_scans > 1) {
    ctparams1.duration_in_secs =
      (si32) ((end_time - start_time) * 86400.0
	      * (ctparams1.duration_in_scans /
		 (ctparams1.duration_in_scans - 1.0)) + 0.5);
  } else {
    ctparams1.duration_in_secs = 0;
  }
  
  ctparams1.n_simple_tracks = n_simple_tracks;
  tfile._nsimples_per_complex[lower_track_num] = ctparams1.n_simple_tracks;

  track_utime[lower_track_num].start_complex = start_time;
  track_utime[lower_track_num].end_complex = end_time;
  
  /*
   * append to the simple track number list and sort it
   */

  simples1 = tfile._simples_per_complex[lower_track_num];
  simples2 = tfile._simples_per_complex[higher_track_num];
  
  for (int isimple = 0; isimple < ntracks2; isimple++) {
    simples1[ntracks1 + isimple] = simples2[isimple];
  } /* isimple */
  
  qsort((char *) simples1, (int) (n_simple_tracks),
	sizeof(si32), _compare_si32s);
  
  /*
   * write out amended complex track params
   */
  
  tfile._complex_params = ctparams1;
  
  if (tfile.WriteComplexParams(lower_track_num)) {
    cerr << "ERROR - " << _progName << "TrConsolidate::_consolidate" << endl;
    cerr << tfile.getErrStr() << endl;
    return -1;
  }

  /*
   * loop through the simple tracks which make up complex track 2
   * changing the complex track number to track 1
   */

  for (int isimple = 0; isimple < ntracks2; isimple++) {

    simple_num = simples2[isimple];

    /*
     * read in simple params
     */

    if (tfile.RewindSimple(simple_num)) {
      cerr << "ERROR - " << _progName << "TrConsolidate::_consolidate" << endl;
      cerr << tfile.getErrStr() << endl;
      return -1;
    }

    /*
     * change complex track num
     */
    
    tfile._simple_params.complex_track_num = lower_track_num;

    /*
     * write amended simple params
     */

    if (tfile.WriteSimpleParams(simple_num)) {
      cerr << "ERROR - " << _progName << "TrConsolidate::_consolidate" << endl;
      cerr << tfile.getErrStr() << endl;
      return -1;
    }

    /*
     * amend track number in each track entry
     */

    nentries = tfile._simple_params.duration_in_scans;
    
    for (int ientry = 0; ientry < nentries; ientry++) {

      if (tfile.ReadEntry()) {
	cerr << "ERROR - " << _progName
	     << "TrConsolidate::_consolidate" << endl;
	cerr << tfile.getErrStr() << endl;
	return -1;
      }

      tfile._entry.complex_track_num = lower_track_num;

      if (tfile.RewriteEntry()) {
	cerr << "ERROR - " << _progName
	     << "TrConsolidate::_consolidate" << endl;
	cerr << tfile.getErrStr() << endl;
	return -1;
      }
      
    } /* ientry */
    
  } /* isimple */

  /*
   * search for the position of the higher complex track number in
   * the complex track num array
   */

  int icomplex;
  for (icomplex = 0;
       icomplex < tfile._header.n_complex_tracks; icomplex++) {
    
    if (tfile._complex_track_nums[icomplex] == higher_track_num) {
      break;
    }
    
  } /* icomplex */
  
  if (icomplex == tfile._header.n_complex_tracks) {
    
    fprintf(stderr, "ERROR - %s:TrConsolidate.\n",
	    _progName.c_str());
    fprintf(stderr, "Problem with complex_track_nums array.\n");
    fprintf(stderr,
	    "Complex track num %d not found.\n", higher_track_num);
    return(-1);
    
  }
  
  /*
   * for all complex tracks with numbers greater than higher_track_num,
   * move their entries down in the complex_track_num array
   */

  for (int jcomplex = icomplex;
       jcomplex < tfile._header.n_complex_tracks - 1; jcomplex++) {
    
    tfile._complex_track_nums[jcomplex] = 
      tfile._complex_track_nums[jcomplex + 1];

  } /* jcomplex */

  int itrack = tfile._header.n_complex_tracks - 1;
  tfile._complex_track_nums[itrack] = 0;
  
  /*
   * free up the complex params for the higher track num
   */
  
  if (tfile.ReuseComplexSlot(higher_track_num)) {
    cerr << "ERROR - " << _progName << "TrConsolidate::_consolidate" << endl;
    cerr << tfile.getErrStr() << endl;
    return -1;
  }

  /*
   * free up simples_per_complex for higher track num
   */
  
  ufree((char *) tfile._simples_per_complex[higher_track_num]);
  tfile._simples_per_complex[higher_track_num] = NULL;
      
  /*
   * reduce number of complex tracks by 1
   */

  tfile._header.n_complex_tracks--;

  /*
   * write header
   */

  if (tfile.WriteHeader()) {
    cerr << "ERROR - " << _progName << "TrConsolidate::_consolidate" << endl;
    cerr << tfile.getErrStr() << endl;
    return -1;
  }

  /*
   * update storms entries for complex track number
   */

  for (size_t istorm = 0; istorm < storms1.size(); istorm++) {
    if (storms1[istorm]->track.status.complex_track_num == higher_track_num) {
      storms1[istorm]->track.status.complex_track_num = lower_track_num;
      storms1[istorm]->track.status.n_simple_tracks = n_simple_tracks;
    }
  }

  return (0);

}

/*****************************************************************************
 * define function to be used for sorting si32's (lowest to highest)
 */

int TrConsolidate::_compare_si32s(const void *v1, const void *v2)
  

{

    si32 *l1, *l2;

    l1 = (si32 *) v1;
    l2 = (si32 *) v2;

    return (*l1 - *l2);

}

/*************************
 * _process_storms1_entry()
 *
 * Process storms1 entry for complex nums
 */

void TrConsolidate::_process_storms1_entry(vector<TrStorm*> &storms1,
					   vector<TrStorm*> &storms2,
					   int storm1_num,
					   int storm2_num)

{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    fprintf(stderr,
	    "PROCESS_STORMS1_ENTRY: storm1_num, storm2_num: %d, %d\n",
	    storm1_num, storm2_num);
  }
  
  TrStorm &storm1 = *storms1[storm1_num];

  bool already_in_list = false;
  for (size_t i = 0; i < _complexNums.size(); i++) {
    if (_complexNums[i] == storm1.track.status.complex_track_num) {
      already_in_list = true;
      break;
    }
  } /* i */

  if (!already_in_list) {
    _complexNums.push_back(storm1.track.status.complex_track_num);
    _storm1Nums.push_back(storm1_num);
  }

  for (int istorm = 0; istorm < storm1.status.n_match; istorm++) {
    if (storm1.match_array[istorm].storm_num != storm2_num) {
      _process_storms2_entry(storms1, storms2, storm1_num,
			     storm1.match_array[istorm].storm_num);
    }
  } /* istorm */

  return;

}

/*************************
 * _process_storms2_entry()
 *
 * Process storms2 entry for complex nums
 */

void TrConsolidate::_process_storms2_entry(vector<TrStorm*> &storms1,
					   vector<TrStorm*> &storms2,
					   int storm1_num,
					   int storm2_num)

{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    fprintf(stderr,
	    "PROCESS_STORMS2_ENTRY: storm1_num, storm2_num: %d, %d\n",
	    storm1_num, storm2_num);
  }
	  
  TrStorm &storm2 = *storms2[storm2_num];

  if (!storm2.status.checked) {
    
    storm2.status.checked = true;

    for (int jstorm = 0; jstorm < storm2.status.n_match; jstorm++) {
      if (storm2.match_array[jstorm].storm_num != storm1_num) {
	_process_storms1_entry(storms1, storms2,
			       storm2.match_array[jstorm].storm_num,
			       storm2_num);
      }
    } /* jstorm */

  } /* if (!storm2.status.checked) */

  return;

}

