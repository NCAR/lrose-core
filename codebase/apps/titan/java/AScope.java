///////////////////////////////////////////////////////////////////////
//
// AScope
//
// JFrame with ParamManager member
//
// Mike Dixon
//
// Oct 2002
//
////////////////////////////////////////////////////////////////////////

import java.io.*;
import java.awt.*;
import java.awt.event.*;
import java.awt.geom.*;
import javax.swing.*;
import javax.swing.event.*;
import java.util.*;

import params.*;

public class AScope extends JFrame

{

    private Parameters _params;
    private JPanel _topPanel;
    private Canvas _canvas;
    private BeamMessage _beam = null;
    private boolean _plotCount = true;
    private boolean _plotDbz = true;
    private boolean _plotSample = false;
    private int _sampleCenter;
    private int _sampleNGates;
    private int _nGates;
    private double _startRange;
    private double _gateSpacing;
    private Color _countColor = Color.RED;
    private Color _dbzColor = Color.BLUE;
    private JSlider _sampleSlider;
    private JSpinner _sampleSpinner;
    private SpinnerNumberModel _sampleSpinnerModel;
    private JButton _unZoomButton;
    private WorldPlot _countPlot;
    private WorldPlot _dbzPlot;

    public AScope(Parameters params) {
	
	super();
	_params = params;

	_plotCount = _params.aScope.plotCount.getValue();
	_plotDbz = _params.aScope.plotDbz.getValue();
	_plotSample = _params.aScope.plotSample.getValue();
	_sampleCenter = _params.aScope.sampleCenter.getValue();
	_sampleNGates = _params.aScope.sampleNGates.getValue();
	_nGates = _params.scan.nGates.getValue();

	// basic window setup

	setTitle("A Scope");
	setSize(_params.aScope.width.getValue(),
		_params.aScope.height.getValue());
	setLocation(_params.aScope.xx.getValue(),
		    _params.aScope.yy.getValue());
	setDefaultCloseOperation(JFrame.HIDE_ON_CLOSE);
	addComponentListener(new MoveResizeListener(_params));
  	_topPanel = new JPanel(new BorderLayout());
  	getContentPane().add(_topPanel);

	// canvas

	_canvas = new Canvas();
	_topPanel.add(_canvas, BorderLayout.CENTER);

	// dbz and count options panel
	
	JToggleButton countToggle =
	    new JToggleButton("Plot count?", _plotCount);
	countToggle.addItemListener(new CountToggleListener());
	JToggleButton dbzToggle =
	    new JToggleButton("Plot dBZ?", _plotDbz);
	dbzToggle.addItemListener(new DbzToggleListener());
  	JPanel countDbzPanel = new JPanel(new BorderLayout());
	countDbzPanel.add(countToggle, BorderLayout.NORTH);
	countDbzPanel.add(dbzToggle, BorderLayout.CENTER);
	JPanel countDbzPanel2 = new JPanel();
	countDbzPanel2.add(countDbzPanel);
	
	// sample panel

	JToggleButton sampleToggle =
	    new JToggleButton("Sample gates?", _plotSample);
	sampleToggle.addItemListener(new SampleToggleListener());
	
	JLabel ngatesLabel = new JLabel("N gates in sample: ");
	_sampleSpinnerModel =
	    new SpinnerNumberModel(new Integer(_sampleNGates),
				   new Integer(1),
				   new Integer(_nGates),
				   new Integer(1));
	_sampleSpinner = new JSpinner(_sampleSpinnerModel);
	_sampleSpinner.setEnabled(_plotSample);
	_sampleSpinner.addChangeListener(new SampleSpinnerListener());
	
  	JPanel ngatesSamplePanel = new JPanel();
	ngatesSamplePanel.add(ngatesLabel);
	ngatesSamplePanel.add(_sampleSpinner);
	
  	JPanel sampleTopPanel = new JPanel();
	sampleTopPanel.add(sampleToggle);
	sampleTopPanel.add(ngatesSamplePanel);
	
	_sampleSlider = new JSlider();
	_sampleSlider.setMinimum(1);
	_sampleSlider.setMaximum(_nGates);
	_sampleSlider.setValue(_sampleCenter);
	_sampleSlider.setEnabled(_plotSample);
	_sampleSlider.addChangeListener(new SampleSliderListener());
	
  	JPanel samplePanel = new JPanel(new BorderLayout());
	samplePanel.add(sampleTopPanel, BorderLayout.NORTH);
	samplePanel.add(_sampleSlider, BorderLayout.CENTER);
	
	// zoom panel
	
	_unZoomButton = new JButton("UnZoom");
	_unZoomButton.setEnabled(false);
	_unZoomButton.addActionListener(new ActionListener() {
                public void actionPerformed(ActionEvent e) {
		    _canvas.setUnzoomed();
		    _unZoomButton.setEnabled(false);
                }
            });

	JButton helpButton = new JButton("Help");
	helpButton.addActionListener(new ActionListener() {
                public void actionPerformed(ActionEvent e) {
		    AScopeInfoFrame info =
			new AScopeInfoFrame(getX() + 30, getY() + 30,
					    600, 300);
                }
            });

  	JPanel zoomPanel = new JPanel(new BorderLayout());
	zoomPanel.add(_unZoomButton, BorderLayout.NORTH);
	zoomPanel.add(helpButton, BorderLayout.CENTER);
	JPanel zoomPanel2 = new JPanel();
	zoomPanel2.add(zoomPanel);
	
	// lower panel
	
  	JPanel lowerPanel = new JPanel(new BorderLayout());
	lowerPanel.add(countDbzPanel2, BorderLayout.WEST);
	lowerPanel.add(samplePanel, BorderLayout.CENTER);
	lowerPanel.add(zoomPanel2, BorderLayout.EAST);
	
	_topPanel.add(lowerPanel, BorderLayout.SOUTH);

    }

