#include "feature.h"

CFeature::CFeature(void) {

}

CFeature::~CFeature(void) {

}

void CFeature::setGaussD(const float sigmaD, vector<float> &gaussD) {
    //----------------------------------------------------------------------
    const int marginD = (int) ceil(2 * sigmaD);
    const int sizeD = 2 * marginD + 1;

    gaussD.resize(sizeD);

    //----------------------------------------------------------------------
    // set m_gaussD
    float denom = 0.0, dtmp;
    int x, xtmp;
    for (x = 0; x < sizeD; ++x) {
        xtmp = x - marginD;
        dtmp = xtmp * exp(-(xtmp * xtmp) / (2 * sigmaD * sigmaD));
        gaussD[x] = dtmp;
        if (0.0 < dtmp)
            denom += dtmp;
    }

    for (x = 0; x < sizeD; ++x)
        gaussD[x] /= denom;
}

void CFeature::setGaussI(const float sigmaI, vector<float> &gaussI) {
    const int marginI = (int) ceil(2 * sigmaI);
    const int sizeI = 2 * marginI + 1;

    gaussI.resize(sizeI);

    //----------------------------------------------------------------------
    // set m_gaussI
    float denom = 0.0, dtmp;
    int x, xtmp;
    for (x = 0; x < sizeI; ++x) {
        xtmp = x - marginI;
        dtmp = exp(-(xtmp * xtmp) / (2 * sigmaI * sigmaI));
        gaussI[x] = dtmp;
        denom += dtmp;
    }
    for (x = 0; x < sizeI; ++x)
        gaussI[x] /= denom;
}

float CFeature::setThreshold(multiset<CPoint> &grid) {
    float ave = 0.0, ave2 = 0.0;
    int count = 0;

    multiset<CPoint>::iterator begin = grid.begin();
    multiset<CPoint>::iterator end = grid.end();

    while (begin != end) {
        count++;
        ave += begin->response_;
        ave2 += begin->response_ * begin->response_;
        begin++;
    }
    if (count == 0)
        count = 1;

    ave /= count;
    ave2 /= count;
    ave2 = sqrt(max(0.0f, ave2 - ave * ave));

    return (float) (ave + ave2);
}

//whether the feature is close to the boundary
int CFeature::isCloseBoundary(const int x, const int y, const int margin) const {
    if (m_mask.empty()) return 0;

    if (x - margin < 0 || m_width < (x + margin) ||
        y - margin < 0 || m_height < (y + margin))
        return 1;

    int j, i, ytmp, xtmp;
    for (j = -margin; j <= margin; ++j) {
        ytmp = y + j;
        for (i = -margin; i <= margin; ++i) {
            xtmp = x + i;
            if (m_mask[ytmp][xtmp] == 0)
                return 1;
        }
    }
    return 0;
}