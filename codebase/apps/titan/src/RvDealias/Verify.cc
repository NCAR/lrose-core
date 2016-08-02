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
///////////////////////////////////////////////////////////////
// Verify.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2003
//
///////////////////////////////////////////////////////////////
//
// Verify writes out verification files
//
////////////////////////////////////////////////////////////////

#include <cerrno>
#include <toolsa/os_config.h>
#include <toolsa/mem.h>
#include <toolsa/file_io.h>
#include <toolsa/DateTime.hh>
#include "Verify.hh"
using namespace std;

///////////////
// Constructor

Verify::Verify(const Params &params) :
  _params(params)
  
{

  _isOK = true;
  _tableFile = NULL;
  
  // open the moments table file
  
  ta_makedir_recurse(_params.verification_dir);
  char tablePath[MAX_PATH_LEN];
  sprintf(tablePath, "%s%s%s", _params.verification_dir,
	  PATH_DELIM, "moments.table");
  if ((_tableFile = fopen(tablePath, "w")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - RvDealias::Verify" << endl;
    cerr << "  Cannot open moments table file";
    cerr << "  " << strerror(errNum) << endl;
    _isOK = false;
  }
  
  // write header

  fprintf(_tableFile, "# Computed and truth moments\n");
  fprintf(_tableFile, "# "
	  "estDbm1 truthDbm1 "
	  "estVel1 truthVel1 "
	  "estWidth1 truthWidth1"
	  "estDbm2 truthDbm2"
	  "estVel2 truthVel2"
	  "estWidth2 truthWidth2"
	  "r1Ratio dbmDiff\n");

  // set up error matrices

  _p1_175_225.setVarName("P1");
  _p1_175_225.setWidth2Limits(1.75, 2.25);
  _p1_175_225.setSdevCscale(0, 4.0, 0.5);
  _p1_175_225.setOutputDir(_params.verification_dir);

  _p1_275_325.setVarName("P1");
  _p1_275_325.setWidth2Limits(2.75, 3.25);
  _p1_275_325.setSdevCscale(0, 4.0, 0.5);
  _p1_275_325.setOutputDir(_params.verification_dir);

  _p1_375_425.setVarName("P1");
  _p1_375_425.setWidth2Limits(3.75, 4.25);
  _p1_375_425.setSdevCscale(0, 4.0, 0.5);
  _p1_375_425.setOutputDir(_params.verification_dir);

  _p2_175_225.setVarName("P2");
  _p2_175_225.setWidth2Limits(1.75, 2.25);
  _p2_175_225.setBiasCscale(-4, 4.0, 1.0);
  _p2_175_225.setSdevCscale(0, 4.0, 0.5);
  _p2_175_225.setOutputDir(_params.verification_dir);

  _p2_275_325.setVarName("P2");
  _p2_275_325.setWidth2Limits(2.75, 3.25);
  _p2_275_325.setBiasCscale(-4, 4.0, 1.0);
  _p2_275_325.setSdevCscale(0, 4.0, 0.5);
  _p2_275_325.setOutputDir(_params.verification_dir);
  
  _p2_375_425.setVarName("P2");
  _p2_375_425.setWidth2Limits(3.75, 4.25);
  _p2_375_425.setBiasCscale(-4, 4.0, 1.0);
  _p2_375_425.setSdevCscale(0, 4.0, 0.5);
  _p2_375_425.setOutputDir(_params.verification_dir);
  
  _v1_175_225.setVarName("V1");
  _v1_175_225.setWidth2Limits(1.75, 2.25);
  _v1_175_225.setSdevCscale(0, 4.0, 0.5);
  _v1_175_225.setOutputDir(_params.verification_dir);

  _v1_275_325.setVarName("V1");
  _v1_275_325.setWidth2Limits(2.75, 3.25);
  _v1_275_325.setSdevCscale(0, 4.0, 0.5);
  _v1_275_325.setOutputDir(_params.verification_dir);
  
  _v1_375_425.setVarName("V1");
  _v1_375_425.setWidth2Limits(3.75, 4.25);
  _v1_375_425.setSdevCscale(0, 4.0, 0.5);
  _v1_375_425.setOutputDir(_params.verification_dir);

  _v2_175_225.setVarName("V2");
  _v2_175_225.setWidth2Limits(1.75, 2.25);
  _v2_175_225.setBiasCscale(-4, 4.0, 1.0);
  _v2_175_225.setSdevCscale(0, 5.0, 0.5);
  _v2_175_225.setOutputDir(_params.verification_dir);

  _v2_275_325.setVarName("V2");
  _v2_275_325.setWidth2Limits(2.75, 3.25);
  _v2_275_325.setBiasCscale(-4, 4.0, 1.0);
  _v2_275_325.setSdevCscale(0, 5.0, 0.5);
  _v2_275_325.setOutputDir(_params.verification_dir);
  
  _v2_375_425.setVarName("V2");
  _v2_375_425.setWidth2Limits(3.75, 4.25);
  _v2_375_425.setBiasCscale(-4, 4.0, 1.0);
  _v2_375_425.setSdevCscale(0, 5.0, 0.5);
  _v2_375_425.setOutputDir(_params.verification_dir);

  _w1_175_225.setVarName("W1");
  _w1_175_225.setWidth2Limits(1.75, 2.25);
  _w1_175_225.setBiasCscale(-5.0, 5.0, 1.0);
  _w1_175_225.setOutputDir(_params.verification_dir);

  _w1_275_325.setVarName("W1");
  _w1_275_325.setWidth2Limits(2.75, 3.25);
  _w1_275_325.setBiasCscale(-5.0, 5.0, 1.0);
  _w1_275_325.setOutputDir(_params.verification_dir);
  
  _w1_375_425.setVarName("W1");
  _w1_375_425.setWidth2Limits(3.75, 4.25);
  _w1_375_425.setBiasCscale(-5.0, 5.0, 1.0);
  _w1_375_425.setOutputDir(_params.verification_dir);

  _w2_175_225.setVarName("W2");
  _w2_175_225.setWidth2Limits(1.75, 2.25);
  _w2_175_225.setBiasCscale(-8.0, 8.0, 1.0);
  _w2_175_225.setSdevCscale(0, 6.0, 1.0);
  _w2_175_225.setOutputDir(_params.verification_dir);

  _w2_275_325.setVarName("W2");
  _w2_275_325.setWidth2Limits(2.75, 3.25);
  _w2_275_325.setBiasCscale(-8.0, 8.0, 1.0);
  _w2_275_325.setSdevCscale(0, 6.0, 1.0);
  _w2_275_325.setOutputDir(_params.verification_dir);
  
  _w2_375_425.setVarName("W2");
  _w2_375_425.setWidth2Limits(3.75, 4.25);
  _w2_375_425.setBiasCscale(-8.0, 8.0, 1.0);
  _w2_375_425.setSdevCscale(0, 6.0, 1.0);
  _w2_375_425.setOutputDir(_params.verification_dir);

}

///////////////
// destructor

Verify::~Verify()

{

  if (_tableFile) {
    fclose(_tableFile);
  }

}

////////////////////////////////
// add to statistics

void Verify::addToStats(double estDbm1, double truthDbm1,
			double estVel1, double truthVel1,
			double estWidth1, double truthWidth1,
			double estDbm2, double truthDbm2,
			double estVel2, double truthVel2,
			double estWidth2, double truthWidth2,
			double r1Ratio)
  
{

  // first add a line to the table
  
  writeToTable(estDbm1, truthDbm1,
	       estVel1, truthVel1,
	       estWidth1, truthWidth1,
	       estDbm2, truthDbm2,
	       estVel2, truthVel2,
	       estWidth2, truthWidth2,
	       r1Ratio);

  // then add stats to the matrices

  // power trip 1

  _p1_175_225.addToStats(estDbm1, truthDbm1,
			 truthDbm1, truthDbm2,
			 truthWidth1, truthWidth2);
  
  _p1_275_325.addToStats(estDbm1, truthDbm1,
			 truthDbm1, truthDbm2,
			 truthWidth1, truthWidth2);
  
  _p1_375_425.addToStats(estDbm1, truthDbm1,
			 truthDbm1, truthDbm2,
			 truthWidth1, truthWidth2);

  // power trip 2
 
  _p2_175_225.addToStats(estDbm2, truthDbm2,
			 truthDbm1, truthDbm2,
			 truthWidth1, truthWidth2);
  
  _p2_275_325.addToStats(estDbm2, truthDbm2,
			 truthDbm1, truthDbm2,
			 truthWidth1, truthWidth2);
  
  _p2_375_425.addToStats(estDbm2, truthDbm2,
			 truthDbm1, truthDbm2,
			 truthWidth1, truthWidth2);
  
  // vel trip 1
  
  _v1_175_225.addToStats(estVel1, truthVel1,
			 truthDbm1, truthDbm2,
			 truthWidth1, truthWidth2);
  
  _v1_275_325.addToStats(estVel1, truthVel1,
			 truthDbm1, truthDbm2,
			 truthWidth1, truthWidth2);
  
  _v1_375_425.addToStats(estVel1, truthVel1,
			 truthDbm1, truthDbm2,
			 truthWidth1, truthWidth2);

  // vel trip 2
 
  _v2_175_225.addToStats(estVel2, truthVel2,
			 truthDbm1, truthDbm2,
			 truthWidth1, truthWidth2);
  
  _v2_275_325.addToStats(estVel2, truthVel2,
			 truthDbm1, truthDbm2,
			 truthWidth1, truthWidth2);
  
  _v2_375_425.addToStats(estVel2, truthVel2,
			 truthDbm1, truthDbm2,
			 truthWidth1, truthWidth2);

  // width trip 1
 
  _w1_175_225.addToStats(estWidth1, truthWidth1,
			 truthDbm1, truthDbm2,
			 truthWidth1, truthWidth2);
  
  _w1_275_325.addToStats(estWidth1, truthWidth1,
			 truthDbm1, truthDbm2,
			 truthWidth1, truthWidth2);
  
  _w1_375_425.addToStats(estWidth1, truthWidth1,
			 truthDbm1, truthDbm2,
			 truthWidth1, truthWidth2);

  // width trip 2
 
  _w2_175_225.addToStats(estWidth2, truthWidth2,
			 truthDbm1, truthDbm2,
			 truthWidth1, truthWidth2);
  
  _w2_275_325.addToStats(estWidth2, truthWidth2,
			 truthDbm1, truthDbm2,
			 truthWidth1, truthWidth2);
  
  _w2_375_425.addToStats(estWidth2, truthWidth2,
			 truthDbm1, truthDbm2,
			 truthWidth1, truthWidth2);
  
}
  
////////////////////////////////
// compute stats and write

void Verify::computeStatsAndWrite()

{

  _p1_175_225.computeStatsAndWrite();
  _p1_275_325.computeStatsAndWrite();
  _p1_375_425.computeStatsAndWrite();

  _p2_175_225.computeStatsAndWrite();
  _p2_275_325.computeStatsAndWrite();
  _p2_375_425.computeStatsAndWrite();

  _v1_175_225.computeStatsAndWrite();
  _v1_275_325.computeStatsAndWrite();
  _v1_375_425.computeStatsAndWrite();

  _v2_175_225.computeStatsAndWrite();
  _v2_275_325.computeStatsAndWrite();
  _v2_375_425.computeStatsAndWrite();

  _w1_175_225.computeStatsAndWrite();
  _w1_275_325.computeStatsAndWrite();
  _w1_375_425.computeStatsAndWrite();

  _w2_175_225.computeStatsAndWrite();
  _w2_275_325.computeStatsAndWrite();
  _w2_375_425.computeStatsAndWrite();

}

  
////////////////////////////////
// write a data line to the file

void Verify::writeToTable(double estDbm1, double truthDbm1,
			  double estVel1, double truthVel1,
			  double estWidth1, double truthWidth1,
			  double estDbm2, double truthDbm2,
			  double estVel2, double truthVel2,
			  double estWidth2, double truthWidth2,
			  double r1Ratio)
  
{

  fprintf(_tableFile, "%g %g ", estDbm1, truthDbm1);
  fprintf(_tableFile, "%g %g ", estVel1, truthVel1);
  fprintf(_tableFile, "%g %g ", estWidth1, truthWidth1);
  fprintf(_tableFile, "%g %g ", estDbm2, truthDbm2);
  fprintf(_tableFile, "%g %g ", estVel2, truthVel2);
  fprintf(_tableFile, "%g %g ", estWidth2, truthWidth2);
  fprintf(_tableFile, "%g %g\n", r1Ratio, truthDbm1 - truthDbm2);
  fflush(_tableFile);

}

