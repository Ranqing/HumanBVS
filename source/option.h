//COption: a struct to load all the parameters needed in CFindMatch

#ifndef SUMMER_OPTION_H
#define SUMMER_OPTION_H

#include "../../Qing/qing_common.h"
#include "../../Qing/qing_dir.h"

struct COption {
public:
    string data_folder_;
    string stereo_name_;
    string frame_name_;
    string cam_name_[2];
    string img_name_[2];
    string msk_name_[2];

    int stereo_id_;
    int max_disp_, min_disp_;
    int crop_w_, crop_h_;
    Point2f crop_points_[2];
    Mat stereo_matrix_;


    int max_level_;
    int cell_size_;
    int ncc_wsize_;
    float ncc_threshold_;
    float prior_threshold_;
    float near_depth_;
    float far_depth_;
    float unit_depth_;


    COption(const string &data_folder);

    ~COption();

    void init(const string &option_filename);
};


#endif //option.h