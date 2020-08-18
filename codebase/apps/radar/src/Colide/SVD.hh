/**
 * @file SVD.hh
 * @brief SVD algorithm
 * @class SVD
 * @brief SVD algorithm
 */
# ifndef    SVD_H
# define    SVD_H

#include <euclid/PointList.hh>

class Line;
class Grid2d;
class Box;

class SVD : public PointList
{
public:
  SVD(const PointList &r);
  virtual ~SVD();
  bool svd(double &slope, double &intercept) const;

  bool lsq_min(const Grid2d &g, Line &l) const;

  bool lsq_min(const Grid2d &mask, const Grid2d &seed, const Grid2d &data,
	       Line &l) const;
  bool lsq_min(const Grid2d &data, const Grid2d &lineData, Line &l) const;
  bool lsq_min(Line &l) const;

private:

  bool _lsq_min_init(int len, const Grid2d &data, double &mux, double &muy,
		     Box &b) const;
  bool _lsq_min_init(int len, const Grid2d &mask, const Grid2d &seed,
		     const Grid2d &data,
   		     double &mux, double &muy, Box &b) const;
  bool _lsq_min_init(int len, const Grid2d &data, const Grid2d &lineData,
		     double &mux, double &muy, Box &b) const;
  bool _lsq_min_init(int len, double &mux, double &muy, Box &b) const;

  bool _lsq_min_second_mom(int len, const Grid2d &data,
			   double mux, double muy, double &sxx,
			   double &sxy, double &syy) const;
  bool _lsq_min_second_mom(int len, const Grid2d &mask, const Grid2d &seed,
			   const Grid2d &data, double mux, double muy,
			   double &sxx,  double &sxy, double &syy) const;
  bool _lsq_min_second_mom(int len, const Grid2d &data, const Grid2d &lineData,
			   double mux, double muy, double &sxx,
			   double &sxy, double &syy) const;
  bool _lsq_min_second_mom(int len, double mux, double muy,
			   double &sxx, double &sxy, double &syy) const;

  bool _lsq_compute(double sxx, double sxy, double syy,
		    double mux, double muy, Box &b, Line &l) const;

};

# endif     /* CLDSVD_H */
