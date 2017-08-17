//for cross-check

#ifndef SUMMER_CMHASH_H
#define SUMMER_CMHASH_H

#include "match.h"
#include "hash/hash.h"

typedef Mhash<cv::Point2i, CMatchValue, Hashfn1> MhashMap;

class CMhash
{
public:
	CMhash(int nh = 0, int nm = 0);
	~CMhash();	
	
	void init(int nh, int nm);
	
	bool isexist(const Point2i tkey, const CMatchValue tvalue);
	bool maxstore(const Point2i tkey, const CMatchValue tvalue, const int maxcount);

	Point2i get_akey(int i);
	CMatchValue get_minvalue(const cv::Point2i tkey);

	int store(const Point2i& tkey, const CMatchValue& tvalue);
	int erase(const Point2i& tkey, const CMatchValue& tvalue);
	int count(const Point2i& tkey);
	int getinit(const Point2i& tkey);
	int getnext(CMatchValue& tvalue);

	void copyfrom(const int src, const vector<CMatch>& matches);    //copy matches to CMhash
	void copyto(const int src, vector<CMatch>& matches);            //copy CMhas to matches

	Point2i * pkeys_;
	MhashMap * pmhash_;
	int numofkeys_;
	int numofvalues_;
	int capacity_;

};

#endif