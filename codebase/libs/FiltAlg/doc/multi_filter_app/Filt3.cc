/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
 ** Copyright (c) 1995, UCAR
 ** University Corporation for Atmospheric Research(UCAR)
 ** National Center for Atmospheric Research(NCAR)
 ** Research Applications Program(RAP)
 ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
 ** All rights reserved. Licenced use only.
 ** Do not copy or distribute without authorization
 *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
//   File: $RCSfile: Filt3.cc,v $
//   Version: $Revision: 1.2 $  Dated: $Date: 2012/01/03 19:05:15 $

/**
 * @file Filt3.cc
 * @brief Filt3 main class
 */
using namespace std; 
#include <cstdio>
#include "Filt3.hh"
#include "Parms.hh"

/*----------------------------------------------------------------*/
Filt3::Filt3(const FiltAlgParams::data_filter_t f,
	     const FiltAlgParms &P) : Filter(f, P)
{
  if (!ok())
    return;

  void *vap = P.app_parms();
  Params *ap = (Params *)vap;

  _filt_3_parm = ap->_parm_filt_3[f.filter_index].parm;
}

/*----------------------------------------------------------------*/
Filt3::~Filt3()
{
}

/*----------------------------------------------------------------*/
void Filt3::initialize_output(const Data &inp,
			      const FiltAlgParams::data_filter_t f,
			      Data &g)
{
  g = Data(f.output_field, Data::GRID3D, f.write_output_field);
}

/*----------------------------------------------------------------*/
void Filt3::filter_print(void) const
{
  LOGSTR(LogStr::DEBUG, "Filtering");
}

/*----------------------------------------------------------------*/
void Filt3::filter_print(const double vlevel) const
{
  LOGFSTR(LogStr::DEBUG, "Filtering vlevel=%lf", vlevel);
}

/*----------------------------------------------------------------*/
bool Filt3::filter(const VlevelSlice &in, const double vlevel,
		   const int vlevel_index, const GridProj &gp, Data &out)
{
  if (!in.is_grid())
  {
    printf("ERROR in filter Filt3, input not a grid\n");
    return false;
  }

  // create local 2d grid for output
  Grid2dW o(in);

  // put filtered output data into output 3d array
  out.add(o, vlevel, vlevel_index, gp);
  return true;
}

/*----------------------------------------------------------------*/
bool Filt3::filter(const Data &in, Data &out)
{
  printf("ERROR in Filt3::Filter(), can't filter 1 data value\n");
  return false;
}

/*----------------------------------------------------------------*/
bool Filt3::create_inputs(const time_t &t, 
			  const vector<Data> &input,
			  const vector<Data> &output)
{
  return true;
}  

/*----------------------------------------------------------------*/
bool Filt3::store_outputs(const Data &o, Info *info,
			  vector<Data> &output)
{
  // store main output to vector
  output.push_back(o);

  return true;
}

/*----------------------------------------------------------------*/
void Filt3::set_info(Info **info) const
{
}

