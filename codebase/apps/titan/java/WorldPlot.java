///////////////////////////////////////////////////////////////////////
//
// WorldPlot
//
// World-coord plotting.
//
// Mike Dixon
//
// Jan 2003
//
////////////////////////////////////////////////////////////////////////

import java.awt.*;
import java.util.*;
import java.awt.geom.*;

public class WorldPlot

{
    
    private int _deviceWidth;
    private int _deviceHeight;
  
    private int _leftMargin;
    private int _rightMargin;
    private int _topMargin;
    private int _bottomMargin;
  
    private double _xMinWorld;
    private double _xMaxWorld;
    private double _yMinWorld;
    private double _yMaxWorld;
  
    private int _plotWidth;
    private int _plotHeight;
  
    private int _xMinDevice;
    private int _yMinDevice;
    private int _xMaxDevice;
    private int _yMaxDevice;
  
    private double _xPixelsPerWorldUnit;
    private double _yPixelsPerWorldUnit;

    private int _axisTickLen = 7;
    private int _nTicksIdeal = 7;
    private int _textSpacer = 5;

    private AffineTransform _transform;
  
    public WorldPlot(int deviceWidth,
		     int deviceHeight,
		     int leftMargin,
		     int rightMargin,
		     int topMargin,
		     int bottomMargin,
		     double xMinWorld,
		     double xMaxWorld,
		     double yMinWorld,
		     double yMaxWorld) {
    
	_deviceWidth = deviceWidth;
	_deviceHeight = deviceHeight;
    
	_leftMargin = leftMargin;
	_rightMargin = rightMargin;
	_topMargin = topMargin;
	_bottomMargin = bottomMargin;
    
	_xMinWorld = xMinWorld;
	_xMaxWorld = xMaxWorld;
	_yMinWorld = yMinWorld;
	_yMaxWorld = yMaxWorld;
    
	_plotWidth = _deviceWidth - _leftMargin - _rightMargin;
	_plotHeight = _deviceHeight - _topMargin - _bottomMargin;
    
	_xMinDevice = _leftMargin;
	_xMaxDevice = _xMinDevice + _plotWidth;
	_yMinDevice = _topMargin + _plotHeight;
	_yMaxDevice = _topMargin;

	_xPixelsPerWorldUnit =
	    (double) _plotWidth / (_xMaxWorld - _xMinWorld);
	_yPixelsPerWorldUnit =
	    -1.0 * ((double) _plotHeight / (_yMaxWorld - _yMinWorld));

	_transform = new AffineTransform();
	_transform.translate(_xMinDevice, _yMinDevice);
	_transform.scale(_xPixelsPerWorldUnit, _yPixelsPerWorldUnit);
	_transform.translate(-_xMinWorld, -_yMinWorld);

    }

    public void setAxisTickLen(int len) {
	_axisTickLen = len;
    }

    public void setNTicksIdeal(int nTicks) {
	_nTicksIdeal = nTicks;
    }

    public double getXDevice(double xWorld) {
	return (xWorld - _xMinWorld) * _xPixelsPerWorldUnit + _xMinDevice;
    }
  
    public double getYDevice(double yWorld) {
	return (yWorld - _yMinWorld) * _yPixelsPerWorldUnit + _yMinDevice;
    }

    public double getXWorld(double xDevice) {
	return (xDevice - _xMinDevice) / _xPixelsPerWorldUnit + _xMinWorld;
    }
  
    public double getYWorld(double yDevice) {
	return (yDevice - _yMinDevice) / _yPixelsPerWorldUnit + _yMinWorld;
    }
  
    // draw a line

    public void drawLine(Graphics2D g2,
			 double x1, double y1, double x2, double y2) {

	double xx1 = getXDevice(x1);
	double yy1 = getYDevice(y1);
	double xx2 = xx1 + (x2 - x1) * _xPixelsPerWorldUnit;
	double yy2 = yy1 + (y2 - y1) * _yPixelsPerWorldUnit;
	g2.draw(new Line2D.Double(xx1, yy1, xx2, yy2));

    }

    // draw lines

    public void drawLines(Graphics2D g2, Point2D.Double[] points) {

	GeneralPath path = new GeneralPath();
    
	float xx = (float) getXDevice(points[0].x);
	float yy = (float) getXDevice(points[0].y);
	path.moveTo(xx, yy);
	for (int i = 1; i < points.length; i++) {
	    xx = (float) getXDevice(points[i].x);
	    yy = (float) getYDevice(points[i].y);
	    path.lineTo(xx, yy);
	}
	g2.draw(path);

    }

    // draw a general path

    public void drawPath(Graphics2D g2, GeneralPath path) {
	path.transform(_transform);
	g2.draw(path);
    }

    // draw a general path clipped to within the margins

    public void drawPathClipped(Graphics2D g2, GeneralPath path) {
	g2.setClip(_xMinDevice, _yMaxDevice, _plotWidth,  _plotHeight);
	path.transform(_transform);
	g2.draw(path);
	g2.setClip(null);
    }

    // Title
    
