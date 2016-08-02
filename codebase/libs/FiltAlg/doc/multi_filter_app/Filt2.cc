/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
 ** Copyright (c) 1995, UCAR
 ** University Corporation for Atmospheric Research(UCAR)
 ** National Center for Atmospheric Research(NCAR)
 ** Research Applications Program(RAP)
 ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
 ** All rights reserved. Licenced use only.
 ** Do not copy or distribute without authorization
 *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
//   File: $RCSfile: Filt2.cc,v $
//   Version: $Revision: 1.2 $  Dated: $Date: 2012/01/03 19:05:15 $

/**
 * @file Filt2.cc
 * @brief Filt2 main class
 */
using namespace std; 
#include <cstdio>
#include "Filt2.hh"
#include "Parms.hh"

/*----------------------------------------------------------------*/
Filt2::Filt2(const FiltAlgParams::data_filter_t f,
	     const FiltAlgParms &P) : Filter(f, P)
{
  if (!ok())
    return;

  void *vap = P.app_parms();
  Params *ap = (Params *)vap;

  _filt_2_parm = ap->_parm_filt_2[f.filter_index].parm;
}

/*----------------------------------------------------------------*/
Filt2::~Filt2()
{
}

/*----------------------------------------------------------------*/
void Filt2::initialize_output(const Data &inp,
			      const FiltAlgParams::data_filter_t f,
			      Data &g)
{
  g = Data(f.output_field, Data::GRID3D, f.write_output_field);
}

/*----------------------------------------------------------------*/
void Filt2::filter_print(void) const
{
  LOGSTR(LogStr::DEBUG, "Filtering");
}

/*----------------------------------------------------------------*/
void Filt2::filter_print(const double vlevel) const
{
  LOGFSTR(LogStr::DEBUG, "Filtering vlevel=%lf", vlevel);
}

/*----------------------------------------------------------------*/
bool Filt2::filter(const VlevelSlice &in, const double vlevel,
		   const int vlevel_index, const GridProj &gp,
		   Data &out)
{
  if (!in.is_grid())
  {
    printf("ERROR in filter Filt2, input not a grid\n");
    return false;
  }

  // create local 2d grid for output
  Grid2dW o(in);

  // put filtered output data into output 3d array
  out.add(o, vlevel, vlevel_index, gp);
  return true;
}

/*----------------------------------------------------------------*/
bool Filt2::filter(const Data &in, Data &out)
{
  printf("ERROR in Filt2::Filter(), can't filter 1 data value\n");
  return false;
}

/*----------------------------------------------------------------*/
bool Filt2::create_inputs(const time_t &t, 
			  const vector<Data> &input,
			  const vector<Data> &output)
{
  return true;
}  

/*----------------------------------------------------------------*/
bool Filt2::store_outputs(const Data &o, Info *info,
			  vector<Data> &output)
{
  // store main output to vector
  output.push_back(o);

  return true;
}

/*----------------------------------------------------------------*/
void Filt2::set_info(Info **info) const
{
}

