//CPhotoSet: a class to manage two image/camera data in a stereo

#ifndef SUMMER_PHOTOSET_H
#define SUMMER_PHOTOSET_H

#include "../../Qing/qing_string.h"

#include "match.h"
#include "photo.h"

class CPhotoPair {
public:
    CPhotoPair(void);

    virtual ~CPhotoPair();

    void free(void);

    void free(const int level);

    void save_pyramid();

    string stereo_folder_;      // stereo_folder:  for saving result
    string cam_names_[2];
    string img_names_[2];
    string msk_names_[2];

    int num_;
    int max_level_;

    Point2f crop_points_[2];
    Mat stereo_matrix_;

    CPhoto photos_[2];

    void compute_match(const int level, const int src, const int dst, const int nccsize, CMatch &outmatch);

    void compute_match_ncc(const int level, const int src, const int dst, const int nccsize,
                           Point2i srcpt, Point2i dstpt, float &ncc);

    static void st_compute_ncc(const vector<vector<float> > &src_ncc_vector,
                               const vector<vector<float> > &dst_ncc_vector,
                               float &ncc);

    static void st_compute_prior(const vector<vector<float> > *src_ncc_vector,
                                 const vector<vector<float> > *dst_ncc_vector,
                                 const float ncc, float &prior);

    void init(const string stereo_folder,
              string *cams, string *imgs, string *msks,
              Point2f *crop_pts, Size crop_sz,
              const int maxlevel, const Mat &stereo_mtx, const int alloc);


};


#endif //photopair.h