#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    playAudio = false;
    useMic = true;
    
    // Setup the sound stream
    soundStream.setup(this, MY_CHANNELS, MY_CHANNELS, MY_SRATE, MY_BUFFERSIZE, MY_NBUFFERS);
    
    
    // Setup audio file playback
    audio.setRate(MY_SRATE);
    audio.openFile(ofToDataPath("temp.wav", true));
    stk::Stk::setSampleRate(MY_SRATE);
    
    // Resize and initialize left and right buffers...
    left.resize( MY_BUFFERSIZE, 0 );
    right.resize( MY_BUFFERSIZE, 0 );
    
    ofSetFullscreen(true);
    ofEnableDepthTest();
    ofEnableSmoothing();
    ofBackground(95,89,93);
    
    fftLeft = ofxFft::create(MY_BUFFERSIZE, OF_FFT_WINDOW_HAMMING);
    fftRight = ofxFft::create(MY_BUFFERSIZE, OF_FFT_WINDOW_HAMMING);
}

//--------------------------------------------------------------
void ofApp::update(){
    ww = ofGetWindowWidth();
    wh = ofGetWindowHeight();
}

//--------------------------------------------------------------
void ofApp::draw(){
    
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if (key == 32) {
        useMic = !useMic;
        playAudio = !playAudio;
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
    mouseX = x;
    mouseY = y;
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    mouseX = x;
    mouseY = y;
    
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
}

// Audio functions
//--------------------------------------------------------------


void ofApp::audioOut(float * output, int bufferSize, int nChannels){
    // Write to output buffer
    if (playAudio) {
        stk::StkFrames frames(bufferSize,2);
        stk::StkFrames noiseFrames(bufferSize,1);
        audio.tick(frames);
        
        stk::StkFrames leftChannel(bufferSize,1);
        // copy the left Channel of 'frames' into `leftChannel`
        frames.getChannel(0, leftChannel, 0);
        
        stk::StkFrames rightChannel(bufferSize, 1);
        frames.getChannel(1, rightChannel, 0);
        
        float nToSL = (float)sunParticles[0]/(maxParticles);
        float nToSR = (float)sunParticles[1]/(maxParticles);
        
        for (int i = 0; i < bufferSize ; i++) {
            leftGain = gainSmoothers[0].tick(leftGainTarget);
            left[i] = leftChannel(i,0)*leftGain*nToSL;
            output[2*i] = left[i];
            
            rightGain = gainSmoothers[1].tick(rightGainTarget);
            right[i] = rightChannel(i,0)*rightGain*nToSR;
            output[2*i+1] = right[i];
        }
    }
}