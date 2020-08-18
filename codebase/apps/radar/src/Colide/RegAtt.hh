# ifndef    RegAtt_hh
# define    RegAtt_hh


#include "ParmRegion.hh"
#include <euclid/PointList.hh>
#include <string>

class Grid2d;
class Box;

/**
 * @class RegAttMeasure
 * @brief region attributes
 *
 * Various measures of size.
 */
class RegAttMeasure
{
public:
  RegAttMeasure();
  RegAttMeasure(double, double, double);
  RegAttMeasure(const PointList &r, bool is_length);
  ~RegAttMeasure();
  void clear(void);
  inline bool min_ok(double min_value) const {return _min >= min_value;}
  inline bool max_ok(double min_value) const {return _max >= min_value;}
  inline bool mean_ok(double min_value) const {return _mean >= min_value;}
  inline double get_min(void) const {return _min;}
  inline double get_max(void) const {return _max;}
  inline double get_mean(void) const {return _mean;}
  void print(const std::string label, FILE *fp) const;
  inline bool is_empty(void) const
  {
    return _min == 0.0 && _max == 0.0 && _mean == 0.0;
  }
  
 private:

    double _mean;
    double _min;
    double _max;

    void _build_width(const PointList &n);
    void _build_length(const PointList &n);
    void _update_measure_values(double imin, double imax, double &nv);
};

/*---------------------------------------------------------------*/
/*! \class RegAtt
 *  \brief attributes of a region
 */
class RegAtt
{
public:
    RegAtt(const PointList &loc, const Grid2d &data, const Grid2d &lineData,
	   const ParmRegion &parm);
    ~RegAtt();
    void clear(void);
    void merge(const RegAtt &final);
    void print(void) const;
    void print(FILE *fp) const;
    bool ok(const ParmRegion &p, bool print) const;
    inline bool extrema_are_set(void) const
    {
      return (_minx.size() > 0 && _maxx.size() > 0 &&
	      _miny.size() > 0 &&_maxy.size() > 0);
    }
    Box extrema(const int nx, const int ny, double maxdist) const;
    bool is_empty(void) const;
    inline double mean_length(void) const {return _len.get_mean();}
    inline double mean_width(void) const {return _width.get_mean();}
    void recompute(const Grid2d &g, const PointList &loc);
  
private:

    double _area;            /*!< area of the region */
    RegAttMeasure _len;
    RegAttMeasure _width;
    PointList _minx;  /*!< extrema along minx line (bounding box) */
    PointList _miny;  /*!< extrema along miny line (bounding box) */
    PointList _maxx;  /*!< extrema along maxx line (bounding box) */
    PointList _maxy;  /*!< extrema along maxy line (bounding box) */

  std::pair<double,double> _min_x_at_min_y; /*!< lower left point */

    void _clear(void);
    void _neighbors_width_length(const PointList &loc, 
				 const Grid2d &image);
    void _neighbors_width_length(const PointList &loc, 
				 const Grid2d &data, const Grid2d &lineData,
				 const ParmRegion &parm);
    bool _is_ok(const ParmRegion &p) const;
    void _ok_print(const ParmRegion &p) const;
    void _set(const PointList &iloc);
    void _set(void);
};

/* External global variables / Non-static global variables / Static globals */


/* External functions / Internal global functions / Internal static functions */

# endif     /* CLDREG_ATT_H */
