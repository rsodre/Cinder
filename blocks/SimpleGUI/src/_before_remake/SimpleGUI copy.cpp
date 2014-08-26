//
// GUI for Cinder based on SimpleGui
// https://github.com/vorg/MowaLibs
// http://forum.libcinder.org/topic/simplegui
//
// Adapted by Roger Sodre
//
// === Original copyright from mowaLibs ===
//
// Copyright (c) 2011, Marcin Ignac / marcinignac.com
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//
// This code is intended to be used with the Cinder C++ library, http://libcinder.org
//
// Temptesta Seven font by Yusuke Kamiyamane http://p.yusukekamiyamane.com/fonts/
// "The fonts can be used free for any personal or commercial projects."

#include <iostream>
#include <sstream>
#include <fstream>
#include <iosfwd>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include "SimpleGUI.h"
#include "cinder/app/AppBasic.h"
#include "cinder/Utilities.h"
#include "cinder/Font.h"
#include "cinder/CinderMath.h"
#include "cinder/Rand.h"

#ifndef RELEASE
//#define BLINK_CONTROLS		// uncomment to color-blink controls when they change
//#define BLINK_FBO				// uncomment to blink whole GUI when anything changes
//#define DEBUG_FBO
#endif

using namespace ci;
using namespace ci::app;

namespace cinder { namespace sgui {
	
	//-----------------------------------------------------------------------------
	
	Font SimpleGUI::textFont = Font();
	ColorA SimpleGUI::darkColor = ColorA(0.3, 0.3, 0.3, 1);
	ColorA SimpleGUI::lightColor = ColorA(1, 1, 1, 1);
	ColorA SimpleGUI::bgColor = ColorA(0, 0, 0, 0.8);
	ColorA SimpleGUI::textColor = ColorA(1,1,1,1);
	ColorA SimpleGUI::mouseOverColor = ColorA(0.75,0.75,0.75,1);
	ColorA SimpleGUI::markerColor = ColorA(1.0,0,0,1);
	Vec2f SimpleGUI::spacing = Vec2f(3, 0);
	Vec2f SimpleGUI::padding = Vec2f(3, 3);
	Vec2f SimpleGUI::radioSize = Vec2f(10, 10);
	Vec2f SimpleGUI::sliderSize = Vec2f(125, 10);
	Vec2f SimpleGUI::labelSize = Vec2f(125, 10);
	Vec2f SimpleGUI::buttonSize = Vec2f(125, 12);
	Vec2f SimpleGUI::buttonGap = Vec2f(6, 0);
	Vec2f SimpleGUI::separatorSize = Vec2f(125, 3);
	Vec2f SimpleGUI::thumbnailSize = Vec2f(125, 125 * 0.75);
	
	
	SimpleGUI::SimpleGUI(App* app) {
		init(app);
		enabled = true;
		mouseControl = NULL;
	}
	// ROGER
	SimpleGUI::~SimpleGUI() {
		for (int i = 0 ; i < controls.size() ; i++)
			delete controls[i];
		controls.clear();
	}
	
	void SimpleGUI::init(App* app) {
		textFont = Font(loadResource("pf_tempesta_seven.ttf"), 8);
		//textFont = Font("Arial", 12);
		selectedControl = NULL;
		cbMouseMove = app->registerMouseMove( this, &SimpleGUI::onMouseMove );
		cbMouseDown = app->registerMouseDown( this, &SimpleGUI::onMouseDown );
		cbMouseUp = app->registerMouseUp( this, &SimpleGUI::onMouseUp );	
		cbMouseDrag = app->registerMouseDrag( this, &SimpleGUI::onMouseDrag );
		
		// ROGER
		bForceRedraw = true;
		mOffset = Vec2f(0,0);
		droppedList = NULL;
		// FBO
		bUsingFbo = true;
		bShouldResize = true;
		cbResize = App::get()->registerResize( this, &SimpleGUI::onResize );
		cbKeyDown = App::get()->registerKeyDown( this, &SimpleGUI::onKeyDown );
	}
	
	FloatVarControl* SimpleGUI::addParam(const std::string& paramName, float* var, float min, float max, float defaultValue) {
		FloatVarControl* control = new FloatVarControl(this, paramName, var, min, max, defaultValue);
		controls.push_back(control);
		return control;
	}
	
	IntVarControl* SimpleGUI::addParam(const std::string& paramName, int* var, int min, int max, int defaultValue) {
		IntVarControl* control = new IntVarControl(this, paramName, var, min, max, defaultValue);
		controls.push_back(control);
		return control;
	}
	
	// ROGER
	ByteVarControl* SimpleGUI::addParam(const std::string& paramName, unsigned char* var, unsigned char defaultValue) {
		ByteVarControl* control = new ByteVarControl(this, paramName, var, defaultValue);
		controls.push_back(control);
		return control;
	}
	
	// ROGER
	FlagVarControl* SimpleGUI::addParamFlag(const std::string& paramName, unsigned char* var, int maxf, unsigned char defaultValue) {
		FlagVarControl* control = new FlagVarControl(this, paramName, var, maxf, defaultValue);
		controls.push_back(control);
		return control;
	}
	
	BoolVarControl* SimpleGUI::addParam(const std::string& paramName, bool* var, bool defaultValue, int groupId) {
		BoolVarControl* control = new BoolVarControl(this, paramName, var, defaultValue, groupId);
		controls.push_back(control);
		return control;
	}
	
	// ROGER
	ColorVarControl* SimpleGUI::addParam(const std::string& paramName, ColorA* var, ColorA defaultValue, int colorModel) {
		ColorVarControl* control = new ColorVarControl(this, paramName, var, defaultValue, colorModel);
		controls.push_back(control);
		return control;
	}
	ColorVarControl* SimpleGUI::addParam(const std::string& paramName, Color* var, Color defaultValue, int colorModel) {
		ColorVarControl* control = new ColorVarControl(this, paramName, var, defaultValue, colorModel);
		controls.push_back(control);
		return control;
	}
	VectorVarControl* SimpleGUI::addParam(const std::string& paramName, Vec4f* var, int vecCount, float min, float max, Vec4f const defaultValue) {
		VectorVarControl* control = new VectorVarControl(this, paramName, var, vecCount, min, max, defaultValue );
		controls.push_back(control);
		return control;
	}
	ArcballVarControl* SimpleGUI::addParam(const std::string& paramName, Vec4f* var, Vec4f const defaultValue) {
		ArcballVarControl* control = new ArcballVarControl(this, paramName, var, defaultValue );
		controls.push_back(control);
		return control;
	}
	TextureVarControl* SimpleGUI::addParam(const std::string& paramName, gl::Texture* var, float scale, bool flipVert) {
		TextureVarControl* control = new TextureVarControl(this, paramName, var, scale, flipVert);
		controls.push_back(control);
		return control;
	}
	
	// ROGER
	ListVarControl*	SimpleGUI::addParamList( const std::string &paramName, int* var, const std::map<int,std::string> &valueLabels ) {
		ListVarControl* control;
		control = new ListVarControl(this, paramName, var, valueLabels);
		controls.push_back(control);
		return control;
	}
	ListVarControl*	SimpleGUI::addParamDropDown( const std::string &paramName, int* var, const std::map<int,std::string> &valueLabels) {
		ListVarControl* control;
		control = new DropDownVarControl(this, paramName, var, valueLabels);
		controls.push_back(control);
		return control;
	}
	LabelControl* SimpleGUI::addParam(const std::string& paramName, std::string* var)
	{
 		LabelControl* control = new LabelControl(this, paramName, var);
		controls.push_back(control);
		return control;
	}
	
	ButtonControl* SimpleGUI::addButton(const std::string& buttonName) {
		ButtonControl* control = new ButtonControl(this, buttonName);
		controls.push_back(control);
		return control;
	}
	
	LabelControl* SimpleGUI::addLabel(const std::string& labelName) {
		LabelControl* control = new LabelControl(this, labelName);
		controls.push_back(control);
		return control;
	}
	
	SeparatorControl* SimpleGUI::addSeparator() {
		SeparatorControl* control = new SeparatorControl(this);
		controls.push_back(control);
		return control;
	}
	
	ColumnControl* SimpleGUI::addColumn(int x, int y) {
		ColumnControl* control = new ColumnControl(this, x, y);
		controls.push_back(control);
		return control;
	}
	
	PanelControl* SimpleGUI::addPanel(const std::string& panelName) {
		PanelControl* control = new PanelControl(this, panelName);
		controls.push_back(control);
		return control;
	}
	
	//
	// ROGER
	//
	Control* SimpleGUI::getControlByName(const std::string & name) {
		for (std::vector<Control*>::iterator it = controls.begin() ; it != controls.end() ; it++) {
			Control* control = *it;
			if (control->name == name) {
				return control;
			}		
		}
		return NULL;
	}
	bool SimpleGUI::hasChanged()
	{
		bool changed = false;
		bool enab = true;
		for (std::vector<Control*>::iterator it = controls.begin() ; it != controls.end() ; it++) {
			Control* control = *it;
			if ( control->type == Control::COLUMN )
				enab = true;
			else if ( control->type == Control::PANEL )
				enab = ((PanelControl*)control)->enabled;
			if ( enab && ( control->hasChanged() || control->hasResized() ) ) {
				control->updateFbo();	// update fbo before GUI drawing
				changed = true;
			}
		}
		return changed;
	}
	bool SimpleGUI::onResize( app::ResizeEvent event )
	{
		for (std::vector<Control*>::iterator it = controls.begin() ; it != controls.end() ; it++) {
			Control* control = *it;
			control->onResize(event);
		}
		bShouldResize = true;
		return false;
	}
	bool SimpleGUI::onKeyDown( app::KeyEvent event )
	{
		if ( mouseControl )
		{
			switch( event.getCode() ) {
				case KeyEvent::KEY_UP:
				case KeyEvent::KEY_RIGHT:
				case KeyEvent::KEY_PLUS:
				case KeyEvent::KEY_EQUALS:
					mouseControl->inc( event.isShiftDown() );
					return true;
				case KeyEvent::KEY_DOWN:
				case KeyEvent::KEY_LEFT:
				case KeyEvent::KEY_MINUS:
				case KeyEvent::KEY_UNDERSCORE:
					mouseControl->dec( event.isShiftDown() );
					return true;
				case 'r':
				case 'R':
				case 'd':
				case 'D':
					mouseControl->reset();
					return true;
				default:
					return mouseControl->updateKeyboard(event.getChar());
			}
		}
		return false;
	}
	//
	// FBO
	//
	void SimpleGUI::draw() {	
		gl::disableDepthRead();	
		gl::disableDepthWrite();

		// check mouse over requesting redraw
		for (std::vector<Control*>::iterator it = controls.begin() ; it != controls.end() ; it++) {
			Control* control = *it;
			control->mustRefresh |= control->updateMouse();
		}

		if (bUsingFbo)
		{
			// Update Fbo
			bool updated = false;
#ifdef DEBUG_FBO
			//printf("frame [%d] GUI Fbo...\n",app::getElapsedFrames());
#endif
			if ( this->hasChanged() || bShouldResize )
			{
				if ( !mFbo || bShouldResize )
				{
					mFbo = gl::Fbo( app::getWindowWidth(), app::getWindowHeight(), true, true, false );
					mFbo.getTexture().setFlipped();
					bShouldResize = false;
					bForceRedraw = true;
#ifdef DEBUG_FBO
					printf("frame [%d] GUI Fbo Resized!\n",app::getElapsedFrames());
#endif
				}
				mFbo.bindFramebuffer();
				if ( bForceRedraw )
					gl::clear( ColorA::zero() );
				gl::setMatricesWindow( mFbo.getSize() );
				gl::setViewport( mFbo.getBounds() );
				//gl::clear( ColorA(1,0,0,0.5) );	// debug size with color
				this->drawGui();
				mFbo.unbindFramebuffer();
				updated = true;
#ifdef DEBUG_FBO
				printf("frame [%d] GUI Fbo updated!\n",app::getElapsedFrames());
#endif
			}
			// draw over
			Rectf bounds = Rectf( Vec2f(0,0), this->getSize() );
			Area srcFlipped = Area( bounds.x1, app::getWindowHeight()-bounds.y1, bounds.x2, app::getWindowHeight()-bounds.y2 );
			gl::setMatricesWindow( getWindowSize() );
			gl::setViewport( getWindowBounds() );
			gl::enableAlphaBlending();
#ifdef BLINK_FBO
			gl::color( ColorA(1,1,1,(updated?0.85:1)) );		// blink FBO when updated
#else
			gl::color( ColorA::white() );
#endif
			//gl::draw( mFbo.getTexture() );
			gl::draw( mFbo.getTexture(), srcFlipped, bounds );
		}
		else
		{
			gl::setMatricesWindow(getWindowSize());
			gl::setViewport( getWindowBounds() );
			gl::color( ColorA::white() );
			this->drawGui();
		}
		
		gl::color( ColorA::white() );
		gl::disableAlphaBlending();
	}
	
