// Authors: Jan Bednarik, Jakub Kvita
// Date: 14.5.2014

#ifndef _STABILIZER_SVD_
#define _STABILIZER_SVD_

#include <opencv2/video/tracking.hpp>
#include <iostream>
#include "Stabilizer.h"

using namespace std;
using namespace cv;

class StabilizerSVD : public Stabilizer {
public:
	StabilizerSVD(OutputFrameType method, int _featurePointsCount) : outFrType(method), firstFrame(true), 
		analyzed(false), featurePointsCount(_featurePointsCount) {
		
		// initialization (hard coded by now)	
		termcrit.type = CV_TERMCRIT_ITER|CV_TERMCRIT_EPS; 
		termcrit.maxCount = 20;
		termcrit.epsilon = 0.03;
		subPixWinSize = Size(10,10);
		winSize = Size(31,31);		

		newTransform = Mat::eye(3, 3, CV_32F);
		lastTransform = Mat::eye(3, 3, CV_32F);
	};

	virtual ~StabilizerSVD() {};

	FramesOffset analyze(VideoCapture& vc);
	void stabilize(Mat& src, Mat& dst);

private:		
	void updateTransform(Mat &);

	OutputFrameType outFrType;
	bool firstFrame;	
	bool analyzed;

	// settings for finding feature points
	TermCriteria termcrit;
	Size subPixWinSize;
	Size winSize;

	int featurePointsCount;
	int foundFeaturePoints;
	Mat oldFrameGray;

	// last frame's feature points
	vector<Point2f> oldPoints;

	Mat newTransform;
	Mat lastTransform;
	vector<Mat> transforms;
};

#endif