    public void drawTitleTopCenter(Graphics2D g2,
				   String title,
				   boolean above) {
	
	Font currentFont = g2.getFont();
	float fontSize = currentFont.getSize2D();
	Font titleFont = currentFont.deriveFont((float) (fontSize * 1.5));
	g2.setFont(titleFont);
	FontMetrics metrics = g2.getFontMetrics();
	Rectangle2D rect = metrics.getStringBounds(title, g2);
	float xx = (float)
	    ((_xMinDevice + _xMaxDevice - rect.getWidth()) / 2.0);
	float yy;
	if (above) {
	    yy = (float)  rect.getHeight();
	} else {
	    yy = (float) (_yMaxDevice + _axisTickLen +
			  _textSpacer + rect.getHeight());
	}
	g2.drawString(title, xx, yy);
	g2.setFont(currentFont);
	
    }
	
    // Legends at top left
    
    public void drawLegendsTopLeft(Graphics2D g2,
				   ArrayList legends) {

	for (int i = 0; i < legends.size(); i++) {
	    String legend = (String) legends.get(i);
	    FontMetrics metrics = g2.getFontMetrics();
	    Rectangle2D rect = metrics.getStringBounds(legend, g2);
	    float xx = (float) (_xMinDevice + _axisTickLen + _textSpacer);
	    float yy = (float) (_yMaxDevice + _axisTickLen +
				i * _textSpacer +
				(i + 1) * rect.getHeight());
	    g2.drawString(legend, xx, yy);
	}

    }
	
    // Legends at top right
    
    public void drawLegendsTopRight(Graphics2D g2,
				    ArrayList legends) {

	for (int i = 0; i < legends.size(); i++) {
	    String legend = (String) legends.get(i);
	    FontMetrics metrics = g2.getFontMetrics();
	    Rectangle2D rect = metrics.getStringBounds(legend, g2);
	    float xx = (float) (_xMaxDevice - _axisTickLen -
				_textSpacer - rect.getWidth());
	    float yy = (float) (_yMaxDevice + _axisTickLen +
				i * _textSpacer +
				(i + 1) * rect.getHeight());
	    g2.drawString(legend, xx, yy);
	}

    }
	
    // left axis
    
    public void drawAxisLeft(Graphics2D g2,
			     String units) {
	
	// axis line

	drawLine(g2, _xMinWorld, _yMinWorld, _xMinWorld, _yMaxWorld);

	// axis units label

	FontMetrics metrics = g2.getFontMetrics();
	Rectangle2D unitsRect = metrics.getStringBounds(units, g2);
	float unitsX = (float) (_xMinDevice - unitsRect.getWidth() - _textSpacer);
	float unitsY = (float) (_yMaxDevice + (unitsRect.getHeight() / 2));
	g2.drawString(units, unitsX, unitsY);

	// tick marks

	double[] ticks = linearTicks(_yMinWorld, _yMaxWorld, _nTicksIdeal);
	if (ticks.length < 2) {
	    return;
	}
	
	double delta = ticks[1] - ticks[0];
	for (int i = 0; i < ticks.length; i++) {
	    
	    double val = ticks[i];
	    double pix = getYDevice(val);
	    g2.draw(new Line2D.Double(_xMinDevice, pix,
				      _xMinDevice + _axisTickLen, pix));
	    
	    String label = getAxisLabel(delta, val);
	    Rectangle2D rect = metrics.getStringBounds(label, g2);
	    float strX = (float) (_xMinDevice - rect.getWidth() - _textSpacer);
	    float strY = (float) (pix + (rect.getHeight() / 2));
	    if (Math.abs(strY - unitsY) > rect.getHeight() + _textSpacer) {
		g2.drawString(label, strX, strY);
	    }
	    
	}
	
    } // drawAxisLeft

    // right axis
  
    public void drawAxisRight(Graphics2D g2,
			      String units) {
	
	// axis line

	drawLine(g2, _xMaxWorld, _yMinWorld, _xMaxWorld, _yMaxWorld);

	// axis units label
	
	FontMetrics metrics = g2.getFontMetrics();
	Rectangle2D unitsRect = metrics.getStringBounds(units, g2);
	float unitsX = (float) (_xMaxDevice + _textSpacer);
	float unitsY = (float) (_yMaxDevice + (unitsRect.getHeight() / 2));
	g2.drawString(units, unitsX, unitsY);

	// tick marks

	double[] ticks = linearTicks(_yMinWorld, _yMaxWorld, _nTicksIdeal);
	if (ticks.length < 2) {
	    return;
	}
	
	double delta = ticks[1] - ticks[0];
	for (int i = 0; i < ticks.length; i++) {
	    
	    double val = ticks[i];
	    double pix = getYDevice(val);
	    g2.draw(new Line2D.Double(_xMaxDevice, pix,
				      _xMaxDevice - _axisTickLen, pix));
	    
	    String label = getAxisLabel(delta, val);
	    Rectangle2D rect = metrics.getStringBounds(label, g2);
	    float strX = (float) (_xMaxDevice + _textSpacer);
	    float strY = (float) (pix + (rect.getHeight() / 2));
	    if (Math.abs(strY - unitsY) > rect.getHeight() + _textSpacer) {
		g2.drawString(label, strX, strY);
	    }
	    
	}
	
    } // drawAxisRight
  
