#include "basic.h"
#include "optimizer.h"
#include "findmatch.h"
#include "photopair.h"

void COptimizer::showNoMatchedPixels(const int src, const int level, const vector<float>& rawdisp, vector<CLinklist>& regions)
{
	int width = fm_.photopair_->photos_[src].get_width(level);
	int height = fm_.photopair_->photos_[src].get_width(level);

	string folder = fm_.get_outfolder_();

	Mat dispmtx;
	PixelsVector2Mat<float>(rawdisp, width, height, 1, dispmtx);
	Mat rgb_dispmtx;
	cvtColor(dispmtx, rgb_dispmtx, CV_GRAY2BGR);

	for (int i = 0; i < regions.size(); ++ i)
	{
		CNode *tmpnode = regions[i].head;
		while (tmpnode != NULL)
		{
			int x = (tmpnode->pixel).x;
			int y = (tmpnode->pixel).y;

			rgb_dispmtx.at<cv::Vec3b>(y, x) = cv::Vec3b(255, 0, 0);
			tmpnode = tmpnode->pnext;
		}
	}

	string savename = folder + "/" + type2string<int>(src) + "_nomatched_" + type2string<int>(level) + ".jpg";
	imwrite(savename, rgb_dispmtx);
}

void COptimizer::findNoMatchedPixels(const vector<float>& rawdisp, const vector<unsigned char>& rawmask,
									 const int width, const int height, const int maxnum, int& totalnum,
									 vector<CLinklist>& regions)
{
	vector<bool> isaccess(width * height, false);

	for (int y = 0; y < height; ++ y)
	{
		for (int x = 0; x < width; ++ x)
		{
			int index = y * width + x;

			if (rawmask[index] != 255 || rawdisp[index] != 0.0f || isaccess[index] == true) 
				continue;

			if ( rawdisp[index] == 0.0f && isaccess[index] == false )
			{
				CLinklist tmpregion;
				CNode * tmpnode = new CNode(Point2i(x, y), NULL);

				CNode * qhead = tmpnode;
				CNode * qtail = tmpnode;
				while ( qhead != NULL )
				{
					Point2i tmppixel = qhead -> pixel;

					int tmpindex = tmppixel.y * width + tmppixel.x;

					if ( rawmask[tmpindex] == 255 && rawdisp[ tmpindex ] == 0.0f && isaccess[ tmpindex ] == false )
					{
						isaccess[ tmpindex ] = true;

						if ( tmpregion.head == NULL )
						{
							tmpregion.head = tmpregion.tail = qhead;
							tmpregion.nodecnt = 1;
						}
						else
						{
							tmpregion.tail -> pnext = qhead;
							tmpregion.tail = tmpregion.tail -> pnext;
							++ tmpregion.nodecnt ;
						}

						if ( tmppixel.y >= 1 )
						{
							CNode * newnode = new CNode( Point2i(tmppixel.x , tmppixel.y - 1), NULL );
							qtail -> pnext = newnode;
							qtail = qtail -> pnext;
						}

						if ( tmppixel.y <= ( height - 2 ) )
						{
							CNode * newnode = new CNode( cv::Point2i(tmppixel.x, tmppixel.y + 1), NULL );
							qtail -> pnext = newnode;
							qtail = qtail -> pnext;
						}

						if ( tmppixel.x >= 1 )
						{
							CNode* newnode = new CNode( cv::Point2i(tmppixel.x - 1, tmppixel.y), NULL );
							qtail -> pnext = newnode;
							qtail = qtail -> pnext;
						}

						if ( tmppixel.x <= ( width - 2) )
						{
							CNode* newnode = new CNode( cv::Point2i(tmppixel.x + 1, tmppixel.y), NULL );
							qtail -> pnext = newnode;
							qtail = qtail -> pnext;
						}
						qhead = qhead -> pnext;
					}
					else   // rawdisp[ tmpindex ] != 0.0f || isaccess[ tmpindex ] == false
					{
						CNode * delnode = qhead;
						qhead = qhead -> pnext;
						delete delnode;
					}

				} //end of while(qhead )

				if ( tmpregion.tail != NULL )
					tmpregion.tail -> pnext = NULL;

				if ( tmpregion.nodecnt != 0 && tmpregion.nodecnt < maxnum )
				{
					regions.push_back(tmpregion);
					totalnum += tmpregion.nodecnt;
				}
				else
				{
					CNode * delnode  = tmpregion.head;

					while ( delnode != NULL )
					{
						Point2i delpixel = delnode -> pixel;
						isaccess[ delpixel.y * width + delpixel.x ] = true;

						CNode * tmpdelnode = delnode;
						delnode = delnode -> pnext;
						delete tmpdelnode;
					}

					tmpregion.head = tmpregion.tail = NULL;
					tmpregion.nodecnt = 0;
				}
			}// end of if(rawdisp[] == 0.0 && isaccess[] == false)
		} 
	}
}