    // set beam message

    public void setBeamMessage(BeamMessage message) {
	_beam = message;
	_canvas.doRepaint();
    }
    
    // inner class for canvas

    private class Canvas extends JPanel

    {
	
	// Zooming GUI rubber banding rectangle info
	private boolean _zoomingRectangleEnabled = true;
	private Zooming _zooming = null;     // null if not currently zooming
	private Zoomed _unZoomed;            // always exists
	private Zoomed _zoomed = null;       // null if not zoomed
	private Zoomed _prevZoom;            // keeps track of previous zoom
    
	public Canvas() {
	    super();
	    _unZoomed = new Zoomed();
	    ZoomMouseListener zoomListener = new ZoomMouseListener();
	    addMouseListener(zoomListener);
	    addMouseMotionListener(zoomListener);
	}

	public void doRepaint() {
	    if (_zooming == null) {
		repaint();
	    }
	}

	public void setUnzoomed() {
	    _zoomed = null;
	}

	public void paintComponent (Graphics g) {
	    
	    super.paintComponent(g);
	    Dimension size = getSize();
	    Graphics2D g2 = (Graphics2D) g;
	    Insets insets = getInsets();

	    // return now if no beam data
	    
	    if (_beam == null) {
		return;
	    }
	    short[] counts = _beam.getCounts();

	    // compute margins
	    
	    int topMargin =
		insets.top + _params.aScope.topMargin.getValue();
	    int bottomMargin =
		insets.bottom + _params.aScope.bottomMargin.getValue();
	    int leftMargin =
		insets.left + _params.aScope.leftMargin.getValue();
	    int rightMargin =
		insets.right + _params.aScope.rightMargin.getValue();

	    // gates and range

	    _nGates = _beam.getNGates();
	    _sampleSlider.setMaximum(_nGates);
	    _sampleSpinnerModel.setMaximum(new Integer(_nGates));
	    _startRange = _beam.getStartRange();
	    _gateSpacing = _beam.getGateSpacing();
	    
	    _unZoomed.minGate = 0;
	    _unZoomed.maxGate = _nGates - 1;
	    _unZoomed.minRange = _startRange - _gateSpacing * 0.5;
	    _unZoomed.maxRange = _startRange + (_nGates - 0.5) * _gateSpacing;
	    int nGatesPlotted = _unZoomed.maxGate - _unZoomed.minGate + 1;
		
	    // count limits

	    _unZoomed.minCount = 0.0;
	    _unZoomed.maxCount = _params.aScope.maxCount.getValue();
	    
	    // dbz limits

	    _unZoomed.minDbz = _params.aScope.minDbz.getValue();
	    _unZoomed.maxDbz = _params.aScope.maxDbz.getValue();

	    // set this zoom

	    Zoomed thisZoom;
	    if (_zoomed != null) {
		thisZoom = _zoomed;
	    } else {
		thisZoom = _unZoomed;
	    }
	    
	    // set up world plotting objects
	    
	    _countPlot = new WorldPlot(size.width, size.height,
				       leftMargin, rightMargin,
				       topMargin, bottomMargin,
				       thisZoom.minGate, thisZoom.maxGate,
				       thisZoom.minCount, thisZoom.maxCount);

	    _dbzPlot = new WorldPlot(size.width, size.height,
				     leftMargin, rightMargin,
				     topMargin, bottomMargin,
				     thisZoom.minRange, thisZoom.maxRange,
				     thisZoom.minDbz, thisZoom.maxDbz);

	    // count trace
	    
  	    if (_plotCount) {
  		GeneralPath countPath = new GeneralPath();
  		for (int i = thisZoom.minGate; i <= thisZoom.maxGate; i++) {
  		    if (i == thisZoom.minGate) {
  			countPath.moveTo((float) (i + 0.5), counts[i]);
  		    } else {
  			countPath.lineTo((float) (i + 0.5), counts[i]);
  		    }
  		}
  		g2.setColor(_countColor);
  		_countPlot.drawPathClipped(g2, countPath);
  		g2.setColor(getForeground());
  	    }
	    
	    // dBZ trace
	    
	    double powerLow = _params.calib.powerLow.getValue();
	    double powerHigh = _params.calib.powerHigh.getValue();
	    double vipLow = _params.calib.vipLow.getValue();
	    double vipHigh = _params.calib.vipHigh.getValue();
	    double xmtPower = _params.calib.xmtPower.getValue();
	    double radarConstant = _params.calib.radarConstant.getValue();
	    double atmosAtten = _params.calib.atmosAtten.getValue();
	    double atmosAttenNm = atmosAtten * 1.85196;
	    double slope = (powerHigh - powerLow) / (vipHigh - vipLow);
	    double xmt1 = radarConstant + xmtPower;
	    float range[] = new float[_nGates];
	    float dbz[] = new float[_nGates];
	    
	    for (int i = 0; i < _nGates; i++) {
		range[i] = (float) (_startRange + i * _gateSpacing);
		double rangeNm = range[i] / 1.85196;
		double log10Range = Math.log(rangeNm) / 2.30259;
		double rangeCorrection =
		    20.0 * log10Range + rangeNm * atmosAttenNm - xmt1;
		double power = (counts[i] - vipLow) * slope + powerLow;
		dbz[i] = (float) (power + rangeCorrection);
	    }
	    
	    if (_plotDbz) {
		GeneralPath dbzPath = new GeneralPath();
		for (int i = thisZoom.minGate; i <= thisZoom.maxGate; i++) {
		    if (i == thisZoom.minGate) {
			dbzPath.moveTo(range[i], dbz[i]);
		    } else {
			dbzPath.lineTo(range[i], dbz[i]);
		    }
		}
		g2.setColor(_dbzColor);
		_dbzPlot.drawPathClipped(g2, dbzPath);
		g2.setColor(getForeground());
	    }

	    // axes
	    
	    _countPlot.drawAxisBottom(g2, "gates");
	    _dbzPlot.drawAxisTop(g2, "km");
	    
	    if (_plotCount) {
		g2.setColor(_countColor);
		_countPlot.drawAxisLeft(g2, "count");
		g2.setColor(getForeground());
	    } else {
		_countPlot.drawLine(g2,
				    thisZoom.minGate, thisZoom.minCount,
				    thisZoom.minGate, thisZoom.maxCount);
	    }
	    
	    if (_plotDbz) {
		g2.setColor(_dbzColor);
		_dbzPlot.drawAxisRight(g2, "dBZ");
		g2.setColor(getForeground());
	    } else {
		_dbzPlot.drawLine(g2,
				  thisZoom.maxRange, thisZoom.minDbz,
				  thisZoom.maxRange, thisZoom.maxDbz);
	    }
	    
	    // sampling
	    
	    int sampleStartGate = 0, sampleEndGate = 0;
	    int sampleMidGate = 0;
	    double sampleMidRange = 0.0;
	    double meanCount = 0;
	    double meanDbz = 0;

	    if (_plotSample) {

		double sumCount = 0.0;
		double sumDbz = 0.0;
		double nn = 0.0;
		sampleStartGate = _sampleCenter - _sampleNGates / 2;
		if (sampleStartGate < thisZoom.minGate) {
		    sampleStartGate = thisZoom.minGate;
		}
		if (sampleStartGate > thisZoom.maxGate) {
		    sampleStartGate = thisZoom.maxGate;
		}
		sampleEndGate = sampleStartGate + _sampleNGates - 1;
		if (sampleEndGate < thisZoom.minGate) {
		    sampleEndGate = thisZoom.minGate;
		}
		if (sampleEndGate > thisZoom.maxGate) {
		    sampleEndGate = thisZoom.maxGate;
		}
		sampleMidGate = (sampleStartGate + sampleEndGate) / 2;
		sampleMidRange = _startRange + sampleMidGate * _gateSpacing;
		for (int i = sampleStartGate; i <= sampleEndGate; i++) {
		    sumCount += counts[i];
		    sumDbz += dbz[i];
		    nn++;
		}
		if (nn == 0) {
		    nn = 1;
		}
		meanCount = sumCount / nn;
		meanDbz = sumDbz / nn;
		
		g2.setColor(Color.YELLOW);
		_countPlot.drawLine(g2,
				    sampleStartGate, thisZoom.minCount,
				    sampleStartGate, thisZoom.maxCount);
		_countPlot.drawLine(g2,
				    sampleEndGate, thisZoom.minCount,
				    sampleEndGate, thisZoom.maxCount);
		g2.setColor(getForeground());

	    }

	    // legends
	    
	    ArrayList leftLegends = new ArrayList();
	    Date date = _beam.getDate();
	    Calendar cal = Calendar.getInstance();
	    cal.setTime(date);
	    String timeStr = new
		String(cal.get(Calendar.YEAR) +
		       "/" + (cal.get(Calendar.MONTH) + 1) +
		       "/" + cal.get(Calendar.DAY_OF_MONTH) +
		       " " + cal.get(Calendar.HOUR_OF_DAY) +
		       ":" + cal.get(Calendar.MINUTE) +
		       ":" + cal.get(Calendar.SECOND) +
		       "." + cal.get(Calendar.MILLISECOND));
	    leftLegends.add(new String("Time: " + timeStr));
	    leftLegends.add
		(new String("El: " + NFormat.f2.format(_beam.getEl())));
	    leftLegends.add
		(new String("Az: " + NFormat.f2.format(_beam.getAz())));
	    leftLegends.add(new String("PRF: " + _beam.getPrf()));
	    _dbzPlot.drawLegendsTopLeft(g2, leftLegends);
	    
	    ArrayList rightLegends = new ArrayList();
	    if (_plotSample) {
		if (_plotCount) {
		    rightLegends.add
			(new String("Sample count: " +
				    NFormat.f2.format(meanCount)));
		}
		if (_plotDbz) {
		    rightLegends.add
			(new String("Sample dbz: " +
				    NFormat.f2.format(meanDbz)));
		}
		rightLegends.add
		    (new String("Sample start gate: " +
				NFormat.f0.format(sampleStartGate)));
		rightLegends.add
		    (new String("Sample end gate: " +
				NFormat.f0.format(sampleEndGate)));
		rightLegends.add
		    (new String("Sample mid gate: " +
				NFormat.f0.format(sampleMidGate)));
		rightLegends.add
		    (new String("Sample mid range(km): " +
				NFormat.f2.format(sampleMidRange)));
	    }
	    _dbzPlot.drawLegendsTopRight(g2, rightLegends);
	    
	    // title
	    
	    int defaultFontSize;
	    _countPlot.drawTitleTopCenter(g2, "A-SCOPE", false);

	}

