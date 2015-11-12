// Authors: Jan Bednarik, Jakub Kvita
// Date: 14.5.2014

#ifndef _PARAMETERS_
#define _PARAMETERS_

#include <string>
#include "Stabilizer.h"
#include "StabilizerGCBPM.h"

using namespace std;

class Parameters {

public:
	Parameters(int _argc, char** _argv) : argc(_argc), argv(_argv), stabilizer(Stabilizer::GCBPM), method(Stabilizer::RAW) {		

		// Defaults for gcbpm
		bit = 5;
		N = 50;
		offset = 40;
		subRegionsX = 2;	
		subRegionsY = 2;
		damping = 0.98;	
		zoom = false;
		leaveOldFrames = false;

		// Defaults for svd
		featurePointsCount = 500;
	};

	void process();

	// stabilizing methods
	Stabilizer::StabilizerType stabilizer;
	Stabilizer::OutputFrameType method;

	string inName;
	string outName;

	// Settings for gcbpm
	int bit;
	int N;
	int offset;
	int subRegionsX;	
	int subRegionsY;
	double damping;	
	bool zoom;
	bool leaveOldFrames;

	// Settings for svd
	int featurePointsCount;

	void debug_printParameters() {
		cout << "Parameters:\n=========" << endl;
		cout << "input name:" << inName << endl;
		cout << "output name:" << outName << endl;
		cout << "stabilizer:" << (int)stabilizer << endl;
		cout << "method:" << (int)method << endl;
		cout << "bit:" << bit << endl;
		cout << "N:" << N << endl;
		cout << "offset:" << offset << endl;
		cout << "subRegionsX:" << subRegionsX << endl;
		cout << "subRegionsY:" << subRegionsY << endl;
		cout << "damping:" << damping << endl;
		cout << "zoom:" << (zoom ? "true" : "false") << endl;
		cout << "leave old frames:" << (leaveOldFrames ? "true" : "false") << endl;
	}

private:
	int argc;
	char** argv;
};

#endif