void COptimizer::findMatchedPixels(const vector<float>& rawdisp, const vector<unsigned char>& rawmask,
                                   const int width, const int height, const int maxnum, int& totalnum,
                                   vector<CLinklist>& regions)
{
	vector<bool> isaccess(width * height, false);
	totalnum = 0;

	for (int y = 0; y < height; ++ y)
	{
		for (int x = 0; x < width; ++ x)
		{
			int index = y * width + x;
			if (rawmask[index] != 255 || rawdisp[index] == 0.0f || isaccess[index] == true)
				continue;

			if (rawdisp[index] != 0.0f && isaccess[index] == false)
			{
				CLinklist tmpregion;
				CNode * tmpnode = new CNode(Point2i(x, y), NULL);

				CNode * qhead = tmpnode;
				CNode * qtail = tmpnode;
				while ( qhead )
				{
					int tmpy =  qhead->pixel.y;
					int tmpx =  qhead->pixel.x;
					int tmpindex = tmpy * width + tmpx;

					if ( rawmask[ tmpindex ] != 255 || rawdisp[tmpindex] == 0.0f || isaccess[tmpindex] == true)
					{
						CNode* delnode = qhead;
						qhead = qhead -> pnext;
						delete delnode;

						continue;
					}

					if ( rawdisp[ tmpindex ] != 0.0f && isaccess[ tmpindex ] == 0)
					{
						isaccess[ tmpindex ] = true;
						if ( tmpregion.head == NULL )
						{
							tmpregion.head = tmpregion.tail = qhead;
							tmpregion.nodecnt = 1;
						}
						else
						{
							tmpregion.tail -> pnext = qhead;
							tmpregion.tail = tmpregion.tail -> pnext;
							++ tmpregion.nodecnt;
							/*if (tmpregion.nodecnt > maxnum)
							break;*/
						}

						if ( tmpy >= 1 )
						{
							CNode* newnode = new CNode(Point2i(tmpx, tmpy - 1), NULL);
							qtail -> pnext = newnode;
							qtail = qtail -> pnext;
						}

						if ( tmpy <= height - 2 )
						{
							CNode* newnode = new CNode(Point2i(tmpx, tmpy + 1), NULL);
							qtail -> pnext = newnode;
							qtail = qtail -> pnext;
						}

						if ( tmpx >= 1 )
						{
							CNode* newnode = new CNode(Point2i(tmpx - 1, tmpy), NULL);
							qtail -> pnext = newnode;
							qtail = qtail -> pnext;
						}

						if ( tmpx <= width - 2 )
						{
							CNode* newnode = new CNode(Point2i(tmpx + 1, tmpy), NULL);
							qtail -> pnext = newnode;
							qtail = qtail -> pnext;
						}
						qhead = qhead -> pnext;
					}
				} //end of while(qhead )

				if (tmpregion.tail != NULL)
					tmpregion.tail -> pnext = NULL;

				if (tmpregion.nodecnt != 0 && tmpregion.nodecnt <= maxnum)
				{
					regions.push_back(tmpregion);
					totalnum += tmpregion.nodecnt;
				}
				else
				{
					CNode * delnode = tmpregion.head;
					while (delnode != NULL)
					{
						Point2i delpixel = delnode->pixel;
						isaccess[delpixel.y * width + delpixel.x ] = true;

						CNode * tmpdelnode = delnode;
						delnode = delnode -> pnext;
						delete tmpdelnode;
					}
					tmpregion.head = tmpregion.tail = NULL;
					tmpregion.nodecnt = 0;
				}

			}
		} // end of for(x)
	} // end of for(y)
}

void COptimizer::erodeRawDisp(vector<float>& rawdisp, vector<unsigned char>& rawmask, const int ksize, const int w, const int h)
{
	Mat mskmtx, dispmtx;
	PixelsVector2Mat<unsigned char>(rawmask, w, h, 1, mskmtx);
	PixelsVector2Mat<float>(rawdisp, w, h, 1, dispmtx);	

	int erosion_elem = 0;
	int erosion_size = ksize;
	int erosion_type;
	if( erosion_elem == 0 ){ erosion_type = cv::MORPH_RECT; }
	else if( erosion_elem == 1 ){ erosion_type = cv::MORPH_CROSS; }
	else if( erosion_elem == 2) { erosion_type = cv::MORPH_ELLIPSE; }

	Mat element = getStructuringElement( erosion_type,
		Size( 2*erosion_size + 1, 2*erosion_size+1 ),
		Point( erosion_size, erosion_size ) );
	/// Apply the dilation operation

	Mat emskmtx;
	erode( mskmtx, emskmtx, element );
	
	rawmask.clear();
	rawmask.resize(w*h,0);
	Mat2PixelsVector<unsigned char>(emskmtx, rawmask);

	for (int i = 0; i < w*h; ++ i)
		if (rawmask[i] != 255 )
			rawdisp[i] = 0.0f;	

	/*cv::Mat edispmtx;
	PixelsVector2Mat<float>(rawdisp, w, h, 1, edispmtx);	
	cv::imshow("erode disp", edispmtx);
	cv::waitKey(0);
	cv::destroyWindow("erode disp");*/
}

