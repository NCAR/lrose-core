/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
** Copyright (c) 1995, UCAR
** University Corporation for Atmospheric Research(UCAR)
** National Center for Atmospheric Research(NCAR)
** Research Applications Program(RAP)
** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
** All rights reserved. Licenced use only.
** Do not copy or distribute without authorization
*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
//   File: $RCSfile: Main.cc,v $
//   Version: $Revision: 1.2 $  Dated: $Date: 2012/01/03 19:05:15 $

/**
 * @file Main.cc
 * @brief driver for FreezingLevel
 */
// #include <niwot_util/UrlWatcher.hh>
// #include <toolsa/pmu.h>
#include <FiltAlg/Algorithm.hh>
#include <FiltAlg/InterfaceFiltAlgParm.hh>
#include "Parms.hh"
using namespace std; 

void tidy_and_exit(int);
Parms P;

/*----------------------------------------------------------------*/
void tidy_and_exit(int i)
{
  InterfaceFiltAlgParm::finish();
  exit(i);
}

/*----------------------------------------------------------------*/
int main(int argc, char **argv)
{
  P = Parms(argc, argv);

  if (!InterfaceFiltAlgParm::driver_init(argv[0], P, tidy_and_exit))
    tidy_and_exit(-1);

  Algorithm A(P);
  if (!A.ok())
  {
    printf("ERROR building algorithm object\n");
    tidy_and_exit(0);
  }


  time_t t;
  while (InterfaceFiltAlgParm::driver_trigger(P, t))
  {
//   while (_U->get_data())
//   {
//     t = _U->get_time();
    printf("--------Triggered %s----------\n", DateTime::strn(t).c_str());
    A.update(t, P);
    printf("Done output. wait for trigger\n");
  }
  tidy_and_exit(0);
}

