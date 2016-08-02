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
 * @file Comb.cc
 */
#include <FiltAlg/Comb.hh>
#include <rapmath/AngleCombiner.hh>

//------------------------------------------------------------------
Comb::Comb(void)
{
  _vlevel_tolerance = 0.0;
  _ok = false;
  _has_confidence = false;
}

//------------------------------------------------------------------
Comb::Comb(const FiltAlgParams &P, const int index)
{
  _ok = true;
  _vlevel_tolerance = P.vlevel_tolerance;
  _has_confidence = false;
  switch (index)
  {
  case 0:
    _build(P.combine0_n, P._combine0);
    break;
  case 1:
    _build(P.combine1_n, P._combine1);
    break;
  case 2:
    _build(P.combine2_n, P._combine2);
    break;
  case 3:
    _build(P.combine3_n, P._combine3);
    break;
  case 4:
    _build(P.combine4_n, P._combine4);
    break;
  case 5:
    _build(P.combine5_n, P._combine5);
    break;
  case 6:
    _build(P.combine6_n, P._combine6);
    break;
  case 7:
    _build(P.combine7_n, P._combine7);
    break;
  case 8:
    _build(P.combine8_n, P._combine8);
    break;
  case 9:
    _build(P.combine9_n, P._combine9);
    break;
  default:
    LOG(ERROR) << "index " << index << " out of range 0 to 9";
    _ok = false;
  }
}

//------------------------------------------------------------------
Comb::Comb(const std::vector<DataWithConf_t> &p, 
	   const std::string &mainConf,
	   const bool mainConfIsInput,
	   const double vlevel_tolerance) :
  _mainConf(mainConf, mainConfIsInput, 1.0)
 {
  _has_confidence = true;
  _vlevel_tolerance = vlevel_tolerance;
  for (size_t i=0; i<p.size(); ++i)
  {
    CombineData c(p[i].name, p[i].is_input, p[i].conf_name,
		  p[i].conf_is_input, p[i].weight);
    _data.push_back(c);
  }
  _ok = true;
}

//------------------------------------------------------------------
Comb::Comb(const std::vector<DataWithoutConf_t> &p, 
	   const double vlevel_tolerance)
{
  _has_confidence = false;
  _vlevel_tolerance = vlevel_tolerance;
  for (size_t i=0; i<p.size(); ++i)
  {
    CombineData c(p[i].name, p[i].is_input, p[i].weight);
    _data.push_back(c);
  }
  _ok = true;
}

//------------------------------------------------------------------
Comb::~Comb()
{
}

//------------------------------------------------------------------
bool Comb::create_inputs(const vector<Data> &input,
			 const vector<Data> &output)
{
  for (int i=0; i<static_cast<int>(_data.size()); ++i)
  {
    if (!_data[i].create_comb_data(input, output))
    {
      return false;
    }
  }

  if (_has_confidence)
  {
    return _mainConf.create_comb_data(input, output);
  }
  else
  {
    return true;
  }
}

//------------------------------------------------------------------
bool Comb::check_data(const Data::Data_t type) const
{
  for (int i=0; i<static_cast<int>(_data.size()); ++i)
  {
    if (!_data[i].type_equals(type))
    {
      return false;
    }
  }
  if (_has_confidence)
  {
    return _mainConf.type_equals(type);
  }
  else
  {
    return true;
  }
}

//------------------------------------------------------------------
bool Comb::max(const double vlevel, VlevelSlice &o) const
{
  // confidence and weight are ignored for max
  for (int i=0; i<static_cast<int>(_data.size()); ++i)
  {
    const VlevelSlice *gi = _data[i].matching_vlevel(vlevel, _vlevel_tolerance);
    if (gi != NULL)
    {
      if (!o.max_slice(*gi))
      {
	return false;
      }
    }
  }
  return true;
}

//------------------------------------------------------------------
bool Comb::average(const double vlevel, const bool orientation,
		   VlevelSlice &o) const
{
  if (orientation)
  {
    _orientation_average(vlevel, o);
  }
  else
  {
    _average(vlevel, o);
  }
  return true;
}

//------------------------------------------------------------------
bool Comb::product(const double vlevel, VlevelSlice &o) const
{
  // confidence and weight are ignored for product
  for (int i=0; i<static_cast<int>(_data.size()); ++i)
  {
    const VlevelSlice *gi = _data[i].matching_vlevel(vlevel, _vlevel_tolerance);
    if (gi != NULL)
    {
      if (!o.product_slice(*gi))
      {
	return false;
      }
    }
  }
  return true;
}

//------------------------------------------------------------------
bool Comb::weighted_sum(const double vlevel, const double w0,
			const bool norm, const bool orientation,
			VlevelSlice &o) const
{
  if (orientation)
  {
    _weighted_orientation_sum(vlevel, w0, norm, o);
    return true;
  }
  else
  {
    return _weighted_sum(vlevel, w0, norm, o);
  }
}

