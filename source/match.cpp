#include "match.h"

istream &operator>>(std::istream &istr, CMatch &rhs) {
    int sx, dx, y;
    float ncc;
    float prior;

    istr >> y >> sx >> dx >> ncc >> prior;

    return istr;
}

ostream &operator<<(std::ostream &ostr, const CMatch &rhs) {
    ostr << rhs.srcpt.y << ' '
         << rhs.srcpt.x << ' '
         << rhs.dstpt.x << ' '
         << rhs.ncc << ' '
         << rhs.prior << endl;

    return ostr;
}

bool operator<(const CMatch &lhs, const CMatch &rhs) {
    return (lhs.ncc < rhs.ncc);
}

bool operator==(const CMatch &lhs, const CMatch &rhs) {
    if (lhs.srcpt == rhs.dstpt && lhs.dstpt == rhs.dstpt)
        return true;
    else
        return false;
}

bool operator<(const CMatchValue &lhs, const CMatchValue &rhs) {
    return (lhs.ncc < rhs.ncc);
}

bool operator==(const CMatchValue &lhs, const CMatchValue &rhs) {
    if (lhs.dstpt == rhs.dstpt)
        return true;
    else
        return false;

}



