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
#include <copyright.h>

//   File: $RCSfile: Parms.cc,v $
//   Version: $Revision: 1.2 $  Dated: $Date: 2012/01/03 19:05:15 $

/**
 * @file Parms.cc
 * @brief 
 * @note
 * @todo
 */

/*----------------------------------------------------------------*/
#include <cstdio>
#include "Parms.hh"
#include <FiltAlg/ParmFiltAlgApp.hh>
using namespace std;

/*----------------------------------------------------------------*/
# ifndef    lint
static char RCSid[] = "$Id: ";
void Parms_rcsprint(void)
{
    printf("rcsid=%s\n", RCSid);
}
# endif     /* not lint */


/*----------------------------------------------------------------*/
Parms::Parms() : FiltAlgParms()
{
}

/*----------------------------------------------------------------*/
Parms::Parms(int argc, char **argv) : FiltAlgParms()
{
  if (!parmFiltAlgAppSet(_main, *this, argc, argv))
    exit(-1);
}

/*----------------------------------------------------------------*/
Parms::~Parms()
{
}

/*----------------------------------------------------------------*/
Params::app_filter_t Parms::name_to_type(const string &name)
{
  if (name == "FILTER_1")
    return Params::FILTER_1;
  else if (name == "FILTER_2")
    return Params::FILTER_2;
  else if (name == "FILTER_3")
    return Params::FILTER_3;
  else
  {
    printf("ERROR unkown app filter parm name %s\n", name.c_str());
    printf("Valid names: FILTER_1,FILTER_2,FILTER_3\n");
    return Params::UNKNOWN;
  }
}

/*----------------------------------------------------------------*/
int
Parms::app_max_elem_for_filter(const FiltAlgParams::data_filter_t f) const
{
  string s = f.app_filter_name;
  Params::app_filter_t t = name_to_type(s);
  switch (t)
  {
  case Params::FILTER_1:
    return _main.parm_filt_1_n;
  case Params::FILTER_2:
    return _main.parm_filt_2_n;
  case Params::FILTER_3:
    return _main.parm_filt_3_n;
  default:
    printf("ERROR unkown app filter parm name %s\n", s.c_str());
    return 0;
  }
}

/*----------------------------------------------------------------*/
void *Parms::app_parms(void) const
{
  return (void *)&_main;
}

