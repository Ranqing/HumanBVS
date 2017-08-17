//SMatch: struct for data structure of matches 
//SMatchNode: for CMhash use

#ifndef SUMMER_MATCH_H
#define SUMMER_MATCH_H

#include "../../Qing/qing_common.h"

struct CMatch {
    CMatch(void) {
        ncc = -1.0f;
        prior = -1.0f;
        srcpt = Point2i(-1, -1);
        dstpt = Point2i(-1, -1);
    }

    CMatch(const Point2i tsrcpt, const Point2i tdstpt, const float tncc, const float tprior) {
        srcpt = tsrcpt;
        dstpt = tdstpt;
        ncc = tncc;
        prior = tprior;
    }

    ~CMatch() {}

    Point2i srcpt;
    Point2i dstpt;
    float ncc;
    float prior;

    //copy constructor?
    inline void operator=(const CMatch &rhs) {
        srcpt = rhs.srcpt;
        dstpt = rhs.dstpt;
        ncc = rhs.ncc;
        prior = rhs.prior;
    }

};

istream &operator>>(istream &istr, CMatch &rhs);

ostream &operator<<(ostream &ostr, const CMatch &rhs);

bool operator<(const CMatch &lhs, const CMatch &rhs);

bool operator==(const CMatch &lhs, const CMatch &rhs);

struct CMatchValue {
    CMatchValue(void) {
        ncc = -1.0f;
        prior = -1.0f;
        dstpt = Point2i(-1, -1);
    }

    CMatchValue(const Point2i tdstpt, const float tncc, const float tprior) {
        ncc = tncc;
        prior = tprior;
        dstpt = tdstpt;
    }

    ~CMatchValue() {}

    Point2i dstpt;
    float ncc;
    float prior;

    inline void operator=(const CMatchValue &rhs) {
        dstpt = rhs.dstpt;
        ncc = rhs.ncc;
        prior = rhs.prior;
    }
};

bool operator<(const CMatchValue &lhs, const CMatchValue &rhs);

bool operator==(const CMatchValue &lhs, const CMatchValue &rhs);

#endif