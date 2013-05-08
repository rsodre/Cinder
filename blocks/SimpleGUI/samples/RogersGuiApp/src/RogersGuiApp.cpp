#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "SimpleGUI.h"
#include <iostream>
#include <sstream>
#include <fstream>

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace ci::sgui;

#define RENDER_TYPE_GROUP 1

#define CONFIG_FILE "settings.sgui.txt"

class RogersGuiApp : public AppBasic {
private:
    SimpleGUI* gui;
    Timer timer;
    float prevTime;
    
    ButtonControl* takeScreenshotButton;
    CallbackId cbTakeScreenshotButtonClick;
	
    TextureVarControl* screenshotTextureControl;
    gl::Texture screenshot;
    gl::Texture nullTex;
	
	// ROGER:: From Grouping example
	float rotation;
	int size;
	ColorA color;
	Color colorBack;
	ColorA colorHSV;
	Color colorBackHSV;
	bool autoRotation;	
	bool fill;
	bool stroke;
	bool fxInvert;
	float thickness;
	
	// ROGER:: NEW
	std::string aString;
	std::map<int,std::string> valueLabels;
	int listVal;
	ListVarControl *theDropDownControl;
	ListVarControl *theListControl;
	float FPS;
	int windowWidth;
	int windowHeight;
	int intValue1;
	int intValue2;
	int intValue3;
	int intValue4;
	int intValue5;
	int intValue6;
	unsigned char byteValue1;
	unsigned char byteValue2;
	unsigned char byteValue3;
	unsigned char flagValue;
	Vec4f vec2Value;
	Vec4f vec3Value;
	Vec4f vec4Value;
	Vec2f xyValue;
	float floatVal1;
	float floatVal2;
	float floatVal3;
	float floatValAxis;
	bool tabFX;
	
public:
	void prepareSettings( Settings *settings );
	void setup();
	void mouseDown( MouseEvent event );	
    void keyDown( KeyEvent event );
    bool takeScreenshotButtonClick( MouseEvent event ); //this event handlers return bool because if they return false the event is marked as intercepted
    bool restartButtonClick( MouseEvent event );    //and your application mouse events handlers won't be called (which is usefull)
	void update();
	void draw();
	// ROGER
	bool addToList( MouseEvent event ); //this event handlers return bool because if they return false the event is marked as intercepted
	bool removeFromList( MouseEvent event ); //this event handlers return bool because if they return false the event is marked as intercepted
};

void RogersGuiApp::prepareSettings( Settings *settings )
{
	settings->setWindowSize( 1024, 768 );
}

