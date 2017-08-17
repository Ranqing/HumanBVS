#include "findmatch.h"
#include "matchorganizer.h"

CMatchOrganizer::CMatchOrganizer(CFindMatch &findmatch) : fm_(findmatch) {

}

CMatchOrganizer::~CMatchOrganizer() {

}

void CMatchOrganizer::init(void) {

    printf("\nCMatchOrganizer initialization\n");

    int maxlevel = fm_.max_level_;

    matches_.clear();
    matches_.resize(maxlevel);
}

void CMatchOrganizer::optimize(const int level) {

}

void CMatchOrganizer::saveMatches(std::string prefix) {
    for (int level = 0; level < fm_.max_level_; ++level)
        saveMatches(prefix, level);
}

void CMatchOrganizer::saveMatches(const std::string prefix, const int level) {
    string folder = fm_.get_outfolder_();

    string jpgname = folder + "/" + prefix + type2string<int>(level) + ".jpg";
    string listname = folder + "/" + prefix + type2string<int>(level) + ".list";

    vector<unsigned char> &srcimage = fm_.photopair_->photos_[0].get_image(level);
    vector<unsigned char> &dstimage = fm_.photopair_->photos_[1].get_image(level);
    int width = fm_.photopair_->photos_[0].get_width(level);
    int height = fm_.photopair_->photos_[0].get_height(level);

    fstream fout(listname.c_str(), std::ios::out);
    if (fout.is_open() == false) {
        cerr << "failed to open " << listname << endl;
        exit(1);
    }

    //20140704
    vector<unsigned char> &srcmask = fm_.photopair_->photos_[0].get_mask(level);
    vector<unsigned char> &dstmask = fm_.photopair_->photos_[1].get_mask(level);

    for (int t = 0; t < width * height; ++t) {
        if (srcmask[t] != 255) {
            srcimage[3 * t + 0] = 0;
            srcimage[3 * t + 1] = 0;
            srcimage[3 * t + 2] = 0;
        }
        if (dstmask[t] != 255) {
            dstimage[3 * t + 0] = 0;
            dstimage[3 * t + 1] = 0;
            dstimage[3 * t + 2] = 0;
        }
    }

    Mat srcmtx, dstmtx, mtxpair, part;
    PixelsVector2Mat<unsigned char>(srcimage, width, height, 3, srcmtx);
    PixelsVector2Mat<unsigned char>(dstimage, width, height, 3, dstmtx);

    mtxpair.create(height, width * 2, CV_8UC3);
    part = mtxpair(cv::Rect(0, 0, width, height));
    srcmtx.copyTo(part);
    part = mtxpair(cv::Rect(width, 0, width, height));
    dstmtx.copyTo(part);

    fout << matches_[level].size() << endl;
    for (int i = 0; i < matches_[level].size(); ++i) {
        CMatch tmpmatch = matches_[level][i];
        fout << tmpmatch.srcpt.y << ' ' << tmpmatch.srcpt.x << ' ' << tmpmatch.dstpt.x << ' '
             << tmpmatch.ncc << ' ' << tmpmatch.prior << std::endl;

        if (prefix == "seeds_" || prefix == "seeds_fused_") {
            line(mtxpair, tmpmatch.srcpt, Point(tmpmatch.dstpt.x + width, tmpmatch.dstpt.y), Scalar(0, 255, 0));
            circle(mtxpair, tmpmatch.srcpt, 1, Scalar(255, 0, 0));
            circle(mtxpair, Point(tmpmatch.dstpt.x + width, tmpmatch.dstpt.y), 1, Scalar(255, 0, 0));
        } else {
            //gray
            circle(mtxpair, tmpmatch.srcpt, 1, Scalar(192, 192, 192));
            circle(mtxpair, Point(tmpmatch.dstpt.x + width, tmpmatch.dstpt.y), 1, Scalar(192, 192, 192));
        }
    }
    fout << -1 << endl;
    imwrite(jpgname, mtxpair);
}

void CMatchOrganizer::readMatches(string prefix) {
    for (int level = 0; level < fm_.max_level_; ++level)
        readMatches(prefix, level);
}

void CMatchOrganizer::readMatches(const string prefix, const int level) {
    string folder = fm_.get_outfolder_();
    string listname = folder + "/" + prefix + type2string<int>(level) + ".list";

    fstream fin(listname.c_str(), std::ios::in);
    if (fin.is_open() == false) {
        cerr << "failed to open " << listname << endl;
        exit(1);
    }

    matches_[level].clear();

    int matchsize;
    float y, sx, dx, ncc, prior;

    fin >> matchsize;
    for (int i = 0; i < matchsize; ++i) {
        fin >> y >> sx >> dx >> ncc >> prior;

        CMatch tmpmatch = CMatch(Point2f(sx, y), Point2f(dx, y), ncc, prior);
        matches_[level].push_back(tmpmatch);
    }
    fin.close();

    printf("Level %d: read %s done, %4d matches\n", level, listname.c_str(), matchsize);
}

void CMatchOrganizer::convertToDisps(const int src, vector<vector<float> > &disps, bool issaved) {
    printf("\nstart to convert matches to disparitys.\n");
    for (int level = fm_.max_level_ - 1; level >= 0; --level)
        convertToDisps(src, level, disps[level], issaved);
}