	void SimpleGUI::drawGui() {	
		mSize = Vec2f();
		
		gl::pushMatrices();
		glDisable( GL_TEXTURE_2D );
		glLineWidth( 1 );
		
		Vec2f position = spacing + mOffset;
		ColumnControl* currColumn = NULL;
		PanelControl* currPanel = NULL;
		bool refreshPanel = false;
		bool colEnabled = true;
		
		for (std::vector<Control*>::iterator it = controls.begin() ; it != controls.end() ; it++) {
			gl::disableAlphaBlending();
			Control* control = *it;
			if (control->type == Control::COLUMN) {
				colEnabled = ((ColumnControl*)control)->enabled;
				if (colEnabled)
				{
					if (currColumn == NULL) { //first column				
						//each column moves everything to the right so we want compensate that 
						position.x -= SimpleGUI::labelSize.x;
					}
					currColumn = (ColumnControl*)control;
					currPanel = NULL;
					refreshPanel = false;
				}
			}
			if (!colEnabled)
				continue;
			// if resized, erase everything under control
			if (control->hasResized() && !refreshPanel)
			{
				refreshPanel = true;
				Rectf eraseArea = Rectf((-SimpleGUI::padding).x,
										(-SimpleGUI::padding).y,
										(SimpleGUI::sliderSize + SimpleGUI::padding).x,
										( mFbo.getSize().y - position.y ) );
#ifdef BLINK_CONTROLS
				gl::color( ColorA( Rand::randFloat(0.25,1.0), Rand::randFloat(0.25,1.0), Rand::randFloat(0.25,1.0), 0.7) );
#else
				gl::color( ColorA::zero() );
#endif
				gl::drawSolidRect(eraseArea + position);
			}
			// save panel
			if (control->type == Control::PANEL)
			{
				float col = false;	// affects whole column
				if ( currPanel )
					if ( currPanel->enabled == false && currPanel->column == true )
						col = true;
				if ( col == false )
				{
					currPanel = (PanelControl*)control;
					if (currPanel->valueHasChanged())
					{
						control->draw(position);	// draw to update valueHasChanged()
						continue;
					}
				}
			}
			if (currPanel != NULL)
				if (!currPanel->enabled)
					continue;
#ifdef BLINK_CONTROLS
			SimpleGUI::bgColor = ColorA( Rand::randFloat(0.25,1.0), Rand::randFloat(0.25,1.0), Rand::randFloat(0.25,1.0), 0.7);
#endif
			if ( bForceRedraw || refreshPanel || control->hasChanged() || control->type == Control::COLUMN )
			{
				Vec2f newPos = control->draw(position);
				control->drawOffset = (newPos - position);
				control->mustRefresh = false;
				position = newPos;
				if (control->panelToSwitch)
					control->panelToSwitch->enabled = ( control->invertSwitch ? ! control->isOn() : control->isOn() );
			}
			else
				position += control->drawOffset;
			
			// ROGER
			if ( position.y > mSize.y )
				mSize.y = position.y;
			if ( position.x > mSize.x )
				mSize.x = position.x;
		}
		mFinalPosition = Vec2f( position.x + SimpleGUI::labelSize.x, 0 );
		
		// ROGER
		// Finish size by adding the same a column adds
		mSize.x += SimpleGUI::labelSize.x + SimpleGUI::spacing.x + SimpleGUI::padding.x*2;
		
		gl::disableAlphaBlending();
		gl::popMatrices();
		
		bForceRedraw = false;
	}
	
	bool SimpleGUI::isEnabled() {
		return enabled;
	}
	
	void SimpleGUI::setEnabled(bool state) {
		enabled = state;
		if (enabled)
			bForceRedraw = true;
	}
	
	void SimpleGUI::dump() {
		for (std::vector<Control*>::iterator it = controls.begin() ; it != controls.end() ; it++) {
			Control* control = *it;
			std::string str = control->toString();
			std::cout << control->name << " : " + str << std::endl;
		}
	}
	
	void SimpleGUI::save(std::string fileName) {
		console() << "SimpleGUI::save " << fileName << std::endl;
		std::fstream file_op(fileName.c_str(), std::ios::out);	
		for (std::vector<Control*>::iterator it = controls.begin() ; it != controls.end() ; it++) {
			Control* control = *it;
			std::string str = control->toString();
			file_op << control->name << " : " + str << std::endl;
		}
		file_op.close();
	}
	
	void SimpleGUI::load(std::string fileName) {
		console() << "SimpleGUI::load " << fileName << std::endl;
		std::fstream file_op(fileName.c_str(), std::ios::in);
		char str[2000];
		if ((file_op.rdstate() & std::ifstream::failbit ) != 0) {
			console() << "SimpleGUI::load " << "failed" << std::endl;
			return;
		}
		while(!file_op.eof()) {
			file_op.getline(str, 2000);
			std::string line(str);
			size_t colonPos = line.find(":");
			
			if (colonPos == std::string::npos) {
				continue;
			}
			
			std::string name = line.substr(0, colonPos);
			std::string value = line.substr(colonPos + 1);
			
			boost::trim(name);
			boost::trim(value);
			
			Control* control = getControlByName(name);
			if (control) {
				control->fromString(value);
			}		
		}    
		file_op.close();
	}
	
	Control * SimpleGUI::getMouseOverControl( Vec2i mousePos ) {
		bool colEnabled = true;
		PanelControl* currPanel = NULL;
		for (std::vector<Control*>::iterator it = controls.begin() ; it != controls.end() ; it++) {
			Control* control = *it;
			// ignore disabled columns
			if (control->type == Control::COLUMN) {
				colEnabled = ((ColumnControl*)control)->enabled;
				currPanel = NULL;
			}
			if (!colEnabled)
				continue;
			// ignore disabled panels
			if (control->type == Control::PANEL) {
				float col = false;	// affects whole column
				if ( currPanel )
					if ( currPanel->enabled == false && currPanel->column == true )
						col = true;
				if ( col == false )
					currPanel = (PanelControl*)control;
			}
			if (currPanel != NULL && !currPanel->enabled) {
				continue;
			}
			// pass on mouse event
			if (control->activeArea.contains(mousePos))
				return control;
		}
		return NULL;
	}
	
	bool SimpleGUI::onMouseMove(MouseEvent event) {
		// save control with mouse over
		mouseControl = this->getMouseOverControl( event.getPos() );
		return false;
	}
	
	bool SimpleGUI::onMouseDown(MouseEvent event) {
		if (!enabled) return false;
		
		// ROGER - pass on mouse event
		if ( mouseControl ) {
			selectedControl = mouseControl;
			selectedControl->onMouseDown(event);
			// Close lost DropDown List
			if (droppedList && droppedList != selectedControl)
			{
				droppedList->close();
				droppedList = NULL;
			}
			// Remember DropDown List
			if (selectedControl->type == Control::DROP_DOWN_VAR)
			{
				DropDownVarControl *l = (DropDownVarControl*) selectedControl;
				if ( l->dropped )
					droppedList = l;
			}
			return true;
		}
		// Close open DropDown list
		if (droppedList)
		{
			droppedList->close();
			droppedList = NULL;
		}
		return false;
	}
	
	bool SimpleGUI::onMouseUp(MouseEvent event) {
		if (!enabled)
			return false;
		if (selectedControl != NULL) {
			selectedControl->onMouseUp(event);
			selectedControl->mustRefresh = true;
			selectedControl = NULL;
			return true;
		}	
		return false;
	}
	
	bool SimpleGUI::onMouseDrag(MouseEvent event) {
		if (!enabled)
			return false;
		if (selectedControl) {
			selectedControl->onMouseDrag(event);
			return true;
		}
		return false;
	}
	
	Vec2f SimpleGUI::getStringSize(const std::string& str) {
		TextLayout text;
		text.addLine(str);
		Surface s = text.render(true);
		return s.getSize();
	}
	
	Rectf SimpleGUI::getScaledWidthRectf(Rectf rect, float scale) {
		float w = rect.getWidth() * math<float>::clamp(scale,0,1);
		return Rectf(
					 rect.getX1(),
					 rect.getY1(),
					 rect.getX1() + ( w ? w : 1.0 ),
					 rect.getY1() + rect.getHeight()
					 );
	}
	
	
	
	
	
	
	//--------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------
	//
	// CONTROL
	//
	
	Control::Control(SimpleGUI *parent) {
		this->parentGui = parent;
		bgColor = SimpleGUI::bgColor;
		drawOffset = Vec2f::zero();
		this->panelToSwitch = NULL;
		this->unitControl = NULL;
		this->invertSwitch = false;
		this->readOnly = false;
		this->displayValue = false;
		this->channelOver = -1;
		this->mustRefresh = true;
	}
	
