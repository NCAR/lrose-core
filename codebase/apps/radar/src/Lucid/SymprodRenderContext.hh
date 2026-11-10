// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1990 - 2016
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** BSD licence applies - see the source distribution for details.
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
////////////////////////////////////////////////////////////////
// SymprodRenderContext.hh
//
// Read-only display state used while rendering symbolic products with Qt.
////////////////////////////////////////////////////////////////

#ifndef SymprodRenderContext_HH
#define SymprodRenderContext_HH

#include <ctime>

class WorldPlot;

struct SymprodRenderContext
{
  SymprodRenderContext(const WorldPlot &worldPlot,
                       time_t epochStart,
                       time_t epochEnd,
                       time_t frameStart,
                       time_t frameEnd,
                       time_t dataTime,
                       double kmAcrossScreen,
                       double iconScale) :
          plot(worldPlot),
          epochStart(epochStart),
          epochEnd(epochEnd),
          frameStart(frameStart),
          frameEnd(frameEnd),
          dataTime(dataTime),
          kmAcrossScreen(kmAcrossScreen),
          iconScale(iconScale),
          showHidden(false),
          showingHidden(false)
  {
  }

  const WorldPlot &plot;
  time_t epochStart;
  time_t epochEnd;
  time_t frameStart;
  time_t frameEnd;
  time_t dataTime;
  double kmAcrossScreen;
  double iconScale;
  bool showHidden;

  // Hidden objects are alternatives within a product. This mutable flag lets
  // the first eligible object win without placing render state in WorldPlot.
  mutable bool showingHidden;
};

#endif
