#include "ofMain.h"
#include "ofApp.h"
#include "ofAppGLFWWindow.h"

//========================================================================
int main( ){
	ofAppGLFWWindow win;
	//win.setNumSamples(8);
	//win.setOrientation(OF_ORIENTATION_90_LEFT);
	//win.setMultiDisplayFullscreen(true);
	//win.set

	ofSetupOpenGL(&win, 800,500, OF_WINDOW);	// <-------- setup the GL context

	// this kicks off the running of my app
	// can be OF_WINDOW or OF_FULLSCREEN
	// pass in width and height too:
	ofRunApp(new ofApp());

}
