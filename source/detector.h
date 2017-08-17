// CDetector: for detecting feature points

#ifndef SUMMER_DETECTOR_H
#define SUMMER_DETECTOR_H

#include "feature/point.h"

class CPhotoPair;

class CFeatureDetector {
public:
    CFeatureDetector(void);

    virtual ~CFeatureDetector();

    //num:
    void init(const CPhotoPair &ppair, const int csize, const int mlevel);

    void run(const CPhotoPair &ppair, const int num, const int csize, const int mlevel);

    void savefeatures(const string folder, bool isshowed);

    void showfeatures(const string folder);

    void readfeatures(const string folder);

    vector<vector<CPoint> > points_[2];

protected:
    const CPhotoPair *ppair_;
    int csize_;
    int max_level_;
};

#endif