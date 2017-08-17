//CFindMatch:  a class to find pixel matches between two-recified image

#ifndef SUMMER_FINDMATCH_H
#define SUMMER_FINDMATCH_H

#include "basic.h"
#include "debug.h"
#include "detector.h"
#include "expand.h"
#include "matchorganizer.h"
#include "optimizer.h"
#include "option.h"
#include "photopair.h"
#include "seed.h"

//class COption;
//class CMatch;
//class CMhash;
//class CPhotoPair;
//class CSeed;
//class CExpand;
//class CMatchOrganizer;
//class COptimizer;

class CFindMatch {
public:
    CFindMatch(void);

    virtual ~CFindMatch();

    void init(const COption &option);

    void run(void);

    void build_aver(void);  // passing nccwsize as kernel size to CImage

    void update_threshold(void);

    void cross_check(CMhash &src_candidates, CMhash &dst_candidates, vector<CMatch> &matches);

    void compute_3d_points(const int src, bool issaved);

    void compute_3d_points(const int src, const int level, bool issaved);

    inline int get_nccwsize_(const int level);

    inline string get_outfolder_();

    string stereo_name_;
    string data_folder_;            // folder storing image/mask/stereo data
    string frame_name_;
    int stereo_id_;
    int max_disp_;
    int min_disp_;
    string cam_names_[2];
    string img_names_[2];
    string msk_names_[2];
    Point2f crop_points_[2];        // image cut-points
    Size crop_size_;
    Mat stereo_matrix_;             // stereo matrix

    int max_level_;                 // level
    int cell_size_;                 // seeding cell size
    int ncc_wsize_;                 // ncc window size
    float ncc_threshold_;           // ncc threshold
    float prior_threshold_;         // prior threshold

    float near_depth_;              // near depth
    float far_depth_;               // far depth
    float unit_depth_;              // unit depth

    //core data member
    CPhotoPair *photopair_;
    CSeed *seeder_;
    CExpand *expander_;
    CMatchOrganizer *matchset_;
    COptimizer *optimizer_;

    //result folder
    string stereo_folder_;

    //results
    vector<vector<float> > disps_;            //disparitys
    vector<vector<Vec3f> > points_;            //3d points

    static const int st_ncandidates_;       //multi-candidates in matching
    void debug();
};

inline int CFindMatch::get_nccwsize_(const int level) {
    int scale = (int) pow(2.0f, level);
    int nccwsize = ncc_wsize_ / scale;

    if (nccwsize % 2 == 0)
        nccwsize = nccwsize + 1;

    nccwsize = max(nccwsize, 11);
    return nccwsize;
}

inline string CFindMatch::get_outfolder_() {
    return stereo_folder_;
}

#endif //findmatch.h