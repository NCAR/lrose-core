

#ifndef SAND_H
#define SAND_H

#include <string.h>

class Sand {
public:
    Sand();
    ~Sand();

    int numberOfCastles();
    
    std::string namesOfCastles();

    void castleFault() throw (Fault);

};

#endif // SAND_H
