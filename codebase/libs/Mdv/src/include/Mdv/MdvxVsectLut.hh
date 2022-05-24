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
////////////////////////////////////////////////////////////////////
// Mdv/MdvxVsectLut.hh
//
// An object of this class is used to hold the lookup table for
// computing vertical sections.
//
// Mike Dixon, RAP, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// November 1999
//
////////////////////////////////////////////////////////////////////

#ifndef MdvxVsectLut_hh
#define MdvxVsectLut_hh

#include <Mdv/Mdvx.hh>
#include <Mdv/MdvxProj.hh>
using namespace std;

class MdvxVsectLutEntry {
public:
  int64_t offsets[4];
  double wts[4];
  bool set;
  MdvxVsectLutEntry() {
    set = false;
  }
};

class MdvxVsectLut
{

public:

  ///////////////////////
  // default constructor
  //
  // You need to call computeSamplePts() and
  // computeOffsets() before using the lookup table.
  
  MdvxVsectLut();
  
  //////////////////////////////////////////
  // constructor which computes lookup table
  
  MdvxVsectLut(const vector<Mdvx::vsect_waypt_t> &waypts,
	       const int n_samples,
	       const MdvxProj &proj);
  
  ///////////////////////
  // destructor
  
  virtual ~MdvxVsectLut();

  /////////////////////
  // compute sample pts
  
  void computeSamplePts(const vector<Mdvx::vsect_waypt_t> &waypts,
			int n_samples);
  
  ///////////////////////////////
  // compute lookup table offsets
  // Must call computeSamplePts() first
  
  void computeOffsets(const MdvxProj &proj);
  
  ///////////////////////////////
  // compute lookup table offsets
  //
  // Does not recompute if no changes have occurred.

  void computeOffsets(const vector<Mdvx::vsect_waypt_t> &waypts,
		      const int n_samples,
		      const MdvxProj &proj);

  ///////////////////////////////
  // compute lookup table weights
  //
  // Must call computeSamplePts() first
  
  void computeWeights(const MdvxProj &proj);

  ////////////////////////////////
  // compute lookup table weights
  //
  // Does not recompute if no changes have occurred.
  
  void computeWeights(const vector<Mdvx::vsect_waypt_t> &waypts,
		      const int n_samples,
		      const MdvxProj &proj);

  // access to geometry

  const vector<Mdvx::vsect_waypt_t> &getWayPts() const {
    return (_wayPts);
  }
  const vector<Mdvx::vsect_samplept_t> &getSamplePts() const {
    return (_samplePts);
  }
  const vector<Mdvx::vsect_segment_t> &getSegments() const {
    return (_segments);
  }
  double getDxKm() const { return (_dxKm); }
  double getTotalLength() const { return (_totalLength); }

  const vector<int64_t> &getOffsets() const {
    return (_offsets);
  }

  const vector<MdvxVsectLutEntry> &getWeights() const {
    return (_weights);
  }

protected:
  
  vector<Mdvx::vsect_waypt_t> _wayPts;
  vector<Mdvx::vsect_samplept_t> _samplePts;
  vector<Mdvx::vsect_segment_t> _segments;
  int _nSamplesRequested;
  double _dxKm;
  double _totalLength;
  MdvxProj _proj;
  vector<int64_t> _offsets;
  vector<MdvxVsectLutEntry> _weights;
  bool _offsetsComputed;
  bool _weightsComputed;

  bool _geometryChanged(const vector<Mdvx::vsect_waypt_t> &waypts,
			const int n_samples,
			const MdvxProj &proj);

private:

};

#endif


