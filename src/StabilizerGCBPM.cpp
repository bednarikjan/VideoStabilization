// Authors: Jan Bednarik, Jakub Kvita
// Date: 14.5.2014

#include <iostream>
#include "StabilizerGCBPM.h"
#include "Exceptions.h"

/**
* Analyzes the video and stores the global motion
* vector (with regards to the previous frame) for each
* frame. Finds the most offset to left/right in x and
* up/down in y axis.
*
* @return pair<(-xMax, xMax), (-yMax, yMax)>
*/
FramesOffset StabilizerGCBPM::analyze(VideoCapture &vc) {
	Mat frame;
	FramesOffset frOff = {{0,0},{0,0}};

	//debug
	int i = 0;
	int frames = (int)vc.get(CV_CAP_PROP_FRAME_COUNT);

	for(vc >> frame; !frame.empty(); vc >> frame) {
		updateGlobalMotion(frame);
		gmList.push_front(pair<int,int>(igmX, igmY));

		if      (igmX < -frOff.x.left) frOff.x.left = -igmX;
		else if (igmX > frOff.x.right) frOff.x.right = igmX;
		if      (igmY < -frOff.y.up) frOff.y.up = -igmY;
		else if (igmY > frOff.y.down) frOff.y.down = igmY;

		// debug
		cout << "frame " << i++ << "/" << frames << "\r";		
	}	

	if(outFrType == INTERSECTION) {
		crop.x = frOff.x.right;
		crop.y = frOff.y.down;
		crop.width  = width  - frOff.x.left - frOff.x.right;
		crop.height = height - frOff.y.up - frOff.y.down;
	} else if (outFrType == UNION) {
		baseFramePos.x = frOff.x.left;
		baseFramePos.y = frOff.y.up;
	}	

	analyzed = true;

	// set the reading head to the first frame in video capture
	vc.set(CV_CAP_PROP_POS_FRAMES, 0.0);

	return frOff;
}

void StabilizerGCBPM::updateGlobalMotion(Mat& frame) {
	if(!firstFrame) {
		// resulting stabilized image
		//Mat stabImg(img1.rows, img1.cols, CV_8UC3, Scalar(0));

		// new bit plane
		Mat newBP(frame.rows, frame.cols, CV_8UC1, Scalar(0));

		// new grayscale frame
		Mat newFrameGray;		
		cvtColor(frame, newFrameGray, CV_BGR2GRAY);		

		// new bitplane
		makeBitPlane(newFrameGray, newBP, bit);				

		// vector of local motion vectors (x and y coordinates separately)
		std::vector<int> lmvX;
		std::vector<int> lmvY;

		// for each subregion find local motion estimation
		for (int srY = 0; srY < subRegionsY; srY++) {
			int bpNewY = srY * srHeight + (srHeight / 2 - N / 2);
			int bpOldY = bpNewY - offset;

			for (int srX = 0; srX < subRegionsX; srX++) {
				int bpNewX = srX * srWidth + (srWidth / 2 - N / 2);
				int bpOldX = bpNewX - offset;

				// continuously storing minimal sum of xored old and ne bit-planes and related offset coordinates
				int minXorSum = N*N;
				int lmX = 0;
				int lmY = 0;

				// for each offset get sum of xor between moved old and new bit-plane roi (N x N)
				Mat newBPSearchWindow(newBP, Rect(bpNewX, bpNewY, N, N));

				for (int y = 0; y < 2 * offset; y++) {
					for (int x = 0; x < 2 * offset; x++) {
						Mat xorRes(N, N, CV_8UC1);
						Scalar xorSum;
						bitwise_xor(Mat(oldBP, Rect(bpOldX + x, bpOldY + y, N, N)), // TODO mozna udelat rucnim pruchodem a ronvnou sumovat
										newBPSearchWindow,
										xorRes);
						xorSum = sum(xorRes);

						if (xorSum[0] < minXorSum) {
							minXorSum = (int) xorSum[0];
							lmX = -offset + x;
							lmY = -offset + y;
						}
					}
				}

				lmvX.push_back(lmX);
				lmvY.push_back(lmY);
			}
		}
		
		//debug_printLocalMotions(lmvX, lmvY, subRegionsX, subRegionsY);

		// get global motion vector (median of new local motion vectors and last global motion vector)
		lmvX.push_back(gmX);
		lmvY.push_back(gmY);

		// update global motion vector
		gmX = median(lmvX);
		gmY = median(lmvY);

		// get integrated global motion vector
		igmX = (int) (damping * igmX + gmX); 
		igmY = (int) (damping * igmY + gmY);				

		// new frame jut got old
		oldFrameGray = newFrameGray.clone();
		oldBP = newBP.clone();

	// prepare first frame
	} else {		
		firstFrame = false;			
		cvtColor(frame, oldFrameGray, CV_BGR2GRAY);	
		oldBP.create(frame.rows, frame.cols, CV_8UC1);
		makeBitPlane(oldFrameGray, oldBP, bit);		
	}	
}

void StabilizerGCBPM::stabilize(Mat& src, Mat& dst) {	
	if(analyzed) {
		igmX = gmList.back().first;
		igmY = gmList.back().second;
		gmList.pop_back();
	} else {
		updateGlobalMotion(src);
	}	
	
	// transform matrix (translate)
	Mat tMat = (Mat_<float>(2, 3) <<
					1, 0, igmX,
					0, 1, igmY);

	// stabilize frame
	switch(outFrType) {
		case RAW:
			warpAffine(src, dst, tMat, Size(src.cols, src.rows));
			break;

		case INTERSECTION:
			warpAffine(src, src, tMat, Size(src.cols, src.rows));
			dst = src(crop);
			break;

		case UNION:
			if(!leaveOldFRames) dst.setTo(0);
			src.copyTo(dst(Rect(baseFramePos.x + igmX, baseFramePos.y + igmY, src.cols, src.rows)));
			break;
	}		
}


void StabilizerGCBPM::makeBitPlane(Mat& src, Mat& dst, int bit) {
    if(bit > 7) bit = 7;
    if(bit < 0) bit = 0;

    for(int y = 0; y < src.rows; y++) {
        for(int x = 0; x < src.cols; x++) {
            uchar curBit = (src.at<uchar>(y, x) >> 7) & 1;

            for(int i = 6; i > (bit - 1); i--) {
                curBit ^= (src.at<uchar>(y, x) >> i) & 1;
            }

            dst.at<uchar>(y, x) = curBit;
        }
    }
}

void StabilizerGCBPM::debug_printLocalMotions(std::vector<int>& lmvX, std::vector<int>& lmvY, int srX, int srY) {
    for(int y = 0; y < srY; y++) {
        for(int x = 0; x < srX; x++) {
            std::cout << "Local Motion vector for subregion (" << x <<  ", " << y << "):" << std::endl;
            std::cout << "[" << lmvX[srX * y + x] << ", " << lmvY[srX * y + x] << "]" << std::endl << std::endl;
        }
    }
}
