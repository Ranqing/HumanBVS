//CMatchOrganizer: a class to organizer matches: 2points, 2disparity

#ifndef SUMMER_MATCH_ORGANIZER_H
#define SUMMER_MATCH_ORGANIZER_H

#include "match.h"

class CFindMatch;

class CMatchOrganizer
{
public:
	CMatchOrganizer(CFindMatch& findmatch);
	virtual ~CMatchOrganizer();

	void init(void) ;
	void optimize(const int level);

	void saveMatches(const string prefix);
	void saveMatches(const string prefix, const int level);
	void readMatches(const string prefix);
	void readMatches(const string prefix, const int level);

	void convertToDisps(const int src, vector<vector<float>>& disps, bool issaved);
	void convertToDisps(const int src, const int level, vector<float>& disps, bool issaved);
	void saveDisps(const int src, const int level, const string prefix, vector<float>& disps);
	
	//widths of grids std::vector<int> m_gwidths;
	//heights of grids std::vector<int> m_gheights;

	vector<vector<CMatch>> matches_;		//all the matches in the pyramid
	
protected:
	CFindMatch& fm_;

};

#endif //matchorganzier.h