#include <iostream>
#include <fstream>
#include <sstream>
#include <iterator>
#include <regex>
#include <string>
#include <algorithm>
 
using namespace std;

void process(string str) {
	cout << str << endl;
}

void format_it(string &command) {
	std::transform(command.begin(), command.end(), command.begin(), ::toupper);
	replace(command.begin(), command.end(), '-', '_');
}

bool process_from_to(string line) {
	bool recognized = false;
		        // from x to y 
	    const std::regex pieces_regex("(remove-ring) in ([_[:alnum:]]+) from[\\s]+([-\\.[:digit:]]+) to ([-\\.[:digit:]]+) km\\.[\\s]"); // ("copy[:space:]+([A-Z]+)[:space:]+to[:space:]+([A-Z]+)");
	    std::smatch pieces_match;
	        string mytest2 = "remove-ring in VG from 34. to -70.";
	        if (std::regex_match(line, pieces_match, pieces_regex)) {
	            //std::cout << line << '\n';
	            for (size_t i = 0; i < pieces_match.size(); ++i) {
	                std::ssub_match sub_match = pieces_match[i];
	                std::string piece = sub_match.str();
	                std::cout << "  submatch " << i << ": " << piece << '\n';
	            }   
	            string command = pieces_match[1];
	            format_it(command);
	            string field = pieces_match[2];
	            string from = pieces_match[3];
	            string to = pieces_match[4];
	            cout << command << " ( " << field << "," << from << "," << to << " )" << endl;
	            recognized = true;
	        } else {
	        	std::cout << "regex_match returned false\n";
	        } 
	return recognized;
}

bool process_action_in(string line) {
	bool recognized = false;
		        // from x to y 
	    const std::regex pieces_regex("(remove-only-surface|assert-bad-flags|copy-bad-flags|BB-unfolding) (in|from|of) ([_[:alnum:]]+)[\\s]"); // ("copy[:space:]+([A-Z]+)[:space:]+to[:space:]+([A-Z]+)");
	    std::smatch pieces_match;
	        string mytest2 = "remove-ring in VG from 34. to -70.";
	        if (std::regex_match(line, pieces_match, pieces_regex)) {
	            //std::cout << line << '\n';
	            for (size_t i = 0; i < pieces_match.size(); ++i) {
	                std::ssub_match sub_match = pieces_match[i];
	                std::string piece = sub_match.str();
	                std::cout << "  submatch " << i << ": " << piece << '\n';
	            }   
	            string command = pieces_match[1];
	            format_it(command);
	            string field = pieces_match[3];
	            cout << command << " ( " << field << " )" << endl;
	            recognized = true;
	        } else {
	        	std::cout << "regex_match returned false\n";
	        } 
	return recognized;
}

bool process_when_above(string line) {
	bool recognized = false;
		        // from x to y 
	    // const std::regex pieces_regex("(and-bad-flags|set-bad-flags|or-bad-flags) when ([_[:alnum:]]+) (above|below) ([-\\.[:digit:]]+)[\\s]"); // ("copy[:space:]+([A-Z]+)[:space:]+to[:space:]+([A-Z]+)");

	    const std::regex pieces_regex("((and|set|or|xor)-bad-flags) when ([_[:alnum:]]+) (above|below) ([-\\.[:digit:]]+)[\\s]"); // ("copy[:space:]+([A-Z]+)[:space:]+to[:space:]+([A-Z]+)");
	    std::smatch pieces_match;

	        if (std::regex_match(line, pieces_match, pieces_regex)) {
	            //std::cout << line << '\n';
	            for (size_t i = 0; i < pieces_match.size(); ++i) {
	                std::ssub_match sub_match = pieces_match[i];
	                std::string piece = sub_match.str();
	                std::cout << "  submatch " << i << ": " << piece << '\n';
	            }   
	            string command = pieces_match[1];
	            format_it(command);
	            string field = pieces_match[3];
	            string above_below = pieces_match[4];
	            format_it(above_below);
	            string value = pieces_match[5];
	            cout << command << "_" << above_below << " ( " << field << "," << value << " )" << endl;
	            recognized = true;
	        } else {
	        	std::cout << "regex_match returned false\n";
	        } 
	return recognized;
}

bool process_on_var_below(string line) {
	bool recognized = false;
		        // from x to y 
	    const std::regex pieces_regex("(threshold) ([_[:alnum:]]+) on ([_[:alnum:]]+) below ([-\\.[:digit:]]+)[\\s]"); // ("copy[:space:]+([A-Z]+)[:space:]+to[:space:]+([A-Z]+)");
	    std::smatch pieces_match;

	        if (std::regex_match(line, pieces_match, pieces_regex)) {
	            //std::cout << line << '\n';
	            for (size_t i = 0; i < pieces_match.size(); ++i) {
	                std::ssub_match sub_match = pieces_match[i];
	                std::string piece = sub_match.str();
	                std::cout << "  submatch " << i << ": " << piece << '\n';
	            }   
	            string command = pieces_match[1];
	            format_it(command);
	            string field1 = pieces_match[2];
	            string field2 = pieces_match[3];
	            string value = pieces_match[4];
	            cout << command << " ( " << field1 << "," << field2 << "," << value << " )" << endl;
	            recognized = true;
	        } else {
	        	std::cout << "regex_match returned false\n";
	        } 
	return recognized;
}

