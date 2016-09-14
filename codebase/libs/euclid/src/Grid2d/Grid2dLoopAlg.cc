/**
 * @file Grid2dLoopAlg.cc
 */
#include <euclid/Grid2dLoopAlg.hh>
#include <euclid/Grid2d.hh>
#include <toolsa/LogStream.hh>
#include <cmath>

//-----------------------------------------------------------------
void Grid2dLoopAlgMean::increment(int x, int y, const Grid2d &G)
{
  double v;
  if (G.getValue(x, y, v))
  {
    _A += v;
    _N ++;
  }
}

//-----------------------------------------------------------------
void Grid2dLoopAlgMean::decrement(int x, int y, const Grid2d &G)
{
  double v;
  if (G.getValue(x, y, v))
  {
    _A -= v;
    _N--;
  }
}

//-----------------------------------------------------------------
bool Grid2dLoopAlgMean::getResult(int minGood, double &result) const
{
  if (_N > minGood)
  {
    result = _A/_N;
    return true;
  }
  else
  {
    return false;
  }
}

//-----------------------------------------------------------------
void Grid2dLoopAlgMeanNoMissing::decrement(int x, int y, const Grid2d &G)
{
  double v;
  if (G.getValue(x, y, v))
  {
    _A -= v;
    _N--;
  }
  else
  {
    _nmissing--;
  }
}

//-----------------------------------------------------------------
void Grid2dLoopAlgMeanNoMissing::increment(int x, int y, const Grid2d &G)
{
  double v;
  if (G.getValue(x, y, v))
  {
    _A -= v;
    _N--;
  }
  else
  {
    _nmissing++;
  }
}

//-----------------------------------------------------------------
bool Grid2dLoopAlgMeanNoMissing::getResult(int minGood,  double &result) const
{
  if (_nmissing <= 0 && _N > 0)
  {
    result = _A/_N;
    return true;
  }
  else
  {
    return false;
  }
}

//-----------------------------------------------------------------
void Grid2dLoopAlgSdev::increment(int x, int y, const Grid2d &G)
{
  //
  // which shows that this can be done using the 'fast' method.
  double v;
  if (G.getValue(x, y, v))
  {
    _A += v*v;
    _B += v;
    _N ++;
  }
}

//-----------------------------------------------------------------
void Grid2dLoopAlgSdev::decrement(int x, int y, const Grid2d &G)
{
  double v;
  if (G.getValue(x, y, v))
  {
    _A -= v*v;
    _B -= v;
    _N--;
  }
}

//-----------------------------------------------------------------
bool Grid2dLoopAlgSdev::getResult(int minGood, double &result) const
{
  if (_N > minGood)
  {
    result = sqrt(_N*_A - _B*_B)/_N;
    return true;
  }
  else
  {
    return false;
  }
}


//-----------------------------------------------------------------
Grid2dLoopAlgMedian::Grid2dLoopAlgMedian(double min, double max,
					 double delta) : Grid2dLoopAlg()
{
  _binMin = min;
  _binMax = max;
  _binDelta = delta;
  _nbin = static_cast<int>((_binMax-_binMin)/_binDelta) + 1;
  for (int i=0; i<_nbin; ++i)
  {
    double v = _binMin + _binDelta*i;
    _bin.push_back(v);
    _counts.push_back(0.0);
  }
  _nc = 0;
}
//-----------------------------------------------------------------
void Grid2dLoopAlgMedian::increment(int x, int y, const Grid2d &G)
{
  double v;
  if (G.getValue(x, y, v))
  {
    int index = static_cast<int>((v - _binMin)/_binDelta);
    if (index < 0)
    {
      index = 0;
    }
    if (index >= _nbin)
    {
      index = _nbin-1;
    }
    _counts[index]++;
    ++_nc;
  }
}

//-----------------------------------------------------------------
void Grid2dLoopAlgMedian::decrement(int x, int y, const Grid2d &G)
{
  double v;
  if (G.getValue(x, y, v))
  {
    _nc--;
    if (_nc < 0)
    {      
      LOG(ERROR) << "negative total count";
    }

    int index = static_cast<int>((v - _binMin)/_binDelta);
    if (index < 0)
    {
      index = 0;
    }
    if (index >= _nbin)
    {
      index = _nbin-1;
    }
    _counts[index]--;
    if (_counts[index] < 0)
    {
      LOG(ERROR) << "negative count of bin " << _bin[index];
    }
  }
}


