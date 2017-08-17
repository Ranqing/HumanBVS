#include "findmatch.h"

#include "../../Qing/qing_dir.h"
#include "../../Qing/qing_ply.h"

#define SUMMER_RELEASE

const int CFindMatch::st_ncandidates_ = 1;

CFindMatch::CFindMatch(void) : photopair_(NULL), seeder_(NULL), expander_(NULL), matchset_(NULL), optimizer_(NULL) {
    photopair_ = new CPhotoPair();
    seeder_ = new CSeed(*this);
    expander_ = new CExpand(*this);
    matchset_ = new CMatchOrganizer(*this);
    optimizer_ = new COptimizer(*this);
}

CFindMatch::~CFindMatch() {
    if (NULL != photopair_) {
        delete photopair_;
        photopair_ = NULL;
    }
    if (NULL != seeder_) {
        delete seeder_;
        seeder_ = NULL;
    }
    if (NULL != expander_) {
        delete expander_;
        expander_ = NULL;
    }
    if (NULL != matchset_) {
        delete matchset_;
        matchset_ = NULL;
    }
    if (NULL != optimizer_) {
        delete optimizer_;
        optimizer_ = NULL;
    }
}

void CFindMatch::init(const COption &option) {
    printf("\nCFindMatch initialization..\n");

    data_folder_ = option.data_folder_;      //"../../20161224/"
    stereo_name_ = option.stereo_name_;
    frame_name_ = option.frame_name_;
    stereo_id_ = option.stereo_id_;
    cam_names_[0] = option.cam_name_[0];
    cam_names_[1] = option.cam_name_[1];
    img_names_[0] = option.img_name_[0];
    img_names_[1] = option.img_name_[1];
    msk_names_[0] = option.msk_name_[0];
    msk_names_[1] = option.msk_name_[1];
    max_disp_ = option.max_disp_;
    min_disp_ = option.min_disp_;
    crop_points_[0] = option.crop_points_[0];
    crop_points_[1] = option.crop_points_[1];
    crop_size_ = Size(option.crop_w_, option.crop_h_);
    stereo_matrix_ = option.stereo_matrix_.clone();

    max_level_ = option.max_level_;
    cell_size_ = option.cell_size_;
    ncc_wsize_ = option.ncc_wsize_;
    ncc_threshold_ = option.ncc_threshold_;
    prior_threshold_ = option.prior_threshold_;
    near_depth_ = option.near_depth_;
    far_depth_ = option.far_depth_;
    unit_depth_ = option.unit_depth_;

    //init result's folder
    qing_create_dir("./result");
    stereo_folder_ = "./result/" + option.frame_name_;
    qing_create_dir(stereo_folder_);
    stereo_folder_ = stereo_folder_ + "/" + stereo_name_;
    qing_create_dir(stereo_folder_);
    cout << "result folder: " << stereo_folder_ << endl;

    int alloc = 1;
    photopair_->init(stereo_folder_, cam_names_, img_names_, msk_names_, crop_points_, crop_size_, max_level_,
                     stereo_matrix_, alloc);
    photopair_->save_pyramid();

    //detect features
    CFeatureDetector *df = new CFeatureDetector();
    if (NULL == df) {
        cerr << "failed to new feature detector..." << endl;
        return;
    }

#ifdef SUMMER_RELEASE
    const int fcsize = 16; //feature cell size
    df->run(*photopair_, 2, fcsize, max_level_);
    df->savefeatures(stereo_folder_, 1);
#else
    df.init(photopair_, fcsize, max_level_);
    df.readfeatures(stereo_folder_);
    df.showfeatures(stereo_folder_);
#endif

    seeder_->init(df->points_);
    expander_->init();
    matchset_->init();
    optimizer_->init();

    disps_.clear();
    disps_.resize(max_level_);
    points_.clear();
    points_.resize(max_level_);
}

