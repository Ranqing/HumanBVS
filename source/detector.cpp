#include "basic.h"
#include "detector.h"
#include "feature/harris.h"
#include "feature/dog.h"
#include "photopair.h"


CFeatureDetector::CFeatureDetector(void) {

}

CFeatureDetector::~CFeatureDetector() {

}

void CFeatureDetector::init(const CPhotoPair &ppair, const int csize, const int mlevel) {
    ppair_ = &ppair;
    csize_ = csize;
    max_level_ = mlevel;
}

void CFeatureDetector::run(const CPhotoPair &ppair, const int num, const int csize, const int mlevel) {
    init(ppair, csize, mlevel);

    //num = 2
    points_[0].clear();
    points_[0].resize(max_level_);
    points_[1].clear();
    points_[1].resize(max_level_);

    int level, index;

    for (level = 0; level < max_level_; ++level) {
        cout << endl << "start to detect features in level " << level << endl;

        for (index = 0; index < num; ++index) {
            cout << "image " << index << "\t";

            // for harris
            const float sigma = 4.0f;
            // for DoG
            const float firstScale = 1.0f;
            const float lastScale = 3.0f;

            //----------------------------------------------------------------------
            // Harris
            {
                CHarris harris;
                multiset<CPoint> result;
                harris.run((ppair_)->photos_[index].get_image(level),    //image
                           (ppair_)->photos_[index].get_mask(level),     //mask
                           vector<unsigned char>(),                 //edge
                           (ppair_)->photos_[index].get_width(level),
                           (ppair_)->photos_[index].get_height(level), csize_, sigma, result);

                multiset<CPoint>::reverse_iterator rbegin = result.rbegin();
                while (rbegin != result.rend()) {
                    points_[index][level].push_back(*rbegin);
                    rbegin++;
                }
            }

            //----------------------------------------------------------------------
            // DoG
            {
                CDog dog;
                multiset<CPoint> result;
                dog.run((ppair_)->photos_[index].get_image(level),
                        (ppair_)->photos_[index].get_mask(level),
                        vector<unsigned char>(),
                        (ppair_)->photos_[index].get_width(level),
                        (ppair_)->photos_[index].get_height(level),
                        csize_, firstScale, lastScale, result);

                multiset<CPoint>::reverse_iterator rbegin = result.rbegin();
                while (rbegin != result.rend()) {
                    points_[index][level].push_back(*rbegin);
                    rbegin++;
                }
            }
        }
    }
}

void CFeatureDetector::savefeatures(const string folder, bool isshowed) {
    cout << endl;

    int level, index, i;
    string savefile;

    for (level = 0; level < max_level_; ++level) {
        for (index = 0; index < 2; ++index) {
            savefile = folder + "/" + ppair_->cam_names_[index] + "_features_" + type2string<int>(level) + ".list";

            fstream ofstr(savefile.c_str(), ios::out);
            if (ofstr.is_open() == false) {
                cerr << "failed to open " << savefile << endl;
                return;
            }

            vector<CPoint> &localpts = points_[index][level];

            ofstr << localpts.size() << endl;
            for (i = 0; i < localpts.size(); i++) {
                ofstr << localpts[i].icoord_.val[0] << ' '
                      << localpts[i].icoord_.val[1] << ' '
                      << localpts[i].type_ << ' '
                      << localpts[i].response_ << endl;
            }
            ofstr.close();
            cout << "saving " << savefile << endl;
        }
    }

    if (isshowed == true)
        showfeatures(folder);
}

void CFeatureDetector::showfeatures(const std::string folder) {
    int level, index, width, height, cellsize, t;
    int w, h, iw, ih, i, x, y, ttype;
    string jpgname;

    for (level = 0; level < max_level_; ++level) {
        for (index = 0; index < 2; ++index) {
            width = ppair_->photos_[index].get_width(level);
            height = ppair_->photos_[index].get_height(level);
            cellsize = 2 * csize_;

            //20140704
            const vector<CPoint> &localpts = points_[index][level];
            const vector<unsigned char> &ctmpimage = ppair_->photos_[index].get_image(level);
            const vector<unsigned char> &ctmpmask = ppair_->photos_[index].get_mask(level);

            vector<unsigned char> tmpimage(width * height * 3);
            vector<unsigned char> tmpmask(width * height);
            copy(ctmpimage.begin(), ctmpimage.end(), tmpimage.begin());
            copy(ctmpmask.begin(), ctmpmask.end(), tmpmask.begin());
            for (t = 0; t < width * height; ++t) {
                if (tmpmask[t] != 255) {
                    tmpimage[3 * t + 0] = 0;
                    tmpimage[3 * t + 1] = 0;
                    tmpimage[3 * t + 2] = 0;
                }
            }

            Mat mtx;

            PixelsVector2Mat<unsigned char>(tmpimage, width, height, 3, mtx);
            jpgname = folder + "/" + ppair_->cam_names_[index] + "_valid_" + type2string<int>(level) + ".jpg";
            imwrite(jpgname, mtx);


            w = (width + cellsize - 1) / cellsize;
            h = (height + cellsize - 1) / cellsize;
            //vertical
            for (iw = 1; iw < w; ++iw)
                line(mtx, Point(cellsize * iw - 1, 0), Point(cellsize * iw - 1, height - 1), Scalar(0, 0, 0));
            //horizontal
            for (ih = 1; ih < h; ++ih)
                line(mtx, Point(0, cellsize * ih - 1), Point(width - 1, cellsize * ih - 1), Scalar(0, 0, 0));
            jpgname = folder + "/" + ppair_->cam_names_[index] + "_blocks_" + type2string<int>(level) + ".jpg";
            imwrite(jpgname, mtx);

            for (i = 0; i < localpts.size(); i++) {
                x = localpts[i].icoord_.val[0];
                y = localpts[i].icoord_.val[1];
                ttype = localpts[i].type_;

                if (ttype == 0)
                    circle(mtx, Point2i(x, y), 1, Scalar(255, 0, 0));  //harris : blue
                else
                    circle(mtx, Point2i(x, y), 1, Scalar(0, 255, 0));  //dog: green
            }

            jpgname = folder + "/" + ppair_->cam_names_[index] + "_features_" + type2string<int>(level) + ".jpg";
            imwrite(jpgname, mtx);
        }
    }
}

void CFeatureDetector::readfeatures(const std::string folder) {
    //num = 2
    points_[0].clear();
    points_[0].resize(max_level_);
    points_[1].clear();
    points_[1].resize(max_level_);

    string filename;
    int level, index, ptsize;
    float x, y, response;
    int type;

    for (level = 0; level < max_level_; ++level) {
        for (index = 0; index < 2; ++index) {
            filename = folder + "/" + ppair_->cam_names_[index] + "_features_" + type2string<int>(level) + ".list";

            //printf("read features points from %s. ", filename.c_str());

            ifstream ifstr(filename.c_str(), ios::in);
            if (!ifstr.is_open()) {
                cerr << "CStereo::readFeaturePts: failed to open " << filename << endl;
                return;
            }

            ifstr >> ptsize;
            for (int i = 0; i < ptsize; i++) {
                ifstr >> x;
                ifstr >> y;
                ifstr >> type;
                ifstr >> response;
                points_[index][level].push_back(CPoint(Vec3f(x, y, 1.0f), type, response));
            }

            //	printf(" %d POINTS.\n" , ptsize);
        }
    }
}