void RogersGuiApp::setup() {
    rotation = 0;
    screenshot = gl::Texture(getWindowWidth(), getWindowHeight()); //uninitialized texture with random pixels from GPU memory
	FloatVarControl *fc;
	IntVarControl *ic;
	BoolVarControl *bc;
	ByteVarControl *btc;
	FlagVarControl *fgc;
	TabControl *tc;
	aString = "whatever";
	
    gui = new SimpleGUI(this);
	gui->lightColor = ColorA(1, 1, 0, 1);
	
	// Optional TABS
	// For each tab added, all subsequent columns will only appear if this tab is enabled
	gui->setName("> MY APP");	// !NEW display app name first of all
	//gui->displayFps(false);		// !NEW display fps at bottom of tabs (ON BY DEFAULT)
	gui->addTab("MAIN TAB");	// !NEW tabbed spaces
	
	gui->addColumn();
	gui->addLabel("> LABELS");
	gui->addLabel("simple label");
	gui->addLabel("this label does not fit in one line, so it is wrapping over multiple lines", true);
    gui->addSeparator();
	gui->addLabel("> READ ONLY");
	gui->addParam("String", &aString, aString);						// !NEW! Update aString will update label
	gui->addParam("Window Width", &windowWidth, 0, 1920, getWindowWidth())->setReadOnly();		// !NEW! No slider, just value
	gui->addParam("Window Height", &windowHeight, 0, 1080, getWindowHeight())->setReadOnly();	// !NEW! No slider, just value
	gui->addParam("FPS", &FPS, 0, 60, 0)->setReadOnly();	// !NEW! No slider, just value
    
	// Textures
    gui->addSeparator();
	gui->addLabel("> TEXTURES");
    takeScreenshotButton = gui->addButton("Take screenshot");    
	takeScreenshotButton->registerClick(this, &RogersGuiApp::takeScreenshotButtonClick);    
    screenshotTextureControl = gui->addParam("Screenshot", &screenshot);
	screenshotTextureControl->setRefreshRate( 0.0 );		// !NEW! let's refresh manually
	//screenshotTextureControl->refreshRate = 0.1;		// !NEW! use this for movies or syphon
    //takeScreenshotButton->unregisterClick(cbTakeScreenshotButtonClick);
    gui->addSeparator();
    gui->addParam("Null Texture example", &nullTex);	// !NEW! Null textures will be marked so
	
    //
	// ROGER:: From Grouping example
	gui->addColumn();
	gui->addLabel("> CONTROLS");
    gui->addButton("Restart Animation")->registerClick(this, &RogersGuiApp::restartButtonClick);
	bc = gui->addParam("Auto Rotation ON", &autoRotation, true);
	bc->nameOff = "Auto Rotation OFF";	// !NEW! Alternative label when OFF
	gui->addParam("Rotation", &rotation, 0, 360, 0)->setDisplayValue();
	gui->addParam("Size", &size, 100, 600, 200)->setDisplayValue();
	gui->addParam("Background", &colorBack, Color(0.1, 0.1, 0.5), SimpleGUI::RGB); //use R,G,B,A sliders
	gui->addParam("Color", &color, ColorA(1, 0.2, 0.2, 0.1), SimpleGUI::RGB)->setDisplayValue(); //use R,G,B,A sliders
	gui->addParam("HSV", &colorBackHSV, Color(0.1, 0.1, 0.5), SimpleGUI::HSV);
	gui->addParam("HSV+A", &colorHSV, ColorA(1, 0.2, 0.2, 0.1), SimpleGUI::HSV);
	gui->addSeparator();
	gui->addLabel("RENDER TYPE");
	gui->addParam("Fill", &fill, true, RENDER_TYPE_GROUP); //if we specify group id, we create radio button set
	bc = gui->addParam("Stroke", &stroke, false, RENDER_TYPE_GROUP); //i.e. only one of the buttons can be active at any time	
	bc->addSwitchPanel("Stroke");
	gui->addParam("Thickness", &thickness, 1, 10);
	
	
	//
	// INTS CONTROL
	gui->addColumn();
	gui->addLabel("> FLOATS");
	fc = gui->addParam("Float prec 1", &floatVal1, 0, 1, 0.5);		// !NEW! float with precision 1
	fc->setDisplayValue();
	fc->setPrecision(1);
	fc = gui->addParam("Float prec 2", &floatVal2, 0, 1, 0.5);		// !NEW! float with precision 2
	fc->setDisplayValue();
	fc->setPrecision(2);
	fc = gui->addParam("Float prec 3", &floatVal3, 0, 1, 0.5);		// !NEW! float with precision 3
	fc->setDisplayValue();
	fc->setPrecision(3);
	fc = gui->addParam("Float axis default", &floatValAxis, 0, 1, 0.5);		// !NEW! float with precision 3
	fc->setDisplayValue();
	fc->axisOnDefault = true;


	//
	// INTS CONTROL
	gui->addColumn();
	gui->addLabel("> INTS");
	gui->addParam("Int Radio (<=10)", &intValue1, 1, 3, 0)->setDisplayValue();		// !NEW! small int range will be displayed as radios
	gui->addParam("Int Radio (<=10)", &intValue2, 1, 10, 0)->setDisplayValue();		// !NEW! small int range will be displayed as radios
	gui->addParam("Int slider (>10)", &intValue3, 1, 11, 0)->setDisplayValue();		// !NEW! Display the value beside labels
	// stepped
    gui->addSeparator();
	ic = gui->addParam("Int Step 2", &intValue4, 0, 10, 0);
	ic->setDisplayValue();
	ic->setStep(2);														// !NEW! int step
	ic = gui->addParam("Int Step 2", &intValue5, 0, 18, 0);
	ic->setDisplayValue();
	ic->setStep(2);														// !NEW! int step
	ic = gui->addParam("Int Step 2", &intValue6, 0, 20, 0);
	ic->setDisplayValue();
	ic->setStep(2);														// !NEW! int step
	
	//
	// BYTE CONTROL
	gui->addColumn();
	gui->addLabel("> BYTES");
	gui->addParam("Byte", &byteValue1, 0)->setDisplayValue();			// !NEW! ranges from 0-255
	btc = gui->addParam("Byte as char", &byteValue2, 0);				// !NEW! display as char
	btc->setDisplayValue();
	btc->displayChar = true;
	btc = gui->addParam("Byte as hex", &byteValue3, 0);					// !NEW! display as hex
	btc->setDisplayValue();
	btc->displayHex = true;
	// FLAGS
    gui->addSeparator();
	fgc = gui->addParamFlag("Byte Flag", &flagValue, 8, 0x55);			// !NEW! Byte Flag
	fgc->setDisplayValue();
	fgc->displayHex = true;
	
	//
	// BYTE CONTROL
	gui->addColumn();
	gui->addLabel("> VECTORS");
	gui->addParam("Vec2", &vec2Value, 2);			// !NEW! Vec2f
	gui->addParam("Vec3", &vec3Value, 3);			// !NEW! Vec2f
	gui->addParam("Vec4", &vec4Value, 4);			// !NEW! Vec2f
	gui->addSeparator();
	gui->addParamXY("XY Canvas", &xyValue, -1.0, 1.0, Vec2f::zero() );
	
	//
	// TAB 2
	//
	gui->addTab("DYNAMIC LISTS");
	gui->addColumn();
	gui->addLabel("> MANAGE LIST");
    gui->addSeparator();
    gui->addButton("Increase List")->registerClick(this, &RogersGuiApp::addToList);
    gui->addButton("Decrease List")->registerClick(this, &RogersGuiApp::removeFromList);
	valueLabels[0] = "val 0";
	valueLabels[1] = "val 1";
	valueLabels[2] = "val 2";
	// Drop-Down
	gui->addColumn();
	gui->addLabel("> DROP-DOWN");
    gui->addSeparator();
	theDropDownControl = gui->addParamDropDown("Drop-Down List", &listVal, valueLabels);	// !NEW! Drop-Down control
    gui->addSeparator();
    gui->addLabel("just a label");
	// Explicit
	gui->addColumn();
	gui->addLabel("> EXPLICIT");
    gui->addSeparator();
	theListControl = gui->addParamList("Explicit List", &listVal, valueLabels);				// !NEW! List control
    gui->addSeparator();
    gui->addLabel("just a label");
	
	
	//
	// TAB 3
	//
	tc = (TabControl*) gui->addTab("FX ON", &tabFX, false);
	tc->nameOff = "FX OFF";
	gui->addColumn();
	gui->addLabel("> FX");
    gui->addSeparator();
	gui->addParam("Invert", &fxInvert, true);
	
	
	//gui->load(CONFIG_FILE); //we load settings after specifying all the
    //params because we need to know their name and type
	
	timer.start();
	prevTime = timer.getSeconds();
	
}

