// Authors: Jan Bednarik, Jakub Kvita
// Date: 14.5.2014

#include <iostream>
#include <cstdlib>
#include <cstdio>
#include "xgetopt.h"
#include "Parameters.h"
#include "Exceptions.h"

using namespace std;

void Parameters::process() {
	int opt;		
    while ((opt = getopt(argc, argv, "i:o:s:m:b:N:f:x:y:d:zl")) != EOF) {		
        switch (opt) {
			case 'i': inName = optarg; break;
			case 'o': outName = optarg; break;
			case 's':
				if (!string("gcbpm").compare(optarg)) {
					stabilizer = Stabilizer::GCBPM;				
				} else if (!string("svd").compare(optarg)) {
					stabilizer = Stabilizer::SVD;
				} else {
					throw Error("Unknown stabilizer type.");
				} 
				break;
			case 'm':
				if (!string("intersection").compare(optarg)) {
					method = StabilizerGCBPM::INTERSECTION;
				} else if (!string("union").compare(optarg)) {
					method = StabilizerGCBPM::UNION;
				} else if (!string("raw").compare(optarg)) {
					method = StabilizerGCBPM::RAW;
				} else {
					throw Error("Unknown crop method.");
				}				
				break;
			case 'b':				
				if(((bit = atoi(optarg)) < 0) || (bit > 7)) throw Usage(); break;
			case 'N':
				if((N = atoi(optarg)) < 1) throw Usage(); break;
			case 'f':
				if((offset = atoi(optarg)) < 1) throw Usage(); break;			
			case 'x':
				if((subRegionsX = atoi(optarg)) < 1) throw Usage(); break;							
			case 'y':
				if((subRegionsY = atoi(optarg)) < 1) throw Usage(); break;
			case 'd':
				if(((damping = atof(optarg)) < 0.0) || (damping > 1.0)) throw Usage(); break;
			case 'z': zoom = true; break;
			case 'l': leaveOldFrames = true; break;
			case 'c': 
				if((featurePointsCount = atoi(optarg)) < 1) throw Usage(); break;
			case '?': throw Error(string("Illegal option: ") + string(argv[optind-1]));								
			default:  throw Error(string("No handler for option: ") + string(argv[optind-1]));								
        }
    }	

	// if output file extension
	string ext;
	switch(method) {		
		case Stabilizer::RAW: ext = "-raw"; break;
		case Stabilizer::INTERSECTION: ext = "-isect"; break;
		case Stabilizer::UNION: ext = "-union"; break;
	}
	outName.insert(outName.length()-4, ext);	
}
