// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
#include <toolsa/copyright.h>

# ifndef    LINELIST_H
# define    LINELIST_H

#include <euclid/Line.hh>
#include <euclid/AttributesEuclid.hh>
#include <vector>

class Grid2d;
class FuzzyF;

/**
 * @file LineList.hh
 * @brief multiple lines, vector of Line objects
 * @class LineList
 * @brief multiple lines, vector of Line objects
 */
class LineList : public AttributesEuclid
{

public:

  /**
   * empty list
   */
  LineList(void);

  /**
   * Destructor
   */
  virtual ~LineList(void);

  /**
   * Constructor that breaks an input line into a list of lines, each
   * no longer than a maximum length, all of equal length
   * @param[in] line  The line to break up
   * @param[in] max_length
   *
   * @note Any attributes in line are lost
   */
  LineList(const Line &line, double max_length);


  /**
   * copy constructor
   * @param[in] l
   */
  LineList(const LineList &l);

  /**
   * operator= method
   * @param[in] l
   */
  LineList & operator=(const LineList &l);

  /**
   * operator== method
   * @param[in] l
   */
  bool operator==(const LineList &l) const;

  /**
   * Create an XML string that represents the entire line list 
   * including attributes
   * @param[in] tag  Tag for entire linelist
   */
  std::string writeXml(const std::string &tag) const;

  /**
   * Parse an XML string to set values for the entire line list 
   * including attributes
   * @param[in] xml  The data
   * @param[in] tag  Tag for entire linelist
   * @return true for success
   */
  bool readXml(const std::string &xml, const std::string &tag);

  /**
   * @return number of lines
   */
  inline int num(void) const {return (int)_line.size();}

  /**
   * @return i'th line copy
   * @param[in] i
   */
  inline Line ithLine(int i) const
  {
    return _line[i];
  }

  /**
   * @return pointer to i'th line
   */
  inline Line *ithLinePtr(int i)
  {
    return &_line[i];
  }
  /**
   * @return pointer to i'th line
   * @param[in] i
   */
  inline const Line *ithConstLinePtr(int i) const
  {
    return &_line[i];
  }

  /**
   * Clear so no lines
   */
  inline void clear(void) { _line.clear();}

  /**
   * Append a line to the liset
   * @param[in] l
   */
  inline void append(const Line &l) {_line.push_back(l);}

  /**
   * Take union of local linelist and lines from input linelist
   * @param[in] l
   *
   * @note checks to prevent redundancy
   */
  void formUnion(const LineList &l);

  /**
   * compute distance from a point to the LineList, which is perpendicular
   * distance to any one of its line segements. 
   * @param[in] x0  Point
   * @param[in] y0  Point
   * @param[out] dist  returned distance (squared)
   * @param[out] index  Index to the line segement that was used
   * @return true if successful
   */
  bool closestDistanceSquared(const double x0, const double y0,
			      double &dist, int &index) const;

  /**
   * at each point set orientation value to average of the orientation
   * of the lines that are within a radius of a point, weighted by an
   * attribute found in each such line
   *
   * @param[in] x  Point to look near for lines
   * @param[in] y  Point to look near for lines
   * @param[in] attribute_name Name of weight attribute in lines
   * @param[in] radius how far from x,y to look for lines
   * @param[in] percentile0  smallest weight value in lines to use it in  ave.
   * @param[in] percentile1  largest weight value in lines to use it in ave.
   * @param[out] orientation  Average orientation
   * @param[out] conf    Confidence 
   */
  bool dataWeightedOrientation(int x, int y, 
			       const std::string &attribute_name,
			       int radius,
			       double percentile0,
			       double percentile1,
			       double &orientation,
			       double &conf) const;

  /**
   * Add an attribute to each line with name
   * @param[in] name  Attribute name
   * @param[in] v  Value
   */
  void addIndividualAttributeDouble(const std::string &name, double v);

  /**
   * Remove the attribute from each line with name
   * @param[in] name  Attribute name
   */
  void removeIndividualAttributeDouble(const std::string &name);

  /**
   * Add an attribute to each line with name
   * @param[in] name  Attribute name
   * @param[in] v  Value
   */
  void addIndividualAttributeInt(const std::string &name, int v);

