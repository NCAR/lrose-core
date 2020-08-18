
# ifndef    Reg_H
# define    Reg_H

#include <euclid/PointList.hh>
#include "RegAtt.hh"

class Grid2d;
class ParmRegion;

class Reg
{
 public:

    //! build  from input l=locs, other inupts as given.
    Reg(//const time_t &t, 
	const PointList &l, const Grid2d &data, const Grid2d &lineData,
	int id, const ParmRegion &parm);

    virtual ~Reg();

  void toImage(Grid2d &g) const;

#ifdef NOTDEF
    void print(void) const;
    void print(int index) const;
    void print(FILE *fp, int index) const;

    inline time_t get_time(void) const {return __time;}
    inline int get_id(void) const {return __id;}
    inline const cldPointList *get_loc(void) const
    {
      return (const cldPointList *)&__loc;
    }
    inline const cldPointList *get_iloc(void) const
    {
      return (const cldPointList *)&__iloc;
    }
    inline cldPointList *get_loc_ptr(void) {return &__loc;}
    inline void set_id(int i) {__id = i;}

    //! return true if region looks good and is hot enough.
    bool ok_check_hot(const FiltRegion &parm, bool do_print) const;

    //! return true if region looks good.
    inline bool ok_check(const FiltRegion &parm, bool do_print) const
    {
      return __att.ok(parm, do_print);
    }

    //! \brief Merge regions by adding lines and points from m to this.
    //! 
    //! NOTE this is only a partial implementation based on need
    void merge(const  cldGrid &g, const Reg &m, const FiltRegion &parm);

    //! fill holes in location points.
    void fill_holes(const int nx, const int ny, int max_fill_area);

    //! expand location points using lower threshhold.
    bool expand_loc(const cldGrid &shape_image, const cldGrid &data,
		    const FiltRegion &parm);

    void set_loc(const cldPointList &l, const cldPointList &iloc);
#endif
  
 protected:

    PointList _uloc;        //!< union of loc and iloc.
    int _id;                //!< i.d. number of the region

 private:
    RegAtt _att;        //!< attributes of the region
    //time_t _time;              //!< unix time
    PointList _loc;     //!< locations of all points
    PointList _iloc;    //!< locations of pts that intersect other reg's

  void _clear(void);
    // double _total_hot_area(const FiltRegion &parm) const;
};


# endif

