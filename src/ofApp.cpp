#include "ofApp.h"
#include "ofEventUtils.h"

//--------------------------------------------------------------
void ofApp::setup()
{
    // set the size of the window
    ofSetWindowShape(750, 250);
    
    int nOutputs = 2;
    int nInputs = 2;
    
    // ---------------------------------------------------------
    // Aubio files setup
    // setup onset object
    onset.setup();
    // listen to onset event
    ofAddListener(onset.gotOnset, this, &ofApp::onsetEvent);
    
    // setup pitch object
    pitch.setup();
    //pitch.setup("yinfft", 8 * bufferSize, bufferSize, sampleRate);
    
    // setup beat object
    beat.setup();
    // listen to beat event
    ofAddListener(beat.gotBeat, this, &ofApp::beatEvent);
    
    // setup mel bands object
    bands.setup();
    
    ofSoundStreamSetup(nOutputs, nInputs, this);

    // setup the gui objects
    int start = 0;
    beatGui.setup("ofxAubioBeat", "settings.xml", start + 10, 10);
    beatGui.add(bpm.setup( "bpm", 0, 0, 250));
    
    start += 250;
    onsetGui.setup("ofxAubioOnset", "settings.xml", start + 10, 10);
    onsetGui.add(onsetThreshold.setup( "threshold", 0, 0, 2));
    onsetGui.add(onsetNovelty.setup( "onset novelty", 0, 0, 10000));
    onsetGui.add(onsetThresholdedNovelty.setup( "thr. novelty", 0, -1000, 1000));
    // set default value
    onsetThreshold = onset.threshold;
    
    start += 250;
    pitchGui.setup("ofxAubioPitch", "settings.xml", start + 10, 10);
    pitchGui.add(midiPitch.setup( "midi pitch", 0, 0, 128));
    pitchGui.add(pitchConfidence.setup( "confidence", 0, 0, 1));
    
    bandsGui.setup("ofxAubioMelBands", "settings.xml", start + 10, 115);
    for (int i = 0; i < 40; i++) {
        bandPlot.addVertex( 50 + i * 650 / 40., 240 - 100 * bands.energies[i]);
    }
    
    // ----------------------------------------------------------
    // From here STK instrument setup
    voicer = new stk::Voicer();
    for (int i = 0; i < 50 ; i++)
    {
        stk::Moog *moog = new stk::Moog();
        voicer->addInstrument(moog);
    }
    note.noteNumber = 38;
    
    gain = 90;
    BPM = 120;
    pos = 0;
    lengthOfOneBeatInSamples = (int)((SAMPLE_RATE*60.0f)/BPM);
    
    noteReverb.setT60(7);
}

//--------------------------------------------------------------
void ofApp::update()
{
    randNumber = rand() % 8;
    note.noteNumber = noteVal[randNumber];
    
    randNumber = rand() % 300 + 80;
    BPM = randNumber;
    lengthOfOneBeatInSamples = (int)((SAMPLE_RATE*60.0f)/BPM);
    onset.setThreshold(onsetThreshold);
}

void ofApp::exit(){
    ofSoundStreamStop();
    ofSoundStreamClose();
}

//--------------------------------------------------------------
void ofApp::draw(){
    // update beat info
    if (gotBeat) {
        ofSetColor(ofColor::green);
        ofRect(90,150,50,50);
        gotBeat = false;
    }
    
    // update onset info
    if (gotOnset) {
        ofSetColor(ofColor::red);
        ofRect(250 + 90,150,50,50);
        gotOnset = false;
    }
    onsetNovelty = onset.novelty;
    onsetThresholdedNovelty = onset.thresholdedNovelty;
    
    // update pitch info
    pitchConfidence = pitch.pitchConfidence;
    if (pitch.latestPitch) midiPitch = pitch.latestPitch;
    bpm = beat.bpm;
    
    // draw
    pitchGui.draw();
    beatGui.draw();
    onsetGui.draw();
    
    ofSetColor(ofColor::orange);
    ofSetLineWidth(3.);
    bandsGui.draw();
    //bandPlot.clear();
    for (int i = 0; i < bandPlot.size(); i++) {
        bandPlot[i].y = 240 - 100 * bands.energies[i];
    }
    bandPlot.draw();
}

void ofApp::audioIn(float * input, int bufferSize, int nChannels){
    // compute onset detection
    onset.audioIn(input, bufferSize, nChannels);
    // compute pitch detection
    pitch.audioIn(input, bufferSize, nChannels);
    // compute beat location
    beat.audioIn(input, bufferSize, nChannels);
    // compute bands
    bands.audioIn(input, bufferSize, nChannels);
}

void ofApp::audioOut(float *output, int bufferSize, int nChannels)
{
    for (int i = 0; i < bufferSize ; i++)
    {
        pos++;
        if(fmod(pos,lengthOfOneBeatInSamples)==0)
            noteOn();
        
        value = voicer->tick();
        value1 = noteReverb.tick(voicer->tick(),0);
        value2 = noteReverb.tick(voicer->tick(),1);
        output[2*i] = value1;
        output[2*i+1] = value2;
    }
    noteOff();
}

void ofApp::noteOn()
{
    note.voiceTag = voicer->noteOn(note.noteNumber,gain);
}

void ofApp::noteOff()
{
    voicer->noteOff(note.voiceTag, gain);
}

//----
void ofApp::onsetEvent(float & time) {
    //ofLog() << "got onset at " << time << " s";
    gotOnset = true;
}

//----
void ofApp::beatEvent(float & time) {
    //ofLog() << "got beat at " << time << " s";
    gotBeat = true;
}