	// Inner class used for tracking the zoom rectangle
	// and click origin

	private class Zooming {
	    
	    public Rectangle rect = new Rectangle(0, 0, 0, 0);
	    public boolean isFirstZoomRect = true;
	    public Point startPoint;
	    public int activeButton;
	    
	    public Zooming(Point start_point, int active_button) {
		startPoint = new Point(start_point.x, start_point.y);
		activeButton = active_button;
	    }
	    
	}
	
	// Inner class for current zoom state

	private class Zoomed {

	    public int minGate;
	    public int maxGate;
	    public double minRange;
	    public double maxRange;
	    public double minCount;
	    public double maxCount;
	    public double minDbz;
	    public double maxDbz;

	    public void print(PrintStream out) {
		out.println("========= Zoom ==========");
		out.println("  minGate: " + minGate);
		out.println("  maxGate: " + maxGate);
		out.println("  minRange: " + minRange);
		out.println("  maxRange: " + maxRange);
		out.println("  minCount: " + minCount);
		out.println("  maxCount: " + maxCount);
		out.println("  minDbz: " + minDbz);
		out.println("  maxDbz: " + maxDbz);
	    }

	}
	
	// zoom mouse listener
	
	private class ZoomMouseListener
	    implements MouseListener, MouseMotionListener {
	    
	    public void mouseClicked(MouseEvent e) {
	    }
	    
	    public void mouseDragged(MouseEvent e) {
		
		if(_zooming == null) {
		    return;
		}
		
		int xx = e.getPoint().x;
		int yy = e.getPoint().y;
		
		// We're zooming. Paint a rectangle to show zoom region.
		// It should have the same aspect ratio as the window, and not
		// Be any smaller than is allowed by max_scale.
		
		int rectWidth = (int) Math.abs(xx - _zooming.startPoint.getX());
		int rectHeight = (int) Math.abs(yy - _zooming.startPoint.getY());
		int rectOriginX = 0;
		int rectOriginY = 0;
		
		// double aspectRatio = (float) getHeight() / getWidth();
		// rectWidth  = (int) Math.abs(xx - _zooming.startPoint.getX());
		// int y_range_1 = (int) (aspectRatio * rectWidth);
		// int y_range_2 = (int) Math.abs(yy - _zooming.startPoint.getY());
		// if(y_range_2 > y_range_1) {
		//   // y-distance determines size
		//   rectHeight = y_range_2;
		//   rectWidth = (int) (rectHeight / aspectRatio);
		// } else {
		//   // x-distance determines size
		//   rectHeight = y_range_1;
		// }
	    
		if(xx > _zooming.startPoint.getX()) {
		    rectOriginX = (int) _zooming.startPoint.getX();
		} else {
		    rectOriginX =
			((int) _zooming.startPoint.getX()) - rectWidth;
		}
		
		if(yy > _zooming.startPoint.getY()) {
		    rectOriginY = (int) _zooming.startPoint.getY();
		} else {
		    rectOriginY =
			((int) _zooming.startPoint.getY()) - rectHeight;
		}
		
		Graphics g = getGraphics();
		g.setColor(Color.red);
		g.setXORMode(getBackground());
		// g.setXORMode(Color.white);
		
		// reverse the previous zooming rectangle outline if this is not the 1st
		if(! _zooming.isFirstZoomRect) {
		    Rectangle prevZoomRect = _zooming.rect;
		    drawZoomRect(g,
				 (int) prevZoomRect.getX(),
				 (int) prevZoomRect.getY(),
				 (int) prevZoomRect.getWidth(),
				 (int) prevZoomRect.getHeight());
		} else {
		    _zooming.isFirstZoomRect = false;
		}
		
		Rectangle zoomRect = _zooming.rect;
		zoomRect.setBounds(rectOriginX, rectOriginY, rectWidth, rectHeight);
		drawZoomRect(g, rectOriginX, rectOriginY, rectWidth, rectHeight);
		g.dispose();
		
	    } // mouseDragged
	    
	    // Draws with zoom rect line thickness of 2
	    
	    private void drawZoomRect(Graphics g, int x, int y, int w, int h) {
		g.drawRect(x, y, w, h);
		g.drawRect(x + 1, y + 1, w - 2, h - 2);
	    }
	    
	    public void mouseEntered(MouseEvent e) {
	    }
	    
	    public void mouseExited(MouseEvent e)	{
	    }
	    
	    public void mouseMoved(MouseEvent e) {
	    }
	    
	    public void mousePressed(MouseEvent e) {
		// Set up our zoom struct with initial click origin,
		// just in case we're trying to zoom
		if(_zoomingRectangleEnabled) {
		    _zooming = new
			Zooming(new Point(e.getX(), e.getY()), e.getButton());
		}
	    }
	    
	    public void mouseReleased(MouseEvent e) {
		
		if(_zooming == null) {
		    return;
		}
		
		int x = e.getPoint().x;
		int y = e.getPoint().y;
		
		// Get the last point into mouseDrag
		mouseDragged(e);
		
		// if the mouse didn't move much, assume we don't want to zoom
		if(((int) _zooming.rect.getWidth()) < 10) {
		    // clear the zooming rectangle outline if this is not the 1st
		    Graphics g = getGraphics();
		    if(g != null) {
			try {
			    g.setColor(Color.red);
			    g.setXORMode(Color.white);
			    drawZoomRect(g,
					 (int) _zooming.rect.getX(),
					 (int) _zooming.rect.getY(),
					 (int) _zooming.rect.getWidth(),
					 (int) _zooming.rect.getHeight());
			} finally {
			    g.dispose();
			}
		    }
		} else {

		    double zoomX1 = _zooming.rect.getX();
		    double zoomY1 = _zooming.rect.getY() + _zooming.rect.getHeight();
		    double zoomX2 = _zooming.rect.getX() + _zooming.rect.getWidth();
		    double zoomY2 = _zooming.rect.getY();

		    Zoomed zoomed = new Zoomed();
		    zoomed.minGate = (int) _countPlot.getXWorld(zoomX1);
		    zoomed.maxGate = (int) (_countPlot.getXWorld(zoomX2) + 1);
		    if (_plotCount) {
			zoomed.minCount = _countPlot.getYWorld(zoomY1);
			zoomed.maxCount = _countPlot.getYWorld(zoomY2);
		    } else {
			if (_zoomed != null) {
			    zoomed.minCount = _zoomed.minCount;
			    zoomed.maxCount = _zoomed.maxCount;
			} else {
			    zoomed.minCount = _unZoomed.minCount;
			    zoomed.maxCount = _unZoomed.maxCount;
			}
		    }
		    if (_plotDbz) {
			zoomed.minDbz = _dbzPlot.getYWorld(zoomY1);
			zoomed.maxDbz = _dbzPlot.getYWorld(zoomY2);
		    } else {
			if (_zoomed != null) {
			    zoomed.minDbz = _zoomed.minDbz;
			    zoomed.maxDbz = _zoomed.maxDbz;
			} else {
			    zoomed.minDbz = _unZoomed.minDbz;
			    zoomed.maxDbz = _unZoomed.maxDbz;
			}
		    }
		    
		    if (zoomed.minGate < _unZoomed.minGate) {
			zoomed.minGate = _unZoomed.minGate;
		    }
		    if (zoomed.maxGate > _unZoomed.maxGate) {
			zoomed.maxGate = _unZoomed.maxGate;
		    }
		    if (zoomed.minCount < _unZoomed.minCount) {
			zoomed.minCount = _unZoomed.minCount;
		    }
		    if (zoomed.maxCount > _unZoomed.maxCount) {
			zoomed.maxCount = _unZoomed.maxCount;
		    }
		    if (zoomed.minDbz < _unZoomed.minDbz) {
			zoomed.minDbz = _unZoomed.minDbz;
		    }
		    if (zoomed.maxDbz > _unZoomed.maxDbz) {
			zoomed.maxDbz = _unZoomed.maxDbz;
		    }
		    zoomed.minRange = _startRange + _gateSpacing * (zoomed.minGate - 0.5);
		    zoomed.maxRange = _startRange + _gateSpacing * (zoomed.maxGate + 0.5);

		    _zoomed = zoomed;
		    _unZoomButton.setEnabled(true);

		}
		
		_zooming = null;
		
	    } // mouseReleased
	    
	} // ZoomMouseListener

    } // Canvas