	void Control::setBackgroundColor(ColorA color) {
		bgColor = color;
	}

	void Control::setReadOnly(bool b)
	{
		readOnly=b;
		if (b)
			displayValue=true;
		this->update();
	}
	
	void Control::drawBackArea(Rectf a)
	{
		gl::enableAlphaBlending();
		gl::color(SimpleGUI::bgColor);
		gl::drawSolidRect( a );
		gl::disableAlphaBlending();
	}


	
	
	
	//--------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------
	//
	// VAR CONTROLS
	//
	
	//-----------------------------------------------------------------------------
	
	FloatVarControl::FloatVarControl(SimpleGUI *parent, const std::string & name, float* var, float min, float max, float defaultValue) : Control(parent) {
		this->type = Control::FLOAT_VAR;
		this->name = name;
		this->var = var;
		this->min = min;
		this->max = max;
		this->percentage = false;
		this->setPrecision( (max-min) <= 1.0 ? 2 : 1 );
		*var = math<float>::clamp( defaultValue, min, max );
		// ROGER
		this->defaultValue = defaultValue;
		this->lastValue = *var;
		this->formatAsTime = false;
		this->axisOnDefault = false;
		this->update();
	}
	
	void FloatVarControl::setPrecision(int p) {
		if (p == 100)
		{
			percentage = true;
			p = 2;
		}
		this->precision = p;
		this->step = (1.0f/pow(10.0f,p));
	}

	// ROGER
	void FloatVarControl::update()
	{
		activeAreaBase = Rectf(0, 
							   SimpleGUI::labelSize.y + SimpleGUI::padding.y, 
							   SimpleGUI::sliderSize.x, 
							   SimpleGUI::labelSize.y + ( !readOnly ? SimpleGUI::padding.y + SimpleGUI::sliderSize.y : 0 ) );
		backArea = Rectf((-SimpleGUI::padding).x, 
						 (-SimpleGUI::padding).y, 
						 (SimpleGUI::sliderSize + SimpleGUI::padding).x, 
						 (SimpleGUI::labelSize + SimpleGUI::padding + ( !readOnly ? SimpleGUI::sliderSize + SimpleGUI::padding : Vec2f::zero() ) ).y );	
	}
	
	bool FloatVarControl::updateMouse()
	{
		// save current active channel
		int oldChannel = channelOver;
		channelOver = ( activeArea.contains(AppBasic::get()->getMousePos()) ? 0 : -1 );
		return ( channelOver != oldChannel );
	}

	
	Vec2f FloatVarControl::draw(Vec2f pos) {
		
		this->lastValue = *var;
		
		activeArea = activeAreaBase + pos;
		
		gl::color(SimpleGUI::bgColor);
		gl::drawSolidRect( backArea + pos );
		
		gl::enableAlphaBlending();
		gl::drawString(name, pos, SimpleGUI::textColor, SimpleGUI::textFont);
		if (displayValue)
			gl::drawStringRight(this->toString(), pos+Vec2f(SimpleGUI::sliderSize.x,0), SimpleGUI::textColor, SimpleGUI::textFont);
		gl::disableAlphaBlending();
		
		if (!readOnly)
		{
			float vd = getNormalizedValue(this->displayedValue(defaultValue,step),min,max);
			float v = getNormalizedValue(this->displayedValue(*var,step),min,max);
			Rectf rd = SimpleGUI::getScaledWidthRectf(activeArea, vd);
			Rectf r = SimpleGUI::getScaledWidthRectf(activeArea, v);
			// back
			gl::color(SimpleGUI::darkColor);
			gl::drawSolidRect(activeArea);
			// color bar
			if (axisOnDefault)
				r = Rectf( r.getLowerRight(), rd.getUpperRight() );
			gl::color(SimpleGUI::lightColor);
			gl::drawSolidRect(r);
			// default value line
			gl::color( axisOnDefault && fabs(r.getWidth())<=1 ? SimpleGUI::lightColor : SimpleGUI::mouseOverColor );
			gl::drawLine(rd.getLowerRight(), rd.getUpperRight());
			// highlight border
			if (this->isHighlighted())
			{
				gl::color(SimpleGUI::mouseOverColor);
				gl::drawStrokedRect(activeArea);
			}
		}
		
		pos.y += backArea.getHeight() + SimpleGUI::spacing.y;	
		return pos;
	}
	
	std::string FloatVarControl::toString() {
		float v = *var;
		if (unitControl)
			v *= unitControl->getValue();
		std::ostringstream ss;
		ss.setf(std::ios::fixed);
		if (formatAsTime)
		{
			ss.precision(2);
			int m = (int) ( v / 60.0f );
			int s = (int) ( v - (m * 60.0f) );
			int f = (int) ( (v-(int)v) * 30.0 );
			if (m)
				ss << m << ":" << std::setfill('0') << std::setw(2) << s;
			else
				ss << s;
			ss << ":" << std::setfill('0') << std::setw(2) << f;
			
		}
		else if (percentage)
		{
			int p = (int) (v * 100.0f);
			ss << p << "%";
		}
		else
		{
			ss.precision(this->precision);
			ss << this->displayedValue( v, step );
		}
		return ss.str();
	}

	
	void FloatVarControl::fromString(std::string& strValue) {
		*var = boost::lexical_cast<float>(strValue);
	}
	
	void FloatVarControl::onMouseDown(MouseEvent event) {
		onMouseDrag(event);	
	}
	
	void FloatVarControl::onMouseDrag(MouseEvent event) {
		float value = (event.getPos().x - activeArea.x1)/(activeArea.x2 - activeArea.x1);
		value = math<float>::max(0.0, math<float>::min(value, 1.0));	
		setNormalizedValue(value);
	}
	void FloatVarControl::setNormalizedValue(float value) {
		float newValue = min + value*(max - min);
		if ( this->displayedValue(newValue,step) != this->displayedValue(*var,step) )
			*var = newValue;
	}
	
	
	//-----------------------------------------------------------------------------
	
	IntVarControl::IntVarControl(SimpleGUI *parent, const std::string & name, int* var, int min, int max, int defaultValue) : Control(parent) {
		this->type = Control::INT_VAR;
		this->name = name;
		this->var = var;
		this->min = min;
		this->max = max;
		this->step = 1;
		if ( var )
			*var = math<int>::clamp( defaultValue, min, max );
		// ROGER
		this->defaultValue = defaultValue;
		this->lastValue = *var;
		this->update();
	}
	
	// ROGER
	void IntVarControl::setStep(int s) {
		step = s;
		if (step > 1)
			*var -= ( *var % step);
		this->update();
	}
	void IntVarControl::update()
	{
		int slots = ( (max - min) / step) + 1;
		int slotsMax = math<int>::max( 10, SimpleGUI::sliderSize.x / (SimpleGUI::radioSize+SimpleGUI::padding).x );
		if ( slots <= slotsMax )
		{
			// RADIO
			float gapY = SimpleGUI::labelSize.y + SimpleGUI::padding.y;
			items.clear();
			for (int n = min ; n <= max ; n += step )
			{
				listVarItem _item =  listVarItem();
				_item.key = n;
				_item.label = ci::toString<int>( n );
				items.push_back( _item );
				
				activeAreaBase = Rectf(0, gapY, SimpleGUI::radioSize.x, gapY + SimpleGUI::radioSize.y);
				for (int i = 0 ; i < items.size(); i++) {
					items[i].activeAreaBase = Rectf((SimpleGUI::radioSize+SimpleGUI::padding).x*i,
													gapY,
													(SimpleGUI::radioSize+SimpleGUI::padding).x*i + SimpleGUI::radioSize.x ,
													gapY + SimpleGUI::radioSize.y );
					activeAreaBase.x2 = items[i].activeAreaBase.x2;
				}
			}
		}
		else
		{
			// SLIDER
			activeAreaBase = Rectf(0, 
								   SimpleGUI::labelSize.y + SimpleGUI::padding.y, 
								   SimpleGUI::sliderSize.x, 
								   SimpleGUI::labelSize.y + ( !readOnly ? SimpleGUI::padding.y + SimpleGUI::sliderSize.y : 0 ) );
		}
		
		backArea = Rectf((-SimpleGUI::padding).x, 
						 (-SimpleGUI::padding).y, 
						 (SimpleGUI::sliderSize + SimpleGUI::padding).x, 
						 (SimpleGUI::labelSize + SimpleGUI::padding + ( !readOnly ? SimpleGUI::sliderSize + SimpleGUI::padding : Vec2f::zero() ) ).y );
	}
	
	bool IntVarControl::updateMouse()
	{
		int oldChannel = channelOver;
		channelOver = ( activeArea.contains(AppBasic::get()->getMousePos()) ? 0 : -1 );
		return ( channelOver != oldChannel );
	}
	
	Vec2f IntVarControl::draw(Vec2f pos) {
		this->lastValue = *var;
		
		activeArea = activeAreaBase + pos;
		
		gl::color(SimpleGUI::bgColor);
		gl::drawSolidRect( backArea + pos );
		
		gl::enableAlphaBlending();
		gl::drawString(name, pos, SimpleGUI::textColor, SimpleGUI::textFont);
		
		// ROGER
		if (displayValue)
			gl::drawStringRight(this->toString(), pos+Vec2f(SimpleGUI::sliderSize.x,0), SimpleGUI::textColor, SimpleGUI::textFont);
		if (!readOnly)
		{
			if (items.size() > 0)
			{
				// RADIO
				for (int i = 0 ; i < items.size() ; i++)
				{
					bool selected = ( *var == min + (i * step) );
					items[i].activeArea = items[i].activeAreaBase + pos;
					gl::color( selected ? SimpleGUI::lightColor : SimpleGUI::darkColor );
					gl::drawSolidRect(items[i].activeArea);
				}
			}
			else
			{
				// SLIDER
				float v = getNormalizedValue(*var,min,max);
				gl::color(SimpleGUI::darkColor);
				gl::drawSolidRect(activeArea);
				gl::color(SimpleGUI::lightColor);
				gl::drawSolidRect(SimpleGUI::getScaledWidthRectf(activeArea, v));
				if (this->isHighlighted())
				{
					gl::color(SimpleGUI::mouseOverColor);
					gl::drawStrokedRect(activeArea);
				}
			}
		}
		gl::disableAlphaBlending();

		pos.y += backArea.getHeight() + SimpleGUI::spacing.y;	
		return pos;	
	}	
	
	std::string IntVarControl::toString() {
		std::stringstream ss;
		ss << *var;
		return ss.str();	
	}
	
	void IntVarControl::fromString(std::string& strValue) {
		*var = boost::lexical_cast<int>(strValue);
	}
	
	void IntVarControl::onMouseDown(MouseEvent event) {
		onMouseDrag(event);
	}
	
