#include "point.h"

CPoint::CPoint(void) {
    response_ = -1.0;
    type_ = -1;
}

CPoint::CPoint(cv::Vec3f ticoord, int ttype, float tresponse) : icoord_(ticoord), type_(ttype), response_(tresponse) {

}

CPoint::~CPoint() {
}

std::istream &operator>>(std::istream &istr, CPoint &rhs) {
    string header;
    char str[1024];

    istr >> str;
    header = string(str);
    istr >> rhs.icoord_.val[0] >> rhs.icoord_.val[1] >> rhs.response_ >> rhs.type_;
    rhs.icoord_.val[2] = 1.0f;

    return istr;
}

std::ostream &operator<<(std::ostream &ostr, const CPoint &rhs) {
    ostr << "POINT0" << endl
         << rhs.icoord_.val[0] << ' ' << rhs.icoord_.val[1] << ' ' << rhs.response_ << ' '
         << rhs.type_;
    return ostr;
}