    private class CountToggleListener implements ItemListener {
	public void itemStateChanged(ItemEvent e) {
	    if (e.getStateChange() == ItemEvent.SELECTED) {
		_plotCount = true;
	    } else {
		_plotCount = false;
	    }
	    _params.aScope.plotCount.setValue(_plotCount);
	}
    }

    private class DbzToggleListener implements ItemListener {
	public void itemStateChanged(ItemEvent e) {
	    if (e.getStateChange() == ItemEvent.SELECTED) {
		_plotDbz = true;
	    } else {
		_plotDbz = false;
	    }
	    _params.aScope.plotDbz.setValue(_plotDbz);
	}
    }

    private class SampleToggleListener implements ItemListener {
	public void itemStateChanged(ItemEvent e) {
	    if (e.getStateChange() == ItemEvent.SELECTED) {
		_plotSample = true;
	    } else {
		_plotSample = false;
	    }
	    _params.aScope.plotSample.setValue(_plotSample);
	    _sampleSlider.setEnabled(_plotSample);
	    _sampleSpinner.setEnabled(_plotSample);
	}
    }

    private class SampleSpinnerListener implements ChangeListener {
	public void stateChanged(ChangeEvent e) {
	    _sampleNGates = _sampleSpinnerModel.getNumber().intValue();
	    _params.aScope.sampleNGates.setValue(_sampleNGates);
	}
    }

    private class SampleSliderListener implements ChangeListener {
	public void stateChanged(ChangeEvent e) {
	    _sampleCenter = _sampleSlider.getValue();
	    _params.aScope.sampleCenter.setValue(_sampleCenter);
	}
    }

    // window move/resize listener

    private class MoveResizeListener extends ComponentAdapter
    {
	private Parameters _params;
	public MoveResizeListener(Parameters params) {
	    super();
	    _params = params;
	}
	
	public void componentMoved(ComponentEvent e) {
	    int paramX = _params.aScope.xx.getValue();
	    int paramY = _params.aScope.yy.getValue();
	    if (paramX != getX() || paramY != getY()) {
		_params.aScope.xx.setValue(getX());
		_params.aScope.yy.setValue(getY());
	    }
	}
	
	public void componentResized(ComponentEvent e) {
	    int paramWidth = _params.aScope.width.getValue();
	    int paramHeight = _params.aScope.height.getValue();
	    if (paramWidth != getWidth() || paramHeight != getHeight()) {
		_params.aScope.width.setValue(getWidth());
		_params.aScope.height.setValue(getHeight());
	    }
	}
	
    }

}