    // bottom axis
    
    public void drawAxisBottom(Graphics2D g2, String units) {
	
	// axis line
	
	drawLine(g2, _xMinWorld, _yMinWorld, _xMaxWorld, _yMinWorld);
	
	// axis units label
	
	FontMetrics metrics = g2.getFontMetrics();
	Rectangle2D unitsRect = metrics.getStringBounds(units, g2);
	float unitsX = (float) (_xMaxDevice - unitsRect.getWidth() / 2);
	float unitsY = (float) (_yMinDevice + (unitsRect.getHeight() + _textSpacer));
	g2.drawString(units, unitsX, unitsY);

	// tick marks

	double[] ticks = linearTicks(_xMinWorld, _xMaxWorld, _nTicksIdeal);
	if (ticks.length < 2) {
	    return;
	}
	
	double delta = ticks[1] - ticks[0];
	for (int i = 0; i < ticks.length; i++) {
	    
	    double val = ticks[i];
	    double pix = getXDevice(val);
	    g2.draw(new Line2D.Double(pix, _yMinDevice,
				      pix, _yMinDevice - _axisTickLen));
	    
	    String label = getAxisLabel(delta, val);
	    Rectangle2D rect = metrics.getStringBounds(label, g2);
	    if ((pix + rect.getWidth() / 2 + _textSpacer) < unitsX) {
		float strX = (float) (pix - rect.getWidth() / 2);
		float strY = unitsY;
		g2.drawString(label, strX, strY);
	    }
	    
	}
	
    } // drawAxisBottom
  
    // top axis
    
    public void drawAxisTop(Graphics2D g2, String units) {
	
	// axis line
	
	drawLine(g2, _xMinWorld, _yMaxWorld, _xMaxWorld, _yMaxWorld);
	
	// axis units label
	
	FontMetrics metrics = g2.getFontMetrics();
	Rectangle2D unitsRect = metrics.getStringBounds(units, g2);
	float unitsX = (float) (_xMaxDevice - unitsRect.getWidth() / 2);
	float unitsY = (float) (_yMaxDevice - (unitsRect.getHeight() - _textSpacer));
	g2.drawString(units, unitsX, unitsY);

	// tick marks
	
	double[] ticks = linearTicks(_xMinWorld, _xMaxWorld, _nTicksIdeal);
	if (ticks.length < 2) {
	    return;
	}

	double delta = ticks[1] - ticks[0];
	for (int i = 0; i < ticks.length; i++) {
	    
	    double val = ticks[i];
	    double pix = getXDevice(val);
	    g2.draw(new Line2D.Double(pix, _yMaxDevice,
				      pix, _yMaxDevice + _axisTickLen));
	    
	    String label = getAxisLabel(delta, val);
	    Rectangle2D rect = metrics.getStringBounds(label, g2);
	    if ((pix + rect.getWidth() / 2 + _textSpacer) < unitsX) {
		float strX = (float) (pix - rect.getWidth() / 2);
		float strY = unitsY;
		g2.drawString(label, strX, strY);
	    }
	    
	}
	
    } // drawAxisTop
  
    static public double[] linearTicks(double minVal,
				       double maxVal,
				       long approxNticks) {
	
	double  approxInterval =
	    (maxVal - minVal) / (double) (approxNticks + 1);
	double logInterval = Math.log(Math.abs(approxInterval)) / 2.30259;
	double intPart = (int) logInterval;
	double fractPart = logInterval - intPart;

	if (fractPart < 0) {
	    fractPart += 1.0;
	    intPart -= 1.0;
	}
	
	double rem = Math.pow(10.0, fractPart);
	double base;
	if (rem > 7.5) {
	    base = 10.0;
	} else if (rem > 3.5) {
	    base = 5.0;
	} else if (rem > 1.5) {
	    base = 2.0;
	} else {
	    base = 1.0;
	}
	
	double deltaTick = (base * Math.pow (10.0, intPart));
	double tickMin = Math.floor(minVal / deltaTick) * deltaTick;
	if (tickMin < minVal) {
	  tickMin += deltaTick;
	}
	int nTicks = (int) ((maxVal - tickMin) / deltaTick) + 1;
	
	double ticks[] = new double[nTicks];
	for (int i = 0; i < nTicks; i++) {
	    ticks[i] = tickMin + i * deltaTick;
	}

	return ticks;
	
    }

    private String getAxisLabel(double delta, double val) {

	if (delta >= 1.0) {
	    return NFormat.f0.format(val);
	} else if (delta >= 0.1) {
	    return NFormat.f1.format(val);
	} else if (delta >= 0.01) {
	    return NFormat.f2.format(val);
	} else if (delta >= 0.001) {
	    return NFormat.f3.format(val);
	} else if (delta >= 0.0001) {
	    return NFormat.f4.format(val);
	} else {
	    return NFormat.e3.format(val);
	}

    }
    
}

