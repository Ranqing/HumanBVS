//CImage:  a class to manage image information, including image pyramids, mask pyramids, edge pyramids

#ifndef SUMMER_IMAGE_H
#define SUMMER_IMAGE_H

#include "../../Qing/qing_common.h"

class CImage {
public:
    CImage(void);
    virtual ~CImage();
    virtual void init(const string img_name, const string msk_name, const Point2f crop_pt, const Size crop_sz, const int maxlevel);

    void build_aver(const vector<int> &ksize);                              //access right: public
    void erode_mask(const int level, const int ksize);                      //access right: public, for optimizer

    // access to image, mask, average value
    Vec3f get_color(const float fx, const float fy, const int level) const;
    Vec3f get_color(const int ix, const int iy, const int level) const;

    int get_mask(const float fx, const float fy, const int level) const;
    int get_mask(const int ix, const int iy, const int level) const;

    Vec3f get_aver(const float fx, const float fy, const int level) const;
    Vec3f get_aver(const int ix, const int iy, const int level) const;

    void set_color(const int ix, const int iy, const int level, const Vec3f &rgb);

    int get_width(const int level = 0) const;
    int get_height(const int level = 0) const;
    int get_validnum(const int level = 0) const;

    // access to image, mask, average
    const vector<unsigned char> &get_image(const int level) const;
    vector<unsigned char> &get_image(const int level);

    const vector<unsigned char> &get_mask(const int level) const;
    vector<unsigned char> &get_mask(const int level);

    const vector<float> &get_aver(const int level) const;
    vector<float> &get_aver(const int level);

    int is_safe(const Vec3f &icoord, const int level) const;
    int is_safe(const Point2f &pixel, const int level) const;
    int is_mask_exist(void) const;                                        // Check if a mask image exists

    //allocate and free memories
    void alloc(const int fast = 0, const int filter = 0);
    void free(void);                                                //free image  memory
    void free(const int level);                                     //free specified level image memory

protected:
    void build_image_mask(const int filter);
    void build_image(const int filter);
    void build_mask(void);

    //0: no memory allocated
    //1: widths and heights allocated
    //2: memory allocated
    int alloc_;
    int max_level_;
    string image_name_;
    string mask_name_;
    Point2f crop_pt_;
    Size crop_sz_;

    vector<vector<unsigned char> > images_;
    vector<vector<unsigned char> > masks_;
    vector<int> widths_;
    vector<int> heights_;
    vector<int> validnums_;
    vector<vector<float>> avers_;
};

inline Vec3f CImage::get_color(const float fx, const float fy, const int level) const {
    if (alloc_ != 2) {
        cerr << "allocate memory of images first." << endl;
        exit(1);
    }

    // Bilinear case
    const int lx = (int) floor(fx);
    const int ly = (int) floor(fy);

    const float dx1 = fx - lx;
    const float dx0 = 1.0f - dx1;
    const float dy1 = fy - ly;
    const float dy0 = 1.0f - dy1;

    if (dx1 == 0 && dy1 == 0)
        return get_color(lx, ly, level);

    const float f00 = dx0 * dy0;
    const float f01 = dx0 * dy1;
    const float f10 = dx1 * dy0;
    const float f11 = dx1 * dy1;

    const int index = 3 * (ly * widths_[level] + lx);
    const int index2 = index + 3 * widths_[level];

    const unsigned char *ucp0 = &images_[level][index] - 1;
    const unsigned char *ucp1 = &images_[level][index2] - 1;
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;

    r += *(++ucp0) * f00 + *(++ucp1) * f01;
    g += *(++ucp0) * f00 + *(++ucp1) * f01;
    b += *(++ucp0) * f00 + *(++ucp1) * f01;
    r += *(++ucp0) * f10 + *(++ucp1) * f11;
    g += *(++ucp0) * f10 + *(++ucp1) * f11;
    b += *(++ucp0) * f10 + *(++ucp1) * f11;

    return Vec3f(r, g, b);
}

inline Vec3f CImage::get_color(const int ix, const int iy, const int level) const {
    if (alloc_ != 2) {
        cerr << "allocate memory of images first." << endl;
        exit(1);
    }

    const int index = (iy * widths_[level] + ix) * 3;
    return Vec3f(images_[level][index], images_[level][index + 1], images_[level][index + 2]);
}

inline void CImage::set_color(const int ix, const int iy, const int level, const Vec3f &rgb) {
    const int index = (iy * widths_[level] + ix) * 3;

    images_[level][index] = (unsigned char) floor(rgb[0] + 0.5f);
    images_[level][index + 1] = (unsigned char) floor(rgb[1] + 0.5f);
    images_[level][index + 2] = (unsigned char) floor(rgb[2] + 0.5f);
}

inline int CImage::get_mask(const float fx, const float fy, const int level) const {
    if (alloc_ != 2) {
        cerr << "allocate memory of images first." << endl;
        exit(1);
    }

    if (masks_[level].empty())
        return 1;

    const int ix = (int) floor(fx + 0.5f);
    const int iy = (int) floor(fy + 0.5f);
    return get_mask(ix, iy, level);
};