void COptimizer::debug()
{
	int maxlevels = fm_.max_level_;
	int src = 0;

	for (int level = maxlevels - 1; level >= 0; -- level)
	{
		printf("\nstart to optimize disparity in level %d\n", level);

		//optimize disp in each level
		int h = fm_.photopair_->photos_[0].get_height(level);
		int w = fm_.photopair_->photos_[0].get_width(level);
		int ksize = fm_.get_nccwsize_(level);

		vector<float>& rawdisp = fm_.disps_[level];
		vector<unsigned char>& rawmask = fm_.photopair_->photos_[0].get_mask(level);
		
		erodeRawDisp(rawdisp, rawmask, ksize, w, h);
		fm_.matchset_->saveDisps(src, level, "erode_disp_", rawdisp);

		fill(level, rawdisp);
		fm_.matchset_->saveDisps(src, level, "fill_disp_", rawdisp);

		//smooth(level, rawdisp);
	}	
}

void COptimizer::fill(const int level, vector<float>& disp)
{
	int width = fm_.photopair_->photos_[0].get_width(level);
	int height = fm_.photopair_->photos_[0].get_height(level);

	vector<float>& rawdisp = fm_.disps_[level];
	vector<unsigned char>& rawmask = fm_.photopair_->photos_[0].get_mask(level);

	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			int index = y * width + x;
			if (rawmask[index]==255 && rawdisp[index]==0)
			{
				rawdisp[index] = rawdisp[index-1];
			}
		}

		for (int x = width - 1; x >= 0; --x)
		{
			int index = y * width + x;
			if (rawmask[index]==255 && rawdisp[index]==0)
			{
				rawdisp[index] = rawdisp[index+1];
			}
		}
	}
}

void COptimizer::smooth(const int level, vector<float>& disp)
{
	int width = fm_.photopair_->photos_[0].get_width(level);
	int height = fm_.photopair_->photos_[0].get_height(level);

	vector<float>& rawdisp = fm_.disps_[level];
	vector<unsigned char>& rawmask = fm_.photopair_->photos_[0].get_mask(level);

	int maxdisp = width * 0.75;
}


//void COptimizer::buildErodeMask(const std::string maskfile)
//{
//	erodemasks.resize(fm_.max_level_);
//	erodevalids.resize(fm_.max_level_);
//
//	cv::Mat maskmtx, binarymtx;
//
//	maskmtx = cv::imread(maskfile);
//	if (maskmtx.data == NULL)
//	{
//		std::cerr << "failed to read in " << maskfile << std::endl;
//		return ;
//	}
//
//	if ( maskmtx.channels() == 1)
//	{
//		cv::threshold(maskmtx, binarymtx, 127, 255, cv::THRESH_BINARY);
//	}
//	else
//	{
//		cv::Mat graymtx;
//		cv::cvtColor(maskmtx, graymtx, CV_BGR2GRAY);
//		cv::threshold(graymtx, binarymtx, 127, 255, cv::THRESH_BINARY);
//	}
//
//	Mat2PixelsVector<unsigned char>(binarymtx, erodemasks[0]);
//	erodevalids[0] = countNonZero(binarymtx); 
//
//	int width, height, upwidth, upheight;
//	for (int level = 1; level < fm_.max_level_; ++ level)
//	{
//		width = maskmtx.cols / (1 << level);
//		height = maskmtx.rows / (1 << level);
//		upwidth = width * 2;
//		upheight = height * 2;
//
//		const int size =  width * height;
//		int tmpvalidnum = 0;
//
//		erodemasks[level].resize(size);
//		for (int y = 0; y < height; ++y)
//		{
//			const int ys[2] = {2 * y, min(upheight - 1, 2 * y + 1)};
//
//			for (int x = 0; x < width; ++x)
//			{
//				const int xs[2] = {2 * x, min(upwidth - 1, 2 * x + 1)};
//
//				int in = 0;
//				int out = 0;
//
//				for (int j = 0; j < 2; ++j)
//				{
//					for (int i = 0; i < 2; ++i)
//					{
//						const int index = ys[j] * upwidth + xs[i];
//						if (erodemasks[level - 1][index])
//							in++;
//						else
//							out++;
//					}
//				}
//
//				const int index = y * width + x;
//
//				//if (out <= in)
//				if (0 < in)
//				{
//					erodemasks[level][index] = (unsigned char)255;
//					++ tmpvalidnum; 
//				}
//				else
//					erodemasks[level][index] = (unsigned char)0;
//			}
//		}
//		erodevalids[level] = tmpvalidnum;
//		//debug
//		char mskname[1024];
//		sprintf_s(mskname, "%s/additional/erode_H01_mask_%d.jpg", fm_.get_outfolder_().c_str(), level);
//		printf("%s\n", mskname);
//		cv::Mat tmskmtx;
//		PixelsVector2Mat(erodemasks[level], width, height, 1, tmskmtx);
//		cv::imwrite(mskname, tmskmtx);
//	}
//}


