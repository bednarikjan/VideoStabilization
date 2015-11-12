// Authors: Jan Bednarik, Jakub Kvita
// Date: 14.5.2014

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include "Stabilizer.h"
#include "StabilizerGCBPM.h"
#include "StabilizerSVD.h"
#include "Parameters.h"
#include "Exceptions.h"

using namespace std;
using namespace cv;

const int beforeAfterVideo = 0;
const int showtime = 0;

int main(int argc, char** argv) {	

	// process parameters
	Parameters params(argc, argv);		

	try {				
		params.process();				
		//params.debug_printParameters();
	} catch (exception &e) {
		cerr << e.what() << endl;
		exit(1);
	} 
	
	int totalFrames;
	int frameNum = 1;

	// capture for video in
	VideoCapture inVideo(params.inName);

	if(!inVideo.isOpened()) {
		cout  << "Could not open capture for " << params.inName << endl;
        return -1;
    }

	// get codec fourCC
    union { int codecInt; char codecChar[5]; } codec;
	codec.codecInt = static_cast<int>(inVideo.get(CV_CAP_PROP_FOURCC));
	codec.codecChar[4] = '\0';

    // debug print fourCC
    cout << "Input video codec fourCC = " << codec.codecChar << endl;

    // input and output video size
	Size inVideoSize = Size((int) inVideo.get(CV_CAP_PROP_FRAME_WIDTH),
                            (int) inVideo.get(CV_CAP_PROP_FRAME_HEIGHT));	
	Size outVideoSize;

	// pick and init stabilizer
	Stabilizer* stabilizer;

	switch(params.stabilizer) {
		case Stabilizer::GCBPM:
			stabilizer = new StabilizerGCBPM(inVideoSize.width, inVideoSize.height, params.method, params.bit, 
											params.N, params.offset, params.subRegionsX, params.subRegionsY, 
											params.damping, params.leaveOldFrames);
			break;

		case Stabilizer::SVD:
			stabilizer = new StabilizerSVD(params.method, params.featurePointsCount);
			break;

		default:
			break;
	}	

	// estimate output video size	
	try {
		FramesOffset frOffset;

		switch (params.method) {
		case Stabilizer::RAW:	
			outVideoSize = inVideoSize;
			break;

		case Stabilizer::INTERSECTION:	
			frOffset = stabilizer->analyze(inVideo);
			outVideoSize.width = inVideoSize.width - frOffset.x.left - frOffset.x.right;
			outVideoSize.height = inVideoSize.height - frOffset.y.up - frOffset.y.down;
			break;

		case Stabilizer::UNION:	
			frOffset = stabilizer->analyze(inVideo);
			outVideoSize.width = inVideoSize.width + frOffset.x.left + frOffset.x.right;
			outVideoSize.height = inVideoSize.height + frOffset.y.up + frOffset.y.down;
			break;		
		}
	} catch (exception &e) {
		cerr << e.what();
		exit(1);
	}
	
	// writer for video out
	VideoWriter outVideo;

	if(beforeAfterVideo) {
		outVideo.open(params.outName, codec.codecInt, inVideo.get(CV_CAP_PROP_FPS), 
			Size(2*inVideoSize.width, inVideoSize.height), true);
	} else {
		outVideo.open(params.outName, codec.codecInt, inVideo.get(CV_CAP_PROP_FPS), 
			outVideoSize, true);
	}
	
	totalFrames = (int)inVideo.get(CV_CAP_PROP_FRAME_COUNT);

    if (!outVideo.isOpened()) {
		cout  << "Could not open the output video for write: " << params.outName << endl;
        return -1;
    }	

	Mat doubleFrame(Size(2*inVideoSize.width, inVideoSize.height), CV_8UC3);
	Mat inFrame;	
	Mat outFrame(outVideoSize, CV_8UC3, Scalar(0,0,0,0));		

	// stabilize each inFrame		
	while(true) {						
		inVideo >> inFrame;

		// end of video
        if(inFrame.empty()) {
            cout << "End of video! bye bye" << endl;
            break;
        }        		

		cout << "frame " << frameNum++ << "/" << totalFrames << "\r";

		// stabilize frame
		stabilizer->stabilize(inFrame, outFrame);

        // save stabilized frame to output video
		if(beforeAfterVideo) {
			inFrame.copyTo(doubleFrame(Rect(0, 0, inFrame.cols, inFrame.rows)));
			outFrame.copyTo(doubleFrame(Rect(inFrame.cols, 0, inFrame.cols, inFrame.rows)));
			outVideo << doubleFrame;
		} else {
			outVideo << outFrame;
		}

		if(showtime) {
			namedWindow("video", WINDOW_AUTOSIZE);
			imshow("video", outFrame);
			if (char(waitKey(10)) == 'q')
				break;
		}				
	}
}