inline int CImage::get_mask(const int ix, const int iy, const int level) const {
    if (alloc_ != 2) {
        cerr << "allocate memory of images first." << endl;
        exit(1);
    }

    if (masks_[level].empty())
        return 1;

    if (ix < 0 || widths_[level] <= ix || iy < 0 || heights_[level] <= iy)
        return 1;

    const int index = iy * widths_[level] + ix;
    return masks_[level][index];
}

inline Vec3f CImage::get_aver(const float fx, const float fy, const int level) const {
    if (avers_[level].empty()) {
        cerr << "allocate memory of images first." << endl;
        exit(1);
    }

    // Bilinear case
    const int lx = (int) floor(fx);
    const int ly = (int) floor(fy);

    const float dx1 = fx - lx;
    const float dx0 = 1.0f - dx1;
    const float dy1 = fy - ly;
    const float dy0 = 1.0f - dy1;

    if (dx1 == 0 && dy1 == 0)
        return get_aver(lx, ly, level);

    const float f00 = dx0 * dy0;
    const float f01 = dx0 * dy1;
    const float f10 = dx1 * dy0;
    const float f11 = dx1 * dy1;

    const int index = 3 * (ly * widths_[level] + lx);
    const int index2 = index + 3 * widths_[level];

    const float *ucp0 = &avers_[level][index] - 1;
    const float *ucp1 = &avers_[level][index2] - 1;
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;

    r += *(++ucp0) * f00 + *(++ucp1) * f01;
    g += *(++ucp0) * f00 + *(++ucp1) * f01;
    b += *(++ucp0) * f00 + *(++ucp1) * f01;
    r += *(++ucp0) * f10 + *(++ucp1) * f11;
    g += *(++ucp0) * f10 + *(++ucp1) * f11;
    b += *(++ucp0) * f10 + *(++ucp1) * f11;

    return Vec3f(r, g, b);
}

inline Vec3f CImage::get_aver(const int ix, const int iy, const int level) const {
    if (avers_[level].empty()) {
        cerr << "compute the avers first." << endl;
        exit(1);
    }

    const int index = (iy * widths_[level] + ix) * 3;
    return Vec3f(avers_[level][index], avers_[level][index + 1], avers_[level][index + 2]);
}


inline int CImage::get_width(const int level) const {
    if (alloc_ == 0) {
        cerr << "allocate memory of images first." << endl;
        exit(1);
    }
    return widths_[level];
}

inline int CImage::get_height(const int level) const {
    if (alloc_ == 0) {
        cerr << "allocate memory of images first." << endl;
        exit(1);
    }

    return heights_[level];
}

inline int CImage::get_validnum(const int level) const {
    if (alloc_ == 0) {
        cerr << "allocate memory of images first." << endl;
        exit(1);
    }
    return validnums_[level];
}

inline const vector<unsigned char> &CImage::get_image(const int level) const {
    if (alloc_ != 2) {
        cerr << "allocate memory of images first." << endl;
        exit(1);
    }

    return images_[level];
}

inline vector<unsigned char> &CImage::get_image(const int level) {
    if (alloc_ != 2) {
        cerr << "allocate memory of images first." << endl;
        exit(1);
    }

    return images_[level];
}

inline const vector<unsigned char> &CImage::get_mask(const int level) const {
    if (alloc_ != 2) {
        cerr << "allocate memory of images first." << endl;
        exit(1);
    }
    return masks_[level];
}

inline vector<unsigned char> &CImage::get_mask(const int level) {
    if (alloc_ != 2) {
        cerr << "allocate memory of images first." << endl;
        exit(1);
    }

    if (!masks_.empty()) return masks_[level];
    //else
    //return std::vector<unsigned char>();
}

inline const vector<float> &CImage::get_aver(const int level) const {
    if (avers_[level].empty()) {
        cerr << "allocate memory of images first." << endl;
        exit(1);
    }
    return avers_[level];
}

inline vector<float> &CImage::get_aver(const int level) {
    if (alloc_ != 2) {
        cerr << "allocate memory of images first." << endl;
        exit(1);
    }

    return avers_[level];
}


inline int CImage::is_safe(const Vec3f &icoord, const int level) const {
    if (icoord[0] < 0.0 || widths_[level] - 2 < icoord[0] ||
        icoord[1] < 0.0 || heights_[level] - 2 < icoord[1])
        return 0;
    else
        return 1;
}

inline int CImage::is_safe(const Point2f &pixel, const int level) const {
    if (pixel.x < 0.0 || widths_[level] - 1 < pixel.x ||
        pixel.y < 0.0 || heights_[level] - 1 < pixel.y)
        return 0;
    else
        return 1;
}

inline int CImage::is_mask_exist(void) const {
    if (masks_[0].empty()) return 0;
    else return 1;
}

//create 1d gaussian filter based on sigma
inline void create_filter(const float sigma, vector<float> &filter)   {
    float sigma2 = 2.0f * sigma * sigma, sum = 0.f;
    int margin = (int) floor(2 * sigma), i;

    filter.resize(2 * margin + 1);
    for (i = -margin; i < margin; ++i) {
        filter[i + margin] = exp(-(i * i) / sigma2);
        sum += filter[i + margin];
    }

    for (i = 0; i < 2 * margin + 1; ++i)
        filter[i] /= sum;
}



#endif //image.h