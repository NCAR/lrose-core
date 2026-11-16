#include "WindGlyphRenderer.hh"

#include <algorithm>
#include <cmath>

#include <QFontMetrics>
#include <QLineF>
#include <QPolygonF>

void WindGlyphRenderer::drawArrow(QPainter &painter,
                                  const QPointF &tail,
                                  const QPointF &head,
                                  const QColor &color,
                                  double lineWidth,
                                  double headSize,
                                  double headAngleDeg)
{
  const QLineF line(tail, head);
  if (line.length() < 0.5) {
    return;
  }
  const QPointF unit = (head - tail) / line.length();
  const QPointF side(-unit.y(), unit.x());
  const double angle = headAngleDeg * M_PI / 180.0;
  const double back = headSize * cos(angle);
  const double across = headSize * sin(angle);

  painter.save();
  painter.setPen(QPen(color, std::max(1.0, lineWidth), Qt::SolidLine,
                      Qt::FlatCap, Qt::MiterJoin));
  painter.drawLine(tail, head);
  painter.drawLine(head, head - back * unit + across * side);
  painter.drawLine(head, head - back * unit - across * side);
  painter.restore();
}

void WindGlyphRenderer::drawBarb(QPainter &painter,
                                 const QPointF &origin,
                                 const QPointF &shaftUnit,
                                 double speedKnots,
                                 double shaftLength,
                                 double lineWidth,
                                 const QColor &color,
                                 bool southernHemisphere,
                                 LabelStyle labelStyle,
                                 double directionDeg)
{
  const double magnitude = std::hypot(shaftUnit.x(), shaftUnit.y());
  if (!std::isfinite(speedKnots) || magnitude < 1.0e-6) {
    return;
  }
  const QPointF unit = shaftUnit / magnitude;
  QPointF side(-unit.y(), unit.x());
  if (southernHemisphere) {
    side = -side;
  }

  painter.save();
  painter.setPen(QPen(color, std::max(1.0, lineWidth), Qt::SolidLine,
                      Qt::FlatCap, Qt::MiterJoin));
  painter.setBrush(Qt::NoBrush);

  const double calmRadius = std::max(2.0, shaftLength / 15.0);
  if (speedKnots < 2.5) {
    painter.drawEllipse(origin, calmRadius, calmRadius);
    painter.restore();
    return;
  }

  const QPointF tip = origin + shaftLength * unit;
  painter.drawEllipse(origin, calmRadius, calmRadius);
  painter.drawLine(origin, tip);

  // Flags are rounded to the conventional nearest five knots and placed from
  // the outer end inward. Reversing only the flag side in the southern
  // hemisphere preserves the geographic shaft direction.
  int knots = std::max(0, static_cast<int>(floor(speedKnots / 5.0 + 0.5)) * 5);
  const double spacing = std::max(2.0, shaftLength / 10.0);
  const double flagLength = std::max(4.0, shaftLength / 3.0);
  QPointF flagAt = tip;
  while (knots >= 50) {
    QPolygonF pennant;
    pennant << flagAt << flagAt - spacing * unit
            << flagAt - spacing * unit + flagLength * side;
    painter.setBrush(color);
    painter.drawPolygon(pennant);
    painter.setBrush(Qt::NoBrush);
    flagAt -= 1.5 * spacing * unit;
    knots -= 50;
  }
  while (knots >= 10) {
    painter.drawLine(flagAt, flagAt + flagLength * side);
    flagAt -= spacing * unit;
    knots -= 10;
  }
  if (knots >= 5) {
    painter.drawLine(flagAt, flagAt + 0.5 * flagLength * side);
  }

  if (labelStyle != LabelStyle::None) {
    int tens = static_cast<int>(floor((directionDeg + 5.0) / 10.0));
    if (tens <= 0) {
      tens = 36;
    }
    const QString text = labelStyle == LabelStyle::DirectionAtOrigin ?
      QString("%1").arg(tens, 2, 10, QChar('0')) :
      QString::number(tens % 10);
    const QPointF labelPoint = labelStyle == LabelStyle::DirectionAtOrigin ?
      origin + QPointF(2.0, 2.0) : tip + 0.2 * shaftLength * unit;
    painter.drawText(labelPoint, text);
  }
  painter.restore();
}
