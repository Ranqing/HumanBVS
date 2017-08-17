#include "image.h"
#include "basic.h"
#include "satable.h"

#include "../../Qing/qing_timer.h"


//#define SUMMER_DEBUG

CImage::CImage(void) {
    alloc_ = 0;
}

CImage::~CImage() {

}

void CImage::init(const string img_name, const string msk_name, const Point2f crop_pt, const Size crop_sz,
                  const int maxlevel) {
    printf("\nCImage initialization..\n");

    max_level_ = maxlevel;
    image_name_ = img_name;
    mask_name_ = msk_name;
    crop_pt_ = crop_pt;
    crop_sz_ = crop_sz;
    alloc_ = 0;
}

void CImage::alloc(const int fast /* = 0 */, const int filter /* = 0 */) {
    //memory already allocated
    if (alloc_ == 1 && fast == 1) return;
    if (alloc_ == 2) return;

    images_.resize(max_level_);
    masks_.resize(max_level_);
    avers_.resize(max_level_);
    widths_.resize(max_level_);
    heights_.resize(max_level_);
    validnums_.resize(max_level_);

    widths_[0] = crop_sz_.width;
    heights_[0] = crop_sz_.height;
    for (int level = 1; level < max_level_; ++level) {
        widths_[level] = widths_[level - 1] / 2;
        heights_[level] = heights_[level - 1] / 2;
    }

    alloc_ = 1;

#ifdef SUMMER_DEBUG
    for (int level = 0; level < max_level_; ++level)
    {
        printf("level = %d: width = %d , height = %d\n", level, widths_[level], heights_[level]);
    }
#endif

    if (fast) return;

    Mat imgmtx = imread(image_name_, 1);
    if (imgmtx.data == NULL) {
        cerr << "failed to read in " << image_name_ << endl;
        return;
    }
    Mat crop_imgmtx = imgmtx(Rect(crop_pt_, crop_sz_)).clone();
    Mat2PixelsVector<unsigned char>(crop_imgmtx, images_[0]);

    if (mask_name_ != "") {
        Mat mskmtx, binarymtx, crop_mskmtx;

        mskmtx = imread(mask_name_, 0);
        if (mskmtx.data == NULL) {
            cerr << "failed to read in " << mask_name_ << endl;
            return;
        }

        if (mskmtx.channels() == 1) {
            threshold(mskmtx, binarymtx, 127, 255, CV_THRESH_BINARY);
        } else {
            Mat graymtx;
            cvtColor(mskmtx, graymtx, CV_BGR2GRAY);
            threshold(graymtx, binarymtx, 127, 255, CV_THRESH_BINARY);
        }

        crop_mskmtx = binarymtx(Rect(crop_pt_, crop_sz_)).clone();
        Mat2PixelsVector<unsigned char>(crop_mskmtx, masks_[0]);
        validnums_[0] = countNonZero(crop_mskmtx);
    }

    //---------------------------------------------
    //build image/mask/edge pyramids

    QingTimer cclock;
    build_image_mask(filter);
    printf("\nbuilding image pyramid and mask pyramid time-cost = %.4f ms\n", cclock.duration());

    alloc_ = 2;
}

void CImage::free(void) {
    if (alloc_ != 0)
        alloc_ = 1;
    else
        alloc_ = 0;

    vector<vector<unsigned char> >().swap(images_);
    vector<vector<unsigned char> >().swap(masks_);
}

void CImage::free(const int level) {
    for (int lvl = 0; lvl < level; ++lvl) {
        vector<unsigned char>().swap(images_[lvl]);
        if (!masks_.empty())
            vector<unsigned char>().swap(masks_[lvl]);
    }
}

