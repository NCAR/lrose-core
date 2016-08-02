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
/**
 * @file DsEnsembleDataTrigger.hh
 * @brief Class that supports simple ensemble data access with some 
 *        assumption about URL naming for ensembles
 * @class DsEnsembleDataTrigger
 * @brief Class that supports simple ensemble data access with some 
 *        assumption about URL naming for ensembles
 *
 * It is assumed the ensemble data members are numbered 1,2,3,...,
 * such that URL's can be represented as  <url header>/e_<number>
 * where <number> = 01, 02, 03, ...
 *
 * This class allows one to do simple sequential triggering of all members,
 * or triggering when a lead time is completely present for all members.
 */
#ifndef DS_ENSEMBLE_DATA_TRIGGER_HH
#define DS_ENSEMBLE_DATA_TRIGGER_HH

#include <dsdata/DsEnsembleAnyTrigger.hh>

class DsEnsembleDataTrigger
{
public:
  /**
   * @enum Trigger_t
   * @brief the triggering modes
   */
  typedef enum
  {
    SIMPLE,       /**< Trigger each gen/lead/ensemble member individually */
    LEADTIME      /**< Trigger each gen/lead for all ensemble members, or
		   * for as many as possible prior to timeout */
  } Trigger_mode_t;

  /**
   * Empty constructor
   */
  DsEnsembleDataTrigger(void);

  /**
   * Constructor for real time, standard naming in which the ensemble
   * data URL is of form <urlHeader>/e_##.
   * 
   * @param[in] urlHeader  Leading portion of URL for ensemble members
   * @param[in] nmember  Number of ensemble members, assumed numbered
   *                     1,2,3,..nmember
   * @param[in] leadSeconds The configured lead times to trigger (others are
   *                        ignored)
   * @param[in] format  Format to use in converting member to a string
   * @param[in] mode    Triggering mode
   */
  DsEnsembleDataTrigger(const std::string &urlHeader, int nmember,
			std::vector<int> leadSeconds,
			const std::string &format = "%02d",
			Trigger_mode_t mode=SIMPLE);
		    
  /**
   * Constructor for real time, non-standard naming, with
   * with both leading and trailing URL substrings and a string member number
   * in between
   *
   * @param[in] urlHeader  Leading portion of URL for ensemble members
   * @param[in] urlRemainder  Tailing portion of URL for ensemble members
   * @param[in] nmember  Number of ensemble members, assumed numbered
   *                     1, 2, 3,..nmember
   * @param[in] leadSeconds The configured lead times to trigger (others are
   *                        ignored)
   * @param[in] format  Format to use in converting member to a string
   * @param[in] mode    Triggering mode
   */
  DsEnsembleDataTrigger(const std::string &urlHeader,
			const std::string &urlRemainder,
			int nmember, std::vector<int> leadSeconds,
			const std::string &format = "%02d",
			Trigger_mode_t mode=SIMPLE);


  /**
   * Constructor for archive mode, standard naming in which the ensemble
   * data URL is of form <urlHeader>/e_##.
   *
   * @param[in] t0  Earliest wanted time.
   * @param[in] t1  Latest wanted time.
   * @param[in] urlHeader  Leading portion of URL for ensemble members
   * @param[in] nmember  Number of ensemble members, assumed numbered
   *                     1, 2, 3,..nmember
   * @param[in] leadSeconds The configured lead times to trigger (others are
   *                        ignored)
   * @param[in] mode    Triggering mode
   */
  DsEnsembleDataTrigger(const time_t &t0, const time_t &t1,
			const std::string &urlHeader, 
			int nmember, std::vector<int> leadSeconds,
			const std::string &format = "%02d",
			Trigger_mode_t model=SIMPLE);

  /**
   * Constructor for archive mode, non-standard naming, with
   * with both leading and trailing URL substrings and a string member number
   * in between
   *
   * @param[in] t0  Earliest wanted time.
   * @param[in] t1  Latest wanted time.
   * @param[in] urlHeader  Leading portion of URL for ensemble members
   * @param[in] urlRemainder  Tailing portion of URL for ensemble members
   * @param[in] nmember  Number of ensemble members, assumed numbered
   *                     1, 2, 3,..nmember
   * @param[in] leadSeconds The configured lead times to trigger (others are
   *                        ignored)
   * @param[in] format  Format to use in converting member to a string
   * @param[in] mode    Triggering mode
   */
  DsEnsembleDataTrigger(const time_t &t0, const time_t &t1,
			const std::string &urlHeader,
			const std::string &urlRemainder, int nmember,
			std::vector<int> leadSeconds,
			const std::string &format = "%02d",
			Trigger_mode_t model=SIMPLE);
  /**
   * Destructor
   */
  virtual ~DsEnsembleDataTrigger(void);

  /**
   * Enable debugging.
   */
  void setDebug(void);

