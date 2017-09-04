/**
 * @file FieldThresh2WithGrid.cc
 */

#include <rapformats/FieldThresh2WithGrid.hh>
#include <euclid/Grid2d.hh>

double FieldThresh2WithGrid::getThresh(int gridIndex) const
{
  if (_gridded)
  {
    return _grid->getDataAt(gridIndex);
  }
  else 
  {
    return FieldThresh::getThresh();
  }
}