//------------------------------------------------------------------
bool Comb::weighted_confidence(const double vlevel, const double w0,
			       const bool norm, VlevelSlice &o) const
{
  if (!_has_confidence)
  {
    LOG(ERROR) << "Tried to take weighted confidence with none";
    return false;
  }

  // start with main input confidence
  const VlevelSlice *c0 = _mainConf.matching_vlevel(vlevel,
						    _vlevel_tolerance);
  if (c0 == NULL)
  {
    LOG(ERROR) << "No main confidence field";
    return false;
  }

  double wsum = w0;

  o = *c0;
  o.multiply_slice(w0);

  // add in remaining inputs
  for (int i=0; i<static_cast<int>(_data.size()); ++i)
  {
    const VlevelSlice *ci =
      _data[i].matching_conf_vlevel(vlevel, _vlevel_tolerance);
    double w = _data[i].get_weight();
    if (ci != NULL)
    {
      VlevelSlice ctmp(*ci);

      // multiply confidence by weight
      ctmp.multiply_slice(w);
      wsum += w;
      if (!o.add_slice(ctmp))
      {
	return false;
      }
    }
  }
  if (norm && wsum != 0.0)
  {
    // at each point divide the output by the weight
    o.multiply_slice(1.0/wsum);
  }
  return true;
}

//------------------------------------------------------------------
bool Comb::max(double &v) const
{
  double vi;

  // confidence and weight are ignored
  for (int i=0; i<static_cast<int>(_data.size()); ++i)
  {
    const Data *gi = _data[i].get_data();
    if (gi != NULL)
    {
      if (!gi->get_1d_value(vi))
      {
	return false;
      }
      else
      {
	if (vi > v)
	{
	  v = vi;
	}
      }
    }
  }
  return true;
}

//------------------------------------------------------------------
bool Comb::average(const bool orientation, double &v) const
{
  // confidence and weight are ignored
  if (orientation)
  {
    AngleCombiner A(static_cast<int>(_data.size())+1);
    A.clearValues();
    A.setGood(0, v, 1.0);
    double vi, ci=1.0;
    for (int i=0; i<static_cast<int>(_data.size()); ++i)
    {
      const Data *gi = _data[i].get_data();
      if (gi != NULL)
      {
	if (!gi->get_1d_value(vi))
	{
	  return false;
	}
	A.setGood(i, vi, ci);
      }
    }
    return A.getCombineAngle(v);
  }
  else
  {
    double counts = 1;
    double vi;
    for (int i=0; i<static_cast<int>(_data.size()); ++i)
    {
      const Data *gi = _data[i].get_data();
      if (gi != NULL)
      {
	if (!gi->get_1d_value(vi))
	{
	  return false;
	}
	else
	{
	  v += vi;
	  counts += 1.0;
	}
      }
    }
    v /= counts;
    return true;
  }
}

//------------------------------------------------------------------
bool Comb::product(double &v) const
{
  // conf/weights ignored
  double vi;
  for (int i=0; i<static_cast<int>(_data.size()); ++i)
  {
    const Data *gi = _data[i].get_data();
    if (gi != NULL)
    {
      if (!gi->get_1d_value(vi))
      {
	return false;
      }
      else
      {
	v *= vi;
      }
    }
  }
  return true;
}

//------------------------------------------------------------------
bool Comb::weighted_sum(const double w0, const bool norm, 
			const bool orientation, double &v) const
{
  if (orientation)
  {
    if (!norm)
    {
      LOG(ERROR) << "Can't have unnormalized orientation sums";
      return false;
    }
  }

  double c0;
  if (!_mainConfidence1d(c0))
  {
    return false;
  }

  if (orientation)
  {
    vector<double> weights = _weights(w0);

    AngleCombiner A(weights);
    A.clearValues();

    A.setGood(0, v, c0);

    for (int i=0; i<static_cast<int>(_data.size()); ++i)
    {
      double vi;
      double ci;
      bool isbad;
      if (_get1dDataConf(i, vi, ci, isbad))
      {
	A.setGood(i+1, vi, ci);
      }
      else
      {
	if (isbad)
	{
	  return false;
	}
      }
    }
    if (!A.getCombineAngle(v))
    {
      v = 0;
    }
  }
  else
  {
    double wsum;

    wsum = w0*c0;

    v *= w0*c0;
    for (int i=0; i<static_cast<int>(_data.size()); ++i)
    {
      double vi, ci;
      bool isbad;
      if (_get1dDataConf(i, vi, ci, isbad))
      {
	double w = _data[i].get_weight();
	v += (vi*ci*w);
	wsum += w*ci;
      }
      else
      {
	if (isbad)
	{
	  return false;
	}
      }
    }

    if (norm)
    {
      if (wsum > 0)
      {
	v /= wsum;
      }
      else
      {
	v = 0;
      }
    }
  }
  return true;
}

