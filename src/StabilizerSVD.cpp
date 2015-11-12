// Authors: Jan Bednarik, Jakub Kvita
// Date: 14.5.2014

#include "StabilizerSVD.h"

using namespace std;
using namespace cv;

FramesOffset StabilizerSVD::analyze(VideoCapture& vc) {
	Mat frame;
	FramesOffset frOff = {{0,0},{0,0}};			

	// push first transform that does nothing
	transforms.push_back(Mat::eye(3, 3, CV_32F));

	//debug
	int i = 0;
	int frames = (int)vc.get(CV_CAP_PROP_FRAME_COUNT);

	for(vc >> frame; !frame.empty(); vc >> frame) {
		updateTransform(frame);
		transforms.push_back(transforms.back() * newTransform);
		cout << "frame " << i++ << "/" << frames << "\r";		
	}	
	transforms.erase(transforms.begin());

	analyzed = true;

	// set the reading head to the first frame in video capture
	vc.set(CV_CAP_PROP_POS_FRAMES, 0.0);

	// simulate camera linear movement
	int step = 2*(int)vc.get(CV_CAP_PROP_FPS);
	int fl, fr;
	for(fl = 0, fr = step - 1; fl < transforms.size(); fl += step, fr += step) {
		if(fr >= transforms.size()) {
			fr = (int)transforms.size() - 1;
			step = fr - fl;			
			if(step == 0) break;
		}
		
		Mat tDiff = (transforms[fr](Rect(2, 0, 1, 2)) - transforms[fl](Rect(2, 0, 1, 2))) * -1.0 / (float)step;		
		for(int i = 0; i < (transforms.size() - fl); i++) {
			transforms[fl + i](Rect(2, 0, 1, 2)) += (tDiff * i);
		}
	}

	return frOff;
}

void StabilizerSVD::updateTransform(Mat &frame) {
	if(!firstFrame) {
		vector<Point2f> newPoints;
		Mat newFrameGray;

		cvtColor(frame, newFrameGray, CV_BGR2GRAY);
	
		// get new points in new image usin LKT
		vector<uchar> status;
		vector<float> err; 
		calcOpticalFlowPyrLK(oldFrameGray, newFrameGray, oldPoints, newPoints, status, err, 
							winSize, 3, termcrit, 0, 0.001);

		// squeeze both point vectors (disregard points not found in new frame)
		int k, i;
		for(i = 0, k = 0; i < newPoints.size(); i++) {        
			if(status[i]) {
				newPoints[k] = newPoints[i];
				oldPoints[k] = oldPoints[i];
				k++;			
			}		        
		}	
		newPoints.resize(k);
		oldPoints.resize(k);		

		// using SVD get translation (T) and rotation (R) matrix
		// centroids
		Point2f cOldPoint(0.f, 0.f);
		Point2f cNewPoint(0.f, 0.f);

		for(int i = 0; i < oldPoints.size(); i++) {
			cOldPoint += oldPoints[i];
			cNewPoint += newPoints[i];
		}
		cOldPoint *= 1.f / (float)oldPoints.size();
		cNewPoint *= 1.f / (float)newPoints.size();		

		Mat H(2,2, CV_32F, Scalar::all(0));	

		for (int i = 0; i < oldPoints.size(); i++) {
			Point2f pa = oldPoints[i] - cOldPoint;
			Point2f pb = newPoints[i] - cNewPoint;
		
			H.at<float>(0, 0) += pa.x * pb.x;
			H.at<float>(0, 1) += pa.x * pb.y;
			H.at<float>(1, 0) += pa.y * pb.x;
			H.at<float>(1, 1) += pa.y * pb.y;
		}

		// singular value decomposition
		cv::SVD svd(H);

		// rotation
		Mat R = svd.vt.t() * svd.u.t();

		// dealing with special reflection case
		if (determinant(R) < 0) {			
			R.at<float>(0, 1) *= -1;
			R.at<float>(1, 1) *= -1;
		}

		// translation
		Mat cOld = (Mat_<float>(2,1) << cOldPoint.x, cOldPoint.y);
		Mat cNew = (Mat_<float>(2,1) << cNewPoint.x, cNewPoint.y);	
		Mat T = -R * cOld + cNew;	

		// get new transform matrix
		Mat t(2,3,CV_32F,Scalar::all(0));
		R.copyTo(t(Rect(0,0,2,2)));
		T.copyTo(t(Rect(2,0,1,2)));
		invertAffineTransform(t, t);		

		// save new transform
		t.copyTo(newTransform(Rect(0, 0, 3, 2)));		

		// new frame just got old
		newFrameGray.copyTo(oldFrameGray);

		// if there are too little points, generate new set
		if(newPoints.size() < 0.75 * foundFeaturePoints) {			
			goodFeaturesToTrack(newFrameGray, oldPoints, featurePointsCount, 0.01, 10);
			cornerSubPix(newFrameGray, oldPoints, subPixWinSize, Size(-1,-1), termcrit);
			foundFeaturePoints = (int)oldPoints.size();
			//cout << "generating new feature points!" << endl;
		} else {
			oldPoints = newPoints;			
		}		
		newPoints.clear();

	} else {
		Mat frameGray;
		cvtColor(frame, frameGray, CV_BGR2GRAY);

		// save frame
		frameGray.copyTo(oldFrameGray);

		// get feature points (corners)
		goodFeaturesToTrack(frameGray, oldPoints, featurePointsCount, 0.01, 10);
		cornerSubPix(frameGray, oldPoints, subPixWinSize, Size(-1,-1), termcrit);
		foundFeaturePoints = (int)oldPoints.size();

		firstFrame = false;
	}
}

void StabilizerSVD::stabilize(Mat& src, Mat& dst) {
	Mat t;
	
	if(analyzed) {
		transforms.front()(Rect(0, 0, 3, 2)).copyTo(t);
		transforms.erase(transforms.begin());
	} else {
		updateTransform(src);		
		lastTransform = lastTransform * newTransform;
		lastTransform(Rect(0, 0, 3, 2)).copyTo(t);
	}

	warpAffine(src, dst, t, dst.size());
}
