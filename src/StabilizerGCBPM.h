// Authors: Jan Bednarik, Jakub Kvita
// Date: 14.5.2014

#ifndef _STABILIZER_GCBPM_
#define _STABILIZER_GCBPM_

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <list>
#include "Stabilizer.h"

using namespace cv;
using namespace std;

class StabilizerGCBPM : public Stabilizer {
public:	
	StabilizerGCBPM(int _width, int _height, OutputFrameType _outFrType,
		int _bit, int _N, int _offset, 
		int _subRegionsX, int _subRegionsY, double _damping,
		bool _leaveOldFrames) {

		width = _width;
		height = _height;
		outFrType = _outFrType;

		bit = _bit;
		N = _N;
		offset = _offset;
		subRegionsX = _subRegionsX;
		subRegionsY = _subRegionsY;
		damping = _damping;		
		leaveOldFRames = _leaveOldFrames;

		// if defaults cannot be used, count optimal N and offset
		srWidth = width / subRegionsX;
		srHeight = height / subRegionsY;
		double minSrSize = srHeight < srWidth ? srHeight : srWidth;
		double NOffRate = (double)N / (double)offset;

		if((2 * offset + N) > minSrSize) {
			N = (int)(minSrSize / (1 + 2 / NOffRate));
			offset = (int)(N / NOffRate);
		}		

		firstFrame = true; 
		gmX = 0;
		gmY = 0;
		igmX = 0;
		igmY = 0;

		crop.x = 0; crop.y = 0; crop.width = width; crop.height = height;
		baseFramePos.x = 0; baseFramePos.y = 0;
		analyzed = false;
	}

	FramesOffset analyze(VideoCapture &vc);
	void stabilize(Mat& src, Mat& dst);

private:
	int width;
	int height;
	OutputFrameType outFrType;

	// settings
	int bit;
    int N;
    int offset;
    int subRegionsX;
    int subRegionsY;
	double damping;
	bool leaveOldFRames;

	// subregions	
	int srWidth;
	int srHeight;

	bool firstFrame;
	Mat oldFrameGray;
	Mat oldBP;			// old bit plane
	
	// global motion vector
	int gmX;
	int gmY;

	// integrated global motion vector
	int igmX;
	int igmY;

	// sequence of global motion vectors to be applied on subsequent frames
	list<pair<int, int> > gmList;

	// INTERSECTION - crop coordinates and size
	Rect crop;

	// UNION - coordinates of upper-left corner of first frame whithin big output frame
	Point baseFramePos;

	bool analyzed;

	void makeBitPlane(Mat& src, Mat& dst, int bit);

	int median(std::vector<int>& v) {
		std::vector<int>::iterator first = v.begin();
		std::vector<int>::iterator last = v.end();
		std::vector<int>::iterator middle = first + (last - first) / 2;
		std::nth_element(first, middle, last);
		
		return *middle;
	}

	void updateGlobalMotion(Mat& frame);

	// debug
	void debug_printLocalMotions(std::vector<int>& lmvX, std::vector<int>& lmvY, int srX, int srY);
};

#endif