//------------------------------------------------------------------
bool Comb::hasNamedData(const std::string &name) const
{
  for (int i=0; i<static_cast<int>(_data.size()); ++i)
  {
    if (_data[i].name_equals(name))
    {
      return true;
    }
  }
  return false;
}

//------------------------------------------------------------------
const Data *Comb::dataPointer(const std::string &name) const
{
  for (int i=0; i<static_cast<int>(_data.size()); ++i)
  {
    if (_data[i].name_equals(name))
    {
      return _data[i].get_data();
    }
  }
  return NULL;
}

//------------------------------------------------------------------
void Comb::_build(const int n, const FiltAlgParams::combine_t *ff)
{
  for (int i=0; i<n; ++i)
  {
    CombineData c(ff[i].name, ff[i].is_input, ff[i].weight);
    _data.push_back(c);
  }
}

//------------------------------------------------------------------
void Comb::_average(const double vlevel, VlevelSlice &o) const
{
  VlevelSlice counts;
  o.init_average(counts);

  for (int i=0; i<static_cast<int>(_data.size()); ++i)
  {
    const VlevelSlice *gi = 
      _data[i].matching_vlevel(vlevel, _vlevel_tolerance);
    if (gi != NULL)
    {
      o.accum_average(*gi, counts);
    }
  }
  o.finish_average(counts);
}

//------------------------------------------------------------------
void Comb::_orientation_average(const double vlevel, VlevelSlice &o) const
{
  AngleCombiner A(static_cast<int>(_data.size())+1);
  _orientation_combine(A, vlevel, o);
}

//------------------------------------------------------------------
bool Comb::_weighted_sum(const double vlevel, const double w0,
			 const bool norm, VlevelSlice &o) const
{
  if (_has_confidence)
  {
    // start with main input confidence
    const VlevelSlice *c0 = _mainConf.matching_vlevel(vlevel,
						      _vlevel_tolerance);
    if (c0 == NULL)
    {
      LOG(ERROR) << "No main confidence field";
      return false;
    }

    // copy of that and multiply by input weight, which also plays the role
    // accumulated weight
    VlevelSlice wsum(*c0);
    wsum.multiply_slice(w0);

    // multiply output (main values) by this weight
    o.product_slice(wsum);

    // add in remaining inputs
    for (int i=0; i<static_cast<int>(_data.size()); ++i)
    {
      const VlevelSlice *gi = 
	_data[i].matching_vlevel(vlevel, _vlevel_tolerance);
      const VlevelSlice *ci =
	_data[i].matching_conf_vlevel(vlevel, _vlevel_tolerance);
      double w = _data[i].get_weight();
      if (gi != NULL && ci != NULL)
      {
	// make copy of each 
	VlevelSlice tmp(*gi);
	VlevelSlice ctmp(*ci);

	// multiply confidence by weight
	ctmp.multiply_slice(w);

	// multiply this input by that weight
	tmp.product_slice(ctmp);

	// add weight to wsum and weighted data to output
	if (!wsum.add_slice(ctmp))
	{
	  return false;
	}
	if (!o.add_slice(tmp))
	{
	  return false;
	}
      }
    }
    if (norm)
    {
      // at each point divide the output by the weight
      o.divide_slice(wsum);
    }
  }
  else
  {
    double wsum=w0;
    o.multiply_slice(w0);
    for (int i=0; i<static_cast<int>(_data.size()); ++i)
    {
      const VlevelSlice *gi = 
	_data[i].matching_vlevel(vlevel, _vlevel_tolerance);
      if (gi != NULL)
      {
	VlevelSlice tmp(*gi);
	double w = _data[i].get_weight();

	tmp.multiply_slice(w);
	wsum += w;
	if (!o.add_slice(tmp))
	{
	  return false;
	}
      }
    }
    if (norm)
    {
      o.multiply_slice(1.0/wsum);
    }
  }
  return true;
}

//------------------------------------------------------------------
void Comb::_weighted_orientation_sum(const double vlevel, const double w0,
				     const bool norm, VlevelSlice &o) const
{
  if (!norm)
  {
    LOG(ERROR) << "Must normalize orientation combinations";
    return;
  }

  // build weights and initialize combiner object
  vector<double> weights = _weights(w0);
  AngleCombiner A(weights);

  // combine
  _orientation_combine(A, vlevel, o);
}

