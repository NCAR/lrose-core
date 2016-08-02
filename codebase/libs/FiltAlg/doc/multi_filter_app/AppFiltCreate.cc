/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
** Copyright (c) 1995, UCAR
** University Corporation for Atmospheric Research(UCAR)
** National Center for Atmospheric Research(NCAR)
** Research Applications Program(RAP)
** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
** All rights reserved. Licenced use only.
** Do not copy or distribute without authorization
*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
//   File: $RCSfile: AppFiltCreate.cc,v $
//   Version: $Revision: 1.2 $  Dated: $Date: 2012/01/03 19:05:15 $

/**
 * @file AppFiltCreate.cc
 * @brief method to create a filter of any type 
 */
#include <cstdio>
#include <string>
#include <vector>
#include <FiltAlg/FiltCreate.hh>
#include <FiltAlg/FiltAlgParms.hh>
#include "Filt1.hh"
#include "Filt2.hh"
#include "Filt3.hh"
#include "Parms.hh"
using namespace std; 

/*----------------------------------------------------------------*/
Filter *appFiltCreate(const FiltAlgParams::data_filter_t f,
		      const FiltAlgParms &P)
{
  string s = f.app_filter_name;
  Params::app_filter_t t = Parms::name_to_type(s);
  switch (t)
  {
  case Params::FILTER_1:
    return new Filt1(f, P);
  case Params::FILTER_2:
    return new Filt2(f, P);
  case Params::FILTER_3:
    return new Filt3(f, P);
  default:
    printf("ERROR unkown app filter parm name %s\n", s.c_str());
    return NULL;
  }
}  
