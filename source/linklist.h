#ifndef SUMMER_LINKLIST_H
#define SUMMER_LINKLIST_H

#include "../../Qing/qing_common.h"

struct CNode{

	CNode()
	{
		pixel = Point2i(-1, -1);
		pnext = NULL;
	}

	CNode(cv::Point2i tpixel, CNode * tnext)
	{
		pixel = tpixel;
		pnext = tnext;
	}

	Point2i pixel;
	CNode * pnext;
};

struct CLinklist{
	
	CLinklist()
	{
		head = tail = NULL;
		nodecnt = 0;
	}

	CNode * head,  *tail;
	int nodecnt ;
};

#endif