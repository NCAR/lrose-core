

ColorMap.cc/.hh uses QtGui/QBrush  
// maps number to color in color scale
// reads a color scale file.

ColorBar.cc/.hh is a widget (Q_OBJECT)
// creates a colorscale widget of a specified width
// returns either a QImage or a QPixmap of the colorscale

ColorMapTemplates.cc/.hh is a widget (Q_OBJECT)
// is a dialog with all the default color scales shown as color bars

ColorTableManager.cc/.hh 
// contains all the predefined color maps

A ColorMap has a name, and a list of colors with min and max range values for each color.

A ColorMap is displayed as a ColorBar.

The ColorMapTemplates is a dialog widget that displays all the known ColorMaps as ColorBars that are clickable labels.

The ColorTableManager keeps a dictionary of the known ColorMaps.  A ColorMap is indexed by its name in the dictionary.

X11ColorMap and x11_rgb_sorted.txt is a lookup to convert the X11 color names to RGB values.
