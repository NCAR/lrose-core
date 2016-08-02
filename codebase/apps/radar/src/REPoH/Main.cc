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
/**
 * @file Main.cc
 * @brief driver
 */
#include "Parms.hh"
#include "AsciiOutput.hh"
#include "MyFiltCreate.hh"

#include <FiltAlg/Algorithm.hh>
#include <FiltAlg/InterfaceFiltAlgParm.hh>
#include <toolsa/pmu.h>

#include <cstdlib>

/*----------------------------------------------------------------*/
static void tidy_and_exit(int i)
{
  PMU_auto_unregister();
  exit(i);
}

/*----------------------------------------------------------------*/
int main(int argc, char **argv)
{
  Parms P = Parms(argc, argv);
  if (!InterfaceFiltAlgParm::driver_init(argv[0], P, tidy_and_exit))
    tidy_and_exit(-1);

  MyFiltCreate fc;

  Algorithm A(P, &fc);
  if (!A.ok())
  {
    printf("ERROR building algorithm object\n");
    tidy_and_exit(0);
  }


  time_t t;
  while (InterfaceFiltAlgParm::driver_trigger(P, t)) {

    PMU_auto_register("Before alg update");

    // run alg

    A.update(t, P);

    // write latest data info files
    AsciiOutput A(P._main.unfiltered_kernel_ascii_path, t);
    A.write_ldata_info();
    AsciiOutput A2(P._main.filtered_kernel_ascii_path, t);
    A2.write_ldata_info();

    PMU_auto_register("After alg update");

  }

  tidy_and_exit(0);

}

