#pragma once

#include "ofMain.h"
#include "ofxAubio.h"
#include "ofxGui.h"
#include "ofxStk.h"

#define SAMPLE_RATE 44100

struct MusicalNote {
    stk::StkFloat noteNumber;
    long voiceTag;
};

class ofApp : public ofBaseApp{
    
public:
    void setup();
    void exit();
    
    void update();
    void draw();
    
    // Audio callback functions
    void audioIn(float *input, int bufferSize, int nChannels);
    void audioOut(float *output,int bufferSize,int nChannels);
    
    void onsetEvent(float & time);
    void beatEvent(float & time);
    
    // Functions to handle noteOn/noteOff
    void noteOn();
    void noteOff();
    
    // Aubio variables
    ofxAubioOnset onset;
    ofxAubioPitch pitch;
    ofxAubioBeat beat;
    ofxAubioMelBands bands;
    
    ofxPanel pitchGui;
    ofxFloatSlider midiPitch;
    ofxFloatSlider pitchConfidence;
    
    ofxPanel beatGui;
    bool gotBeat;
    ofxFloatSlider bpm;
    
    ofxPanel onsetGui;
    bool gotOnset;
    ofxFloatSlider onsetThreshold;
    ofxFloatSlider onsetNovelty;
    ofxFloatSlider onsetThresholdedNovelty;
    
    ofxPanel bandsGui;
    ofPolyline bandPlot;
    
    // Stk classes object definition
    stk::Voicer *voicer;
    stk::NRev noteReverb;
    
    MusicalNote note;
    
    // C2 Major Scale (!!subject to change!!)
    int noteVal[8] = {12,14,16,17,19,21,23,24};
    
    // Volume parameter
    float gain;
    float value,value1,value2;
    
    // Variables to handle beats
    float lengthOfOneBeatInSamples;
    int pos;
    int BPM;
    
    
    // Extraneous variables
    int randNumber;
};