	void IntVarControl::onMouseDrag(MouseEvent event) {
		if (!readOnly)
		{
			if (items.size())
			{
				//RADIOS
				for (int i = 0 ; i < items.size() ; i++) {
					if (items[i].activeArea.contains(event.getPos())) {
						*this->var = items[i].key;
					}
				}
			}
			else
			{
				// SLIDER
				float value = (event.getPos().x - activeArea.x1)/(activeArea.x2 - activeArea.x1);
				value = math<float>::max(0.0, math<float>::min(value, 1.0));
				setNormalizedValue(value);
			}
		}
	}
	void IntVarControl::setNormalizedValue(float value) {
		int newValue = min + value*(max - min);
		if (step > 1)
			newValue -= ( newValue % step);
		if (newValue != *var)
			*var = newValue;
	}
	
	//-----------------------------------------------------------------------------
	// ROGER
	ByteVarControl::ByteVarControl(SimpleGUI *parent, const std::string & name, unsigned char* var, unsigned char defaultValue) : Control(parent) {
		this->type = Control::INT_VAR;
		this->name = name;
		this->var = var;
		this->min = 0;
		this->max = 255;
		*var = math<unsigned char>::clamp( defaultValue, min, max );
		// ROGER
		this->defaultValue = defaultValue;
		this->lastValue = *var;
		this->update();
		displayChar = false;
		displayHex = false;
	}
	
	void ByteVarControl::update()
	{
		activeAreaBase = Rectf(0, 
							   SimpleGUI::labelSize.y + SimpleGUI::padding.y, 
							   SimpleGUI::sliderSize.x, 
							   SimpleGUI::labelSize.y + ( !readOnly ? SimpleGUI::padding.y + SimpleGUI::sliderSize.y : 0 ) );
		
		backArea = Rectf((-SimpleGUI::padding).x, 
						 (-SimpleGUI::padding).y, 
						 (SimpleGUI::sliderSize + SimpleGUI::padding).x, 
						 (SimpleGUI::labelSize + SimpleGUI::padding + ( !readOnly ? SimpleGUI::sliderSize + SimpleGUI::padding : Vec2f::zero() ) ).y );
	}
	
	
	Vec2f ByteVarControl::draw(Vec2f pos) {
		this->lastValue = *var;
		
		activeArea = activeAreaBase + pos;
		
		gl::color(SimpleGUI::bgColor);
		gl::drawSolidRect( backArea + pos );
		
		gl::enableAlphaBlending();
		gl::drawString(name, pos, SimpleGUI::textColor, SimpleGUI::textFont);
		
		// ROGER
		if (displayValue)
		{
			std::stringstream ss;
			ss << (int)(*var);
			if (displayChar)
			{
				char ch[3];
				sprintf(ch,"%c",(isprint(*var)?(*var):'?'));
				ss << " / " << ch;
			}
			if (displayHex)
			{
				char hx[5];
				sprintf(hx,"0x%x",(*var));
				ss << " / " << hx;
			}
			gl::drawStringRight(ss.str(), pos+Vec2f(SimpleGUI::sliderSize.x,0), SimpleGUI::textColor, SimpleGUI::textFont);
		}
		if (!readOnly)
		{
			float v = getNormalizedValue(*var,min,max);
			gl::color(SimpleGUI::darkColor);
			gl::drawSolidRect(activeArea);
			gl::color(SimpleGUI::lightColor);
			gl::drawSolidRect(SimpleGUI::getScaledWidthRectf(activeArea, v));
		}
		gl::disableAlphaBlending();
		
		pos.y += backArea.getHeight() + SimpleGUI::spacing.y;	
		return pos;	
	}	
	
	std::string ByteVarControl::toString() {
		std::stringstream ss;
		ss << (int)(*var);
		return ss.str();
	}
	
	void ByteVarControl::fromString(std::string& strValue) {
		*var = (unsigned char) boost::lexical_cast<int>(strValue);
	}
	
	void ByteVarControl::onMouseDown(MouseEvent event) {
		onMouseDrag(event);
	}
	
	void ByteVarControl::onMouseDrag(MouseEvent event) {
		if (!readOnly)
		{
			float value = (event.getPos().x - activeArea.x1)/(activeArea.x2 - activeArea.x1);
			value = math<float>::max(0.0, math<float>::min(value, 1.0));
			setNormalizedValue(value);
		}
	}
	void ByteVarControl::setNormalizedValue(float value) {
		unsigned char newValue = min + (unsigned char) (value * (max - min));
		if (newValue != *var)
			*var = newValue;
	}
	
	//-----------------------------------------------------------------------------
	// ROGER
	FlagVarControl::FlagVarControl(SimpleGUI *parent, const std::string & name, unsigned char* var, int maxf, unsigned char defaultValue) : ByteVarControl(parent,name,var,defaultValue) {
		this->touchedItem = -1;
		this->setMaxFlags( maxf );
	}
	
	void FlagVarControl::setMaxFlags(int maxf) {
		this->maxFlags = maxf;
		this->update();
	}
	
	void FlagVarControl::update()
	{
		float gapY = SimpleGUI::labelSize.y + SimpleGUI::padding.y;
		activeAreaBase = Rectf(0, gapY, SimpleGUI::radioSize.x, gapY + SimpleGUI::radioSize.y);
		
		items.clear();
		for (int n = 0 ; n < maxFlags ; n ++ )
		{
			listVarItem _item = listVarItem();
			_item.key = ( 0x1 << n );
			_item.label = ci::toString<int>( n+1 );
			items.push_back( _item );
			
			items[n].activeAreaBase = Rectf((SimpleGUI::radioSize+SimpleGUI::padding).x*n,
											gapY,
											(SimpleGUI::radioSize+SimpleGUI::padding).x*n + SimpleGUI::radioSize.x ,
											gapY + SimpleGUI::radioSize.y );
			activeAreaBase.x2 = items[n].activeAreaBase.x2;
		}
		
		backArea = Rectf((-SimpleGUI::padding).x, 
						 (-SimpleGUI::padding).y, 
						 (SimpleGUI::sliderSize + SimpleGUI::padding).x, 
						 (SimpleGUI::labelSize + SimpleGUI::padding + ( !readOnly ? SimpleGUI::sliderSize + SimpleGUI::padding : Vec2f::zero() ) ).y );
	}
	
	Vec2f FlagVarControl::draw(Vec2f pos) {
		this->lastValue = *var;
		
		activeArea = activeAreaBase + pos;
		
		gl::color(SimpleGUI::bgColor);
		gl::drawSolidRect( backArea + pos );
		
		gl::enableAlphaBlending();
		gl::drawString(name, pos, SimpleGUI::textColor, SimpleGUI::textFont);
		
		// ROGER
		if (displayValue)
		{
			std::stringstream ss;
			// numbers
			for (int i = 0 ; i < items.size() ; i++)
			{
				if ( *var & (unsigned char) items[i].key )
				{
					if (flagChars.length() >= i+1)
						ss << flagChars[i];
					else
						ss << (i+1);
				}
			}
			// hex
			if (displayHex)
			{
				char hx[5];
				sprintf(hx,"0x%x",(*var));
				ss << "/" << hx;
			}
			gl::drawStringRight(ss.str(), pos+Vec2f(SimpleGUI::sliderSize.x,0), SimpleGUI::textColor, SimpleGUI::textFont);
		}
		if (!readOnly)
		{
			// RADIO
			for (int i = 0 ; i < items.size() ; i++)
			{
				bool selected = ( *var & (unsigned char) items[i].key );
				items[i].activeArea = items[i].activeAreaBase + pos;
				gl::color( selected ? SimpleGUI::lightColor : SimpleGUI::darkColor );
				gl::drawSolidRect(items[i].activeArea);
			}
		}
		gl::disableAlphaBlending();
		
		pos.y += backArea.getHeight() + SimpleGUI::spacing.y;	
		return pos;	
	}	

	void FlagVarControl::onMouseDown(MouseEvent event) {
		this->touchedItem = -1;
		this->touchedState = -1;
		onMouseDrag(event);
	}
	
	void FlagVarControl::onMouseDrag(MouseEvent event) {
		if (!readOnly)
		{
			int item = -1;
			for (int i = 0 ; i < items.size() ; i++) {
				if (items[i].activeArea.contains(event.getPos())) {
					item = i;
					break;
				}
			}
			if ( item >= 0 )
			{
				// Save 1st touched flag state to replicate action
				if ( this->touchedState == -1 )
					this->touchedState = ( *var & (unsigned char) items[item].key );
				// set/unset
				if ( this->touchedState == false )
					(*this->var) |= items[item].key;		// turn ON
				else
					(*this->var) &= ~(items[item].key);		// turn OFF
				touchedItem = item;
			}
		}
	}
	
	
	//-----------------------------------------------------------------------------
	
	BoolVarControl::BoolVarControl(SimpleGUI *parent, const std::string & name, bool* var, bool defaultValue, int groupId) : Control(parent) {
		this->type = Control::BOOL_VAR;
		this->name = name;
		this->nameOff = name;
		this->var = var;
		this->groupId = groupId;
		*var = defaultValue;
		// ROGER
		this->defaultValue = defaultValue;
		this->lastValue = *var;
		this->asButton = false;
		this->update();
	}	
	
	void BoolVarControl::update() {
		if (this->asButton)
		{
			activeAreaBase = Rectf(0,0,SimpleGUI::sliderSize.x,SimpleGUI::buttonSize.y );
			backArea = Rectf((-SimpleGUI::padding).x,
							 (-SimpleGUI::padding).y,
							 (SimpleGUI::buttonSize + SimpleGUI::padding).x,
							 (SimpleGUI::buttonSize + SimpleGUI::padding).y );
		}
		else {
			activeAreaBase = Rectf(0, 0, SimpleGUI::radioSize.x, SimpleGUI::radioSize.y);
			backArea = Rectf((-SimpleGUI::padding).x,
							 (-SimpleGUI::padding).y,
							 (SimpleGUI::sliderSize + SimpleGUI::padding).x,
							 (SimpleGUI::sliderSize + SimpleGUI::padding).y );
		}
	}	
	
	Vec2f BoolVarControl::draw(Vec2f pos) {
		this->lastValue = *var;
		
		activeArea = activeAreaBase + pos;
		
		gl::color(SimpleGUI::bgColor);
		gl::drawSolidRect(backArea + pos);
		
		gl::color((*var) ? SimpleGUI::lightColor : SimpleGUI::darkColor);
		gl::drawSolidRect(activeArea);
		
		float x = pos.x + ( asButton ? SimpleGUI::buttonGap.x : SimpleGUI::radioSize.x + SimpleGUI::padding.x*2 );
		gl::enableAlphaBlending();
		gl::drawString( ((*var)?name:nameOff), Vec2f(x, pos.y), (asButton&&(*var) ? SimpleGUI::darkColor : SimpleGUI::textColor), SimpleGUI::textFont);					
		gl::disableAlphaBlending();
		
		pos.y += backArea.getHeight() + SimpleGUI::spacing.y;	
		return pos;
	}	
	
