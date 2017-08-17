#include "option.h"

COption::COption(const string &data_folder) : data_folder_(data_folder) {
    max_level_ = 4;
    cell_size_ = 16;
    ncc_wsize_ = 11;
    ncc_threshold_ = 0.85f;
    prior_threshold_ = 0.95f;
    near_depth_ = 150.0f;
    far_depth_ = 280.0f;
    unit_depth_ = 5.0f;
    stereo_matrix_.create(4, 4, CV_32F);

    cout << "data folder: " << data_folder_ << endl;
    cout << "current directory: ";
    qing_cwd();
}

COption::~COption(void) {

}

void COption::init(const string &option_filename) {
    std::ifstream ifstr(option_filename, std::ios::in);
    if (ifstr.is_open() == false) {
        printf("can't open %s\n", option_filename.c_str());
    }

    ifstr >> stereo_name_;
    ifstr >> stereo_id_;
    ifstr >> cam_name_[0];
    ifstr >> cam_name_[1];
    ifstr >> frame_name_;
    ifstr >> img_name_[0];
    ifstr >> img_name_[1];
    ifstr >> msk_name_[0];
    ifstr >> msk_name_[1];
    ifstr >> crop_points_[0].x >> crop_points_[0].y;
    ifstr >> crop_points_[1].x >> crop_points_[1].y;
    ifstr >> crop_w_ >> crop_h_;
    ifstr >> max_disp_ >> min_disp_;

    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            ifstr >> stereo_matrix_.at<float>(i, j);


//  ifstr >> max_level_;
//	ifstr >> cell_size_;
//	ifstr >> ncc_wsize_;
//	ifstr >> ncc_threshold_;
//	ifstr >> prior_threshold_;
//	ifstr >> near_depth_;
//	ifstr >> far_depth_;
//	ifstr >> unit_depth_;

    img_name_[0] = data_folder_ + "/Humans_frame/" + frame_name_ + "/" + img_name_[0];
    img_name_[1] = data_folder_ + "/Humans_frame/" + frame_name_ + "/" + img_name_[1];
    msk_name_[0] = data_folder_ + "/Humans_mask/" + frame_name_ + "/" + msk_name_[0];
    msk_name_[1] = data_folder_ + "/Humans_mask/" + frame_name_ + "/" + msk_name_[1];

    cout << "data folder: " << data_folder_ << endl;
    cout << img_name_[0] << endl;
    cout << img_name_[1] << endl;
    cout << msk_name_[0] << endl;
    cout << msk_name_[1] << endl;
    cout << "crop points: Left - " << crop_points_[0] << "\tRight - " << crop_points_[1] << endl;
    cout << "crop size: " << crop_h_ << "\t" << crop_w_ << endl;
    cout << "end of initialization of option\n";
}
