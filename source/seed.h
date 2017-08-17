//CSeed: for seed matches

#ifndef SUMMER_SEED_H
#define SUMMER_SEED_H

#include "feature/point.h"
#include "mhash.h"

class CFindMatch;

class CSeed {
public:
    CSeed(CFindMatch &findmatch);

    virtual ~CSeed() {};

    void init(const vector<vector<CPoint> > *points);

    void run(void);

    void local_seed_matching(const int level, const int src, const int dst,
                             multimap<int, int> &src_featuresllist,
                             multimap<int, int> &dst_featuresllist,
                             CMhash &out_candidates);

    void pre_computation(const int level, const int index, const int iy, const int count,
                         multimap<int, int> &featuresllist,
                         multimap<int, int>::iterator &aiter,
                         vector<Point2i> &points,
                         vector<vector<vector<float> > > &nccvectors);

protected:
    CFindMatch &fm_;
    vector<vector<CPoint>> points_[2];

};

#endif