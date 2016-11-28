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
    ofAddListener(onset.gotOnset, this, &ofApp::onsetEvent);
    
    // setup pitch object
    pitch.setup();
    
    // setup beat object
    beat.setup();
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
    // From here STK instrument setup
    voicer = new stk::Voicer();
    for (int i = 0; i < 50 ; i++)
    {
        stk::Moog *moog = new stk::Moog();
        voicer->addInstrument(moog);
    }
    note.noteNumber = 38;
    
    gain = 90;
    pos = 0;
    lengthOfOneBeatInSamples = (int)((SAMPLE_RATE*60.0f)/BPM);
    
    noteReverb.setT60(3);
    
    playback = false;
    micOn = true;
    midiCounter = 0;
    silenceCounter = 0;
}

//--------------------------------------------------------------
void ofApp::update()
{
    randNumber = rand() % 8;
    note.noteNumber = noteVal[randNumber];
    midiCounter+=noteVal[randNumber];
    
    if(midiCounter>300)
    {
        note.noteNumber = 55;
        midiCounter = 0;
        gain = 300;
    }
    else
        gain = 90;
    randNumber = rand() % 200 + 80;
    BPM = randNumber;
    //BPM = 100;
    lengthOfOneBeatInSamples = (int)((SAMPLE_RATE*60.0f)/BPM);
    
    // Aubio stuff
    onset.setThreshold(onsetThreshold);
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
        if(playback)
            playback = false;
    }

    // draw
    pitchGui.draw();
}

void ofApp::keyPressed(int key)
{
    if(key == 'p')
        playback = !playback;
    if(key == 'm')
        micOn = !micOn;
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
            if(silenceCounter>12)
            {
                playback = true;
                silenceCounter = 0;
            }
        }
    
        value1 = noteReverb.tick(voicer->tick(),0);
        value2 = noteReverb.tick(voicer->tick(),1);
        
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