void CMatchOrganizer::convertToDisps(const int src, const int level, vector<float> &disps, bool issaved) {
    printf("level %d matches convert to disp\n", level);

    int width = fm_.photopair_->photos_[src].get_width(level);
    int height = fm_.photopair_->photos_[src].get_height(level);
    int msize = matches_[level].size();

    disps.resize(width * height, 0.0f);
    for (int i = 0; i < msize; ++i) {
        CMatch tmpmatch = matches_[level][i];
        Point2i tmppt = (src == 0) ? tmpmatch.srcpt : tmpmatch.dstpt;

        int tmpdisp = abs(tmpmatch.srcpt.x - tmpmatch.dstpt.x);
        int tmpindex = tmppt.y * width + tmppt.x;

        disps[tmpindex] = tmpdisp;
    }
    printf("convert to disp done\n");

    if (issaved == true) {
        printf("save level %d disparity.\n", level);
        saveDisps(src, level, "disp_", disps);
    }
}

void CMatchOrganizer::saveDisps(const int src, const int level, const string prefix, vector<float> &disps) {
    string folder = fm_.get_outfolder_();
    string filename = folder + "/" + type2string<int>(src) + "_" + prefix + type2string<int>(level);

    string savename = filename + ".list";
    fstream fout(savename.c_str(), ios::out);
    if (fout.is_open() == false) {
        std::cerr << "failed to open " << savename << std::endl;
        exit(1);
    }

    int width = fm_.photopair_->photos_[src].get_width(level);
    int height = fm_.photopair_->photos_[src].get_height(level);

    float maxdisp = 0.0f;
    for (int i = 0; i < width * height; ++i) {
        fout << disps[i];
        if (i && (i % width == 0))
            fout << endl;
        else
            fout << ' ';

        if (disps[i] > maxdisp)
            maxdisp = disps[i];
    }

    //debug for better disparity
    if (level == 0) maxdisp = 992;
    else if (level == 1) maxdisp = 497;
    else if (level == 2) maxdisp = 249;
    else if (level == 3) maxdisp = 121;

    string jpgname = filename + ".jpg";

    Mat dispmtx(height, width, CV_8UC1);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int index = y * width + x;
            int tmpdisp = disps[index];

            if (maxdisp != 0.0f)
                tmpdisp = tmpdisp * 255 / maxdisp;
            if (maxdisp < 255)
                tmpdisp = tmpdisp;
            dispmtx.at<unsigned char>(y, x) = tmpdisp;
        }
    }
    imwrite(jpgname, dispmtx);
}

//void CMatchOrganizer::compute3dPoints(const int src,  std::vector<std::vector<cv::Vec3f>>& points, bool issaved)
//{
//	printf("\nstart to compute disparitys to 3d points .\n");
//	for (int level = fm_.max_level_; level >= 0; -- level)
//		compute3dPoints(src, level, points[level], issaved);
//}
//
//void CMatchOrganizer::compute3dPoints(const int src, const int level, std::vector<cv::Vec3f>& points, bool issaved)
//{
//	printf("level %d 3d points computation ", level);
//
//	int width = fm_.photopair_.photos_[src].getWidth(level);
//	int height = fm_.photopair_.photos_[src].getHeight(level);
//	int scale = 1 << level;
//	int basedisp = (fm_.start_points_[0].x / scale) - (fm_.start_points_[1].x / scale);
//
//	cv::Point2i startpt = fm_.start_points_[src];
//	startpt = cv::Point2i(startpt.x / scale, startpt.y / scale);
//	
//	cv::Mat stereomtx = fm_.stereo_matrix_.clone();
//	for (int i = 0; i < 3; ++ i)
//		stereomtx.at<float>(i, 3) /= scale;
//
//	std::vector<unsigned char> image = fm_.photopair_.photos_[src].getImage(level);
//	std::vector<unsigned char> mask = fm_.photopair_.photos_[src].getMask(level);
//
//	int validnum = 0;
//	std::vector<cv::Vec3b> colors(width * height);
//	points.resize(width * height);
//	for (int y = 0; y < height; ++ y)
//	{
//		for (int x = 0; x < width; ++ x)
//		{
//			int index = y * width + x;
//
//			if (mask[index] != 255 || disps[index] == 0.0f) 
//			{
//				points_[level][index] = cv::Vec3f(PT_UNDEFINED, PT_UNDEFINED, PT_UNDEFINED );
//				colors[index] = cv::Vec3b(0, 0, 0);
//				continue;
//			}
//			
//			++ validnum; 
//			int sx = x + startpt.x;
//			int sy = y + startpt.y;
//			float tmpdisp = disps_[level][index] + basedisp;
//
//			cv::Vec4f tmpvec = cv::Vec4f(sx * 1.0f, sy * 1.0f, tmpdisp, 1.0f);
//			cv::Vec4f resvec ;
//			mul(stereomtx, tmpvec, resvec);
//
//			points_[level][index] = cv::Vec3f( resvec.val[0] / resvec.val[3],	resvec.val[1] / resvec.val[3], resvec.val[2] / resvec.val[3] );
//			colors[index] = cv::Vec3b( image[3 * index + 2], image[3 * index + 1], image[3 * index + 0]);
//		}
//	}
//	printf(" done. %6d 3d points.\n", validnum);
//
//	if (issaved)
//	{
//		string filename = "output/"+fm_.stereo_name_+"/pointcloud/pointcloud_" + type2string<int>(level) + ".ply";
//		printf("save level %d 3d points.\n", validnum );
//		writePLYDepthData(filename, points_[level], width, height, validnum, PT_HAS_COLOR, colors);
//	}
//}