//-----------------------------------------------------------------
bool Grid2dLoopAlgMedian::getResult(int minGood, double &result) const
{
  return getPercentile(0.5, minGood, result);
}

//-----------------------------------------------------------------
bool Grid2dLoopAlgMedian::getPercentile(double pct, int minGood, 
					double &result) const
{
  if (_nc < minGood)
  {
    return false;
  }
  else
  {
    return _pcntile(pct, result);
  }
}

//----------------------------------------------------------------
bool Grid2dLoopAlgMedian::_pcntile(double pct, double &result) const
{
  double fpt = pct*static_cast<double>(_nc);
  int ipt = static_cast<int>(fpt);
  int count=0;
  for (int i=0; i<_nbin; ++i)
  {
    count += static_cast<int>(_counts[i]);
    if (count >= ipt)
    {
      result = _bin[i];
      return true;
    }
  }
  LOG(ERROR) << "getting percentile " << pct;
  return false;
}


//----------------------------------------------------------------
bool Grid2dLoopAlgMedian::_count(double pct, double &result) const
{
  double fpt = pct*static_cast<double>(_nc);
  int ipt = static_cast<int>(fpt);
  int count=0;
  for (int i=0; i<_nbin; ++i)
  {
    count += static_cast<int>(_counts[i]);
    if (count >= ipt)
    {
      return _counts[i];
    }
  }
  LOG(ERROR) << "getting count " << pct;
  return false;
}

//-----------------------------------------------------------------
void Grid2dLoopAlgTexture::increment(int x, int y, const Grid2d &G)
{
  if (_isX)
  {
    if (y-1 >= 0)
    {
      double v1, v2;
      if (G.getValue(x, y, v1) && G.getValue(x, y-1, v2))
      {
	_A += (v1-v2)*(v1-v2);
	_N++;
      }
    }
  }
  else
  {
    if (x-1 >= 0)
    {
      double v1, v2;
      if (G.getValue(x, y, v1) && G.getValue(x-1, y, v2))
      {
	_A += (v1-v2)*(v1-v2);
	_N++;
      }
    }
  }
}


//-----------------------------------------------------------------
void Grid2dLoopAlgTexture::decrement(int x, int y, const Grid2d &G)
{
  if (_isX)
  {
    if (y-1 >= 0)
    {
      double v1, v2;
      if (G.getValue(x, y, v1) && G.getValue(x, y-1, v2))
      {
	_A -= (v1-v2)*(v1-v2);
	_N--;
      }
    }
  }
  else
  {
    if (x-1 >= 0)
    {
      double v1, v2;
      if (G.getValue(x, y, v1) && G.getValue(x-1, y, v2))
      {
	_A -= (v1-v2)*(v1-v2);
	_N--;
      }
    }
  }
}


//-----------------------------------------------------------------
bool Grid2dLoopAlgTexture::getResult(int minGood, double &result) const
{
  if (_N > minGood)
  {
    result = _A/_N;
    return true;
  }
  else
  {
    return false;
  }
}


//-----------------------------------------------------------------
bool Grid2dLoopAlgSpeckle::getResult(int minGood, double &result) const
{
  if (_nc < minGood)
  {
    return false;
  }
  else
  {
    double p25, p75;
    if (_pcntile(0.25, p25) && _pcntile(0.75, p75))
    {
      result = p75 - p25;
      return true;
    }
    else
    {
      return false;
    }
  }
}

//-----------------------------------------------------------------
bool Grid2dLoopAlgSpeckleInterest::getResult(int minGood, 
					     double &result) const
{
  if (_nc < minGood)
  {
    return false;
  }
  else
  {
    double p25, p50, p75, c25, c50, c75;
    if (_pcntile(0.25, p25) && _pcntile(0.50, p50) && _pcntile(0.75, p75) &&
	_count(0.25, c25) && _count(0.50, c50) && _count(0.75, c75))
    {
      result = 1.0;
      result *= _fuzzyDataDiff.apply(p50-p25);
      result *= _fuzzyDataDiff.apply(p75-p50);
      double num = getNum();
      c25 /= num;
      c50 /= num;
      c75 /= num;
      result *= _fuzzyCountPctDiff.apply(fabs(c50-c25));
      result *= _fuzzyCountPctDiff.apply(fabs(c75-c50));
      return true;
    }
    else
    {
      return false;
    }
  }
}

