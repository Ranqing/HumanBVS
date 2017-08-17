//#include "omp.h"

#include "findmatch.h"
#include "mhash.h"
#include "seed.h"
#include "../../Qing/qing_timer.h"

CSeed::CSeed(CFindMatch &findmatch) : fm_(findmatch) {
}

void CSeed::init(const vector<vector<CPoint> > *points) {
    points_[0].clear();
    points_[0].resize(fm_.max_level_);
    points_[1].clear();
    points_[1].resize(fm_.max_level_);

    int ptssize;

    printf("\n");

    for (int level = 0; level < fm_.max_level_; ++level) {
        ptssize = points[0][level].size();

        points_[0][level].resize(ptssize);
        copy(points[0][level].begin(), points[0][level].end(), points_[0][level].begin());

        ptssize = points[1][level].size();
        points_[1][level].resize(ptssize);
        copy(points[1][level].begin(), points[1][level].end(), points_[1][level].begin());

        printf("CSeed::Level %d : 0 - %-6d POINTS , 1 - %-6d POINTS.\n", level, points_[0][level].size(),
               points_[1][level].size());
    }
}

void CSeed::run(void) {
    int multicnt = CFindMatch::st_ncandidates_;
//	int g_ncore = omp_get_num_procs();

    cerr << "CSeed::run()" << endl;

//#pragma omp parallel for num_threads(g_ncore)
    for (int level = 0; level < fm_.max_level_; ++level) {
        printf("\nlevel %d seeds matches start. \n", level);

        QingTimer time_counter;

        multimap<int, int> featuresllist[2]; //features link list
        for (int i = 0; i < 2; i++) {
            int ptsize = points_[i][level].size();

            for (int j = 0; j < ptsize; ++j) {
                int x = (int) (points_[i][level][j].icoord_.val[0]);
                int y = (int) (points_[i][level][j].icoord_.val[1]);

                featuresllist[i].insert(pair<int, int>(y, x));
            }
        }

        int nm = (points_[0][level].size() + points_[1][level].size()) * multicnt;
        int nh = nm;
        printf("nh = %d, nm = %d\n", nh, nm);

        CMhash candidates[2];     //matches candidates

        candidates[0].init(nh, nm);
        candidates[1].init(nh, nm);

        local_seed_matching(level, 0, 1, featuresllist[0], featuresllist[1], candidates[0]);
        local_seed_matching(level, 1, 0, featuresllist[1], featuresllist[0], candidates[1]);

        fm_.cross_check(candidates[0], candidates[1], fm_.matchset_->matches_[level]);

        printf("level %d seeds matches end.  time cost = %.4f ms. \n", level, time_counter.duration());
    }

    //double end = omp_get_wtime();
}

//compute all the points in y - scan line
//changed siter feed back to localSeedMatching
void CSeed::pre_computation(const int level, const int index, const int iy, const int count,
                            multimap<int, int> &featuresllist, multimap<int, int>::iterator &aiter,
                            vector<Point2i> &points, vector<vector<vector<float> > > &nccvectors) {

    int nccsize = fm_.get_nccwsize_(level);
    int tmpcnt = 0;
    while (tmpcnt < count) {
        int ix = aiter->second;

        points[tmpcnt] = cv::Point2i(ix, iy);

        fm_.photopair_->photos_[index].compute_ncc_vector(level, nccsize, Point2i(ix - 1, iy),
                                                          nccvectors[tmpcnt * 3 + 0]);
        fm_.photopair_->photos_[index].compute_ncc_vector(level, nccsize, Point2i(ix, iy), nccvectors[tmpcnt * 3 + 1]);
        fm_.photopair_->photos_[index].compute_ncc_vector(level, nccsize, cv::Point2i(ix + 1, iy),
                                                          nccvectors[tmpcnt * 3 + 2]);

        ++tmpcnt;
        ++aiter;
    }
}

void CSeed::local_seed_matching(const int level, const int src, const int dst, multimap<int, int> &src_featuresllist,
                                multimap<int, int> &dst_featuresllist, CMhash &out_candidates) {
    float nccthreshold = fm_.ncc_threshold_;
    float priorthreshold = fm_.prior_threshold_;

    int width = fm_.photopair_->photos_[src].get_width(level);
    int height = fm_.photopair_->photos_[src].get_height(level);
    int nccsize = fm_.get_nccwsize_(level);

    int mindisp = 0;
    int maxdisp = fm_.max_disp_ >> level;

    printf("nccsize = %d, nccthreshold = %.2f, priorthreshold = %.2f, disparity threshold : %d ~ %d\n",
           nccsize, nccthreshold, priorthreshold, mindisp, maxdisp);

    multimap<int, int>::iterator siter = src_featuresllist.begin();
    while (siter != src_featuresllist.end()) {
        int sy = (*siter).first;
        int scount = src_featuresllist.count(sy);

        //dst image pointer
        int dcount = dst_featuresllist.count(sy);
        multimap<int, int>::iterator diter = dst_featuresllist.find(sy);

        if (dcount == 0 || diter == dst_featuresllist.end()) {
            while (scount--)
                siter++;
            continue;
        }

        int maxcount = min(CFindMatch::st_ncandidates_, min(scount, dcount));

        vector<Point2i> src_points(scount);
        vector<Point2i> dst_points(dcount);
        vector<vector<vector<float> > > src_nccvectors(scount * 3);
        vector<vector<vector<float> > > dst_nccvectors(dcount * 3);
        pre_computation(level, src, sy, scount, src_featuresllist, siter, src_points, src_nccvectors);
        pre_computation(level, dst, sy, dcount, dst_featuresllist, diter, dst_points, dst_nccvectors);

        for (int tscount = 0; tscount < scount; ++tscount) {
            Point2f tmpsrcpt = src_points[tscount];
            for (int tdcount = 0; tdcount < dcount; ++tdcount) {
                Point2f tmpdstpt = dst_points[tdcount];

                int tmpdisp = ((src == 0) ? (tmpsrcpt.x - tmpdstpt.x) : (tmpdstpt.x - tmpsrcpt.x));
                if (tmpdisp > mindisp && tmpdisp <= maxdisp) {
                    float tmpncc, tmpprior;
                    vector<vector<float> > *tmp_src_nccvector = &src_nccvectors[tscount * 3];
                    vector<vector<float> > *tmp_dst_nccvector = &dst_nccvectors[tdcount * 3];

                    fm_.photopair_->st_compute_ncc(src_nccvectors[tscount * 3 + 1], dst_nccvectors[tdcount * 3 + 1],
                                                   tmpncc);
                    fm_.photopair_->st_compute_prior(tmp_src_nccvector, tmp_dst_nccvector, tmpncc, tmpprior);

                    if ((tmpncc >= nccthreshold || fabs(tmpncc - nccthreshold) <= 0.01) &&
                        (tmpprior >= priorthreshold || fabs(tmpprior - priorthreshold) <= 0.01)) {
                        Point2f tmpkey = tmpsrcpt;
                        CMatchValue tmpvalue = CMatchValue(tmpdstpt, tmpncc, tmpprior);

                        bool issaved = out_candidates.maxstore(tmpkey, tmpvalue, maxcount);
                    }
                }
            }
        }
    }
}
