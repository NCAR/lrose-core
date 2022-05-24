#ifndef SPREADSHEETVIEW_HH
#define SPREADSHEETVIEW_HH

#include <iostream>
#include <fstream>
#include <string>
#include <map>

using namespace std; 

class SoloScriptTranslator
{

public:

  SoloScriptTranslator();
  virtual ~SoloScriptTranslator();

  enum unfold_alg {AC_WIND, LOCAL_WIND, FIRST_GOOD_GATE, UNKNOWN};

  void format_it(string &command);
  //void format_field_by_reference(string &field);
  //void construct_new_field(string &field);
  bool process_from_to(string line, std::iostream& javascript);
  bool process_action_in(string line, std::iostream& javascript);
  bool process_when_above(string line, std::iostream& javascript);
  bool process_on_var_below(string line, std::iostream& javascript);
  bool process_copy(string line, std::iostream& javascript);
  bool process_assignment(string line, std::iostream& javascript);
  bool process_action_no_args(string line, std::iostream& javascript);
  bool process_action_extra_args(string line, std::iostream& javascript);

  bool process_bool(string line, std::iostream& javascript, unfold_alg *BB_use);
  bool process_action_unfold(string line, std::iostream& javascript, unfold_alg BB_use);
  bool process_solo_comment(string line, std::iostream& javascript);

  void translate(ifstream& solo_script, std::iostream& javascript);

private:
  std::map<string, int> field_map_raw;
  std::map<string, int> field_map_derived;

  void reference_as_assignment(string &field); // , string &next_reference);
  void reference_as_source(string &field); // , string &next_reference);


};

#endif
