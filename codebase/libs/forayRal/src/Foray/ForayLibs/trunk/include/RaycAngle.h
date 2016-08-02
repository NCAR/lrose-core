//
//
//
//

#ifndef RAYCANGLE_H
#define RAYCANGLE_H

#include <string>

namespace ForayUtility {

class RaycAngle {
public:
    RaycAngle();
    RaycAngle(const double initAngle);
    ~RaycAngle();
    RaycAngle(const RaycAngle &ra);
    RaycAngle &operator= (const RaycAngle &ra);
    RaycAngle &operator= (const double da);

    RaycAngle &operator+= (const double da);
    RaycAngle &operator-= (const double da);


    bool operator==(const RaycAngle &ra) const;
    bool operator<(const RaycAngle &ra) const;
    bool operator>(const RaycAngle &ra) const;

    double value() const;
    double abs_delta(const RaycAngle &) const;
    
    static std::string csv_full_head(std::string head);
    std::string        csv_full_line();

private:

    void setAngle(const double inAngle);

    int        scaleAngle_;
    const static int scale_;
    const static int scale360_;
    const static int scale180_;

};
}




#endif // RAYCANGLE_H