  /**
   * Return the next available ensemble data specification, for the
   * case of mode=SIMPLE
   *
   * @param[out] gt  Generation time
   * @param[out] lt  Lead time seconds
   * @param[out] member  Ensemble member number 1,2,..
   *
   * @return true if there is more data, false if no more or error
   */
  bool trigger(time_t &gt, int &lt, int &member);

  /**
   * Return the next available ensemble data specification, for the
   * case of mode=LEADTIME
   *
   * @param[out] gt  Generation time
   * @param[out] lt  Lead time seconds
   * @param[out] members  Ensemble member numbers that are available
   * @param[out] complete  True if the list of URLs is all of them,
   *                       false if it was a timeout, or another gen time
   *                       has started triggering prior to completion.
   *
   * @return true if values are set, false if there is no more data.
   */
  bool trigger(time_t &gt, int &lt, std::vector<int> &members,
	       bool &complete);

  /**
   * @return the full URL specification for a particular ensemble member
   *         using internal state
   * @param[in] member  Ensemble member number 1, 2, ..
   */
  std::string memberName(int member) const;

  /**
   * parse a full URL to pull out ensemble member number and return the 
   * member number, using internal state.
   *
   * @param[in] url  Full URL assumed consistent with internal state
   * @return member number 1, 2, ..   or -1 for error in parsing
   */
  int parseUrl(const std::string &url) const;

  /**
   * Set maximum age param.
   *
   * @param[in] maxSeconds
   *
   * See DsEnsembleAnyTrigger for meaning of this param
   */
  void setMaximumAgeSeconds(const int maxSeconds);

  /**
   * Set max wait param.
   *
   * @param[in] maxSeconds
   *
   * See DsEnsembleAnyTrigger for meaning of this param
   */
  void setTriggerMaxWaitSeconds(const int maxSeconds);

  /**
   * Set maximum seconds param
   * 
   * @param[in] maxSeconds
   *
   * See DsEnsembleLeadTrigger for meaning of this param. 
   *
   * @note only meaningful when mode=LEADTIME
   */
  void setMaxSecondsBeforeTimeout(const int maxSeconds);

  /**
   * Set max seconds before disable param
   *
   * @param[in] maxSeconds
   *
   * See DsEnsembleLeadTrigger for meaning of this param. 
   *
   * @note only meaningful when mode=LEADTIME
   */
  void setMaxSecondsBeforeDisable(const int maxSeconds);

  /**
   * Set a 'persistant disable' status flag. 
   *
   * @param[in] status true or false
   *
   * See DsEnsembleLeadTrigger for meaning of this param. 
   *
   * @note only meaningful when mode=LEADTIME
   */
  void setPersistantDisable(const bool status);

  /**
   * Filter out a substring from all URL's for use in debug output messages.
   *
   * @param[in] remove  Each URL has this substring removed from it when
   *                    used in debug output messages
   *
   * As an example, if a member URL = "mdvp:://localhost::PROJECT/data0"
   * and remove = "mdvp:://localhost::PROJECT/", the output messages will
   * have only "data0", not the full URL.
   */
  void filterNames(const std::string &remove);

  /**
   * @return the format string for member numbers
   */
  inline std::string getMemberFormat(void) const {return _format;}

  /**
   * @return the full URL for the input specifications, standard naming.
   *
   * @param[in] urlHeader  Leading portion of URL for ensemble members
   * @param[in] member  Member number 1, 2, ..
   * @param[in] format  Format to use in converting member to a string
   */
  static std::string memberName(const std::string &urlHeader, int member,
				const std::string &format="%02d");

  /**
   * @return the full URL for the input specifications, non standard naming 
   *
   * @param[in] urlHeader  Leading portion of URL for ensemble members
   * @param[in] urlRemainder  Tailing portion of URL for ensemble members
   * @param[in] member  Member number 1, 2, ..
   * @param[in] format  Format to use in converting member to a string
   *
   * The URL is given by urlHeader<member>urlRemainder where <member>
   * is the string representation of the member number
   */
  static std::string memberName(const std::string &urlHeader,
				const std::string &urlRemainder, int member,
				const std::string &format="%02d");

  /**
   * @return the default format string for member numbers
   */
  inline static std::string defaultFormat(void) {return "%02d";}

protected:

  /**
   * The triggering object, a pointer because it can be a derived type
   */
  DsEnsembleAnyTrigger *_trigger;

private:
  
  /**
   * The full URL names for the complete set of ensemble members 
   */
  std::vector<std::string> _urls;

  bool _standard;            /**< True for standard formatting */
  std::string _urlHeader;    /**< Leading portion of URL */
  std::string _urlRemainder; /**< Trailing portion of URL, non-standard only */
  std::string _format;       /**< format to use in converting member to string*/
  Trigger_mode_t _mode;      /**< Triggering mode */
  
  void _buildUrls(int nmember);
};

#endif
