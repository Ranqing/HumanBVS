#include "basic.h"
#include "match.h"
#include "photo.h"
#include "photopair.h"

CPhotoPair::CPhotoPair(void) {
    num_ = 2;
}

CPhotoPair::~CPhotoPair() {

}

void CPhotoPair::init(const string stereo_folder, string *cams, string *imgs, string *msks,
                      Point2f *crop_pts, Size crop_sz, const int maxlevel, const Mat &stereo_mtx, const int alloc) {

    printf("\nCPhotoPair initialization..\n");

    stereo_folder_ = stereo_folder;
    cam_names_[0] = cams[0];
    cam_names_[1] = cams[1];
    img_names_[0] = imgs[0];
    img_names_[1] = imgs[1];
    msk_names_[0] = msks[0];
    msk_names_[1] = msks[1];

    num_ = 2;
    max_level_ = maxlevel;
    crop_points_[0] = crop_pts[0];
    crop_points_[1] = crop_pts[1];
    stereo_matrix_ = stereo_mtx.clone();

    for (int i = 0; i < num_; ++i) {
        photos_[i].init(img_names_[i], msk_names_[i], crop_points_[i], crop_sz,  max_level_);

        if (alloc)
            photos_[i].alloc();     //allocate images pyramid and masks pyramid, including widths and heights
        else
            photos_[i].alloc(1);    //allocate widths and heights of pyramid
    }
}

void CPhotoPair::free(void) {
    for (int index = 0; index < num_; ++index)
        photos_[index].free();
}

void CPhotoPair::free(const int level) {
    for (int index = 0; index < num_; ++index)
        photos_[index].free(level);
}

void CPhotoPair::save_pyramid() {
    string folder = stereo_folder_;
    string imgname, mskname;
    int i, level;

    for (i = 0; i < 2; i++) {
        for (level = 0; level < max_level_; level++) {
            vector<unsigned char> &tmp_img = photos_[i].get_image(level);
            vector<unsigned char> &tmp_msk = photos_[i].get_mask(level);

            imgname = folder + "/img_" + cam_names_[i] + "_" + qing_int_2_string(level) + ".jpg";
            mskname = folder + "/msk_" + cam_names_[i] + "_" + qing_int_2_string(level) + ".jpg";

            int width = photos_[i].get_width(level);
            int height = photos_[i].get_height(level);

            Mat timgmtx, tmskmtx;
            PixelsVector2Mat(tmp_img, width, height, 3, timgmtx);
            PixelsVector2Mat(tmp_msk, width, height, 1, tmskmtx);

            imwrite(imgname, timgmtx);
            cout << "saving " << imgname << endl;
            imwrite(mskname, tmskmtx);
            cout << "saving " << mskname << endl;
        }
    }

    printf("\n");

    for (int level = 0; level < max_level_; ++level)
        printf("level %d : 0 - %-7d valid pixels, 1 - %-7d valid pixels. \n", level, photos_[0].get_validnum(level),
               photos_[1].get_validnum(level));
    printf("\n");

}

void CPhotoPair::st_compute_ncc(const vector<vector<float> > &src_ncc_vector,
                                const vector<vector<float> > &dst_ncc_vector, float &ncc) {
    if (src_ncc_vector.size() == 0 || dst_ncc_vector.size() == 0) {
        ncc = -1.0;
        return;
    }

    double fenzi = 0.0, fenmu1 = 0.0, fenmu2 = 0.0;
    double tmp, srctmp, dsttmp;

    int srcsize = (int) src_ncc_vector.size();
    int dstsize = (int) dst_ncc_vector.size();

    for (int i = 0; i < srcsize; i++) {
        if (src_ncc_vector[i].size() == 0 || dst_ncc_vector[i].size() == 0)
            continue;

        tmp = 0.0;
        srctmp = 0.0;
        dsttmp = 0.0;

        for (int ch = 0; ch < (int) src_ncc_vector[i].size(); ch++) {
            tmp += src_ncc_vector[i][ch] * dst_ncc_vector[i][ch];
            srctmp += src_ncc_vector[i][ch] * src_ncc_vector[i][ch];
            dsttmp += dst_ncc_vector[i][ch] * dst_ncc_vector[i][ch];
        }
        fenzi += tmp;
        fenmu1 += srctmp;
        fenmu2 += dsttmp;
    }

    if (fenmu1 == 0 || fenmu2 == 0)
        ncc = 0.0;
    else
        ncc = (fenzi * 1.0) / sqrt(fenmu1 * fenmu2);
}

void CPhotoPair::st_compute_prior(const vector<vector<float> > *src_ncc_vector,
                                  const vector<vector<float> > *dst_ncc_vector, const float ncc, float &prior) {
    if (ncc == -1.0) {
        prior = -1.0;
        return;
    }

    float dst_leftncc, dst_rightncc;
    float src_leftncc, src_rightncc;
    float maxncc = 0.4;

    st_compute_ncc(src_ncc_vector[1], dst_ncc_vector[0], dst_leftncc);
    st_compute_ncc(src_ncc_vector[1], dst_ncc_vector[2], dst_rightncc);
    maxncc = max(max(dst_leftncc, dst_rightncc), maxncc);

    st_compute_ncc(src_ncc_vector[0], dst_ncc_vector[1], src_leftncc);
    st_compute_ncc(src_ncc_vector[2], dst_ncc_vector[1], src_rightncc);
    maxncc = max(max(src_leftncc, src_rightncc), maxncc);

    prior = (ncc * ncc) / maxncc;
}

void CPhotoPair::compute_match(const int level, const int src, const int dst, const int nccsize, CMatch &outmatch) {
    vector<vector<float> > src_nccvector[3];
    vector<vector<float> > dst_nccvector[3];

    Point2f srcpt = outmatch.srcpt;
    Point2f dstpt = outmatch.dstpt;

    photos_[src].compute_ncc_vector(level, nccsize, srcpt, src_nccvector[1]);
    photos_[src].compute_ncc_vector(level, nccsize, Point2i(srcpt.x - 1, srcpt.y), src_nccvector[0]);
    photos_[src].compute_ncc_vector(level, nccsize, Point2i(srcpt.x + 1, srcpt.y), src_nccvector[2]);

    photos_[dst].compute_ncc_vector(level, nccsize, dstpt, dst_nccvector[1]);
    photos_[dst].compute_ncc_vector(level, nccsize, Point2i(dstpt.x - 1, dstpt.y), dst_nccvector[0]);
    photos_[dst].compute_ncc_vector(level, nccsize, Point2i(dstpt.x + 1, dstpt.y), dst_nccvector[2]);

    st_compute_ncc(src_nccvector[1], dst_nccvector[1], outmatch.ncc);
    st_compute_prior(src_nccvector, dst_nccvector, outmatch.ncc, outmatch.prior);
}

void CPhotoPair::compute_match_ncc(const int level, const int src, const int dst, const int nccsize, Point2i srcpt,
                                   Point2i dstpt, float &ncc) {
    vector<vector<float> > src_nccvector, dst_nccvector;

    photos_[src].compute_ncc_vector(level, nccsize, srcpt, src_nccvector);
    photos_[dst].compute_ncc_vector(level, nccsize, dstpt, dst_nccvector);
    st_compute_ncc(src_nccvector, dst_nccvector, ncc);
}