	std::string BoolVarControl::toString() {
		std::stringstream ss;
		ss << *var;
		return ss.str();	
	}
	
	void BoolVarControl::fromString(std::string& strValue) {
		int value = boost::lexical_cast<int>(strValue);	
		*var = value ? true : false;
	}
	
	void BoolVarControl::onMouseDown(MouseEvent event) {
		*this->var = ! *this->var;
	}
	
	//-----------------------------------------------------------------------------
	
	// ROGER
	ColorVarControl::ColorVarControl(SimpleGUI *parent, const std::string & name, Color* var, Color defaultValue, int colorModel) : Control(parent) {
		this->type = Control::COLOR_VAR;
		this->name = name;
		this->var = var;
		this->varA = NULL;
		this->colorModel = colorModel;
		this->displayHex = false;
		*var = defaultValue;
		activeTrack = 0;
		// ROGER
		this->defaultValue = defaultValue;
		this->lastValue = *var;
		alphaControl = false;
		this->update();
	}
	
	ColorVarControl::ColorVarControl(SimpleGUI *parent, const std::string & name, ColorA* var, ColorA defaultValue, int colorModel) : Control(parent) {
		this->type = Control::COLOR_VAR;
		this->name = name;
		this->var = NULL;
		this->varA = var;
		this->colorModel = colorModel;
		*var = defaultValue;
		activeTrack = 0;
		// ROGER
		this->lastValueA = *var;
		alphaControl = true;
		this->update();
	}
	
	void ColorVarControl::update() {
		channelCount = (alphaControl?4:3);
		for (int i = 0 ; i < channelCount ; i++)
			activeAreasBase[i] = Rectf(0,
									   SimpleGUI::labelSize.y + SimpleGUI::sliderSize.y*i + SimpleGUI::padding.y*(i+1),
									   SimpleGUI::sliderSize.x,
									   SimpleGUI::sliderSize.y*(i+1) + SimpleGUI::padding.y*(i+1) + SimpleGUI::labelSize.y );
		activeAreaBase = Rectf(
							   0, 
							   SimpleGUI::labelSize.y, 
							   SimpleGUI::sliderSize.x, 
							   SimpleGUI::sliderSize.y*channelCount + SimpleGUI::padding.y*channelCount + SimpleGUI::labelSize.y );
		backArea = Rectf((-SimpleGUI::padding).x,
						 (-SimpleGUI::padding).y,
						 (SimpleGUI::sliderSize + SimpleGUI::padding).x,
						 (SimpleGUI::labelSize + SimpleGUI::sliderSize*channelCount + SimpleGUI::padding*(channelCount+1)).y );
		previewArea = Rectf((SimpleGUI::sliderSize.x * 0.75),
							0,
							SimpleGUI::sliderSize.x,
							SimpleGUI::sliderSize.y );
	}
	
	bool ColorVarControl::updateMouse()
	{
		int oldChannel = channelOver;
		channelOver = -1;
		for (int i = 0 ; i < channelCount ; i++)
			if (activeAreas[i].contains(AppBasic::get()->getMousePos()))
				channelOver = i;
		return ( channelOver != oldChannel );
	}

	void ColorVarControl::setByteValue(int ch, unsigned char b)
	{
		if (channelOver)
		{
			float v = math<float>::clamp( b, 0.0f, 255.0f ) / 255.0f;
			if (alphaControl)
				(*varA)[channelOver] = v;
			else
				(*var)[channelOver] = v;
		}
	}
	unsigned char ColorVarControl::getByteValue(int ch)
	{
		return (char) math<float>::clamp( (alphaControl ? (*varA)[ch] : (*var)[ch]) * 255.0f, 0.0f, 255.0f);
	}

	// keyboard inc/dec
	void ColorVarControl::inc(bool shifted)
	{
		this->incdec( shifted ? 16 : 1 );
	}
	void ColorVarControl::dec(bool shifted)
	{
		this->incdec( shifted ? -16 : -1 );
	}
	void ColorVarControl::incdec(int step)
	{
		if (channelOver >= 0)
		{
			float b = (float) this->getByteValue( channelOver );
			b = math<float>::clamp( b+step, 0.0f, 255.0f );
			this->setByteValue( channelOver, (unsigned char) b );
		}
	}

	// keya 1-F
	bool ColorVarControl::updateKeyboard(char c)
	{
		if (channelOver >= 0)
		{
			if ( (c >= '0' && c <= '9') || (toupper(c) >= 'A' && toupper(c) <= 'F') )
			{
				unsigned char b = this->getByteValue(channelOver);
				char hx[3];
				sprintf(hx,"%2X",b);
				hx[0] = hx[1];
				hx[1] = toupper(c);
				b = ( hx[0] - ( hx[0] >= 'A' ? 'A'-10 : '0' ) ) * 16;
				b += hx[1] - ( hx[1] >= 'A' ? 'A'-10 : '0' );
				this->setByteValue(channelOver,b);
				return true;
			}
		}
		return false;
	}
	
	Vec2f ColorVarControl::draw(Vec2f pos) {
		if (alphaControl)
			this->lastValueA = *varA;
		else
			this->lastValue = *var;
		for (int i = 0 ; i < channelCount ; i++)
			activeAreas[i] = activeAreasBase[i] + pos;
		
		Vec4f values;
		if (colorModel == SimpleGUI::RGB) {
			if (alphaControl)
				values = Vec4f(varA->r, varA->g, varA->b, varA->a);
			else
				values = Vec4f(var->r, var->g, var->b);
		}
		else {
			Vec3f hsv = ( alphaControl ? rgbToHSV(*varA) : rgbToHSV(*var) );
			values.x = hsv.x;
			values.y = hsv.y;
			values.z = hsv.z;
			if (alphaControl)
				values.w = varA->a;
		}
		
		activeArea = activeAreaBase + pos;
		for (int i = 0 ; i < channelCount ; i++)
			activeAreas[i] = activeAreasBase[i] + pos;
		
		gl::color(SimpleGUI::bgColor);
		gl::drawSolidRect(backArea+pos);
		
		gl::enableAlphaBlending();
		gl::drawString(name, pos, SimpleGUI::textColor, SimpleGUI::textFont);
		gl::disableAlphaBlending();
		
		// ROGER
		if (alphaControl)
			gl::color( *varA );
		else
			gl::color( *var );
		gl::drawSolidRect(previewArea+pos);
		
		for (int i = 0 ; i < channelCount ; i++)
		{
			gl::color(SimpleGUI::darkColor);
			gl::drawSolidRect(activeAreas[i]);
			if (this->isHighlighted(i))
			{
				gl::color(SimpleGUI::mouseOverColor);
				gl::drawStrokedRect(activeAreas[i]);
			}
		}
		gl::color(SimpleGUI::lightColor);
		for (int i = 0 ; i < channelCount ; i++)
		{
			Rectf r = SimpleGUI::getScaledWidthRectf(activeAreas[i], values[i]);
			gl::drawLine(Vec2f(r.x2, r.y1), Vec2f(r.x2, r.y2));				
		}
		if (displayHex)
		{
			gl::enableAlphaBlending();
			for (int i = 0 ; i < channelCount ; i++)
			{
				char hx[3];
				sprintf(hx,"%02X",(unsigned int)((*var)[i]*255));
				gl::drawStringRight(hx, activeAreas[i].getUpperRight()+Vec2f(0,0), SimpleGUI::textColor, SimpleGUI::textFont);
			}
			gl::disableAlphaBlending();
		}
		
		pos.y += backArea.getHeight() + SimpleGUI::spacing.y;	
		return pos;
	}
	
	void ColorVarControl::fromString(std::string& strValue) {
		std::vector<std::string> strs;
		boost::split(strs, strValue, boost::is_any_of("\t "));
		for (int i = 0 ; i < channelCount ; i++)
			if (alphaControl)
				(*varA)[i] = boost::lexical_cast<double>(strs[i]);
			else
				(*var)[i] = boost::lexical_cast<double>(strs[i]);
	}
	
	std::string ColorVarControl::toString() {
		std::stringstream ss;
		if (alphaControl)
			ss << varA->r << " " << varA->g << " " << varA->b << " " << varA->a;
		else
			ss << var->r << " " << var->g << " " << var->b;
		return ss.str();
	}
	
	
	void ColorVarControl::onMouseDown(MouseEvent event) {
		for (int i = 0 ; i < channelCount ; i++)
		{
			if (activeAreas[i].contains(event.getPos())) {
				activeTrack = i+1;
				break;
			}
		}
		onMouseDrag(event);
	}
	
	void ColorVarControl::onMouseDrag(MouseEvent event) {	
		float value = (event.getPos().x - activeArea.x1)/(activeArea.x2 - activeArea.x1);
		value = math<float>::max(0.0, math<float>::min(value, 1.0));	
		
		if (colorModel == SimpleGUI::RGB) {
			switch (activeTrack) {
				case 1: ( alphaControl ? varA->r : var->r ) = value; break;
				case 2: ( alphaControl ? varA->g : var->g ) = value; break;
				case 3: ( alphaControl ? varA->b : var->b ) = value; break;
				case 4: varA->a = value; break;				
			}
		}
		else {
			Vec3f hsv = ( alphaControl ? rgbToHSV(*varA) : rgbToHSV(*var) );
			switch (activeTrack) {
				case 1: hsv.x = value; break;
				case 2: hsv.y = value; break;
				case 3: hsv.z = value; break;
				case 4: varA->a = value; break;				
			}
			if (alphaControl)
				*varA = ColorA(CM_HSV, hsv.x, hsv.y, hsv.z, varA->a);
			else
				*var = Color(CM_HSV, hsv.x, hsv.y, hsv.z);
		}
	}
	
	//-----------------------------------------------------------------------------
	// VECTOR CONTROL
	//
	VectorVarControl::VectorVarControl(SimpleGUI *parent, const std::string & name, Vec4f* var, int vc, float min, float max, Vec4f defaultValue) : Control(parent) {
		this->type = Control::VECTOR_VAR;
		this->name = name;
		this->var = var;
		this->vecCount = vc;
		this->min = min;
		this->max = max;
		this->setPrecision( (max-min) <= 1.0 ? 2 : 1 );
		this->displayValue = true;
		*var = defaultValue;
		activeTrack = 0;
		// ROGER
		this->defaultValue = defaultValue;
		this->lastValue = *var;
		this->update();
	}
	
	void VectorVarControl::setPrecision(int p) {
		this->precision = p;
		this->step = (1.0f/pow(10.0f,p));
	}

	bool VectorVarControl::updateMouse()
	{
		int oldChannel = channelOver;
		channelOver = -1;
		for (int i = 0 ; i < vecCount ; i++)
			if (activeAreas[i].contains(AppBasic::get()->getMousePos()))
				channelOver = i;
		return ( channelOver != oldChannel );
	}
	
