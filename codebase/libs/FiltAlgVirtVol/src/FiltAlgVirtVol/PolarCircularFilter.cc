#include <FiltAlgVirtVol/PolarCircularFilter.hh>
#include <FiltAlgVirtVol/PolarCircularTemplate.hh>
#include <FiltAlgVirtVol/Histo.hh>
#include <euclid/Grid2d.hh>
#include <euclid/GridAlgs.hh>
#include <Mdv/MdvxProj.hh>
#include <toolsa/LogStream.hh>
#include <cmath>

//------------------------------------------------------------------
static bool _percentLessThan(const std::vector<double> &dataInBox,
			     double min, double &p)
{
  p = 0.0;
  if (dataInBox.empty())
  {
    return false;
  }
  double count= (double)(dataInBox.size());
  
  for (size_t i=0; i<dataInBox.size(); ++i)
  {
    if (fabs(dataInBox[i]) < min)
    {
      ++p;
    }
  }
  p = p/count;
  return true;
}

//------------------------------------------------------------------
static bool _largePosNeg(const std::vector<double> &dataInBox,
			 double thresh, double &p)
{
  p = 0.0;
  if (dataInBox.empty())
  {
    return false;
  }
  double npos=0.0;
  double nneg = 0.0;
  
  for (size_t i=0; i<dataInBox.size(); ++i)
  {
    if (fabs(dataInBox[i]) > thresh)
    {
      if (dataInBox[i] > 0)
      {
	++npos;
      }
      else
      {
	++nneg;
      }
    }
  }
  if (npos+nneg > 0)
  {
    p = fabs(npos-nneg)/(npos+nneg);
    return true;
  }
  else
  {
    return false;
  }
}

//------------------------------------------------------------------
static bool _max(const std::vector<double> &dataInBox, double &max)
{
  max=0.0;
  bool first = true;
  for (size_t i=0; i<dataInBox.size(); ++i)
  {
    if (first)
    {
      first = false;
      max = dataInBox[i];
    }
    else
    {
      if (dataInBox[i] > max) max = dataInBox[i];
    }
  }
  return !first;
}

//------------------------------------------------------------------
static bool _averageAndGeThresh(const std::vector<double> &dataInBox,
				double thresh, double &ave)
{
  double count=0.0;
  ave = 0.0;
  bool exceed=false;
  for (size_t i=0; i<dataInBox.size(); ++i)
  {
    count += 1;
    ave += dataInBox[i];
    if (dataInBox[i] >= thresh)
    {
      exceed = true;
    }
  }
  if (count  > 0 && exceed)
  {
    ave /= count;
    return true;
  }
  else
  {
    return false;
  }
}


//------------------------------------------------------------------
static bool _average(const std::vector<double> &dataInBox, double &ave)
{
  double count=0.0;
  ave = 0.0;
  for (size_t i=0; i<dataInBox.size(); ++i)
  {
    count += 1;
    ave += dataInBox[i];
  }
  if (count  > 0)
  {
    ave /= count;
    return true;
  }
  else
  {
    return false;
  }
}

//------------------------------------------------------------------
void PolarCircularFilter::dilate(Grid2d &a, const PolarCircularTemplate &pt)
{
  Grid2d out(a);
  out.setAllMissing();
  for (int y=0; y<out.getNy(); ++y)
  {
    for (int x=0; x<out.getNx(); ++x)
    {
      if (!a.isMissing(x, y))
      {
	vector<double> dataInBox = pt.dataInsideCircle(x, y, a);;
	double max;
	if (_max(dataInBox, max))
	{
	  out.setValue(x, y, max);
	}
      }
    }
  }
  a = out;
}


//------------------------------------------------------------------
void PolarCircularFilter::smooth(Grid2d &a, const PolarCircularTemplate &pt)
{
  Grid2d out(a);
  out.setAllMissing();
  for (int y=0; y<out.getNy(); ++y)
  {
    for (int x=0; x<out.getNx(); ++x)
    {
      if (!a.isMissing(x, y))
      {
	vector<double> dataInBox = pt.dataInsideCircle(x, y, a);;
	double max;
	if (_average(dataInBox, max))
	{
	  out.setValue(x, y, max);
	}
      }
    }
  }
  a = out;
}

//------------------------------------------------------------------
void PolarCircularFilter::smoothWithThresh(Grid2d &a,
					   const PolarCircularTemplate &pt,
					   double thresh,
					   double uninterestingValue)
{
  Grid2d out(a);
  double markerValue = -999;
  out.setAllToValue(markerValue);
  for (int y=0; y<out.getNy(); ++y)
  {
    for (int x=0; x<out.getNx(); ++x)
    {
      if (!a.isMissing(x, y))
      {
	vector<double> dataInBox = pt.dataInsideCircle(x, y, a);;
	double max;
	if (_averageAndGeThresh(dataInBox, thresh, max))
	{
	  out.setValue(x, y, max);
	}
      }
    }
  }
  GridAlgs aa(out);
  aa.change(markerValue, uninterestingValue);
  a = aa;
}

//------------------------------------------------------------------
void PolarCircularFilter::percentLessThan(Grid2d &a, const PolarCircularTemplate &pt, double min)
{
  Grid2d out(a);
  out.setAllMissing();
  for (int y=0; y<out.getNy(); ++y)
  {
    for (int x=0; x<a.getNx(); ++x)
    {
      if (!a.isMissing(x, y))
      {
	vector<double> dataInBox = pt.dataInsideCircle(x, y, a);;
	double p;
	if (_percentLessThan(dataInBox, min, p))
	{
	  out.setValue(x, y, p);
	}
      }
    }
  }
  a = out;
}

//------------------------------------------------------------------
void PolarCircularFilter::largePosNeg(Grid2d &a, const PolarCircularTemplate &pt, double thresh)
{
  Grid2d out(a);
  out.setAllMissing();
  for (int y=0; y<out.getNy(); ++y)
  {
    for (int x=0; x<out.getNx(); ++x)
    {
      if (!a.isMissing(x, y))
      {
	vector<double> dataInBox = pt.dataInsideCircle(x, y, a);
	double p;
	if (_largePosNeg(dataInBox, thresh, p))
	{
	  out.setValue(x, y, p);
	}
      }
    }
  }
  a = out;
}

//------------------------------------------------------------------
void PolarCircularFilter::median(Grid2d &a, const PolarCircularTemplate &pt, double binMin,
				 double binMax, double binDelta)
{
  Grid2d out(a);
  out.setAllMissing();
  Histo H(binDelta, binMin, binMax);
  for (int y=0; y<out.getNy(); ++y)
  {
    for (int x=0; x<out.getNx(); ++x)
    {
      H.clear();
      vector<double> dataInBox = pt.dataInsideCircle(x, y, a);
      if (dataInBox.empty())
      {
	LOG(DEBUG_VERBOSE) << "No data x=" << x;
      }
      else
      {
	for (size_t i=0; i<dataInBox.size(); ++i)
	{
	  H.addValue(dataInBox[i]);
	  double m;
	  if (H.getMedian(m))
	  {
	    out.setValue(x, y, m);
	  }
	  else
	  {
	    LOG(WARNING) << "No median x=" << x;
	  }
	}
      }
    }
  }
  a = out;
}