void CFindMatch::run(void) {
    build_aver(); //need debug

    //--------------------------------------------------------------------------------------
    //seed generation
    seeder_->run();
    matchset_->saveMatches("seeds_");

    //--------------------------------------------------------------------------------------
    //expansion
    ncc_threshold_ = 0.65;
    prior_threshold_ = 0.75;

    int TIMES = 1;
    for (int t = 0; t < TIMES; ++t) {
        expander_->run();  //expander_.blockrun();
        update_threshold();
    }
    matchset_->saveMatches("expansion_");
    matchset_->convertToDisps(0, disps_, true);
    compute_3d_points(0, 1);

    //optzer_.run();
    //compute3dPoints(0, 1);
}

void CFindMatch::debug() {
    build_aver(); //need debug

    //--------------------------------------------------------------------------------------
    //seed generation
    matchset_->readMatches("seeds_");
    matchset_->readMatches("expansion_");

#ifndef SUMMER_RELEASE
    //read disp
    //optzer_.debug();
    disps_.resize(max_level_);
    vector<vector<SMatch>> fusedmatches(max_level_);
    vector<vector<SMatch>> expandedmatches(max_level_);
    for (int level = max_level_ - 1; level >= 0; level --)
    {
        int w = photopair_.photos_[0].getWidth(level);
        int h = photopair_.photos_[0].getHeight(level);
        disps_[level].resize(w*h);

        cout << endl;

        string fn = get_outfolder_() + "/0_disp_" + type2string<int>(level) + ".list";
        readDispList(fn, w, h, disps_[level]);

        string sfmatchfn = get_outfolder_() + "/seed_fused_" + type2string<int>(level) + ".list";
        readMatches(sfmatchfn,fusedmatches[level]);

        string expmatchfn = get_outfolder_() + "/expansion_" + type2string<int>(level) + ".list";
        readMatches(expmatchfn, expandedmatches[level]);

        /*vector<SMatch> before(0), vanish(0), after(0);
        compareMatches(fusedmatches[level], expandedmatches[level], before, vanish, after);
        string bffn = get_outfolder_() + "/debug/before_expansion_" + type2string<int>(level) + ".list";
        string vnfn = get_outfolder_() + "/debug/vanishin_expansion_" + type2string<int>(level) + ".list";
        string affn = get_outfolder_() + "/debug/after_expansion_" + type2string<int>(level) + ".list";
        saveMatches(bffn, before);
        saveMatches(vnfn, vanish);
        saveMatches(affn, after);

        std::vector<unsigned char>& srcimage = photopair_.photos_[0].getImage(level);
        std::vector<unsigned char>& dstimage = photopair_.photos_[1].getImage(level);
        string bfImfn = get_outfolder_() + "/debug/before_expansion_" + type2string<int>(level) + ".jpg";
        string vnImfn = get_outfolder_() + "/debug/vanishin_expansion_" + type2string<int>(level) + ".jpg";
        string afImfn = get_outfolder_() + "/debug/after_expansion_" + type2string<int>(level) + ".jpg";

        showMatches(bfImfn, srcimage, dstimage, w, h, 3, before);
        showMatches(vnImfn, srcimage, dstimage, w, h, 3, vanish);
        showMatches(afImfn, srcimage, dstimage, w, h, 3, after);*/

    }
    optimizer_->debug();
#endif

    //--------------------------------------------------------------------------------------
    //expansion
    ncc_threshold_ = 0.65;
    prior_threshold_ = 0.75;

    int level, w, h;
    for (level = max_level_ - 1; level >= 0; level--) {
        w = photopair_->photos_[0].get_width(level);
        h = photopair_->photos_[0].get_height(level);
        disps_[level].resize(w * h);

        cout << endl;

        string fn = get_outfolder_() + "/0_disp_" + type2string<int>(level) + ".list";
        readDispList(fn, w, h, disps_[level]);
        matchset_->saveDisps(0, level, "disp_", disps_[level]);
    }

    optimizer_->debug();
    compute_3d_points(0, 1);

}

void CFindMatch::build_aver(void) {
    vector<int> wsize(max_level_);
    for (int level = 0; level < max_level_; ++level)
        wsize[level] = get_nccwsize_(level);

    printf("\nfindmatch:: build_aver()\n");

    printf("photopair_::[0]:: build_aver()\n");
    photopair_->photos_[0].build_aver(wsize);
    printf("photopair_::[1]:: build_aver()\n");
    photopair_->photos_[1].build_aver(wsize);
}