	// keyboard inc/dec
	void VectorVarControl::inc(bool shifted)
	{
		this->incdec( step * ( shifted ? 10.0 : 1.0 ) );
	}
	void VectorVarControl::dec(bool shifted)
	{
		this->incdec( -step * ( shifted ? 10.0 : 1.0 ) );
	}
	void VectorVarControl::incdec(float step)
	{
		if (channelOver >= 0)
			(*var)[channelOver] = math<float>::clamp( (*var)[channelOver]+step, min, max );
	}

	void VectorVarControl::update() {
		for (int i = 0 ; i < vecCount ; i++)
			activeAreasBase[i] = Rectf(0,
									   SimpleGUI::labelSize.y + SimpleGUI::sliderSize.y*i + SimpleGUI::padding.y*(i+1),
									   SimpleGUI::sliderSize.x,
									   SimpleGUI::sliderSize.y*(i+1) + SimpleGUI::padding.y*(i+1) + SimpleGUI::labelSize.y );
		activeAreaBase = Rectf(
							   0, 
							   SimpleGUI::labelSize.y, 
							   SimpleGUI::sliderSize.x, 
							   SimpleGUI::sliderSize.y*vecCount + SimpleGUI::padding.y*vecCount + SimpleGUI::labelSize.y );
		backArea = Rectf((-SimpleGUI::padding).x,
						 (-SimpleGUI::padding).y,
						 (SimpleGUI::sliderSize + SimpleGUI::padding).x,
						 (SimpleGUI::labelSize + SimpleGUI::sliderSize*vecCount + SimpleGUI::padding*(vecCount+1)).y );
		previewArea = Rectf((SimpleGUI::sliderSize.x * 0.75),
							0,
							SimpleGUI::sliderSize.x,
							SimpleGUI::sliderSize.y );
	}
	
	Vec2f VectorVarControl::draw(Vec2f pos) {
		this->lastValue = *var;
		
		activeArea = activeAreaBase + pos;
		for (int i = 0 ; i < vecCount ; i++)
			activeAreas[i] = activeAreasBase[i] + pos;
		
		gl::color(SimpleGUI::bgColor);
		gl::drawSolidRect(backArea+pos);
		
		gl::enableAlphaBlending();
		gl::drawString(name, pos, SimpleGUI::textColor, SimpleGUI::textFont);
		gl::disableAlphaBlending();
		
		for (int i = 0 ; i < vecCount ; i++)
		{
			gl::color(SimpleGUI::darkColor);
			gl::drawSolidRect(activeAreas[i]);
			if (this->isHighlighted(i))
			{
				gl::color(SimpleGUI::mouseOverColor);
				gl::drawStrokedRect(activeAreas[i]);
			}
		}
		gl::color(SimpleGUI::lightColor);
		for (int i = 0 ; i < vecCount ; i++)
		{
			float v = getNormalizedValue(this->displayedValue((*var)[i],step),min,max);
			Rectf r = SimpleGUI::getScaledWidthRectf(activeAreas[i], v);
			gl::drawLine(Vec2f(r.x2, r.y1), Vec2f(r.x2, r.y2));				
		}
		
		// values
		if (displayValue)
		{
			gl::enableAlphaBlending();
			for (int i = 0 ; i < vecCount ; i++)
			{
				std::ostringstream ss;
				ss.setf(std::ios::fixed);
				ss.precision(this->precision);
				ss << this->displayedValue( (*var)[i], step );
				gl::drawStringRight(ss.str(), activeAreas[i].getUpperRight()+Vec2f(0,0), SimpleGUI::textColor, SimpleGUI::textFont);
			}
			gl::disableAlphaBlending();
		}

		pos.y += backArea.getHeight() + SimpleGUI::spacing.y;	
		return pos;
	}
	
	void VectorVarControl::fromString(std::string& strValue) {
		std::vector<std::string> strs;
		boost::split(strs, strValue, boost::is_any_of("\t "));
		for (int i = 0 ; i < vecCount ; i++)
			(*var)[i] = boost::lexical_cast<double>(strs[i]);
	}
	
	std::string VectorVarControl::toString() {
		std::stringstream ss;
		ss << var->x << " " << var->y << " " << var->z << " " << var->z;
		return ss.str();
	}
	
	
	void VectorVarControl::onMouseDown(MouseEvent event) {
		for (int i = 0 ; i < vecCount ; i++)
		{
			if (activeAreas[i].contains(event.getPos())) {
				activeTrack = i;
				break;
			}
		}
		onMouseDrag(event);
	}
	
	void VectorVarControl::onMouseDrag(MouseEvent event) {	
		float value = (event.getPos().x - activeArea.x1)/(activeArea.x2 - activeArea.x1);
		value = math<float>::max(0.0, math<float>::min(value, 1.0));	
		if (activeTrack >= 0)
			setNormalizedValue(activeTrack,value);
	}
	void VectorVarControl::setNormalizedValue(int vec, float value) {
		float newValue = min + value*(max - min);
		if ( this->displayedValue(newValue,step) != this->displayedValue((*var)[vec],step) )
			(*var)[vec] = newValue;
	}
	
	//-----------------------------------------------------------------------------
	// ARCBALL QUATERNION CONTROL
	// from sample project: samples/ImageHeightField
	//
	ArcballVarControl::ArcballVarControl(SimpleGUI *parent, const std::string & name, Vec4f* var, Vec4f defaultValue) : Control(parent) {
		this->type = Control::ARCBALL_VAR;
		this->name = name;
		this->var = var;
		this->resetting = false;
		this->rotating = false;
		this->cameraScale = Vec3f::one();
		*var = defaultValue;
		this->defaultValue = defaultValue;
		this->lastValue = *var;
		this->defaultValue = defaultValue;
		texSize.x = (int) SimpleGUI::thumbnailSize.y;
		texSize.y = (int) SimpleGUI::thumbnailSize.y;
		// resize gui
		Vec2f nameSize = (texSize.length() ? SimpleGUI::labelSize : Vec2f::zero());
		float gap = ( texSize.x < SimpleGUI::sliderSize.x ? (SimpleGUI::sliderSize.x-texSize.x)/2.0 : 0 );
		activeAreaBase = Rectf(0, 
							   nameSize.y + SimpleGUI::padding.y, 
							   SimpleGUI::sliderSize.x,
							   nameSize.y + SimpleGUI::padding.y + texSize.y );
		// 0: arcball
		activeAreasBase[0] = Rectf(gap, 
								   nameSize.y + SimpleGUI::padding.y, 
								   gap + texSize.x,
								   nameSize.y + SimpleGUI::padding.y + texSize.y );
		// 1: reset button
		activeAreasBase[1] = Rectf((SimpleGUI::sliderSize - SimpleGUI::radioSize).x,
								   nameSize.y + SimpleGUI::padding.y, 
								   SimpleGUI::sliderSize.x,
								   nameSize.y + SimpleGUI::padding.y + SimpleGUI::radioSize.y );
		backArea = Rectf((-SimpleGUI::padding).x, 
						 (-SimpleGUI::padding).y, 
						 (SimpleGUI::sliderSize + SimpleGUI::padding).x, 
						 (nameSize + SimpleGUI::padding + texSize + SimpleGUI::padding ).y );	
		//
		// Setup Arcball
		//mArcball.setQuat( Quatf( Vec3f( 0.0577576f, -0.956794f, 0.284971f ), 3.68f ) );
		//mArcball.setQuat( Quatf( Vec3f::zAxis(), 0 ) );
		mArcball.setQuat( Quatf(*var) );
		mArcball.setRadius( SimpleGUI::sliderSize.x * 0.5f );
		this->onResize( app::ResizeEvent(getWindowSize()) );
		//printf("ARCBALL = %.4f %.4f %.4f %.4f\n",var->x,var->y,var->z,var->w);
		//
		// init FBO
		float w = texSize.x * 2;
		float h = texSize.y * 2;
		float fovGl = toDegrees( atanf((h/2.0f)/h) * 2.0f );
		fbo = gl::Fbo( w, h, false );
		cam = CameraPersp( w, h, 1.0 );
		cam.setPerspective( fovGl, 1.0, 0.1, h*2 );
		cam.lookAt( Vec3f(0,0,h), Vec3f::zero() );
		this->updateFbo();
	}
	
	void ArcballVarControl::updateFbo() {
		mArcball.setQuat( Quatf(*var) );
		//printf("ARCBALL FBO = %.4f %.4f %.4f %.4f\n",var->x,var->y,var->z,var->w);
		// draw FBO
		fbo.bindFramebuffer();
		glPolygonOffset( 1.f, 1.f );
		glEnable( GL_POLYGON_OFFSET_FILL );
		gl::enableDepthRead();
		gl::enableDepthWrite();
		gl::clear( ColorA::zero() );
		//gl::setMatricesWindow( fbo.getSize() );
		gl::setMatrices( cam );
		gl::setViewport( fbo.getBounds() );
		glPushMatrix();
		//gl::translate( fbo.getSize() * 0.5f );
		gl::rotate( mArcball.getQuat() );
		gl::scale( 1,-1,1 );
		gl::scale( cameraScale );
		float r = fbo.getHeight()*0.5f;
		gl::color( Color::red()*0.8f );
		gl::drawVector( Vec3f::zero(), Vec3f::xAxis()*r*0.3f, r*0.7f, r*0.1f );
		gl::color( Color::green()*0.8f );
		gl::drawVector( Vec3f::zero(), Vec3f::yAxis()*r*0.3f, r*0.7f, r*0.1f );
		gl::color( Color::blue()*0.8f );
		gl::drawVector( Vec3f::zero(), Vec3f::zAxis()*r*0.3f, r*0.7f, r*0.1f );
		gl::color( Color::white() );
		gl::drawColorCube( Vec3f::zero(), Vec3f::one()*r*0.6f );
		if (this->isHighlighted(0))
		{
			glLineWidth(2);
			gl::color( Color::white() );
			gl::drawStrokedCube( Vec3f::zero(), Vec3f::one()*r*0.6f );
			glLineWidth(1);
		}
		glPopMatrix();
		glDisable( GL_POLYGON_OFFSET_FILL );
		fbo.unbindFramebuffer();
	}
	
	bool ArcballVarControl::updateMouse()
	{
		int oldChannel = channelOver;
		channelOver = ( activeAreas[0].contains(AppBasic::get()->getMousePos()) ? 0 : -1 );
		//printf("ARC CH OVER %d\n",channelOver);
		return ( channelOver != oldChannel );
	}
	
