#ifndef SUMMER_POINT_H
#define SUMMER_POINT_H

#include "../../../Qing/qing_common.h"

class CPoint {
public:
    CPoint(void);

    CPoint(Vec3f ticoord, int ttype, float tresponse);

    virtual ~CPoint();

    Vec3f icoord_;                  //2D coordinate
    Vec4f coord_;                   //3D coordinate

    int type_;                      //0: Harris, 1: DoG
    int itmp_;                      //temporary variable, used to store original image_id in initial match
    float response_;                //response


    bool operator<(const CPoint &rhs) const {
        return response_ < rhs.response_;
    }

    friend std::istream &operator>>(std::istream &istr, CPoint &rhs);

    friend std::ostream &operator<<(std::ostream &ostr, const CPoint &rhs);
};

std::istream &operator>>(std::istream &istr, CPoint &rhs);

std::ostream &operator<<(std::ostream &ostr, const CPoint &rhs);

#endif