//------------------------------------------------------------------
void Comb::_orientation_combine(AngleCombiner &A, const double vlevel,
				VlevelSlice &o) const  
{
  if (o.is_grid())
  {
    int nx = o.getNx();
    int ny = o.getNy();
    for (int y=0; y<ny; ++y)
    {
      for (int x=0; x<nx; ++x)
      {
	A.clearValues();
	double v, c;
	if (_get2dMainDataConf(o, vlevel, x, y, v, c))
	{
	  A.setGood(0, v, c);
	}
	else
	{
	  A.setBad(0);
	}

	for (int i=0; i<static_cast<int>(_data.size()); ++i)
	{
	  if (_get2dDataConf(i, vlevel, x, y, v, c))
	  {
	    A.setGood(i+1, v, c);
	  }
	  else
	  {
	    A.setBad(i+1);
	  }
	}
	double a;
	if (A.getCombineAngle(a))
	{
	  o.setValue(x, y, a);
	}
	else
	{
	  o.setMissing(x, y);
	}
      }
    }
  }
  else
  {
    double v, c;
    A.clearValues();

    if (_get1dMainDataConf(o, v, c))
    {
      A.setGood(0, v, c);
    }
    else
    {
      A.setBad(0);
    }
    for (int i=0; i<static_cast<int>(_data.size()); ++i)
    {
      bool isbad;
      if (_get1dDataConf(i, v, c, isbad))
      {
	A.setGood(i+1, v, c);
      }
      else
      {
	A.setBad(i+1);
      }
    }
    double a;
    if (A.getCombineAngle(a))
    {
      o.set_1d_value(a);
    }
    else
    {
      // o.setMissing(x, y, a);
    }
  }
}


//------------------------------------------------------------------
std::vector<double> Comb::_weights(const double w0) const
{
  vector<double> ret;
  ret.push_back(w0);
  for (size_t i=0; i<_data.size(); ++i)
  {
    ret.push_back(_data[i].get_weight());
  }
  return ret;
}


//------------------------------------------------------------------
bool Comb::_mainConfidence1d(double &conf) const
{
  if (_has_confidence)
  {
    const Data *g = _mainConf.get_data();
    if (g != NULL)
    {
      if (g->get_1d_value(conf))
      {
	return true;
      }
    }
    LOG(ERROR) << "No main confidence";
    return false;
  }
  else
  {
    conf = 1.0;
    return true;
  }
}

//------------------------------------------------------------------
bool Comb::_get1dDataConf(const int i, double &vi, double &ci,
			  bool &isbad) const
{
  isbad = false;

  const Data *g = _data[i].get_data();
  if (g == NULL)
  {
    return false;
  }
  if (!g->get_1d_value(vi))
  {
    isbad = true;
    return false;
  }

  if (_has_confidence)
  {
    const Data *c = _data[i].get_conf_data();
    if (c == NULL)
    {
      return false;
    }
    if (!c->get_1d_value(ci))
    {
      isbad = true;
      return false;
    }
  }
  else
  {
    ci = 1.0;
  }
  return true;
}

//------------------------------------------------------------------
bool Comb::_get1dMainDataConf(const VlevelSlice &o, double &vi,
			      double &ci) const
{
  if (!o.get_1d_value(vi))
  {
    return false;
  }

  if (_has_confidence)
  {
    const Data *c = _mainConf.get_data();
    if (c == NULL)
    {
      return false;
    }
    if (!c->get_1d_value(ci))
    {
      return false;
    }
  }
  else
  {
    ci = 1.0;
  }
  return true;
}

//------------------------------------------------------------------
bool Comb::_get2dDataConf(const int i, const double vlevel,
			  const int x, const int y,
			  double &vi, double &ci) const
{
  const VlevelSlice *v = _data[i].matching_vlevel(vlevel, _vlevel_tolerance);
  if (v == NULL)
  {
    return false;
  }
  if (_has_confidence)
  {
    const VlevelSlice *c = _data[i].matching_conf_vlevel(vlevel,
							 _vlevel_tolerance);
    if (c == NULL)
    {
      return false;
    }
    else
    {
      return c->getValue(x, y, ci) && v->getValue(x, y, vi);
    }
  }
  else
  {
    ci = 1.0;
    return v->getValue(x, y, vi);
  }
}

//------------------------------------------------------------------
bool Comb::_get2dMainDataConf(const VlevelSlice &o, const double vlevel,
			      const int x, const int y,
			      double &vi, double &ci) const
{
  if (!o.getValue(x, y, vi))
  {
    return false;
  }

  if (_has_confidence)
  {
    const VlevelSlice *c = _mainConf.matching_vlevel(vlevel, _vlevel_tolerance);
    if (c == NULL)
    {
      return false;
    }
    else
    {
      return c->getValue(x, y, ci);
    }
  }
  else
  {
    ci = 1.0;
  }
  return true;
}
