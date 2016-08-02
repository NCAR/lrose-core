//
//
//

#ifndef FAULT_H
#define FAULT_H

#include <vector>
#include <string>

class Fault {

public:
    Fault();
    Fault(std::string msg);
    ~Fault();
    //    Fault(const Fault &src);
    Fault &operator=(const Fault &src);

    std::string msg    ();
    std::string msg    (std::string prefix);
    void        add_msg(std::string msg);

private:

    //std::string msg_;
    
    std::vector<std::string> msg_;
};


#endif // FAULT_H