	Vec2f ArcballVarControl::draw(Vec2f pos) {
		this->lastValue = *var;
		printf("ARCBALL DRAW = %.4f %.4f %.4f %.4f\n",var->x,var->y,var->z,var->w);

		mArcball.setCenter( pos + backArea.getSize() * 0.5f );

		activeArea = activeAreaBase + pos;
		for (int i = 0 ; i < 2 ; i++)
			activeAreas[i] = activeAreasBase[i] + pos;

		gl::enableAlphaBlending();

		// draw gui
		gl::color(SimpleGUI::bgColor);
		gl::drawSolidRect(backArea+pos);
		if (name.length())
			gl::drawString(name, pos, SimpleGUI::textColor, SimpleGUI::textFont);
		
		// draw arcball
		gl::color(ColorA::white());
		gl::draw(fbo.getTexture(), activeAreas[0]);
		if (this->isHighlighted(0))
		{
			//gl::color(SimpleGUI::mouseOverColor);
			//gl::drawStrokedRect(activeAreas[0]);
		}

		// draw reset button
		gl::color( resetting ? SimpleGUI::lightColor : SimpleGUI::darkColor );
		gl::drawSolidRect(activeAreas[1]);
		gl::color(SimpleGUI::bgColor);
		gl::drawString("x", activeAreas[1].getUpperLeft()+Vec2f(2,-1), SimpleGUI::textColor, SimpleGUI::textFont);

		gl::disableAlphaBlending();

		pos.y += backArea.getHeight() + SimpleGUI::spacing.y;
		return pos;
	}
	
	void ArcballVarControl::onMouseDown(MouseEvent event)
	{
		if (activeAreas[0].contains(event.getPos()))
		{
			dragStart = event.getPos();
			rotating = true;
			mArcball.mouseDown( event.getPos() );
			activeTrack = 0;
		}
		if (activeAreas[1].contains(event.getPos()))
		{
			*var = defaultValue;
			resetting = true;
			mustRefresh = true;
			printf("ARCBALL RESET TO DEFAULT = %.4f %.4f %.4f %.4f\n",var->x,var->y,var->z,var->w);
		}
	}
	void ArcballVarControl::onMouseDrag(MouseEvent event)
	{
		if (rotating)
		{
			mArcball.mouseDrag( event.getPos() );
			*var = mArcball.getQuat().getVec4();
		}
	}
	void ArcballVarControl::onMouseUp(MouseEvent event)
	{
		rotating = resetting = false;
		mustRefresh = true;
		activeTrack = -1;
	}
	void ArcballVarControl::onResize( app::ResizeEvent event )
	{
		mArcball.setWindowSize( getWindowSize() );
		//mArcball.setRadius( getWindowHeight() / 2.0f );
	}


	//-----------------------------------------------------------------------------
	// TEXTURE CONTROL
	//
	TextureVarControl::TextureVarControl(SimpleGUI *parent, const std::string & name, gl::Texture* var, float scale, bool flipVert) : Control(parent) {
		this->type = Control::TEXTURE_VAR;
		this->name = name;
		this->var = var;
		this->scale = scale;
		this->flipVert = flipVert;
		this->refreshRate = 1.0f;		// 0.0 means never refresh
		this->refreshTime = 0.0f;
		this->resized = false;
		this->resizeTexture();
	}
	
	void TextureVarControl::resizeTexture()
	{
		Vec2i textureSize;
		if (*var) {
			texSize.x = var->getWidth();
			texSize.y = var->getHeight();
			textureSize.y = (int) SimpleGUI::thumbnailSize.y;
			textureSize.x = (int) SimpleGUI::thumbnailSize.y * var->getAspectRatio();
		}
		else {
			// NO TEXTURE!!
			texSize.x = textureSize.x = (int) SimpleGUI::thumbnailSize.x;
			texSize.y = textureSize.y = (int) SimpleGUI::thumbnailSize.y;
		}
		
		Vec2f nameSize = (name.length() ? SimpleGUI::labelSize : Vec2f::zero());
		float gap = ( textureSize.x < SimpleGUI::sliderSize.x ? (SimpleGUI::sliderSize.x-textureSize.x)/2.0 : 0 );
		activeAreaBase = Rectf(gap, 
							   nameSize.y + SimpleGUI::padding.y, 
							   gap + textureSize.x,
							   nameSize.y + SimpleGUI::padding.y + textureSize.y );
		backArea = Rectf(
						 (-SimpleGUI::padding).x, 
						 (-SimpleGUI::padding).y, 
						 (SimpleGUI::sliderSize + SimpleGUI::padding).x, 
						 (nameSize + SimpleGUI::padding + textureSize + SimpleGUI::padding ).y );	
		resized = true;
	}
	
	bool TextureVarControl::valueHasChanged()
	{
		bool shouldRefresh = ( *var && ( refreshTime == 0.0 || (refreshRate > 0.0 && (getElapsedSeconds() - refreshTime) >= refreshRate)) ) ? true : false;
		return (shouldRefresh || resized);
	}
	
	bool TextureVarControl::hasResized()
	{
		/*
		if (!resized)
		{
			if (!*var)
			{
				if (texSize.x != (int) SimpleGUI::thumbnailSize.x || texSize.y != (int) SimpleGUI::thumbnailSize.y)
					this->resizeTexture();
			}
			else if (texSize.x != var->getWidth() || texSize.y != var->getHeight())
				this->resizeTexture();
		}
		return resized;
		 */
		return false;
	}
	
	Vec2f TextureVarControl::draw(Vec2f pos) {
		activeArea = activeAreaBase + pos;
		refreshTime = getElapsedSeconds();
		resized = false;
		
		gl::color(SimpleGUI::bgColor);
		gl::drawSolidRect(backArea+pos);
		
		gl::enableAlphaBlending();
		if (name.length())
			gl::drawString(name, pos, SimpleGUI::textColor, SimpleGUI::textFont);
		
		gl::color(ColorA(1,1,1,1));
		if (*var)
			gl::draw(*var, activeArea);
		else
		{
			gl::drawLine( activeArea.getUpperLeft(), activeArea.getLowerRight() );
			gl::drawLine( activeArea.getLowerLeft(), activeArea.getUpperRight() );
		}
		gl::disableAlphaBlending();
		
		pos.y += backArea.getHeight() + SimpleGUI::spacing.y;
		return pos;
	}
	

	
	//-----------------------------------------------------------------------------
	// LIST CONTROL
	//
	ListVarControl::ListVarControl(SimpleGUI *parent, const std::string & name, int* var, const std::map<int,std::string> &valueLabels) : Control(parent) {
		this->type = Control::LIST_VAR;
		this->name = name;
		this->var = var;
		this->defaultValue = defaultValue;
		this->lastValue = *var;
		this->lastSize = valueLabels.size();
		this->update( valueLabels );
	}
	
	void ListVarControl::update(const std::map<int,std::string> &valueLabels) {
		this->items.clear();
		// re-create list
		bool found = false;
		std::map<int,std::string>::const_iterator it = valueLabels.begin();
		for (int i = 0 ; i < valueLabels.size(); i++) {
			listVarItem _item =  listVarItem();
			_item.key = (int)(it->first);
			_item.label = const_cast<char*>( (it->second).c_str() );;
			items.push_back( _item );
			//std::cout << "LIST [" << this->valueLabels.size() << "] " << key << " = " << val << std::endl;
			
			// selected value still in the list
			if (_item.key == *var)
				found = true;
			// selected value lost: get previous
			else if (!found && _item.key > *var)
			{
				if (it == valueLabels.begin())
					*var = it->first;
				else {
					advance(it, -1);
					*var = it->first;
					advance(it, 1);
				}
				found = true;
			}
			advance(it, 1);
		}
		if(!found && valueLabels.size()>0)
			*var = valueLabels.rbegin()->first;
		// resize graphics
		this->resize();
	}
	
	void ListVarControl::resize() {
		float gapY = ( name.length() ? SimpleGUI::labelSize.y + SimpleGUI::padding.y : 0 );
		activeAreaBase = Rectf(0, gapY, SimpleGUI::buttonSize.x, gapY);
		for (int i = 0 ; i < items.size(); i++) {
			items[i].activeAreaBase = Rectf(0,
											gapY + (SimpleGUI::buttonSize.y+SimpleGUI::padding.y)*i,
											SimpleGUI::buttonSize.x,
											gapY + (SimpleGUI::buttonSize.y+SimpleGUI::padding.y)*i + SimpleGUI::buttonSize.y );
			activeAreaBase.x2 = items[i].activeAreaBase.x2;
			activeAreaBase.y2 = items[i].activeAreaBase.y2;
		}
		backArea = Rectf((-SimpleGUI::padding).x,
						 (-SimpleGUI::padding).y,
						 (SimpleGUI::padding + SimpleGUI::buttonSize).x,
						 gapY + ((SimpleGUI::buttonSize+SimpleGUI::padding)*items.size() ).y );
	}
	
	void ListVarControl::drawHeader(Vec2f pos) {
		this->lastValue = *var;
		this->lastSize = items.size();
		
		// background
		gl::color(SimpleGUI::bgColor);
		gl::drawSolidRect(backArea+pos);
		
		// control name
		if ( name.length() )
		{
			gl::enableAlphaBlending();
			gl::drawString(name, pos, SimpleGUI::textColor, SimpleGUI::textFont);
			gl::disableAlphaBlending();
		}
	}
	
	void ListVarControl::drawList(Vec2f pos) {
		// update active area
		for (int i = 0 ; i < items.size() ; i++)
			items[i].activeArea = items[i].activeAreaBase + pos;
		// draw items background
		for (int i = 0 ; i < items.size() ; i++) {
			gl::color(this->isSelected(i) ? SimpleGUI::lightColor : SimpleGUI::darkColor);
			gl::drawSolidRect(items[i].activeArea);
		}
		
		// draw items labels
		gl::enableAlphaBlending();
		for (int i = 0 ; i < items.size() ; i++) {
			gl::drawString(items[i].label, 
						   items[i].activeArea.getUpperLeft() + SimpleGUI::buttonGap,
						   this->isSelected(i) ? SimpleGUI::darkColor : SimpleGUI::textColor, 
						   SimpleGUI::textFont);
		}
		gl::disableAlphaBlending();
	}
	
	Vec2f ListVarControl::draw(Vec2f pos) {
		// Update active area
		activeArea = activeAreaBase + pos;
		
		this->drawHeader(pos);
		this->drawList(pos);
		
		pos.y += backArea.getHeight() + SimpleGUI::spacing.y;	
		return pos;
	}
	
	std::string ListVarControl::toString() {
		std::stringstream ss;
		ss << *var;
		return ss.str();
	}
	
	void ListVarControl::fromString(std::string& strValue) {
		*var = boost::lexical_cast<int>(strValue);
	}
	
	void ListVarControl::onMouseDown(MouseEvent event) {
		for (int i = 0 ; i < items.size() ; i++) {
			if (items[i].activeArea.contains(event.getPos())) {
				*this->var = items[i].key;
			}
		}
	}
	
