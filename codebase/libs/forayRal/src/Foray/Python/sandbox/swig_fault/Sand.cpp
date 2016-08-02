

#include "Fault.h"
#include "Sand.h"


Sand::Sand(){

}


Sand::~Sand(){

}

int Sand::numberOfCastles(){

    return 3;
}

std::string Sand::namesOfCastles(){
    std::string names("greenhill\nforestglenn\nriverside");
    return names;
}

void Sand::castleFault() throw (Fault){
    throw Fault("Fault from Castle\n");
}
