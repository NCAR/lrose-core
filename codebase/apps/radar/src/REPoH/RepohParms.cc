/**
 * @file Parms.cc
 */

//------------------------------------------------------------------
#include "RepohParms.hh"
#include <FiltAlgVirtVol/ParmAlgorithmApp.hh>

//------------------------------------------------------------------
RepohParms::RepohParms() : AlgorithmParms()
{
}

//------------------------------------------------------------------
RepohParms::RepohParms(const std::string &parmFileName) :
  AlgorithmParms()
{
  if (!parmAlgorithmAppSet(parmFileName, _main, *this))
  {
    exit(1);
  }
}
//------------------------------------------------------------------
RepohParms::~RepohParms()
{
}

//------------------------------------------------------------------
void RepohParms::print(void)
{
  _main.print(stdout, PRINT_VERBOSE);
  AlgorithmParams::print(stdout, PRINT_VERBOSE);
}

//------------------------------------------------------------------
void RepohParms::printInputOutputs(void) const
{
  printf("INPUTS:\n");
  for (int i=0; i<input_n; ++i)
  {
    printf("\t%s\n", _input[i]);
  }

  printf("OUTPUTS:\n");
  for (int i=0; i<output_n; ++i)
  {
    printf("\t%s\n", _output[i]);
  }
}

// //------------------------------------------------------------------
// const void *RepohParms::appParms(void) const
// {
//   return (void *)&_main;
// }