  /**
   * Remove the attribute from each line with name
   * @param[in] name  Attribute name
   */
  void removeIndividualAttributeInt(const std::string &name);

  /**
   * Add an attribute to each line for special time 
   * @param[in] t  Time
   */
  void addIndividualAttributeTime(const time_t &t);

  /**
   * Add an attribute to each line for motion
   * @param[in] mv  Motion
   */
  void addIndividualAttributeMotionVector(const MotionVector &mv);

  /**
   * Debug print
   */
  void print(void) const;

  /**
   * Debug print
   * @param[in] fp
   */
  void print(FILE *fp) const;

  /**
   * print endpoints of the list (first and last) and number of points
   * @return string with this information, preceded by a name
   * @param[in] name
   */
  std::string sprintEndsAndNum(const std::string &name) const;

  /**
   * print endpoints of the list (first and last) and number of points
   * @return string with this information, preceded by a name[index]
   * @param[in] name
   * @param[in] index
   */
  std::string sprintEndsAndNum(const std::string &name, int index) const;

  /**
   * Connect one linelist to another and return the linelist that has both.
   *
   * Take contents of local linelist and connect in l. with an expected
   * common point between the lists, with the common point being any of
   * the 'extreme' endpoints.  If it makes sense to do so, the input linelist
   * is reversed prior to doing the connecting, so the endpoints match up 
   * properly.
   *
   * @param[in] l
   */
  LineList connectOpposing(const LineList &l) const;

  /**
   * Append all the lines from input linelist to local linelist
   * @note does not check for redundancy
   *
   * @param[in] l
   */
  void appendToList(const LineList &l);

  /**
   * reverse the list. last line becomes first, and endpoints of each line
   * are reversed
   */
  void reverseOrder(void);

  /**
   * Clear image points that are 'inside' the linelist.
   * (ahead of 0th endpoint of 0th line and behind 1th endpoint of last line.)
   *
   * @param[in,out] f  Grid from which to remove data
   */
  void clearBetween(Grid2d &f) const;

  /**
   * @return the cumulative length of the list.
   * 
   * Always gives the sum of the lengths of the lines.
   */
  double length(void) const;

  /**
   * @return the cumulative length of the list.
   *
   * If it is an unconnected list, it is the summed distances between center
   * points. If it is a connected list, it is the sum of the lengths of the
   * lines.
   */
  double cumulativeLength(void) const;

  /**
   * @return length of the subset of lines from 0 to i0-1 and i1+1 to the end
   *
   * @param[in] i0  Index
   * @param[in] i1  Index
   */
  double nonsubsetLength(int i0, int i1) const;

  /**
   * starting at one end, remove full line segements and a portion of
   * a final line segment so that the cumulative_length() equals a value
   *
   * @param[in] which  0 for 0th end, 1 for 1th end
   * @param[in] len
   */
  void removeFromEnd(int which, double len);

  /**
   * Remove a particular line from a linelist.
   * @param[in] a  Line to remove
   * @return true if found and removed
   */
  bool removeElement(const Line &a);

  /**
   * Remove a particular line from a linelist.
   * @param[in] i  index to Line to remove
   * @return true if in range and removed
   */
  bool removeElement(int i);

  /**
   * Remove range of elements.
   * @param[in] i0  Lower index to remove
   * @param[in] i1  upper index to remove
   * @return true if in range and removed
   */
  bool removeElements(int i0, int i1);

  /**
   * Add motion attribute to linelist using input images.
   *
   * Takes average of motion around each line segment to set the motion
   * for that line segment.
   *
   * @param[in] motion_angle  Grid with motion angles in it
   * @param[in] motion_magnitude  Grid with motion magnitudes in it
   * @param[in] debug  True for more debugging
   */
  void addMotion(const Grid2d &motion_angle, const Grid2d &motion_magnitude,
		 bool debug=false);

  /**
   * increase number of lines 
   *
   * interpolate so that maximum line length is no longer than input length
   * @param[in] max_length  The maximum length for  a line
   */
  void spacingFilter(double max_length);


  /**
   * append lines from l, indices i0 to i1, to local linelist.
   *
   * if reverse is true, each line is reversed prior to appending.
   *
   * @param[in] l  Linelist to pull lines out of
   * @param[in] i0  initial index
   * @param[in] i1  final index
   * @param[in] reverse  true to reverse endpoints
   */
  void appendLines(const LineList &l, int i0,  int i1, bool reverse);

