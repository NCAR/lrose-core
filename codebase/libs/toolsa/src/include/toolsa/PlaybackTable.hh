// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// © University Corporation for Atmospheric Research (UCAR) 2009-2010. 
// All rights reserved.  The Government's right to use this data and/or 
// software (the "Work") is restricted, per the terms of Cooperative 
// Agreement (ATM (AGS)-0753581 10/1/08) between UCAR and the National 
// Science Foundation, to a "nonexclusive, nontransferable, irrevocable, 
// royalty-free license to exercise or have exercised for or on behalf of 
// the U.S. throughout the world all the exclusive rights provided by 
// copyrights.  Such license, however, does not include the right to sell 
// copies or phonorecords of the copyrighted works to the public."   The 
// Work is provided "AS IS" and without warranty of any kind.  UCAR 
// EXPRESSLY DISCLAIMS ALL OTHER WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
// ANY IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
// PURPOSE.  
//  
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
#include <toolsa/copyright.h>
/**
 * @file PlaybackTable.hh 
 * @brief List of files and the time written, and additional information,
 *        used for playback of MDV and netCDF data that simulates real time
 * @class PlaybackTable
 * @brief List of files and the time written, and additional information,
 *        used for playback of MDV and netCDF data that simulates real time
 */

#ifndef PLAYBACK_TABLE_H
#define PLAYBACK_TABLE_H

#include <vector>
#include <string>

//----------------------------------------------------------------
class PlaybackTable
{
public:
  /**
   * Empty constructor (starts with an empty state)
   */
  PlaybackTable(void);

  /**
   * Constructor that reads in an existing playback table
   *
   * @param[in] fileName  The path of the file to read
   */
  PlaybackTable(const std::string &fileName);

  /**
   * Destructor
   */
  virtual ~PlaybackTable(void);

  /**
   * @return true if object is good
   */
  inline bool ok(void) const {return _ok;}
  
  /**
   * @return number of lines in the table
   */
  inline int num(void) const {return (int)_files.size();}

  /**
   * Print in readable ascii
   */
  void print(void) const;
  
  /**
   * Merge two objects and order the results in increasing time written order
   *
   * @param[in] f  Object to merge into local object
   */
  void merge(const PlaybackTable &f);

  /**
   * Append a line to the table, observations data
   *
   * @param[in] fileName  Name of data file (full path)
   * @param[in] twritten  Time data file was written to disk
   * @param[in] dir       directory where latest data info files go for
   *                      the data (top data directory, with yyyymmdd subdir)
   * @param[in] genTime   data time
   */
  void append(const std::string &fileName, const time_t &twritten,
	      const std::string &dir, const time_t &gentime);
  
  /**
   * Append a line to the table, forecast data
   *
   * @param[in] fileName  Name of data file (full path)
   * @param[in] twritten  Time data file was written to disk
   * @param[in] dir       directory where latest data info files go for
   *                      the data (top data directory, with yyyymmdd subdir)
   * @param[in] genTime   forecast gen time
   * @param[in] leadSeconds  Lead time seconds
   */
  void append(const std::string &fileName, const time_t &twritten,
	      const std::string &dir, const time_t &genTime,
	      int leadSeconds);
  
  /**
   * Order the contents oldest to newest time written
   */
  void sortFiles(void);
  
  /**
   * Write contents out to a file
   *
   * @param[in] outputFname  Name of file to create
   */
  void write(const std::string &outputFname);


  /**
   * Return information for the i'th line in the state
   *
   * @param[in] i  Index to lines
   * @param[out] file  Full path of the file
   * @param[out] isObs  True for obs data, false for forecast data
   * @param[out] dir    Top directory for the file, where latest data info
   *                    is written
   * @param[out] genTime  Time of the data 
   * @param[out] leadTime Lead time seconds, when isObs is false 
   * @param[out] secondsSinceStart  Number of seconds between the time
   *                                this data was written, and the 0th
   *                                data in the state was written
   */
  void read(int i, std::string &file, bool &isObs, std::string &dir,
	    time_t &genTime, int &leadTime, int &secondsSinceStart) const;

protected:
private:

  /**
   * @class Line
   * @brief One line in the state, that corresponds to one data file
   */
  class Line
  {
  public:
    /**
     * Constructor parses a line from a file
     * @param[in] fileLine  The line from a file to parse
     */
    Line(const std::string &fileLine);

    /**
     * Constructor from input fields, obs data
     * @param[in] fileName  Name of data file (full path)
     * @param[in] twritten  Time data file was written to disk
     * @param[in] dir       directory where latest data info files go for
     *                      the data (top data directory, with yyyymmdd subdir)
     * @param[in] genTime   forecast gen time
     */
    Line(const std::string &fileName, const time_t &twritten,
	 const std::string &dir, const time_t &genTime);

    /**
     * Constructor from input fields, forecast data
     * @param[in] fileName  Name of data file (full path)
     * @param[in] twritten  Time data file was written to disk
     * @param[in] dir       directory where latest data info files go for
     *                      the data (top data directory, with yyyymmdd subdir)
     * @param[in] genTime   forecast gen time
     * @param[in] leadSeconds  Lead time seconds
     */
    Line(const std::string &fileName, const time_t &twritten,
	 const std::string &dir, const time_t &genTime,
	 int leadSeconds);

    /**
     * Destructor
     */
    ~Line(void);

    /**
     * @return true if object is good
     */
    inline bool ok(void) const {return _ok;}

    /**
     * @return _timeWritten
     */
    inline time_t getTwritten(void) const {return _timeWritten;}

    /**
     * Print in readable format
     */
    void print(void) const;

    /**
     * Operator < 
     *
     * @param[in] f  Object to compare to
     *
     * @return true if local object is < f
     */
    bool operator<(const Line &f) const;

    /**
     * @return XML representation of state
     */
    std::string xml(void) const;
    
    /**
     * Return information for the line
     *
     * @param[out] file  Full path of the file
     * @param[out] isObs  True for obs data, false for forecast data
     * @param[out] dir    Top directory for the file, where latest data info
     *                    is written
     * @param[out] genTime  Time of the data 
     * @param[out] leadTime Lead time seconds, when isObs is false 
     */
    void read(std::string &file, bool &isObs, std::string &dir,
	      time_t &genTime, int &leadTime) const;
    
  protected:
  private:

    bool _ok;                /**< Status of object */
    std::string _fileName;   /**< Path of file */
    time_t _timeWritten;     /**< Time file was written to disk */
    bool _isObs;             /**< True if data is obs, false if forecast */
    std::string _dir;        /**< Top dir where ldata info goes */
    time_t _genTime;         /**< Gen time (forecast) or data time (obs) */
    int _leadtime;           /**< Lead time seconds (forecast) */

    static const std::string _fileTag;
    static const std::string _twrittenTag;
    static const std::string _isObsTag;
    static const std::string _dirTag;
    static const std::string _gentimeTag;
    static const std::string _leadsecondsTag;
    
  };
  
  bool _ok;                 /**< Status of object */
  std::vector<Line> _files; /**< The individual files */

  static const std::string _lineTag;  /**< tag in XML for each line */

  std::string _xml(void) const;

};

#endif
