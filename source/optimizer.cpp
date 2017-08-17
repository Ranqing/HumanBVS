#include "optimizer.h"
#include "findmatch.h"
#include "basic.h"

COptimizer::COptimizer(CFindMatch &findmatch) : fm_(findmatch) {
}

COptimizer::~COptimizer() {
}

//hierarchy optimize
void COptimizer::init() {
    printf("\nCOptimizer initialization..\n");

    //20140703： for test
    //std::string newmask = fm_.get_outfolder_() + "/additional/erode_" + fm_.camera_name_[0] + "_mask_0.jpg";
    //buildErodeMask(newmask);
}

void COptimizer::run() {
    int maxlevels = fm_.max_level_;

    for (int level = maxlevels - 1; level >= 0; --level) {
        printf("\nstart to optimize disparity in level %d\n", level);

        eliminate(0, level);
        interpolate(0, level);
        //bspline(0, level);
    }
}

void COptimizer::interpolate(const int src, const int level) {
    vector<float> &rawdisp = fm_.disps_[level];
    vector<unsigned char> &rawmask = fm_.photopair_->photos_[src].get_mask(level);

    //std::vector<unsigned char>& rawmask = erodemasks[level];

    int width = fm_.photopair_->photos_[src].get_width(level);
    int height = fm_.photopair_->photos_[src].get_height(level);
    int validnum = fm_.photopair_->photos_[src].get_validnum(level);
    int maxnum = validnum /** 2 / width*/;  //validnum
    int totalnum = 0;                    //all not-matched pixels

    vector<CLinklist> regions(0);

    findNoMatchedPixels(rawdisp, rawmask, width, height, maxnum, totalnum, regions);
    printf("Level %d in image %d: not matched pixels = %d , ", level, src, totalnum);
    showNoMatchedPixels(src, level, rawdisp, regions);
    printf("start to interpolation.\n");

    int ksize = 5;
    double sigma = 1.2;
    double *gmask;
    Gauss2DFilter<double>(ksize, sigma, gmask);

    for (int i = 0; i < regions.size(); ++i) {
        CNode *tmpnode = regions[i].head;
        while (tmpnode != NULL) {
            int y = (tmpnode->pixel).y;
            int x = (tmpnode->pixel).x;
            int tmpindex = y * width + x;     // 插值此处像素的视差值

            //interpolation
            float interdisp = 0.0f;

            double weight, dnom = 0.0f;
            int offset = ksize / 2, nbx, nby, nbindex;

            for (int j = -offset; j <= offset; ++j) {
                nby = y + j;
                if (nby < 0 || nby >= height) continue;

                for (int i = -offset; i <= offset; ++i) {
                    nbx = x + i;
                    if (nbx < 0 || nbx >= width) continue;

                    nbindex = nby * width + nbx;

                    if (rawmask[nbindex] != 255 || rawdisp[nbindex] == 0)
                        continue;

                    weight = gmask[(j + offset) * ksize + (i + offset)];
                    interdisp += rawdisp[nbindex] * weight;
                    dnom += weight;
                }
            }

            if (dnom == 0.0)
                interdisp = 0.0;
            else
                interdisp = interdisp / dnom;

            rawdisp[tmpindex] = interdisp;
            tmpnode = tmpnode->pnext;
        }
    }

    fm_.matchset_->saveDisps(src, level, "inter_disp_", rawdisp);
    //fm_.matchset_.saveDisps(src, level, "inter_erode_disp_", fm_.disps_[level]);
}

void COptimizer::eliminate(const int src, const int level) {
    int ksize = fm_.get_nccwsize_(level);
    int width = fm_.photopair_->photos_[src].get_width(level);
    int height = fm_.photopair_->photos_[src].get_height(level);
    int allsize = width * height;

    vector<float> &rawdisp = fm_.disps_[level];
    vector<unsigned char> &rawmask = fm_.photopair_->photos_[src].get_mask(level);
    for (int i = 0; i < allsize; ++i)
        if (rawmask[i] != 255)
            rawdisp[i] = 0.0f;
    fm_.matchset_->saveDisps(src, level, "erode_disp_", rawdisp);

    //eliminate small expanded regions
    int maxnum = width;
    int totalnum;
    vector<CLinklist> regions(0);

    findMatchedPixels(rawdisp, rawmask, width, height, maxnum, totalnum, regions);
    printf("Level %d in image %d: eliminate pixels = %d \n", level, src, totalnum);

    for (int i = 0; i < regions.size(); ++i) {
        CNode *tmpnode = regions[i].head;
        while (tmpnode != NULL) {
            int tmpx = tmpnode->pixel.x;
            int tmpy = tmpnode->pixel.y;
            int tmpindex = tmpy * width + tmpx;

            if (rawdisp[tmpindex] == 0 && rawmask[tmpindex] == 255) {
                cerr << "error in find small matched pixels." << endl;
                exit(0);
            }
            rawdisp[tmpindex] = 0.0f;
            tmpnode = tmpnode->pnext;
        }
    }

    //fm_.matchset_.saveDisps(src, level, "eliminate_erode_disp_", rawdisp);
    fm_.matchset_->saveDisps(src, level, "eliminate_disp_", fm_.disps_[level]);
}