  /**
   * @return true if the 1'th endpoint of each line is the 0th end point
   * of the next line for all lines
   */
  bool isConnected(void) const;

  /**
   * Force the linelist to be connected by adding segments as needed
   *
   * Do not add any Attributes to the additional segments
   */
  void makeConnected(void);

  /**
   * Force the linelist to be connected by adding segments as needed,
   * then reconnect replacing any short segments by moving the
   * endpoint of the two adjacent lines to the average of the two
   *
   * Do not add any Attributes to the additional segments
   *
   * @param[in] small   Remove all lines whose length <= small
   */
  void makeConnectedAndRemoveSmall(const double small);

  /**
   * return center (based on lengths) of the entire linelist.
   *
   * @param[out] x  Center point
   * @param[out] y  Center point
   *
   * @return true for success
   */
  bool centerLocation(double &x, double &y) const;

  /**
   * return average distance between lines and the input
   * @param[in] l  List to compare to
   * @param[out] sep  Average separation
   *
   * @return true for success
   */
  bool averageSeparation(const LineList &l, double &sep) const;

  /**
   * return average speed of each line in the list, weighted by lengths
   */
  double lengthWeightedSpeed(void) const;

  /**
   * @return true if local list belongs to same boundary as the input list.
   *
   * it 'belongs' in the sense that it is 'next' to the input list and
   * oriented similarly.
   *
   * @param[in] l1  List to compare to
   * @param[in] maxdist  Definition of 'next to'
   * @param[in] angle_maxdist  a fuzzy function from angle diff between
   *                           the lists to max allowed distance between
   *                           the lists.
   *
   * @note The local and input lists need to be connected linelists
   */
  bool proximateBoundaries(const LineList &l1,
			    double maxdist,
			    const FuzzyF &angle_maxdist) const;

  /**
   * Rotate the entire line list by an angle
   * @param[in] angle
   * @param[in] change_endpts  If true, change endpoints of each line as needed
   *                           so that _x0 <= _x1 after rotation
   */
  void rotate(double angle, bool change_endpts);

  /**
   * @return enclosing box
   */
  Box extrema(void) const;

  /**
   * return average orientation of line list (length weighted).
   *
   * @param[out]  a  Average
   *
   * @return true for success
   */
  bool averageOrientation(double &a) const;

  /**
   * return average orientation
   *
   * return average orientation of p percentage length of line list
   * from which'th end (0 or 1), out of the line backwards, in the
   * hopes of setting up an extension to the linelist at that end.
   *
   * @param[in] p Percentage
   * @param[in] which  0 or 1
   * @param[out]  a  Average
   *
   * @return true for success
   */
  bool averageOrientation(double p, int which, double &a) const;


  /**
   * @return max speed from all lines if can, else 0.0
   */
  double maxSpeed(void) const;

  /**
   * return average quality from lines
   *
   * @param[out] q  Returned value
   *
   * @return true for success
   */
  bool getAveQuality(double &q) const;

  /**
   * return maximum quality from lines
   *
   * @param[out] q  Returned value
   *
   * @return true for success
   */
  bool getMaxQuality(double &q) const;

  /**
   * @return true if vector from x,y at deg (out forever) intersects linelist.
   *
   * @param[in] x  Start point of vector
   * @param[in] y  Start point of vector
   * @param[in] deg  Degrees orientation of vector
   * @param[out] xp  Intersection point when return is true
   * @param[out] yp  Intersection point when return is true
   */
  bool vectorIntersects(double x, double y, double deg, double &xp,
			double &yp) const;

  /**
   * Extend linelist.
   *
   * extend at which'th end a length plen percent of total length,
   * in the average direction indicated by the pdir percent length lines
   * at that end.
   *
   * @param[in] which 0 or 1
   * @param[in] plen  Percent to extend
   * @param[in] pdir  Percent of lines to use to get average direction
   */
  void extend(int which, double plen, double pdir);

  /**
   * @return true if motion vectors are all 0.0 or missing.
   */
  bool velocitiesAllZero(void) const;

  /**
   * make motions perpendicular to lines.
   *
   * adjust motion direction to closest to what it was perpendicular
   * to each line.
   */
  void adjustMotionDirections(void);

