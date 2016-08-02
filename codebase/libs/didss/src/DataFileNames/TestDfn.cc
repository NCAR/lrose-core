/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
 ** Copyright UCAR (c) 1992 - 1997
 ** University Corporation for Atmospheric Research(UCAR)
 ** National Center for Atmospheric Research(NCAR)
 ** Research Applications Program(RAP)
 ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
 ** All rights reserved. Licenced use only.
 ** Do not copy or distribute without authorization
 ** 1997/9/26 14:18:54
 *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
///////////////////////////////////////////////////////////////
// TestDfn.cc
//
// Unit test for DataFileNames object
//
// Mike Dixon, RAP, NCAR, Boulder, CO, USA
//
// Jan 2005
//
///////////////////////////////////////////////////////////////

#include <string>
#include <vector>
#include <iostream>
#include <didss/DataFileNames.hh>
#include <toolsa/DateTime.hh>
using namespace std;

// main

int main(int argc, char **argv)

{

  typedef pair<string, time_t> pr;
  vector<pr> test;
  test.push_back(pr("/tmp/mdv/ctrec/20040321/g_120000/f_00003612.mdv",
		    1079874012));
  test.push_back(pr("/tmp/mdv/ctrec/20040321/123456.mdv", 1079872496));
  test.push_back(pr("/tmp/mdv/ctrec/20040321/123456", 1079872496));
  test.push_back(pr("/tmp/mdv/ctrec/20040321/20040321#123456.mdv", 1079872496));
  test.push_back(pr("/tmp/mdv/ctrec/20040321/20040321123456.mdv", 1079872496));
  test.push_back(pr("/tmp/mdv/ctrec/20040321123456", 1079872496));
  test.push_back(pr("/tmp/mdv/ctrec/200403211234", 1079872440));
  test.push_back(pr("/tmp/mdv/ctrec/2004032112", 1079870400));
  test.push_back(pr("/tmp/mdv/ctrec/20040321", 1079870400));
  test.push_back(pr("/tmp/mdv/ctrec/2004032112.tm1344", 1079919840));
  test.push_back(pr("/tmp/mdv/ctrec/20040321_1244", 1079873040));
  test.push_back(pr("/tmp/mdv/ctrec/20040321.21-22", 1079902800));
  test.push_back(pr("/tmp/mdv/ctrec/a05032b", 1107259200));
  test.push_back(pr("/tmp/mdv/ctrec/20040321_130304", 1079874184));
  test.push_back(pr("/tmp/mdv/ctrec/20040321+13", 1079874000));

  int iret = 0;
  for (size_t ii = 0; ii < test.size(); ii++) {
    bool dateOnly;
    time_t dataTime;
    if (DataFileNames::getDataTime(test[ii].first.c_str(),
				   dataTime, dateOnly)) {
      cerr << "ERROR - TestDfn" << endl;
      cerr << "  Cannot get data time for path: " << test[ii].first << endl;
      iret = -1;
    } else if (dataTime != test[ii].second) {
      cerr << "ERROR - TestDfn" << endl;
      cerr << "  Incorrect time for path: " << test[ii].first << endl;
      cerr << "  Computed time: " << DateTime::str(dataTime) << endl;
      cerr << "  Correct time: " << DateTime::str(test[ii].second) << endl;
      iret = -1;
    } else {
      cerr << "Success on: " << test[ii].first << endl;
      cerr << "  Time: " << DateTime::str(dataTime) << ", " << dataTime << endl;
    } 
  } // ii

  // clear the array, and test with a file pattern supplied
  test.clear();
  test.push_back(pr("/tmp/mdv/ctrec/20101118.gfs.t06z.1bmhs.tm00.bufr_d", 1290060000));
  for (size_t ii = 0; ii < test.size(); ii++) {
    bool dateOnly;
    time_t dataTime;
    if (DataFileNames::getDataTime(test[ii].first.c_str(), "YYYYMMDD.gfs.tHHz.1bmhs.tm00.bufr_d",
                                   dataTime, dateOnly)) {
      cerr << "ERROR - TestDfn" << endl;
      cerr << "  Cannot get data time for path: " << test[ii].first << endl;
      iret = -1;
    } else if (dataTime != test[ii].second) {
      cerr << "ERROR - TestDfn" << endl;
      cerr << "  Incorrect time for path: " << test[ii].first << endl;
      cerr << "  Computed time: " << DateTime::str(dataTime) << endl;
      cerr << "  Correct time: " << DateTime::str(test[ii].second) << endl;
      iret = -1;
    } else {
      cerr << "Success on: " << test[ii].first << endl;
      cerr << "  Time: " << DateTime::str(dataTime) << ", " << dataTime << endl;
    }
  } // ii

  return iret;

}