void RogersGuiApp::mouseDown( MouseEvent event ) {
}

bool RogersGuiApp::takeScreenshotButtonClick( MouseEvent event ) {
    screenshot.update(copyWindowSurface());
	screenshotTextureControl->refresh();		// !NEW! Force texture refresh
    return false;
}

// ROGER
bool RogersGuiApp::addToList( MouseEvent event ) {
	int i = valueLabels.size();
	std::stringstream ss;
	ss << "new " << i;
	valueLabels[i] = ss.str();
	theListControl->update( valueLabels );
	theDropDownControl->update( valueLabels );
    return false;
}
bool RogersGuiApp::removeFromList( MouseEvent event ) {
	if (valueLabels.size())
	{
		valueLabels.erase( valueLabels.rbegin()->first );
		theListControl->update( valueLabels );
		theDropDownControl->update( valueLabels );
	}
    return false;
}

bool RogersGuiApp::restartButtonClick( MouseEvent event ) {
    rotation = 0;
    return false;
}

void RogersGuiApp::keyDown( KeyEvent event ) {
	switch(event.getChar()) {				
		case 'd': gui->dump(); break; //prints values of all the controls to the console			
		case 'l': gui->load(CONFIG_FILE); break;
		case 's': gui->save(CONFIG_FILE); break;					
	}
	switch(event.getCode()) {
		case KeyEvent::KEY_ESCAPE: quit(); break;
	}
}

void RogersGuiApp::update() {
}

void RogersGuiApp::draw(){
    float currTime = timer.getSeconds();
	float deltaTime = currTime - prevTime;
	prevTime = currTime;
    
	if (autoRotation)
		rotation += deltaTime/2;
	//while (rotation > 360.0)
	//	rotation -= 360.0;
	
    gl::clear( colorBack );
    gl::enableAdditiveBlending();
    gl::disableDepthRead();    
    gl::pushMatrices();
    gl::translate(Vec3f(getWindowWidth()/2, getWindowHeight()/2, 0));
    
    gl::color( color );    
    for(int i=0;i<50; i++) {
        gl::rotate(Vec3f(0, 0, rotation));
        gl::drawSolidRect(Rectf(20, 20, 140, 140));
    }
    
    gl::color(ColorA(0.5, 0.25, 0.0, 0.05));    
    for(int i=0;i<50; i++) {
        gl::rotate(Vec3f(0, 0, rotation/4));
        gl::drawSolidRect(Rectf(50, 50, 340, 340));
    }
    
    gl::enableDepthRead();    
    gl::disableAlphaBlending();
    gl::popMatrices();	
	
	if (tabFX)
	{
		if (fxInvert)
		{
			glEnable( GL_BLEND );
			glBlendFunc( GL_ONE_MINUS_DST_COLOR, GL_ZERO );
			gl::color( Color::white() );
			gl::drawSolidRect( getWindowBounds() );
			glDisable( GL_BLEND );
		}
	}

	FPS = getAverageFps();
	gui->draw();
	
}

void disableAlphaBlending()
{

	
	//std::cout << "LIST = " << listVal << std::endl;
}


CINDER_APP_BASIC( RogersGuiApp, RendererGl )
