//
//
//
//
//

#ifndef DORADEBLOCK_H
#define DORADEBLOCK_H

#include <string>
#include <vector>

#include "Fault.h"
#include "Buffer.h"

#include "KeyAccess.h"



//
// forward declarations
//
class DoradeBlockSswb;
class DoradeBlockVold;
class DoradeBlockRadd;
class DoradeBlockParm;
class DoradeBlockCelv;
class DoradeBlockCsfd;
class DoradeBlockSwib;
class DoradeBlockRyib;
class DoradeBlockRdat;
class DoradeBlockXstf;
class DoradeBlockRktb;

//
//
//
class DoradeBlock : public ForayUtility::KeyAccess {
public:
    DoradeBlock();
    virtual ~DoradeBlock();

    virtual bool        decode(ForayUtility::Buffer &buffer)         throw(ForayUtility::Fault) = 0;
    virtual void        encode(ForayUtility::Buffer &buffer)         throw(ForayUtility::Fault) = 0;
    virtual std::string listEntry() = 0;
    
    std::vector<int>    getIntegerVector()                           throw(ForayUtility::Fault);
    std::vector<double> getDoubleVector()                            throw(ForayUtility::Fault);

    void           set_integer_vector(std::vector<int>    &vector)   throw(ForayUtility::Fault);
    void           set_double_vector (std::vector<double> &vector)   throw(ForayUtility::Fault);
    

    virtual DoradeBlockSswb * castToDoradeBlockSswb() { return NULL;}
    virtual DoradeBlockVold * castToDoradeBlockVold() { return NULL;}
    virtual DoradeBlockRadd * castToDoradeBlockRadd() { return NULL;}
    virtual DoradeBlockParm * castToDoradeBlockParm() { return NULL;}
    virtual DoradeBlockCelv * castToDoradeBlockCelv() { return NULL;}
    virtual DoradeBlockCsfd * castToDoradeBlockCsfd() { return NULL;}
    virtual DoradeBlockSwib * castToDoradeBlockSwib() { return NULL;}
    virtual DoradeBlockRyib * castToDoradeBlockRyib() { return NULL;}
    virtual DoradeBlockRdat * castToDoradeBlockRdat() { return NULL;}
    virtual DoradeBlockXstf * castToDoradeBlockXstf() { return NULL;}
    virtual DoradeBlockRktb * castToDoradeBlockRktb() { return NULL;}

protected:

    std::vector<int>              integerVector_;
    std::vector<double>           doubleVector_;
    std::vector<double>::iterator doubleIterator_;

};

#endif  // DORADEBLOCK
