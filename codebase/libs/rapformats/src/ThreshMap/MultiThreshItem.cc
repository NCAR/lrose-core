/**
 * @file MultiThreshItem.cc
 */
#include <rapformats/MultiThreshItem.hh>
#include <rapformats/TileInfo.hh>
#include <toolsa/LogStream.hh>
#include <cstdio>

//-------------------------------------------------------------------
void MultiThreshItem::logDebug(void) const
{
  LOG(DEBUG) << "Gen:" << _genHour << ":" << _genMin << ":" << _genSec;
  _multiThresh.logDebug(_leadTime, _tileIndex);
}
//-------------------------------------------------------------------
void MultiThreshItem::print(const TileInfo &info) const
{
  printf("Gen:%02d:%02d:%02d  ", _genHour, _genMin, _genSec);
  _multiThresh.print(_leadTime, _tileIndex, info);
}