void CImage::build_image_mask(const int filter) {
    build_image(filter);

    if (mask_name_ != "")
        build_mask();
    else
        for (int level = 0; level < max_level_; ++level)
            validnums_[level] = widths_[level] * heights_[level];

#ifdef SUMMER_DEBUG
    for (int level = 0; level < max_level_; ++ level)
    {
        cv::Mat test_imagemtx, test_maskmtx;

        int t_width = widths_[level];
        int t_height = heights_[level];

        PixelsVector2Mat<unsigned char>(images_[level], t_width, t_height, 3, test_imagemtx);
        PixelsVector2Mat<unsigned char>(masks_[level], t_width, t_height, 1, test_maskmtx);

        /*char showname[1024];
        sprintf_s(showname, "%d-th level image", level);
        cv::imshow(showname, test_imagemtx);
        sprintf_s(showname, "%d-th level mask", level);
        cv::imshow(showname, test_maskmtx);
        cv::waitKey(0);
        cv::destroyAllWindows();*/
    }
#endif
}

void CImage::build_image(const int filter) {
    Mat mask = (Mat_<float>(4, 4) << 1.0f, 3.0f, 3.0f, 1.0f,
            3.0f, 9.0f, 9.0f, 3.0f,
            3.0f, 9.0f, 9.0f, 3.0f,
            1.0f, 3.0f, 3.0f, 1.0f);

    float total = 64.0f, denom;
    int x, y, level, size, j, i, ytmp, xtmp, index;

    for (y = 0; y < 4; y++)
        for (x = 0; x < 4; x++)
            mask.at<float>(y, x) = (mask.at<float>(y, x) / total);

    for (level = 1; level < max_level_; ++level) {
        size = widths_[level] * heights_[level] * 3;
        images_[level].resize(size);

        for (y = 0; y < heights_[level]; ++y) {
            for (x = 0; x < widths_[level]; ++x) {
                Vec3f color;
                if (filter == 2)
                    color[0] = color[1] = color[2] = 255.0f;

                denom = 0.0f;

                for (j = -1; j < 3; ++j) {
                    ytmp = 2 * y + j;
                    if (ytmp < 0 || (heights_[level - 1] - 1) < ytmp) continue;

                    for (i = -1; i < 3; ++i) {
                        xtmp = 2 * x + i;
                        if (xtmp < 0 || (widths_[level - 1] - 1) < xtmp) continue;

                        index = (ytmp * widths_[level - 1] + xtmp) * 3;

                        if (filter == 0) {
                            float tmask = mask.at<float>(j + 1, i + 1);

                            color[0] += tmask * (double) images_[level - 1][index];
                            color[1] += tmask * (double) images_[level - 1][index + 1];
                            color[2] += tmask * (double) images_[level - 1][index + 2];
                            denom += tmask;
                        } else if (filter == 1) {
                            color[0] = max((double) color[0], (double) images_[level - 1][index]);
                            color[1] = max((double) color[1], (double) images_[level - 1][index + 1]);
                            color[2] = max((double) color[2], (double) images_[level - 1][index + 2]);
                        } else {
                            color[0] = min((double) color[0], (double) images_[level - 1][index]);
                            color[1] = min((double) color[1], (double) images_[level - 1][index + 1]);
                            color[2] = min((double) color[2], (double) images_[level - 1][index + 2]);
                        }
                    } // end of i-while
                }  // end of j-while

                if (filter == 0)
                    color /= denom;

                index = (y * widths_[level] + x) * 3;

                images_[level][index] = (unsigned char) ((int) floor(color[0] + 0.5f));
                images_[level][index + 1] = (unsigned char) ((int) floor(color[1] + 0.5f));
                images_[level][index + 2] = (unsigned char) ((int) floor(color[2] + 0.5f));
            }
        }
    }
}

