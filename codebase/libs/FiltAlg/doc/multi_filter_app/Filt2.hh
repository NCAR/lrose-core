/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
 ** Copyright (c) 1995, UCAR
 ** University Corporation for Atmospheric Research(UCAR)
 ** National Center for Atmospheric Research(NCAR)
 ** Research Applications Program(RAP)
 ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
 ** All rights reserved. Licenced use only.
 ** Do not copy or distribute without authorization
 *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
//   File: $RCSfile: Filt2.hh,v $
//   Version: $Revision: 1.1 $  Dated: $Date: 2010/08/16 23:33:17 $

/**
 * @file Filt2.hh 
 * @class Filt2
 */

#ifndef Filt2_H
#define Filt2_H
#include <cstdio>
#include <FiltAlg/Filter.hh>
using namespace std;

/*----------------------------------------------------------------*/
class Filt2 : public Filter
{
public:
  Filt2(const FiltAlgParams::data_filter_t f, const FiltAlgParms &P);
  virtual ~Filt2();
  #include <FiltAlg/FilterVirtualFunctions.hh>
protected:
private:

  double _filt_2_parm;

};

#endif