bool process_copy(string line) {
	bool recognized = false;
	    //  [\\s] is blank space
	    const std::regex pieces_regex("copy[\\s]+([_[:alnum:]]+) to ([_[:alnum:]]+)[\\s]+"); // ("copy[:space:]+([A-Z]+)[:space:]+to[:space:]+([A-Z]+)");
	    std::smatch pieces_match;
	        string mytest = "copy VG to VP1";
	        if (std::regex_match(line, pieces_match, pieces_regex)) {
	            //std::cout << line << '\n';
	            for (size_t i = 0; i < pieces_match.size(); ++i) {
	                std::ssub_match sub_match = pieces_match[i];
	                std::string piece = sub_match.str();
	                std::cout << "  submatch " << i << ": " << piece << '\n';
	            }   
	            cout << pieces_match[2] << " = " << pieces_match[1] << endl;
	            recognized = true;
	        } else {
	        	std::cout << "regex_match returned false\n";
	        }  
	return recognized; 
}

bool process_assignment(string line) {
	bool recognized = false;
	//  [\\s] is blank space
	const std::regex pieces_regex("([-[:alpha:]]+) is ([-\\.[:digit:]]+)[\\s]*(|degrees|gates)[\\s]*");

	std::smatch pieces_match;

	        if (std::regex_match(line, pieces_match, pieces_regex)) {

	            for (size_t i = 0; i < pieces_match.size(); ++i) {
	                std::ssub_match sub_match = pieces_match[i];
	                std::string piece = sub_match.str();
	                std::cout << "  submatch " << i << ": " << piece << '\n';
	            }   
	            string command = pieces_match[1];
	            format_it(command);
	            string value = pieces_match[2];
	            cout << command << " = " << value << endl;
	            recognized = true;
	        } else {
	        	std::cout << "regex_match returned false\n";
	        }  
	return recognized; 
}

bool process_action_no_args(string line) {
	bool recognized = false;
		        // from x to y 
	    const std::regex pieces_regex("(clear-bad-flags)[\\s]");

	    std::smatch pieces_match;

	        if (std::regex_match(line, pieces_match, pieces_regex)) {
	            //std::cout << line << '\n';
	            for (size_t i = 0; i < pieces_match.size(); ++i) {
	                std::ssub_match sub_match = pieces_match[i];
	                std::string piece = sub_match.str();
	                std::cout << "  submatch " << i << ": " << piece << '\n';
	            }   
	            string command = pieces_match[1];
	            format_it(command);
	            cout << command << " ( )" << endl;
	            recognized = true;
	        } else {
	        	std::cout << "regex_match returned false\n";
	        } 
	return recognized;
}

int main()
{

   std::string text = "Quick brown fox";
   std::regex vowel_re("a|e|i|o|u");
   std::regex solo(" IS ");
   std::regex solo2(" IN "); 
   std::regex solo_copy("COPY VG TO V1"); // => V1 = VG  
   std::ifstream script_file;

   
   script_file.open("/Users/brenda/data/solo_scripts/sed_med_auto", std::ios::in);

   if (script_file.is_open()) {
     std::string line;
     while (getline(script_file, line)) {
     	//size_t length = line.size();
     	//line[length-1] = '\0';
     	std::cout << "|" << line << "| \n";
     	bool recognized = false;
 /*
        // maybe instead of regular expressions, just separate by blank space, then rearrange tokens?
        stringstream iss(line);
        string str;
        while (iss >> str) {
       	   process(str);
        }
*/

    // ----
  
	        //  
       if (!recognized) recognized = process_action_no_args(line);
       if (!recognized) recognized = process_copy(line);
       if (!recognized) recognized = process_from_to(line);
       if (!recognized) recognized = process_action_in(line);
       if (!recognized) recognized = process_when_above(line);
       if (!recognized) recognized = process_on_var_below(line);
       if (!recognized) recognized = process_assignment(line);
       if (!recognized) {
		   // write the results to an output iterator
	       //std::regex_replace(std::ostreambuf_iterator<char>(std::cout),
	        //  
	        //               line.begin(), line.end(), vowel_re, "*");
	       //std::transform(line.begin(), line.end(), line.begin(), ::toupper);
	       //std::string line1, line2, line3;
	       //line1 = std::regex_replace(line, solo, " = ");

	       //line2 = std::regex_replace(line1, solo2, "(");
	       //std::cout << line2 << std::endl;
	       //if (line2.find("(") != std::string::npos) line2.append(")");
	 
	       // construct a string holding the results
	       //std::cout << line2 << std::endl;
       }
       if (!recognized) {
       	  cout << "ERROR not recognized " << line << endl;
       }


// -----

	        
	        std::cout << std::endl;

// -----


       //std::cout << '\n' << std::regex_replace(text, vowel_re, "[$&]") << '\n';
       
     }
     script_file.close();
   }
   
}
