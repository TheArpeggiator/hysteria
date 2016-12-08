#include "ofApp.h"
#include "ofEventUtils.h"

//--------------------------------------------------------------
void ofApp::setup()
{
    // set the size of the window
    //ofSetWindowShape(750, 250);
    
    int nOutputs = 2;
    int nInputs = 2;
    
    // ---------------------------------------------------------
    // Aubio files setup
    // setup onset object
    onset.setup();
    ofAddListener(onset.gotOnset, this, &ofApp::onsetEvent);
    
    // setup pitch object
    pitch.setup();
    
    // setup beat object
    beat.setup();
    ofAddListener(beat.gotBeat, this, &ofApp::beatEvent);
    
    // setup mel bands object
    bands.setup();
    
    ofSoundStreamSetup(nOutputs, nInputs, this);
    r2d2.openFile(ofToDataPath("r2d2-scream.wav",true));
    stk::Stk::setSampleRate(44100.0);
    r2d2.setRate(1);
    
    // setup the gui objects
    int start = 0;
    beatGui.setup("ofxAubioBeat", "settings.xml", start + 10, 10);
    beatGui.add(bpm.setup( "bpm", 0, 0, 250));
    
    start += 250;
    onsetGui.setup("ofxAubioOnset", "settings.xml", start + 10, 10);
    onsetGui.add(onsetThreshold.setup( "threshold", 0, 0, 2));
    onsetGui.add(onsetNovelty.setup( "onset novelty", 0, 0, 10000));
    onsetGui.add(onsetThresholdedNovelty.setup( "thr. novelty", 0, -1000, 1000));
    onsetThreshold = onset.threshold;
    
    start += 250;
    pitchGui.setup("ofxAubioPitch", "settings.xml", start + 10, 10);
    pitchGui.add(midiPitch.setup( "midi pitch", 0, 0, 128));
    pitchGui.add(pitchConfidence.setup( "confidence", 0, 0, 1));
    
    bandsGui.setup("ofxAubioMelBands", "settings.xml", start + 10, 115);
    for (int i = 0; i < 40; i++)
    {
        bandPlot.addVertex( 50 + i * 650 / 40., 240 - 100 * bands.energies[i]);
    }
    
    // ----------------------------------------------------------
    // STK instrument setup
    voicer = new stk::Voicer();
    for (int i = 0; i < 50 ; i++)
    {
        stk::Moog *moog = new stk::Moog();
        voicer->addInstrument(moog);
    }
    note.noteNumber = 38;
    
    BPM = 80;
    lengthOfOneBeatInSamples = (int)((SAMPLE_RATE*60.0f)/BPM);
    
    noteReverb.setT60(2 );
    
    // Audio I/O setup
    gain = 600;
    pos = 0;
    playback = false;
    micOn = true;
    midiCounter = 0;
    silenceCounter = 0;     // Counter to check if(!playback)
    scale = 0;
    r2d2Counter = 0;
    
    // ----------------------------------------------------------
    // Particle physics setup statements
    ofSetFrameRate(60.0);
    ofBackground(0, 0, 0);
    silenceEmitter.setPosition(ofVec3f(640,400));
    silenceEmitter.posSpread = ofVec3f(10.0,10.0,0.0);
    silenceEmitter.velSpread = ofVec3f(25.0,25.0);
    silenceEmitter.life = 10.0;
    silenceEmitter.lifeSpread = 5.0;
    silenceEmitter.numPars = 10;
    silenceEmitter.color = ofColor(200,200,255);
    silenceEmitter.colorSpread = ofColor(20,20,0);
    silenceEmitter.size = 32;
    
    soundEmitter.setPosition(ofVec3f(640,400));
    soundEmitter.velSpread = ofVec3f(10.0,10);
    soundEmitter.life = 69;
    soundEmitter.lifeSpread = 5.0;
    soundEmitter.numPars = 10.0;
    soundEmitter.color = ofColor(200,100,100);
    soundEmitter.colorSpread = ofColor(50,50,50);
    soundEmitter.size = 32;
    
    vectorField.allocate(128, 128, 3);
    
    ofLoadImage(pTex, "p.png");
    ofLoadImage(p1Tex, "p1.png");
    ofLoadImage(p2Tex, "p2.png");
    
    rotAcc = 4500;
    gravAcc = 11160;
    drag = 0.622;
    fieldMult = 1.9;
    displayMode = 0;
    
    ofEnableBlendMode(OF_BLENDMODE_ADD);
    ofToggleFullscreen();
}

