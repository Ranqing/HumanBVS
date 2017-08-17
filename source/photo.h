//CPhoto: a class to manage a image information related to a camera
//Inference: a CPhoto also is a CImage / a CCamera

#ifndef SUMMER_PHOTO_H
#define SUMMER_PHOTO_H

#include "image.h"

class CPhoto : public CImage {
public:
    CPhoto(void);

    virtual ~CPhoto();

    void init(const string img_name, const string msk_name, const Point2f &crop_pt, const Size& crop_sz, const int maxlevel);

    void compute_ncc_vector(const int level, const int nccsize, Point2f pt, vector<vector<float> > &nccvector);

    void compute_ncc_vector(const int level, const int nccsize, Point2i pt, vector<vector<float> > &nccvector);
};

#endif //photo.h