  /**
   * reconnect the endpoints.
   *
   * on entry it is assumed the 1th endpoint of line i should connect
   *  to the 0th endpoint of line i+1 for all i
   */
  void reconnectAsNeeded(void);
    
  /**
   * Add data attributes to all lines
   * @param[in] data  Data grid to use
   */
  void adjustForData(const Grid2d &data);

  /**
   * Move the line list using the motion vectors, for given time interval
   * @param[in] seconds  Number of seconds 
   */
  void extrapolate(double seconds);

  /**
   * adjust magnitudes of motion vectors.
   *
   * average of magnitudes of motion vectors near each line are used to
   * create new magnitides in original directions..
   *
   * @param[in] speedSmoothingLen Length along the linelist over which to
   *                                average
   */
  void averageSpeeds(double speedSmoothingLen);
    
  /**
   * Set grid to a value where the linelist is
   * @param[in,out] image  Where to write
   * @param[in] value
   */
  void toGrid(Grid2d &image, double value) const;

  /**
   * remove small lines from list
   *
   * @param[in] reconnect  True to reconnect previous and next lines after
   *                       removals by replacing with an average center point.
   */
  void removeSmall(bool reconnect);

  /**
   * remove lines smaller than a particular value from list
   *
   * @param[in] small   Any line with length <= small is removed
   * @param[in] reconnect  True to reconnect previous and next lines after
   *                       removals by replacing with an average center point.
   */
  void removeSmall(double small, bool reconnect);

  /**
   * Return maximum max data value, and index of object that had it.
   * This comes from Attribute information
   *
   * @param[out] index  Index of line with max data
   * @param[out] max  The max value
   *
   * @return true if values set
   *
   */
  bool getMaxMaxDataValue(int &index, double &max) const;

  /**
   * Rreturn max of averages for subset of elements.
   *
   * return maximum average data value for objects that have 
   * max data value equal to input.  return index of object that had it.
   *
   * This comes from Attribute information
   *
   * @param[in] maxv  
   *
   * @param[out] index  Index of line with max average data
   * @param[out] ave  The max average value
   *
   * @return true if values set
   */
  bool maxAveAtMaxDataValue(double maxv, int &index, double &ave) const;

  /**
   * order the lines in the list, reversing endpoints as needed.
   *
   * On exit the line list has endpoints reversed, or its empty.
   */
  void order(void);

  /**
   *  return index to line with minimum x value.
   *
   * (unless its a linelist that tends to be oriented more vertically than
   * horizontally, in which case go with the line with
   * minimum y value).
   */
  int minimumIndex(void) const;

  /**
   * return index to line with maximum x value.
   *
   * (unless its a linelist that tends to be oriented more vertically than
   * horizontally, in which case go with the line with
   * maximum y value).
   */
  int maximumIndex(void) const;

  /**
   * @return index to list element with minimum x or y value.
   * @param[in] isY  true to get minimum y, false to get minimum x
   */
  int indexToMinimum(bool isY) const;

  /**
   * @return index to list element with maximum x or y value.
   * @param[in] isY  true to get maximum y, false to get maximum x
   */
  int indexToMaximum(bool isY) const;

  /**
   * reverse 'handedness' of each line's motion vector
   */
  void reverseHandedness(void);
  
  /**
   * @eturn index to local list line closest to input line, -1 for none
   * @param[in] line
   */
  int indexClosestToLine(const Line &line) const;

  /**
   * remove redundant list objects, first averaging attributes of multiples.
   */
  void removeRedundancies(void);

  /**
   * Return average speed, quality and direction using attributes for
   * quality and motion
   * @param[in] isFast  True to use a faster algorithm
   * @param[out] speed   Average speed
   * @param[out] quality Average quality
   * @param[ou] dx  Average direction x
   * @param[ou] dy  Average direction y
   *
   * dx*dx + dy*dy = 1
   *
   * @return true if averages were gotten
   */
  bool averageSpeedQualityDir(bool isFast,  double &speed,
				 double &quality, double &dx, double &dy) const;

  /**
   * Remove any line from the local list that does not have a MotionVector
   */
  void nonMissingVel(void);

  /**
   * Remove any line from the local list that does have a MotionVector
   */
  void missingVel(void);
  