void CFindMatch::update_threshold(void) {
    ncc_threshold_ -= 0.05f;
    prior_threshold_ -= 0.05f;
}

void CFindMatch::cross_check(CMhash &src_candidates, CMhash &dst_candidates, vector<CMatch> &matches) {
    matches.clear();

    int keysnum = src_candidates.numofkeys_;
    int i, sx, sy, isunique;
    Point2f tmp_skey, tmp_dkey, re_skey;
    CMatchValue tmp_svalue, tmp_dvalue;

    for (i = 0; i < keysnum; i++) {
        tmp_skey = src_candidates.get_akey(i);

        if (src_candidates.getinit(tmp_skey)) {
            sx = tmp_skey.x;
            sy = tmp_skey.y;

            while (src_candidates.getnext(tmp_svalue)) {
                tmp_dkey = tmp_svalue.dstpt;
                isunique = false;

                if (dst_candidates.getinit(tmp_dkey)) {
                    while (dst_candidates.getnext(tmp_dvalue)) {
                        re_skey = tmp_dvalue.dstpt;

                        if (re_skey == tmp_skey) {
                            isunique = true;
                            matches.push_back(CMatch(tmp_skey, tmp_svalue.dstpt, tmp_svalue.ncc, tmp_svalue.prior));
                            break;
                        }
                    }
                }
            }
        }
    }

    printf("after cross-checking: %d , %d ->  %d \n", src_candidates.numofvalues_, dst_candidates.numofvalues_,
           matches.size());
}

void CFindMatch::compute_3d_points(const int src, bool issaved) {
    printf("\nstart to compute disparity to 3d points. \n");
    for (int level = max_level_ - 1; level >= 0; --level)
        compute_3d_points(src, level, issaved);
}

void CFindMatch::compute_3d_points(const int src, const int level, bool issaved) {
    printf("level %d 3d points computation ", level);

    int width = photopair_->photos_[src].get_width(level);
    int height = photopair_->photos_[src].get_height(level);
    int scale = 1 << level;
    int base = (crop_points_[0].x / scale) - (crop_points_[1].x / scale);

    Point2i startpt = Point2i(crop_points_[src].x / scale, crop_points_[src].y / scale);

    Mat stereomtx = stereo_matrix_.clone();
    for (int i = 0; i < 3; ++i)
        stereomtx.at<float>(i, 3) /= scale;

    vector<unsigned char> &image = photopair_->photos_[src].get_image(level);
    vector<unsigned char> &mask = photopair_->photos_[src].get_mask(level);
    vector<float> &disps = disps_[level];
    vector<Point3f> points(width * height);
    vector<Vec3b> colors(width * height);

    int validnum = 0, y, x, index, sx, sy;
    float tmp_disp;
    for (y = 0; y < height; ++y) {
        for (x = 0; x < width; ++x) {
            index = y * width + x;

            if (mask[index] != 255 || disps[index] == 0.0f) {
                points[index] = Vec3f(PT_UNDEFINED, PT_UNDEFINED, PT_UNDEFINED);
                colors[index] = Vec3b(0, 0, 0);
                continue;
            }

            ++validnum;
            sx = x + startpt.x;
            sy = y + startpt.y;
            tmp_disp = disps[index] + base;

            Vec4f tmpvec = Vec4f(sx * 1.0f, sy * 1.0f, tmp_disp, 1.0f);
            Vec4f resvec;
            mul(stereomtx, tmpvec, resvec);

            points[index] = Point3f(resvec.val[0] / resvec.val[3], resvec.val[1] / resvec.val[3],
                                    resvec.val[2] / resvec.val[3]);
            colors[index] = Vec3b(image[3 * index + 2], image[3 * index + 1], image[3 * index + 0]);
        }
    }
    printf(" done. %6d 3d points.\n", validnum);

    if (issaved) {
        string filename = get_outfolder_() + "/pointcloud_" + type2string<int>(level) + ".ply";
        printf("save level %d 3d points.\n", validnum);
        writePLY(filename, width, height, validnum, PT_HAS_COLOR, points, colors);
    }
}