//---------------------------------------------------------------
void ofApp::update()
{
    // STK stuff
    // ----------------------------------------------------------
    randNumber = rand() % 8;
    note.noteNumber = noteVal[scale][randNumber];
    midiCounter+=noteVal[scale][randNumber];
    
    if(midiCounter>=1000)
    {
        note.noteNumber = 55;
        midiCounter = 0;
        gain = 900;
    }
    else
        gain = 600;
    BPM = rand() % 240 + 100;
    lengthOfOneBeatInSamples = (int)((SAMPLE_RATE*60.0f)/BPM);
    
    // Aubio stuff
    // ----------------------------------------------------------
    onset.setThreshold(onsetThreshold);
    
    // Particle Physics stuff
    // ----------------------------------------------------------
    for(int y = 0; y < vectorField.getHeight(); y++)
        for(int x=0; x< vectorField.getWidth(); x++){
            int index = vectorField.getPixelIndex(x, y);
            float angle = ofNoise(x/(float)vectorField.getWidth()*4.0, y/(float)vectorField.getHeight()*4.0,ofGetElapsedTimef()*0.05)*stk::TWO_PI*2.0;
            ofVec2f dir(cos(angle), sin(angle));
            dir.normalize().scale(ofNoise(x/(float)vectorField.getWidth()*4.0, y/(float)vectorField.getHeight()*4.0,ofGetElapsedTimef()*0.05+10.0));
            vectorField.setColor(x, y, ofColor_<float>(dir.x,dir.y, 0));
        }
    
    float dt = min(ofGetLastFrameTime(), 1.0/10.0);
    particleSystem.gravitateTo(ofPoint(ofGetWidth()/2,ofGetHeight()/2), gravAcc, 1, 10.0, false);
    particleSystem.rotateAround(ofPoint(ofGetWidth()/2,ofGetHeight()/2), rotAcc, 10.0, false);
    particleSystem.applyVectorField(vectorField.getData(), vectorField.getWidth(), vectorField.getHeight(), vectorField.getNumChannels(), ofGetWindowRect(), fieldMult);
    if(ofGetMousePressed(2)){
        particleSystem.gravitateTo(ofPoint(mouseX,mouseY), gravAcc, 1, 10.0, false);
    }
    
    particleSystem.update(dt, drag);
    
    if(playback)
        particleSystem.addParticles(soundEmitter);
    if(!playback)
        particleSystem.addParticles(silenceEmitter);
}

void ofApp::exit()
{
    ofSoundStreamStop();
    ofSoundStreamClose();
}

//--------------------------------------------------------------
void ofApp::draw()
{
    // update pitch info
    if (pitch.pitchConfidence > 0.85)
        pitchConfidence = pitch.pitchConfidence;
    if (pitch.latestPitch && pitchConfidence > 0.85)
        midiPitch = ceil(pitch.latestPitch);
    
    if(midiPitch == 60)
    {
        r2d2Counter++;
        if(playback)
            playback = false;
        scale = rand() % 3;
        soundEmitter.color = ofColor(rand() % 250 + 180,rand() % 250 + 180,rand() % 250 + 180);
    }
    
    ofSetLineWidth(2.0);
    if (displayMode == 1) {
        particleSystem.draw(pTex);
    }
    else if(displayMode == 2) {
        particleSystem.draw(p1Tex, p2Tex);
    }
    else {
        particleSystem.draw();
    }
    
    // Aubio draw function
    //pitchGui.draw();
}

void ofApp::keyPressed(int key)
{
    if(key == 'p')
        playback = !playback;
    if(key == 'm')
        micOn = !micOn;
    if(key == 'g')
    {
        std::cout<<"Width = "<<ofGetWidth()/2<<endl;
        std::cout<<"Height = "<<ofGetHeight()/2<<endl;
    }
    
}

void ofApp::audioIn(float * input, int bufferSize, int nChannels)
{
    if(micOn)
    {
        // compute pitch detection
        pitch.audioIn(input, bufferSize, nChannels);
    }
}

void ofApp::audioOut(float *output, int bufferSize, int nChannels)
{
    for (int i = 0; i < bufferSize ; i++)
    {
        pos++;
        if(fmod(pos,lengthOfOneBeatInSamples)==0)
        {
            noteOn();
            
            if(!playback)
                silenceCounter++;
            if(silenceCounter>30)
            {
                playback = true;
                silenceCounter = 0;
            }
        }
        if(pos>88200)
            pos = 0;
        
        value1 = noteReverb.tick(voicer->tick(),0);
        value2 = noteReverb.tick(voicer->tick(),1);
        value = r2d2.tick();
        
        if(playback)
        {
            output[2*i] = value1;
            output[2*i+1] = value2;
        }
        else
        {
            output[2*i] = 0;
            output[2*i+1] = 0;
        }
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