  /**
   * Set the MotionVector for each line to the MotionVector gotten using
   * inputs.
   * @param[in] vx  Motion vector x component
   * @param[in] vy  Motion vector y component
   */
  void setIndividualVel(double vx, double vy);

  /**
   * Set the quality for each line to the input
   *
   * @param[in] quality
   */
  void setIndividualQuality(const double quality);

  /**
   * Get the weighted mean and variance values for motion of list elements,
   * using quality as the weight
   *
   * @param[out] vx  mean motion x component
   * @param[out] xvar  Variance of x component of motion
   * @param[out] vy  mean motion y component
   * @param[out] yvar  Variance of y component of motion
   * @param[out] speed mean speed
   * @param[out] speedVar variance of speed
   * @param[in] minq  Ignore lines whose quality is less than minq
   *
   * @return true if values were computed
   */
  bool meanVariance(double &vx, double &xvar, double &vy, double &yvar,
		     double &speed, double &speedVar, double minq) const;

  /**
   * Compute the weighted average of MotionVector for the lines, using
   * quality as the weight
   * @param[out] mv  The average motion vector
   * @return true if successful
   */
  bool qualWeightedMotion(MotionVector &mv) const;

  /**
   * Compute the average speed amongst all lines that are close to an input
   * line.  
   * @param[in] o  The line to be close to.
   * @param[in] maxdist  The maximum distance from o to a line to be 'close'
   * @param[out] s  The speed
   * @return true if successful
   */
  bool bestLocalSpeed(const Line &o, double maxdist, double &s) const;

  /**
   * @return true if input object has a line in common with local object
   * sans attributes
   *
   * @param[in] m  List to compare against
   */
  bool hasCommonLines(const LineList &m) const;

  /**
   * return average length weighted angle diff between motion vectors of
   * lines that are common between the input and the local Lines
   * (if any).  (local - input).
   * If no lines are common, returns overall motion angle difference.
   *
   * @param[in] m
   * @param[out] ave
   *
   * @return true if able to do so
   */
  bool averageLocalAngleDiff(const LineList &m, double &ave) const;

  /**
   * @return true if the length weighted average local angle diff
   * (see above) is bad based on inputs.
   * 
   * @param[in] m
   * @param[in] maxDiff  If the local angle diff exceeds maxDiff, it is 'bad'
   */
  bool averageLocalAngleDiffIsBad(const LineList &m,
				       double maxDiff) const;
  
  /**
   * return average length weighted speed difference (local - input) 
   * between motions of lines that are common between input and local
   * If any. If none, return diff of motion of overall. 
   *
   * @param[in] mj
   * @param[out] diff
   *
   * @return true if successful
   */
  bool averageLocalSpeedDiff(const LineList &mj, double &diff) const;

private:
    
  /**
   * Vector of Line objects
   */
  std::vector<Line> _line;

  void _init(void);
  void _removeUp(double len);
  void _removeDown(double len);
  std::vector<std::pair<double, double> > 
  _createDataweightVector(int x, int y, const std::string &att_name,
			  int radius) const;
  LineList _connectOpposingNonTrivial(const LineList &l) const;
  bool _ends(double &x0, double &y0, double &x1, double &y1) const;

  /*
   * Copy removing zero length, 1 step
   */
  void _removeSmall1(int i0, int i1, const Line &t0, const Line &t1);
  void _reconnectEndpoints(void);

  /*
   * Return the higher index line that may or may not be reversed
   */
  bool _reversePair(int high_index,
		     Line &l0, /* lower index line copy , passed in*/
		     bool use_zero, double thresh,  LineList &lnew,
		     Line &l1) const;
  bool _isOrientedVertical(void) const;

  // take average over all elements that match (non-attribte) o in the input
  // list as regards attributes, return the average of those.
  AttributesEuclid _averageMatchAttributes(const Line &o) const;

  bool _averageSpeedQualityDir(double &speed, double &quality, double &dx,
			       double &dy) const;

  // get the average non-missing non-zero speed and direction,
  // and the average non-missing quality
  // return 1 if computations were successful.
  bool _averageNonzeroSpeedQualityDir(double &speed, double &quality,
					  double &dx, double &dy) const;
  bool _bestLocalSpeed(const Line &o, double maxdist, double &ret) const;
};


#endif
