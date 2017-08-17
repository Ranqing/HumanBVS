#include "photo.h"

CPhoto::CPhoto(void) {
}

CPhoto::~CPhoto() {

}

void CPhoto::init(const string img_name, const string msk_name, const Point2f &crop_pt, const Size &crop_sz,
                  const int maxlevel) {
    cout << "initialization of photo: " << endl;
    cout << "img filename : " << img_name << endl;
    cout << "msk filename : " << msk_name << endl;

    CImage::init(img_name, msk_name, crop_pt, crop_sz, maxlevel);
}

void CPhoto::compute_ncc_vector(const int level, const int nccsize, Point2f pt, vector<vector<float> > &nccvector) {
    nccvector.clear();
    nccvector.resize(nccsize * nccsize);

    int width = get_width(level);
    int height = get_height(level);
    int offset = nccsize / 2;
    int i, j, inneridx;
    float tmpy, tmpx;

    if (is_safe(pt, level) == false) return;

    Vec3f tmpaver = get_aver(pt.x, pt.y, level);

    for (j = -offset; j <= offset; ++j) {
        tmpy = pt.y + j;
        if (tmpy < 0 || tmpy >= height) continue;

        for (i = -offset; i <= offset; ++i) {
            tmpx = pt.x + i;
            if (tmpx < 0 || tmpx >= width) continue;
            if (get_mask(tmpx, tmpy, level) != 255) continue;

            Vec3f tmpcolor = get_color(tmpx, tmpy, level);

            inneridx = (j + offset) * nccsize + (i + offset);
            //(image[tmpindex * 3 + 0] - avercolor[0]);
            nccvector[inneridx].push_back(tmpcolor.val[0] - tmpaver.val[0]);
            //(image[tmpindex * 3 + 1] - avercolor[1]);
            nccvector[inneridx].push_back(tmpcolor.val[1] - tmpaver.val[1]);
            //(image[tmpindex * 3 + 2] - avercolor[2]);
            nccvector[inneridx].push_back(tmpcolor.val[2] - tmpaver.val[2]);
        }
    }
}

void CPhoto::compute_ncc_vector(const int level, const int nccsize, Point2i pt, vector<vector<float> > &nccvector) {
    nccvector.clear();
    nccvector.resize(nccsize * nccsize);

    int width = get_width(level);
    int height = get_height(level);
    int idx, tmpidx, inneridx, tmpy, tmpx, offset, i, j;

    if ((pt.x < 0 || pt.x > width - 1) || (pt.y < 0 || pt.y > height - 1))
        return;

    vector<unsigned char> &image = get_image(level);
    vector<unsigned char> &mask = get_mask(level);
    vector<float> &aver = get_aver(level);

    idx = (pt.y * width + pt.x) * 3;
    offset = nccsize / 2;

    float tmpaver[3];
    tmpaver[0] = aver[idx + 0];
    tmpaver[1] = aver[idx + 1];
    tmpaver[2] = aver[idx + 2];

    for (j = -offset; j <= offset; ++j) {
        tmpy = pt.y + j;
        if (tmpy < 0 || (height - 1) < tmpy) continue;

        for (i = -offset; i <= offset; ++i) {
            tmpx = pt.x + i;
            if (tmpx < 0 || (width - 1) < tmpx) continue;

            tmpidx = tmpy * width + tmpx;

            if (mask[tmpidx] != 255) {
                //printf("mask != 255 , tmpx = %d, tmpy = %d, innerindex = %d\n", tmpx, tmpy, innerindex);
                continue;
            }

            inneridx = (j + offset) * nccsize + (i + offset);

            //(image[tmpindex * 3 + 0] - avercolor[0]);
            nccvector[inneridx].push_back(image[tmpidx * 3 + 0] - tmpaver[0]);
            //(image[tmpindex * 3 + 1] - avercolor[1]);
            nccvector[inneridx].push_back(image[tmpidx * 3 + 1] - tmpaver[1]);
            //(image[tmpindex * 3 + 2] - avercolor[2]);
            nccvector[inneridx].push_back(image[tmpidx * 3 + 2] - tmpaver[2]);
        }
    }
}