	//-----------------------------------------------------------------------------
	// DROP-DOWN LIST CONTROL
	//
	DropDownVarControl::DropDownVarControl(SimpleGUI *parent, const std::string & name, int* var, const std::map<int,std::string> &valueLabels) : ListVarControl(parent,name,var,valueLabels) {
		this->type = Control::DROP_DOWN_VAR;
		this->dropped = false;
		this->dropButtonGap = Vec2f( 0, (SimpleGUI::buttonSize + SimpleGUI::padding).y + 2 );
		this->resize();
	}
	
	void DropDownVarControl::resize() {
		if (dropped)
		{
			this->ListVarControl::resize();
			activeAreaBase.y2 += dropButtonGap.y;
			backArea.y2 += dropButtonGap.y;
			for (int i = 0 ; i < items.size() ; i++)
				items[i].activeArea += dropButtonGap;
		}
		else
		{
			float gapY = ( name.length() ? SimpleGUI::labelSize.y + SimpleGUI::padding.y : 0 );
			activeAreaBase = Rectf(0,
								   gapY,
								   SimpleGUI::buttonSize.x,
								   gapY+(SimpleGUI::buttonSize).y );
			backArea = Rectf((-SimpleGUI::padding).x,
							 (-SimpleGUI::padding).y,
							 (SimpleGUI::buttonSize + SimpleGUI::padding).x,
							 gapY+(SimpleGUI::buttonSize + SimpleGUI::padding).y );
		}
		// button area
		dropButtonActiveAreaBase = activeAreaBase;
		dropButtonActiveAreaBase.y2 = activeAreaBase.y1 + SimpleGUI::buttonSize.y;
	}
	
	Vec2f DropDownVarControl::draw(Vec2f pos) {
		this->lastDropped = dropped;
		activeArea = activeAreaBase + pos;
		dropButtonActiveArea = dropButtonActiveAreaBase + pos;
		
		this->drawHeader(pos);
		
		// current value back
		Rectf b = Rectf( activeArea.getUpperLeft(), activeArea.getUpperLeft()+SimpleGUI::buttonSize);
		gl::color(SimpleGUI::darkColor);
		gl::drawSolidRect( b );
		// current value
		for (int i = 0 ; i < items.size() ; i++) {
			bool selected = ( items[i].key == *var );
			if (selected)
			{
				gl::enableAlphaBlending();
				gl::drawString(items[i].label,
							   b.getUpperLeft() + SimpleGUI::buttonGap,
							   SimpleGUI::textColor,
							   SimpleGUI::textFont);
				gl::disableAlphaBlending();
				break;
			}
		}
		
		// arrow down
		float h = SimpleGUI::buttonSize.y;
		Vec2f astart = activeArea.getUpperLeft() + Vec2f( SimpleGUI::buttonSize.x-h, 0); 
		Rectf abase = Rectf( astart, astart+Vec2f(h,h) );
		gl::color(SimpleGUI::lightColor);
		gl::drawSolidRect(abase);
		gl::enableAlphaBlending();
		gl::drawString( "V", abase.getUpperLeft()+Vec2f(3,0), SimpleGUI::darkColor, SimpleGUI::textFont);
		gl::disableAlphaBlending();
		
		if (dropped)
		{
			Vec2f p = activeArea.getUpperLeft() + Vec2f(0,SimpleGUI::buttonSize.y+2);
			gl::color(SimpleGUI::darkColor*2);
			gl::drawLine( p, p + Vec2f(SimpleGUI::buttonSize.x, 0) );
			this->drawList(pos+dropButtonGap);
		}
		
		pos.y += backArea.getHeight() + SimpleGUI::spacing.y;	
		return pos;
	}
	
	void DropDownVarControl::onMouseDown(MouseEvent event) {
		// Drop/Undrop
		if (activeArea.contains(event.getPos()))
		{
			if ( ! dropped )
			{
				this->open();
				return;
			}
			else
			{
				if (dropButtonActiveArea.contains(event.getPos()))
				{
					this->close();
					return;
				}
				else
				{
					for (int i = 0 ; i < items.size() ; i++) {
						if (items[i].activeArea.contains(event.getPos())) {
							*this->var = items[i].key;
							this->close();
							return;
						}
					}
				}
			}
		}
		// Pick Value
		else if (dropped && activeArea.contains(event.getPos()))
		{
		}
	}
	
	
	//-----------------------------------------------------------------------------	
	
	ButtonControl::ButtonControl(SimpleGUI *parent, const std::string & name) : Control(parent) {
		this->type = Control::BUTTON;
		this->name = name;		
		this->pressed = false;
		this->lastPressed = this->pressed;
		this->lastName = name;
		// ROGER
		this->callbackId = 0;
		this->triggerUp = false;
		activeAreaBase = Rectf(0,
							   0,
							   SimpleGUI::sliderSize.x,
							   SimpleGUI::buttonSize.y );
		backArea = Rectf((-SimpleGUI::padding).x,
						 (-SimpleGUI::padding).y,
						 (SimpleGUI::buttonSize + SimpleGUI::padding).x,
						 (SimpleGUI::buttonSize + SimpleGUI::padding).y );
	}
	
	Vec2f ButtonControl::draw(Vec2f pos) {
		lastName = name;
		lastPressed = pressed;
		activeArea = activeAreaBase + pos;
		
		gl::color(SimpleGUI::bgColor);
		gl::drawSolidRect(backArea + pos);
		
		gl::color(pressed ? SimpleGUI::lightColor : SimpleGUI::darkColor);
		gl::drawSolidRect(activeArea);
		
		gl::enableAlphaBlending();
		gl::drawString(name, pos + SimpleGUI::buttonGap, pressed ? SimpleGUI::darkColor : SimpleGUI::textColor, SimpleGUI::textFont);
		gl::disableAlphaBlending();
		
		pos.y += backArea.getHeight() + SimpleGUI::spacing.y;
		return pos;
	}
	
	void ButtonControl::onMouseDown(MouseEvent event) {
		pressed = true;
		if ( !triggerUp )
			fireClick();
	}
	
	void ButtonControl::onMouseUp(MouseEvent event) {
		if ( triggerUp && pressed )
			fireClick();
		pressed = false;
	}
	
	// ROGER
	void ButtonControl::onMouseDrag(MouseEvent event) {
		pressed = activeArea.contains(event.getPos());
	}
	
	
	void ButtonControl::fireClick() {
		MouseEvent event;
		bool handled = false;
		if ( ! callbacksClick.empty() )
		{
			for( CallbackMgr<bool (MouseEvent)>::iterator cbIter = callbacksClick.begin(); ( cbIter != callbacksClick.end() ) && ( ! handled ); ++cbIter ) {
				handled = (cbIter->second)( event );
			}
		}
	}
	
	//-----------------------------------------------------------------------------	
	
	LabelControl::LabelControl(SimpleGUI *parent, const std::string & name) : Control(parent) {
		this->name = name;
		this->lastName = name;
		this->var = NULL;
		this->lastVar = "";
		this->setup();
	}
	LabelControl::LabelControl(SimpleGUI *parent, const std::string & name, std::string * var) : Control(parent) {
		this->name = name;
		this->lastName = name;
		this->var = var;
		this->lastVar = (*var);
		this->setup();
	}
	void LabelControl::setup() {
		this->type = Control::LABEL;
		backArea = Rectf((-SimpleGUI::padding).x,
						 (-SimpleGUI::padding).y,
						 (SimpleGUI::sliderSize + SimpleGUI::padding).x,
						 (SimpleGUI::labelSize + SimpleGUI::padding).y );
	}
	
	bool LabelControl::valueHasChanged() {
		if ( name.compare(lastName) != 0 )
			return true;
		if (var)
			if ( (*var).compare(lastVar) != 0 )
				return true;
		return false;
	};	
	
	void LabelControl::setText(const std::string& text) {
		name = text;
	}
	
	Vec2f LabelControl::draw(Vec2f pos) {
		lastName = name;
		if (var)
			lastVar = (*var);
		if (bgColor) {
			gl::color(bgColor);
		}
		else {
			gl::color(SimpleGUI::bgColor);
		}
		gl::drawSolidRect(backArea+pos);
		
		gl::enableAlphaBlending();
		if ( name.length() )
		{
			gl::drawString(name, pos, SimpleGUI::textColor, SimpleGUI::textFont);
			if (var)
				gl::drawStringRight((*var), pos+Vec2f(SimpleGUI::sliderSize.x,0), SimpleGUI::textColor, SimpleGUI::textFont);
		}
		else if (var)
			gl::drawString((*var), pos, SimpleGUI::textColor, SimpleGUI::textFont);					
		gl::disableAlphaBlending();
		
		pos.y += backArea.getHeight() + SimpleGUI::spacing.y;
		return pos;
	}
	
	//-----------------------------------------------------------------------------		
	
	SeparatorControl::SeparatorControl(SimpleGUI *parent) : Control(parent) {
		this->type = Control::SEPARATOR;
		this->name = "Separator";
		// ROGER
		//backArea = Rectf( -SimpleGUI::padding, SimpleGUI::separatorSize + SimpleGUI::padding );
		backArea = Rectf(-SimpleGUI::padding.x,
						 0,
						 SimpleGUI::separatorSize.x + SimpleGUI::padding.x,
						 SimpleGUI::separatorSize.y );
	}	
	
	Vec2f SeparatorControl::draw(Vec2f pos) {
		
		// original
		//gl::color(SimpleGUI::bgColor);
		//gl::drawSolidRect(activeArea);		
		
		//activeArea = backArea+pos-Vec2f(0,SimpleGUI::padding.y);
		//gl::color(SimpleGUI::lightColor);
		//gl::drawSolidRect(activeArea);		
		
		pos.y += backArea.getHeight() + SimpleGUI::spacing.y;
		return pos;
	}
	
	//-----------------------------------------------------------------------------		
	
	ColumnControl::ColumnControl(SimpleGUI *parent, int x, int y) : AreaControl(parent) {
		this->x = x;
		this->y = y;
		this->type = Control::COLUMN;
		this->name = "Column";	
	}	
	
	Vec2f ColumnControl::draw(Vec2f pos) {
		this->lastEnabled = enabled;
		if (enabled)
		{
			if (this->x == 0 && this->y == 0) {
				pos.x += SimpleGUI::labelSize.x + SimpleGUI::spacing.x + SimpleGUI::padding.x*2;
				pos.y = SimpleGUI::spacing.y + SimpleGUI::padding.y*2;
			}
			else {
				pos.x = this->x;
				pos.y = this->y;
			}
		}
		return pos;
	}
	
	//-----------------------------------------------------------------------------		
	
	PanelControl::PanelControl(SimpleGUI *parent, const std::string& panelName) : AreaControl(parent) {
		this->column = false;
		this->type = Control::PANEL;
		this->name = ( panelName.length() ? panelName : "Panel" );
	}	
	
	Vec2f PanelControl::draw(Vec2f pos) {
		this->lastEnabled = enabled;
		return pos;
	}
	
	//-----------------------------------------------------------------------------	
	
} //namespace sgui
} //namespace cinder