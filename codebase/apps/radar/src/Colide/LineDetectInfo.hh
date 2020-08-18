/**
 * @file LineDetectInfo.hh 
 * @brief 
 * @class LineDetectInfo
 * @brief 
 */

#ifndef LINE_DETECT_INFO_H
#define LINE_DETECT_INFO_H

class Grid2d;
class LineDetect;
class LineDetectOffsets;
class LineDetection;

//------------------------------------------------------------------
class LineDetectInfo
{
public:
  /**
   * Constructor
   *
   */
  inline LineDetectInfo(const LineDetection *alg, int y, int nx, int ny,
			double missing, const Grid2d *inData,
			const Grid2d *dirData,
			const LineDetect *ld,
			const LineDetectOffsets *off,
			//double dir_missing,
			Grid2d *outData) :
    _alg(alg), _y(y), _nx(nx), _ny(ny), _missing(missing), _inData(inData),
    _dirData(dirData),
    _ld(ld), _off(off), //_dir_missing(dir_missing),
    _outData(outData) {}
  
  /**
   * Destructor
   */
  inline virtual ~LineDetectInfo(void) {}

  const LineDetection *_alg;
  int _y;
  int _nx;
  int _ny;
  double _missing;
  const Grid2d *_inData;
  const Grid2d *_dirData;
  const LineDetect *_ld;
  const LineDetectOffsets *_off;
  // double _dir_missing;
  Grid2d *_outData;
  // Grid2d *_outDir;


protected:
private:

};

#endif
