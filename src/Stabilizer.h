// Authors: Jan Bednarik, Jakub Kvita
// Date: 14.5.2014

#ifndef _STABILIZER_
#define _STABILIZER_

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace cv;

struct FramesOffset {	
	struct {
		int left;
		int right;
	} x;

	struct {
		int up;
		int down;
	} y;
};

class Stabilizer {
public:
	enum StabilizerType {GCBPM, SVD};	
	enum OutputFrameType {RAW, INTERSECTION, UNION};

	Stabilizer() {}
	virtual ~Stabilizer() {}

	virtual FramesOffset analyze(VideoCapture&) = 0;
	virtual void stabilize(Mat& src, Mat& dst) = 0;
};

#endif