// Shared Qt geometry for station and gridded wind symbols.

#ifndef WIND_GLYPH_RENDERER_HH
#define WIND_GLYPH_RENDERER_HH

#include <QColor>
#include <QPainter>
#include <QPointF>

class WindGlyphRenderer
{
public:
  enum class LabelStyle {
    None,
    DirectionAtOrigin,
    DirectionTensAtTip
  };

  static void drawArrow(QPainter &painter,
                        const QPointF &tail,
                        const QPointF &head,
                        const QColor &color,
                        double lineWidth,
                        double headSize,
                        double headAngleDeg);

  // shaftUnit points from the observation location into the direction from
  // which the wind arrives. Speed is expressed in knots for standard flags.
  static void drawBarb(QPainter &painter,
                       const QPointF &origin,
                       const QPointF &shaftUnit,
                       double speedKnots,
                       double shaftLength,
                       double lineWidth,
                       const QColor &color,
                       bool southernHemisphere,
                       LabelStyle labelStyle = LabelStyle::None,
                       double directionDeg = 0.0);
};

#endif
