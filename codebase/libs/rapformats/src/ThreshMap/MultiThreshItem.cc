/**
 * @file MultiThreshItem.cc
 */
#include <rapformats/MultiThreshItem.hh>
#include <toolsa/LogStream.hh>
#include <cstdio>

//-------------------------------------------------------------------
void MultiThreshItem::logDebug(void) const
{
  LOG(DEBUG) << "Gen:" << _genHour << ":" << _genMin << ":" << _genSec;
  _multiThresh.logDebug(_leadTime, _tileIndex);
}
//-------------------------------------------------------------------
void MultiThreshItem::print(void) const
{
  printf("Gen:%02d:%02d:%02d  ", _genHour, _genMin, _genSec);
  _multiThresh.print(_leadTime, _tileIndex);
}
