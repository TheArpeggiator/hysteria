# hysteria
Use of the openFrameworks tool to create an audio visual experience

Dependencies required to run this application are:
openFramework addons - ofxAubio, ofxParticles, ofxStk

Description
-----------
Have computers taking to each other (basically). 
Each computer will have an assigned voice. i.e. set freq range. Since range is set, they know what to 'listen' for.
Each computer is named as a 'heretic'. When a heretic speaks, one of the notes being played belongs to one of the other voices. That triggers the other heretic to stop talking.

Sometimes the last note will be missed, this causes both heretics to speak together. If a heretic is silent for too long, he starts talking again with an exclamation. This system should in theory go on forever. To add more to the confusion, more than two computers should be added to the system. The more unstable it becomes, the more fun it is!

Usage
-----
No real interaction for the listener. Just kick back, relax and watch computers nerding out.

Author
------
Rahul Agnihotri (ragni@ccrma.stanford.edu)