void CImage::build_mask(void) {
    int level, size, tmp_vnum, y, x;
    int ys[2], xs[2], in, out, j, i, idx;

    for (level = 1; level < max_level_; ++level) {
        size = widths_[level] * heights_[level];
        tmp_vnum = 0;

        masks_[level].resize(size);
        for (y = 0; y < heights_[level]; ++y) {
            ys[0] = 2 * y;
            ys[1] = min(heights_[level - 1] - 1, 2 * y + 1);

            for (x = 0; x < widths_[level]; ++x) {
                xs[0] = 2 * x;
                xs[1] = min(widths_[level - 1] - 1, 2 * x + 1);

                in = 0;
                out = 0;

                for (j = 0; j < 2; ++j) {
                    for (i = 0; i < 2; ++i) {
                        idx = ys[j] * widths_[level - 1] + xs[i];
                        if (masks_[level - 1][idx])
                            in++;
                        else
                            out++;
                    }
                }

                idx = y * widths_[level] + x;

                if (0 < in) {
                    masks_[level][idx] = (unsigned char) 255;
                    ++tmp_vnum;
                } else
                    masks_[level][idx] = (unsigned char) 0;
            }
        }
        validnums_[level] = ++tmp_vnum;
    }
}


//build average image : box filer
void CImage::build_aver(const vector<int> &ksize) {
    int level, pixel_num, idx, new_idx;

    for (level = 0; level < max_level_; ++level) {

        pixel_num = widths_[level] * heights_[level];

        vector<float> bpixels(pixel_num * 3, 0.0f);
        vector<float> gpixels(pixel_num * 3, 0.0f);
        vector<float> rpixels(pixel_num * 3, 0.0f);

        for (idx = 0; idx < pixel_num * 3; idx += 3) {
            new_idx = idx / 3;

            bpixels[new_idx] = images_[level][idx + 0];
            gpixels[new_idx] = images_[level][idx + 1];
            rpixels[new_idx] = images_[level][idx + 2];
        }

        vector<long> bpixels_sat(0), gpixels_sat(0), rpixels_sat(0);
        compute_sat(bpixels, widths_[level], heights_[level], bpixels_sat);
        compute_sat(gpixels, widths_[level], heights_[level], gpixels_sat);
        compute_sat(rpixels, widths_[level], heights_[level], rpixels_sat);

        vector<float> bpixels_mean(0), gpixels_mean(0), rpixels_mean(0);
        compute_mean(bpixels_sat, widths_[level], heights_[level], ksize[level], bpixels_mean);
        compute_mean(gpixels_sat, widths_[level], heights_[level], ksize[level], gpixels_mean);
        compute_mean(rpixels_sat, widths_[level], heights_[level], ksize[level], rpixels_mean);

        for (int i = 0; i < pixel_num; i++) {
            avers_[level].push_back(bpixels_mean[i]);
            avers_[level].push_back(gpixels_mean[i]);
            avers_[level].push_back(rpixels_mean[i]);
        }
    }

    //box filter
    /*for (int level = 0; level < max_level_; ++ level)
    {
        cv::Mat imgmtx, meanmtx;
        PixelsVector2Mat<unsigned char>(images_[level], widths_[level], heights_[level], 3, imgmtx);

        cv::Mat boxkernel(ksize[level], ksize[level], CV_32F);
        for (int i = 0; i < boxkernel.rows; ++ i)
        {
            for (int j = 0; j < boxkernel.cols; ++ j)
            {
                double m = 1.0 / ( ksize[level] * ksize[level] );
                boxkernel.at<float>(i,j) = m;
            }
        }

        cv::filter2D(imgmtx, meanmtx, -1, boxkernel);
        cv::imshow("MEANMTX", meanmtx);
        cv::waitKey(0);
        cv::destroyWindow("MEANMTX");
    }*/
}

void CImage::erode_mask(const int level, const int ksize) {
    vector<unsigned char> &rawmask = get_mask(level);
    int width = get_width(level);
    int height = get_height(level);

    Mat mskmtx, erode_mskmtx;
    PixelsVector2Mat<unsigned char>(rawmask, width, height, 1, mskmtx);

    //int ksize = 45;
    Mat element(ksize, ksize, CV_8U, cv::Scalar(1));
    erode(mskmtx, erode_mskmtx, element);
    Mat2PixelsVector<unsigned char>(erode_mskmtx, rawmask);
    printf("mask erosion done. ksize = %d\n", ksize);
}

