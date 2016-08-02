/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
 ** Copyright UCAR (c) 1992 - 1999
 ** University Corporation for Atmospheric Research(UCAR)
 ** National Center for Atmospheric Research(NCAR)
 ** Research Applications Program(RAP)
 ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
 ** All rights reserved. Licenced use only.
 ** Do not copy or distribute without authorization
 ** 1999/03/14 14:18:54
 *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
/////////////////////////////////////////////////////////////
// PPIField.hh
//
// PPIField object
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2000
//
///////////////////////////////////////////////////////////////

#ifndef PPIField_HH
#define PPIField_HH

#include <vector>
#include <string>
#include <iostream>
using namespace std;

class sRadl;
class ScanParams;
class RapicRay;

////////////////////////
// This class

class PPIField {
  
public:

  // constructor

  PPIField();

  // destructor
  
  ~PPIField();

  // set methods

  void setName(const string &name,
               const ScanParams &sparams);
  void setTime(time_t val) { _time = val; }
  void setScanNum(int val) { _scanNum = val; }
  void setMaxGates(int val) { _maxGates = val; }
  void setRangeRes(double val) { _rangeRes = val; }
  void setStartRange(double val) { _startRange = val; }

  // get methods

  string getName() const { return _name; }
  string getUnits() const { return _units; }
  time_t getTime() const { return _time; }
  int getScanNum() const { return _scanNum; }
  int getMaxGates() const { return _maxGates; }
  double getRangeRes() const { return _rangeRes; }
  double getStartRange() const { return _startRange; }
  double getScale() const { return _scale; }
  double getBias() const { return _bias; }
  vector<RapicRay *> getRays() const { return _rays; }
  
  // add a beam
  
  void addRay(const sRadl *radial, const ScanParams &sparams,
              bool isBinary, double target_elev);

  // print

  void print(ostream &out);

  // print full
  
  void printFull(ostream &out);

  // convertRangeResolution for Beijing data
  // convertNexradResolution

  void convertNexradResolution(int maxGatesLimit);

protected:

private:
  
  string _name;
  string _units;
  time_t _time;
  int _scanNum;
  int _maxGates;
  double _rangeRes;
  double _startRange;
  double _scale;
  double _bias;
  vector<RapicRay *> _rays;
  
};

#endif
