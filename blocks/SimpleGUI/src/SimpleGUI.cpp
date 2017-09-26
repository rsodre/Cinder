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
//

#include "cinder/app/AppBasic.h"
#include "cinder/Utilities.h"
#include "cinder/Font.h"
#include "cinder/CinderMath.h"
#include "cinder/Rand.h"
#include "SimpleGUI.h"
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <sstream>
#include <fstream>
#include <iosfwd>
#ifdef QB
#include "qb.h"
#endif

#ifndef RELEASE
//#define DEBUG_FBO
#endif

#define RANDOM_COLOR	ColorA( Rand::randFloat(0.25,1.0), Rand::randFloat(0.25,1.0), Rand::randFloat(0.25,1.0), 0.7)

#define SLIDER_COLOR	( locked ? SimpleGUI::lockedColor : SimpleGUI::sliderColor )
#define TEXT_COLOR		( important ? SimpleGUI::sliderColor : SimpleGUI::textColor )

#ifdef QB
#define SGUI_DENSITY	QB_RENDER_DENSITY
#define SGUI_SIZE		QB_RENDER_SIZE
#define SGUI_BOUNDS		QB_RENDER_BOUNDS
#define SGUI_WIDTH		QB_RENDER_WIDTH
#define SGUI_HEIGHT		QB_RENDER_HEIGHT
#else
#define SGUI_DENSITY	1.0f
#define SGUI_SIZE		app::getWindowSize()
#define SGUI_BOUNDS		app::getWindowBounds()
#define SGUI_WIDTH		app::getWindowWidth()
#define SGUI_HEIGHT		app::getWindowHeight()
#endif

using namespace ci::app;

namespace cinder { namespace sgui {
	
	//-----------------------------------------------------------------------------
	
	Font SimpleGUI::textFont = Font();
	
	ColorA SimpleGUI::darkColor = ColorA(0.3, 0.3, 0.3, 1);
	ColorA SimpleGUI::lockedColor = ColorA(0.5, 0.5, 0.5, 1);
	ColorA SimpleGUI::sliderColor = ColorA(1, 1, 1, 1);
	ColorA SimpleGUI::bgColor = ColorA(0, 0, 0, 0.75);
	ColorA SimpleGUI::textColor = ColorA(1,1,1,1);
	ColorA SimpleGUI::textColor2 = ColorA(0.3, 0.3, 0.3, 1);
	ColorA SimpleGUI::mouseOverColor = ColorA(0.75,0.75,0.75,1);
	ColorA SimpleGUI::markerColor = ColorA(0.55,0.55,0.55,1);
	
	Vec2f SimpleGUI::spacing = Vec2f(3, 0);
	Vec2f SimpleGUI::padding = Vec2f(3, 3);
	Vec2f SimpleGUI::radioSize = Vec2f(10, 10);
	Vec2f SimpleGUI::sliderSize = Vec2f(128, 10);
	Vec2f SimpleGUI::labelSize = Vec2f(128, 10);
	Vec2f SimpleGUI::buttonSize = Vec2f(128, 12);
	Vec2f SimpleGUI::tabSize = Vec2f(128, 24);
	Vec2f SimpleGUI::buttonGap = Vec2f(4, 0);
	Vec2f SimpleGUI::tabGap = Vec2f(6, 6);
	Vec2f SimpleGUI::separatorSize = Vec2f(128, 3);
	Vec2f SimpleGUI::thumbnailSize = Vec2f(128, 128 / 1.7777);
	
	SimpleGUI::SimpleGUI(App* app) {
		init(app);
		mouseControl = NULL;
		theTab = NULL;			// The current active tab
		lastTab = NULL;		// The last active tab
		bBlink = false;
	}
	// ROGER
	SimpleGUI::~SimpleGUI() {
		for (int i = 0 ; i < controls.size() ; i++)
			delete controls[i];
		controls.clear();
	}
	
	void SimpleGUI::init(App* app) {
		SimpleGUI::textFont = Font(loadResource("pf_tempesta_seven.ttf"), 8, GL_NEAREST);	// original
		//SimpleGUI::textFont = Font(loadResource("pf_tempesta_seven_ROGER.ttf"), 8);		// tuned
		//SimpleGUI::textFont = Font("Arial", 12);
		selectedControl = NULL;
		app->setFpsSampleInterval( 0.2 );
		mAlpha = 1.0f;

		// register events

		// Conenct own signals because of multi window
		getWindow()->connectResize		( & SimpleGUI::onResize, this );
		getWindow()->connectKeyDown		( & SimpleGUI::onKeyDown, this );
		getWindow()->connectMouseMove	( & SimpleGUI::onMouseMove, this );
		getWindow()->connectMouseDown	( & SimpleGUI::onMouseDown, this );
		getWindow()->connectMouseUp		( & SimpleGUI::onMouseUp, this );
		getWindow()->connectMouseDrag	( & SimpleGUI::onMouseDrag, this );
		getWindow()->connectFileDrop	( & SimpleGUI::onFileDrop, this );

		// ROGER
		bForceRedraw = true;
		mOffset = Vec2f(0,0);
		droppedList = NULL;
		bDisplayFps = true;
		mCurrentFps = 0;
		// FBO
		bUsingFbo = true;
		bShouldResize = true;
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
	XYVarControl* SimpleGUI::addParamXY(const std::string& paramName, Vec2f* var, float min, float max, Vec2f const defaultValue) {
		XYVarControl* control = new XYVarControl(this, paramName, var, min, max, defaultValue );
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
	LabelControl* SimpleGUI::addParam(const std::string& paramName, std::string* var, const std::string & defaultValue)
	{
 		LabelControl* control = new LabelControl(this, paramName, var, defaultValue);
		controls.push_back(control);
		return control;
	}
	
	ButtonControl* SimpleGUI::addButton(const std::string& buttonName, const std::string& name2) {
		ButtonControl* control = new ButtonControl(this, buttonName, name2);
		controls.push_back(control);
		return control;
	}
	
	LabelControl* SimpleGUI::addLabel(const std::string& labelName, bool wrap) {
		LabelControl* control = new LabelControl(this, labelName);
		if (wrap)
			control->setWrap(true);
		controls.push_back(control);
		return control;
	}
	
	SeparatorControl* SimpleGUI::addSeparator() {
		SeparatorControl* control = new SeparatorControl(this);
		controls.push_back(control);
		return control;
	}
	
	TabControl* SimpleGUI::addTab(const std::string& tabName, bool *var, bool defaultValue) {
		TabControl* control = new TabControl(this, tabName, var, defaultValue);
		controls.push_back(control);
		lastTab = control;
		if ( theTabs.size() == 0 )	// first tab
		{
			control->defaultSelected = true;
			theTab = control;
		}
		theTabs.push_back(control);
		return control;
	}
	int SimpleGUI::getTabId() {
		return ( theTab != NULL ? theTab->tabId : 0 );
	}
	void SimpleGUI::setTab(int t)
	{
		if (theTabs.size())
		{
			if ( t >= theTabs.size() )
				t = theTabs.size() - 1;
			theTab = theTabs[t];
			bForceRedraw = true;
		}
	}

	ColumnControl* SimpleGUI::addColumn(const std::string & colName) {
		ColumnControl* control = new ColumnControl(this, colName);
		control->tab = lastTab;
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
	bool SimpleGUI::anythingChanged()
	{
		for (std::vector<Control*>::iterator it = controls.begin() ; it != controls.end() ; it++) {
			Control* control = *it;
			if ( control->visible && control->mustRefresh )
				return true;
		}
		return false;
	}
	
	

	
	////////////////////////////////////////////////
	//
	// DRAW LOOP
	//
	// UPDATE
	//
	void SimpleGUI::update() {
		PanelControl* currPanel = NULL;
		ColumnControl* currCol = NULL;
		bool colEnabled = true;
		bool panelEnabled = true;
		for (std::vector<Control*>::iterator it = controls.begin() ; it != controls.end() ; it++) {
			Control* control = *it;
			// in a column
			if (control->type == Control::COLUMN) {
				currCol = ((ColumnControl*)control);
				colEnabled = currCol->isEnabled() && currCol->tab == theTab;
				panelEnabled = true;
				currPanel = NULL;
			}
			// in a panel
			if (control->type == Control::PANEL) {
				currPanel = (PanelControl*)control;
				panelEnabled = currPanel->isEnabled();
				colEnabled &= ! ( !panelEnabled && currPanel->column );	// affects whole column
			}
			// tab are always enabled
			// panels must be draws always just to update status
			if (control->type == Control::TAB)
				control->visible = true;
			else if (colEnabled && panelEnabled)
				control->visible = true;
			else
				control->visible = false;
			// force update
			if (control->visible) {
				//printf("control->hasChanged() [%d]\n",control->hasChanged());
				//printf("control->hasChanged() [%d]\n",control->controlHasResized());
				if (control->hasChanged() || control->controlHasResized()) {
					control->updateFbo();	// update fbo before GUI drawing
					control->mustRefresh = true;
					control->switchPanels();
				}
				else if (control->updateMouse())
					control->mustRefresh = true;
				else if (bForceRedraw)
					control->mustRefresh = true;
			}
		}
	}
	//
	// DRAW
	//
	void SimpleGUI::draw() {

		gl::disableDepthRead();
		gl::disableDepthWrite();

		if (bUsingFbo)
		{
#ifdef DEBUG_FBO
			//printf("frame [%d] GUI Fbo...\n",app::getElapsedFrames());
#endif
			// Update Fbo
			bool updated = false;
			if ( !mFbo || bShouldResize )
			{
				mFbo = gl::Fbo( SGUI_WIDTH, SGUI_HEIGHT, true, true, false );
				mFbo.getTexture().setFlipped();
				bShouldResize = false;
				bForceRedraw = true;
#ifdef DEBUG_FBO
				printf("frame [%d] GUI Fbo Resized!\n",app::getElapsedFrames());
#endif
			}
			
			// Draw to FBO
			mFbo.bindFramebuffer();
			bForceRedraw |= (theTab != lastTab);
			if ( bForceRedraw )
				gl::clear( ColorA::zero() );
			gl::setMatricesWindow( mFbo.getSize() );
			gl::setViewport( mFbo.getBounds() );
			gl::scale(SGUI_DENSITY, SGUI_DENSITY);
			//gl::clear( ColorA(1,0,0,0.5) );	// debug size with color
			this->drawGui();
			mFbo.unbindFramebuffer();
			updated = true;
#ifdef DEBUG_FBO
			printf("frame [%d] GUI Fbo updated!\n",app::getElapsedFrames());
#endif

			// draw over
			Rectf bounds = Rectf( Vec2f(0,0), this->getSize() * SGUI_DENSITY );
			Area srcFlipped = Area( bounds.x1, SGUI_HEIGHT-bounds.y1, bounds.x2, SGUI_HEIGHT-bounds.y2 );
			gl::setMatricesWindow( SGUI_SIZE );
			gl::setViewport( SGUI_BOUNDS );
			gl::enableAlphaBlending();
			gl::color( ColorA(1,1,1,mAlpha.value()) );
			gl::draw( mFbo.getTexture(), srcFlipped, bounds );
			if (bBlink)
			{
				gl::color( ColorA(1,1,1,0.5) );
				gl::drawStrokedRect(bounds);
			}
		}
		else
		{
			gl::setMatricesWindow(SGUI_SIZE);
			gl::setViewport( SGUI_BOUNDS );
			gl::color( ColorA::white() );
			this->drawGui();
		}
		
		gl::color( ColorA::white() );
		gl::disableAlphaBlending();
	}
	
	void SimpleGUI::drawGui() {
		if (bForceRedraw)
			mSize = Vec2f::zero();
		
		gl::pushMatrices();
		glDisable( GL_TEXTURE_2D );
		glLineWidth( 1 );
		
		Vec2f position = spacing;
		ColumnControl* currColumn = NULL;
		PanelControl* currPanel = NULL;
		bool clearDown = false;
		bool clearAll = false;
		
		// Draw tabs first
		if (this->shouldDrawTabs())
		{
			mTabsBounds = Rectf( position - SimpleGUI::padding, position + SimpleGUI::separatorSize + SimpleGUI::padding*2 );
			// offset like a column
			position.x += SimpleGUI::padding.x*2;
			position.y = SimpleGUI::spacing.y + SimpleGUI::padding.y*2;
			// app name
			if (appName.length())
			{
				if (bForceRedraw)
					position = this->drawLabel(position, appName);
				else
					position += Vec2f( 0, SimpleGUI::sliderSize.y + SimpleGUI::padding.y*2 );
			}
			// draw!
			for (std::vector<Control*>::iterator it = controls.begin() ; it != controls.end() ; it++) {
				Control* control = *it;
				if (control->type == Control::TAB) {
					TabControl* tab = (TabControl*) *it;
					if (tab->isEnabled())
					{
						tab->select(tab == theTab);
						if (bForceRedraw || control->mustRefresh)
							position = this->drawControl(position, tab);
						else
							position += control->drawOffset;
						if (tab->blocking)
							break;
					}
				}
			}
			lastTab = theTab;
			//display FPS
			if (bDisplayFps)
			{
#ifndef RELEASE
				std::ostringstream ss;
				ss << app::getElapsedFrames();
				position = this->drawLabel(position,"Frame",ss.str());
#endif
				float fps = App::get()->getAverageFps();
				if (bForceRedraw || Control::displayedValue(mCurrentFps,1) != Control::displayedValue(fps,1))
				{
					mCurrentFps = fps;
					std::ostringstream ss;
					ss.setf(std::ios::fixed);
					ss.precision(1);
					ss << mCurrentFps;
					position = this->drawLabel(position,"Framerate",ss.str());
				}
				else
					position.y += (SimpleGUI::sliderSize + SimpleGUI::padding*2).y;
			}
			if ( position.y > mSize.y )
				mSize.y = position.y;
			mTabsBounds.y2 = position.y;
		}
		
		position += mOffset;
		
		// Draw Columns
		if ( this->anythingChanged() || bForceRedraw )
		{
			mSize.x = 0;
			for (std::vector<Control*>::iterator it = controls.begin() ; it != controls.end() ; it++) {
				Control* control = *it;
				TabControl* tab = NULL;
				gl::disableAlphaBlending();
				// Tabs already drawn
				if (control->type == Control::TAB)
				{
					if (tab)
						if (tab->blocking)
							break;
					tab = (TabControl*) control;
					continue;
				}
				// Draw new column
				if (control->type == Control::COLUMN)
				{
					if (currColumn == NULL && !this->shouldDrawTabs()) { //first column
						//each column moves everything to the right so we want compensate that
						position.x -= SimpleGUI::labelSize.x;
					}
					currColumn = (ColumnControl*)control;
					currPanel = NULL;
					clearDown = false;
				}
				// if resized, erase everything under control
				if (control->controlHasResized() && !clearDown && !clearAll)
				{
					Rectf eraseArea;
					if (control->type == Control::COLUMN)
					{
						clearAll = true;
						eraseArea = Rectf((position + SimpleGUI::sliderSize + SimpleGUI::padding).x, 0,
										  mFbo.getSize().x, mFbo.getSize().y );
					}
					else
					{
						
						clearDown = true;
						eraseArea = Rectf((position - SimpleGUI::padding).x,
										  (position - SimpleGUI::padding).y,
										  (position + SimpleGUI::sliderSize + SimpleGUI::padding).x,
										  mFbo.getSize().y );
					}
					if (bBlink)
						gl::color( RANDOM_COLOR );
					else
						gl::color( ColorA::zero() );
					gl::drawSolidRect(eraseArea);
				}
				// Draw Column to update valueHasChanged()
				if (control->type == Control::COLUMN)
				{
					if (control->visible)
					{
						position = this->drawControl(position, control);
						currColumn->backArea = Rectf( position - SimpleGUI::padding * 2, position + SimpleGUI::separatorSize + SimpleGUI::padding );
					}
					else
						this->drawControl(position, control);
					continue;
				}
				// Draw Panel to update valueHasChanged()
				if (control->type == Control::PANEL)
				{
					currPanel = (PanelControl*)control;
					this->drawControl(position, control);
					continue;
				}
				// not visible!
				if (!control->visible)
					continue;
				if ( bForceRedraw || clearDown || clearAll || control->mustRefresh )
					position = this->drawControl(position, control);
				else
					position += control->drawOffset;
				
				// Resize column
				if (currColumn)
					currColumn->backArea.y2 = position.y;
				
				// Resize GUI
				if ( position.y > mSize.y )
					mSize.y = position.y;
				if ( position.x > mSize.x )
					mSize.x = position.x;
			}
			mFinalPosition = Vec2f( position.x + SimpleGUI::labelSize.x, 0 );
			
			// ROGER
			// Finish size by adding the same a column adds
			mSize.x += SimpleGUI::labelSize.x + SimpleGUI::spacing.x + SimpleGUI::padding.x*2;
		}
		
		// COLUMN BORDERS DEBUG
		if (bBlink)
		{
			for (std::vector<Control*>::iterator it = controls.begin() ; it != controls.end() ; it++)
			{
				Control* control = *it;
				if ( control->type == Control::COLUMN && control->visible )
				{
					gl::color( mouseColumn == control ? Color::yellow() : Color::cyan() );
					gl::drawStrokedRect( ((ColumnControl*)control)->backArea );
				}
			}
			gl::color( mouseTabs ? Color::yellow() : Color::cyan() );
			gl::drawStrokedRect( mTabsBounds );
		}
		
		gl::disableAlphaBlending();
		gl::popMatrices();
		
		bForceRedraw = false;
	}
	
	Vec2f SimpleGUI::drawControl(Vec2f pos, Control *control) {
		if (bBlink)
			SimpleGUI::bgColor = RANDOM_COLOR;
		//printf("DRAW CONTROL [%s]\n",control->name.c_str());
		Vec2f newPos = control->draw(pos);
		control->drawOffset = (newPos - pos);
		control->mustRefresh = false;
		control->lastEnabled = control->enabled;
		control->lastName = control->name;
		return newPos;
	}
	
	Vec2f SimpleGUI::drawLabel(Vec2f pos, std::string left, std::string right) {
		Rectf backArea = Rectf((-SimpleGUI::padding).x,
							   (-SimpleGUI::padding).y,
							   (SimpleGUI::sliderSize + SimpleGUI::padding).x,
							   (SimpleGUI::sliderSize + SimpleGUI::padding).y );
		if (bBlink)
			SimpleGUI::bgColor = RANDOM_COLOR;
		gl::color(SimpleGUI::bgColor);
		gl::drawSolidRect(backArea + pos);
		gl::enableAlphaBlending();
		gl::drawString( left, pos, SimpleGUI::textColor, SimpleGUI::textFont);
		if (right.length())
			gl::drawStringRight(right, pos+Vec2f(SimpleGUI::sliderSize.x,0), SimpleGUI::textColor, SimpleGUI::textFont);
		gl::disableAlphaBlending();
		pos.y += backArea.getHeight();
		return pos;
	}
	
	void SimpleGUI::setVisible(bool state) {
		//printf("GUI fade to %d   a %.2f\n",(int)state,mAlpha.value());
		if ( state )
		{
			if ( mAlpha == 0.0 )
				mAlpha = 0.001f;	// already ON
			timeline().apply( &mAlpha, 1.0f, 0.333f, EaseInCubic() );
			bForceRedraw = true;
		}
		else
			timeline().apply( &mAlpha, 0.0f, 0.333f, EaseOutCubic() );
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
	
	////////////////////////////
	//
	// EVENTS
	//
	void SimpleGUI::onResize()
	{
		for (std::vector<Control*>::iterator it = controls.begin() ; it != controls.end() ; it++) {
			Control* control = *it;
			control->onResize();
		}
		bShouldResize = true;
	}
	void SimpleGUI::onKeyDown( app::KeyEvent & event )
	{
		// Pass to Mouse
		if ( mouseControl )
		{
			switch( event.getCode() ) {
				case KeyEvent::KEY_RIGHT:
				case KeyEvent::KEY_PLUS:
				case KeyEvent::KEY_EQUALS:
					mouseControl->inc( event.isShiftDown() );
					event.setHandled();
					return;
				case KeyEvent::KEY_UP:
					mouseControl->incY( event.isShiftDown() );
					event.setHandled();
					return;
				case KeyEvent::KEY_LEFT:
				case KeyEvent::KEY_MINUS:
				case KeyEvent::KEY_UNDERSCORE:
					mouseControl->dec( event.isShiftDown() );
					event.setHandled();
					return;
				case KeyEvent::KEY_DOWN:
					mouseControl->decY( event.isShiftDown() );
					event.setHandled();
					return;
				case 'r':
				case 'R':
					mouseControl->reset();
					event.setHandled();
					return;
				default:
					event.setHandled( mouseControl->updateKeyboard(event.getChar()) );
					return;
			}
		}
		
		// gui keys
		switch( event.getCode() ) {
			case KeyEvent::KEY_BACKQUOTE:
				// tab up
				{
					int t = getTabId();
					if ( t > 0 )
						this->setTab(t-1);
				}
				event.setHandled();
				return;
			//case KeyEvent::KEY_a:		// Da conflito com color HEX
			case KeyEvent::KEY_TAB:
				// tab down
				{
					int t = getTabId();
					if ( t < theTabs.size() - 1 )
						this->setTab(t+1);
				}
				event.setHandled();
				return;
			case 'k':
			case 'K':
				if (event.isMetaDown()) // COMMAND
				{
					bBlink = ! bBlink;
					this->bForceRedraw = true;
					return;
				}
		}
	}

	//
	// MOUSE
	//
	Control * SimpleGUI::getMouseOverControl( Vec2i mousePos ) {
		for (std::vector<Control*>::iterator it = controls.begin() ; it != controls.end() ; it++)
		{
			Control* control = *it;
			if (!control->visible)
				continue;
			if (control->locked)
				continue;
			// pass on mouse event
			if (control->activeArea.contains(mousePos))
				return control;
		}
		return NULL;
	}
	ColumnControl * SimpleGUI::getMouseOverColumn( Vec2i mousePos ) {
		for (std::vector<Control*>::iterator it = controls.begin() ; it != controls.end() ; it++)
		{
			Control* col = *it;
			if ( col->type == Control::COLUMN && col->visible && col->enabled )
				if (((ColumnControl*)col)->backArea.contains(mousePos))
					return (ColumnControl*) col;
		}
		return NULL;
	}
	void SimpleGUI::onMouseMove(app::MouseEvent & event) {
		mouseControl = this->getMouseOverControl( event.getPos() );
		mouseColumn = this->getMouseOverColumn( event.getPos() );
		mouseTabs = mTabsBounds.contains( event.getPos() );
	}
	void SimpleGUI::onMouseDown(app::MouseEvent & event) {
		if ( ! this->isVisible() )
			return;
		// update mouse over
		this->onMouseMove( event );
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
			event.setHandled();
			return;
		}
		// Close open DropDown list
		if (droppedList)
		{
			droppedList->close();
			droppedList = NULL;
		}
		// block events thru GUI
		if (mouseColumn || mouseTabs)
			event.setHandled();
	}
	void SimpleGUI::onMouseUp(app::MouseEvent & event) {
		if ( ! this->isVisible() )
			return;
		// update mouse over
		this->onMouseMove( event );
		// pass event
		if (selectedControl != NULL) {
			selectedControl->onMouseUp(event);
			selectedControl->mustRefresh = true;
			selectedControl = NULL;
			// am I above other?
			this->onMouseMove(event);
			if (mouseControl)
				mouseControl->mustRefresh = true;
			event.setHandled();
		}
		// block events thru GUI
		//if (mouseColumn || mouseTabs)
		//	event.setHandled();
	}
	void SimpleGUI::onMouseDrag(app::MouseEvent & event) {
		if ( ! this->isVisible() )
			return;
		// update mouse over
		//this->onMouseMove( event );
		// pass event
		if (selectedControl) {
			selectedControl->onMouseDrag(event);
			event.setHandled();
		}
		// block events thru GUI
		if (mouseColumn || mouseTabs)
			event.setHandled();
	}
	void SimpleGUI::onFileDrop(FileDropEvent & event) {
		if ( ! this->isVisible() )
			return;
		Control * c = this->getMouseOverControl( event.getPos() );
		if ( c )
			if ( c->fileDrop(event) )
				event.setHandled();
	}
	
	
	
	
	//--------------------------------------------------------------------------------------------
	//NEW
	//
	void gControl::_pushift( gControl *c )	{
		this->_push( c );
		this->_shift( c );
	}
	void gControl::_push( gControl *c )	{
		mChildren.push_back(c);
	}
	void gControl::_shift( gControl *c ) {
		// shift new control
		c->mArea.offset( Vec2i( 0, mArea.getHeight() ) );
		c->mAreaActive.offset( Vec2i( 0, mArea.getHeight() ) );
		// resize box
		if ( mArea.getWidth() < c->mArea.getWidth() )
			mArea.x2 = mArea.x1 + c->mArea.getWidth();
		mArea.y2 += c->mArea.getHeight();
	}
	void gControl::_draw( Vec2i & start ) {
		this->_privateDraw( start );
		for (std::vector<gControl*>::iterator it = mChildren.begin() ; it != mChildren.end() ; it++) {
			gControl* c = *it;
			c->_privateDraw( start );
		}
	};
	
	gBoxFloat::gBoxFloat( gControl *p ) : gControl( p ) {
		this->_pushift( new gControlLabel(this) );
		this->_pushift( new gControlSlider(this) );
	}
	void gBoxFloat::_privateDraw( Vec2i & start ) {
		gl::color( RANDOM_COLOR );
		gl::drawSolidRect( mArea + start );
	};
	
	//
	// LABEL
	gControlLabel::gControlLabel( gControl *p ) : gControl( p ) {
		mArea.add( (SimpleGUI::padding + SimpleGUI::sliderSize + SimpleGUI::padding).x,
				  (SimpleGUI::padding + SimpleGUI::sliderSize).y );
		mAreaActive.add( SimpleGUI::sliderSize );
		mAreaActive.offset( SimpleGUI::padding );
	}
	void gControlLabel::_privateDraw( Vec2i & start ) {
		gl::color( RANDOM_COLOR );
		gl::drawSolidRect( mArea + start );
		gl::color( RANDOM_COLOR );
		gl::drawSolidRect( mAreaActive + start );
	};
	
	//
	// SLIDER
	gControlSlider::gControlSlider( gControl *p ) : gControl( p ) {
		mArea.add( (SimpleGUI::padding + SimpleGUI::sliderSize + SimpleGUI::padding).x,
				  (SimpleGUI::padding + SimpleGUI::sliderSize).y );
		mAreaActive.add( SimpleGUI::sliderSize );
		mAreaActive.offset( SimpleGUI::padding );
	}
	void gControlSlider::_privateDraw( Vec2i & start ) {
		gl::color( RANDOM_COLOR );
		gl::drawSolidRect( mArea + start );
		gl::color( RANDOM_COLOR );
		gl::drawSolidRect( mAreaActive + start );
	};
	//
	//NEW
	//--------------------------------------------------------------------------------------------
	
	
	
	//--------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------
	//
	// CONTROL
	//
	
	Control::Control(SimpleGUI *parent, const std::string & name) {
		this->parentGui = parent;
		this->name = this->lastName = name;
		this->enabled = this->lastEnabled = true;
		this->locked = false;
		bgColor = SimpleGUI::bgColor;
		drawOffset = Vec2f::zero();
		this->unitControl = NULL;
		this->readOnly = false;
		this->important = false;
		this->displayValue = false;
		this->channelOver = -1;
		this->mustRefresh = true;
		this->visible = true;
		this->pregap = true;
		this->postgap = true;
		this->slim = false;
		this->label = NULL;
		this->axisOnDefault = false;
		this->axisOnZero = false;
		this->axisOnMax = false;

		//NEW
		gc = NULL;
	}
	
	void Control::setBackgroundColor(ColorA color) {
		bgColor = color;
	}

	Control* Control::setReadOnly(bool b)
	{
		readOnly=b;
		if (b)
			displayValue=true;
		this->update();
		return this;	// chained setters
	}
	
	Control* Control::setImportant(bool b)
	{
		important = b;
		if (label)
			label->setImportant( b );
		mustRefresh = true;
		return this;	// chained setters
	}
	
	void Control::drawBackArea(Rectf a)
	{
		gl::enableAlphaBlending();
		gl::color(SimpleGUI::bgColor);
		gl::drawSolidRect( a );
		gl::disableAlphaBlending();
	}

	void Control::switchPanels() {
		for (auto it = panelsToSwitch.begin() ; it != panelsToSwitch.end() ; ++it )
			(*it)->enable( this->isOn() );
		for (auto it = panelsToSwitchInv.begin() ; it != panelsToSwitchInv.end() ; ++it )
			(*it)->enable( ! this->isOn() );
	}

	//
	// GENERIC FLOAT SLIDER
	//
	// Returns  0.0 .. 1.0
	float Control::sliderGetMouseDragPos(Vec2i pos, Rectf activeArea) {
		float value = (pos.x - activeArea.x1)/(activeArea.x2 - activeArea.x1);
		value = math<float>::max(0.0, math<float>::min(value, 1.0));
		return value;
	}

	
	//--------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------
	//
	// VAR CONTROLS
	//
	
	//-----------------------------------------------------------------------------
	
	FloatVarControl::FloatVarControl(SimpleGUI *parent, const std::string & name, float* var, float min, float max, float defaultValue) : Control(parent,name) {
		this->type = Control::FLOAT_VAR;
		this->var = var;
		this->min = min;
		this->max = max;
		this->complement = 0.0;
		this->percentage = false;
		this->setPrecision( (max-min) <= 1.0 ? 2 : 1 );
		*var = math<float>::clamp( defaultValue, min, max );
		// ROGER
		this->defaultValue = defaultValue;
		this->lastValue = *var;
		this->formatAsTimecode = false;
		this->formatAsTime = false;
		this->displaySign = false;
		this->update();
		//NEW
		//gc = new gBoxFloat( NULL );
	}
	
	FloatVarControl* FloatVarControl::setPrecision(int p) {
		if (p == 100)
		{
			percentage = true;
			p = 2;
		}
		this->precision = p;
		this->step = (1.0f/pow(10.0f,p));
		return this;
	}

	void FloatVarControl::setComplement(float c) {
		// remove last complement
		*var = *var - complement;
		// add complement clamping
		float clamped = math<float>::clamp( (*var) + c, min, max );
		complement = clamped - *var;
		*var = clamped;
	}

	// ROGER
	void FloatVarControl::update()
	{
		if (slim)
		{
			activeAreaBase = Rectf(0,
								   SimpleGUI::padding.y,
								   SimpleGUI::sliderSize.x,
								   SimpleGUI::padding.y + SimpleGUI::sliderSize.y );
			backArea = Rectf((-SimpleGUI::padding).x,
							 (-SimpleGUI::padding).y,
							 (SimpleGUI::sliderSize + SimpleGUI::padding).x,
							 (SimpleGUI::sliderSize + SimpleGUI::padding).y );
		}
		else
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
	}
	
	bool FloatVarControl::updateMouse()
	{
		// save current active channel
		int oldChannel = channelOver;
		channelOver = ( activeArea.contains(AppBasic::get()->getMousePosMainWindow()) ? 0 : -1 );
		return ( channelOver != oldChannel && ! locked );
	}

	
	Vec2f FloatVarControl::draw(Vec2f pos) {
		this->lastValue = *var;
		if (!enabled)
			return pos;
		
		activeArea = activeAreaBase + pos;
		
		gl::color(SimpleGUI::bgColor);
		gl::drawSolidRect( backArea + pos );
		
		gl::enableAlphaBlending();

		if ( ! slim )
		{
			gl::drawString(name, pos, TEXT_COLOR, SimpleGUI::textFont);
			if (displayValue)
				gl::drawStringRight(this->toString(), pos+Vec2f(SimpleGUI::sliderSize.x,0), TEXT_COLOR, SimpleGUI::textFont);
			gl::disableAlphaBlending();
		}

		if (!readOnly)
		{
			float vd = getNormalizedValue(this->displayedValue((axisOnZero?0:defaultValue),step),min,max);
			float v = getNormalizedValue(this->displayedValue(*var,step),min,max);
			Rectf rd = SimpleGUI::getScaledWidthRectf(activeArea, vd);
			Rectf r = SimpleGUI::getScaledWidthRectf(activeArea, v);
			// back
			gl::color(SimpleGUI::darkColor);
			gl::drawSolidRect(activeArea);
			// color bar
			if (axisOnMax)
			{
				Rectf mr = SimpleGUI::getScaledWidthRectf(activeArea, max);
				r = Rectf( r.getLowerRight(), mr.getUpperRight() );
			}
			if (axisOnDefault || axisOnZero)
				r = Rectf( r.getLowerRight(), rd.getUpperRight() );
			gl::color(SLIDER_COLOR);
			gl::drawSolidRect(r);
			// default value line
			gl::color( (axisOnDefault||axisOnZero) && fabs(r.getWidth())<=1 ? SLIDER_COLOR : SimpleGUI::markerColor );
			gl::drawLine(rd.getLowerRight(), rd.getUpperRight());
			// highlight border
			if (this->isHighlighted())
			{
				gl::color(SimpleGUI::mouseOverColor);
				gl::drawStrokedRect(activeArea);
			}
			if (slim && displayValue)
				gl::drawStringRight(this->toString(), activeArea.getUpperRight(), SimpleGUI::textColor, SimpleGUI::textFont);
		}
		
		//NEW
		//Vec2i p = Vec2i( pos.x, pos.y );
		//gc->_draw( p );
		
		pos.y += backArea.getHeight() + SimpleGUI::spacing.y;	
		return pos;
	}
	
	std::string FloatVarControl::toString() {
		float v = *var;
		if (unitControl)
			v *= unitControl->getValue();
		std::ostringstream ss;
		ss.setf(std::ios::fixed);
		if (formatAsTimecode)
			ss << toTimecode(v);
		else if (formatAsTime)
			ss << toTime(v);
		else
		{
			v = this->displayedValue( v, step );
			if (displaySign && v > 0.0)
				ss << "+";
			if (percentage)
			{
				if (this->precision > 2)
				{
					ss.precision(this->precision-2);
					ss << (v * 100.0f) << "%";
				}
				else
				{
					int p = (int)roundf(v * 100.0f);
					ss << p << "%";
				}
			}
			else
			{
				ss.precision(this->precision);
				ss << v << suffix;
			}
		}
		return ss.str();
	}

	
	void FloatVarControl::fromString(std::string& strValue) {
		*var = boost::lexical_cast<float>(strValue);
	}
	
	void FloatVarControl::onMouseDown(app::MouseEvent & event) {
		onMouseDrag(event);	
	}
	
	void FloatVarControl::onMouseDrag(app::MouseEvent & event) {
		float value = this->sliderGetMouseDragPos(event.getPos(), activeArea);
		this->setNormalizedValue(value);
	}
	void FloatVarControl::setNormalizedValue(float value) {
		float newValue = min + value * (max - min);
		newValue = this->displayedValue(newValue,step);
		if ( newValue != *var )
			*var = newValue;
	}
	
	
	//-----------------------------------------------------------------------------
	
	IntVarControl::IntVarControl(SimpleGUI *parent, const std::string & name, int* var, int min, int max, int defaultValue) : Control(parent,name) {
		this->type = Control::INT_VAR;
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
			float gapY = ( slim ? SimpleGUI::padding.y : SimpleGUI::labelSize.y + SimpleGUI::padding.y );
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
			backArea = Rectf((-SimpleGUI::padding).x,
							 (-SimpleGUI::padding).y,
							 (SimpleGUI::sliderSize + SimpleGUI::padding).x,
							 (SimpleGUI::labelSize + SimpleGUI::padding).y + ( readOnly ? 0 : gapY ) );
		}
		else
		{
			// SLIDER
			if (slim)
			{
				activeAreaBase = Rectf(0,
									   SimpleGUI::padding.y,
									   SimpleGUI::sliderSize.x,
									   SimpleGUI::padding.y + SimpleGUI::sliderSize.y );
				backArea = Rectf((-SimpleGUI::padding).x,
								 (-SimpleGUI::padding).y,
								 (SimpleGUI::sliderSize + SimpleGUI::padding).x,
								 (SimpleGUI::sliderSize + SimpleGUI::padding).y );
			}
			else
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
		}
		
	}
	
	bool IntVarControl::updateMouse()
	{
		int oldChannel = channelOver;
		channelOver = ( activeArea.contains(AppBasic::get()->getMousePosMainWindow()) ? 0 : -1 );
		return ( channelOver != oldChannel );
	}
	
	Vec2f IntVarControl::draw(Vec2f pos) {
		this->lastValue = *var;
		if (!enabled)
			return pos;
		
		activeArea = activeAreaBase + pos;
		
		gl::color(SimpleGUI::bgColor);
		gl::drawSolidRect( backArea + pos );
		
		gl::enableAlphaBlending();

		if ( ! slim )
		{
			gl::drawString(name, pos, TEXT_COLOR, SimpleGUI::textFont);
			if (displayValue)
				gl::drawStringRight(this->toString(), pos+Vec2f(SimpleGUI::sliderSize.x,0), TEXT_COLOR, SimpleGUI::textFont);
			
		}
		if (!readOnly)
		{
			if (items.size() > 0)
			{
				// RADIO
				for (int i = 0 ; i < items.size() ; i++)
				{
					bool selected = ( *var == min + (i * step) );
					items[i].activeArea = items[i].activeAreaBase + pos;
					gl::color( selected ? SLIDER_COLOR : SimpleGUI::darkColor );
					gl::drawSolidRect(items[i].activeArea);
				}
				if (slim)
					gl::drawStringRight(this->toString(), activeArea.getUpperRight() + Vec2f(12,0), SimpleGUI::textColor, SimpleGUI::textFont);
			}
			else
			{
				// SLIDER
				float v = getNormalizedValue(*var,min,max);
				float vd = getNormalizedValue(defaultValue,min,max);
				Rectf r = SimpleGUI::getScaledWidthRectf(activeArea, v);
				Rectf rd = SimpleGUI::getScaledWidthRectf(activeArea, vd);
				// back
				gl::color(SimpleGUI::darkColor);
				gl::drawSolidRect(activeArea);
				// default value line
				gl::color( SimpleGUI::markerColor );
				gl::drawLine(rd.getLowerRight(), rd.getUpperRight());
				// value bar
				gl::color(SLIDER_COLOR);
				gl::drawSolidRect(r);
				if (this->isHighlighted())
				{
					gl::color(SimpleGUI::mouseOverColor);
					gl::drawStrokedRect(activeArea);
				}
				if (slim && displayValue)
					gl::drawStringRight(this->toString(), activeArea.getUpperRight(), SimpleGUI::textColor, SimpleGUI::textFont);
			}
		}
		gl::disableAlphaBlending();

		pos.y += backArea.getHeight() + SimpleGUI::spacing.y;	
		return pos;	
	}	
	
	std::string IntVarControl::toString() {
		std::stringstream ss;
		ss << *var;
		ss << suffix;
		return ss.str();
	}
	
	void IntVarControl::fromString(std::string& strValue) {
		*var = boost::lexical_cast<int>(strValue);
	}
	
	void IntVarControl::onMouseDown(app::MouseEvent & event) {
		onMouseDrag(event);
	}
	
	void IntVarControl::onMouseDrag(app::MouseEvent & event) {
		if (!readOnly)
		{
			if (items.size())
			{
				//RADIOS
				for (int i = 0 ; i < items.size() ; i++) {
					if (items[i].activeArea.contains(event.getPos())) {
						*var = items[i].key;
					}
				}
			}
			else
			{
				// SLIDER
				float value = this->sliderGetMouseDragPos(event.getPos(), activeArea);
				this->setNormalizedValue(value);
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
	ByteVarControl::ByteVarControl(SimpleGUI *parent, const std::string & name, unsigned char* var, unsigned char defaultValue) : Control(parent,name) {
		this->type = Control::INT_VAR;
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
	
	bool ByteVarControl::updateMouse()
	{
		int oldChannel = channelOver;
		channelOver = ( activeArea.contains(AppBasic::get()->getMousePosMainWindow()) ? 0 : -1 );
		return ( channelOver != oldChannel );
	}
	
	Vec2f ByteVarControl::draw(Vec2f pos) {
		this->lastValue = *var;
		if (!enabled)
			return pos;
		
		activeArea = activeAreaBase + pos;
		
		gl::color(SimpleGUI::bgColor);
		gl::drawSolidRect( backArea + pos );
		
		gl::enableAlphaBlending();
		gl::drawString(name, pos, SimpleGUI::textColor, SimpleGUI::textFont);
		
		// ROGER
		if (displayValue)
		{
			std::stringstream ss;
			ss << (int)roundf(*var);
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
			gl::color(SLIDER_COLOR);
			gl::drawSolidRect(SimpleGUI::getScaledWidthRectf(activeArea, v));
			if (this->isHighlighted())
			{
				gl::color(SimpleGUI::mouseOverColor);
				gl::drawStrokedRect(activeArea);
			}
		}
		gl::disableAlphaBlending();
		
		pos.y += backArea.getHeight() + SimpleGUI::spacing.y;	
		return pos;	
	}	
	
	std::string ByteVarControl::toString() {
		std::stringstream ss;
		ss << (int)roundf(*var);
		ss << suffix;
		return ss.str();
	}
	
	void ByteVarControl::fromString(std::string& strValue) {
		*var = (unsigned char) boost::lexical_cast<int>(strValue);
	}
	
	void ByteVarControl::onMouseDown(app::MouseEvent & event) {
		onMouseDrag(event);
	}
	
	void ByteVarControl::onMouseDrag(app::MouseEvent & event) {
		if (!readOnly)
		{
			float value = this->sliderGetMouseDragPos(event.getPos(), activeArea);
			this->setNormalizedValue(value);
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
		if (!enabled)
			return pos;

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
				gl::color( selected ? SLIDER_COLOR : SimpleGUI::darkColor );
				gl::drawSolidRect(items[i].activeArea);
			}
		}
		gl::disableAlphaBlending();
		
		pos.y += backArea.getHeight() + SimpleGUI::spacing.y;	
		return pos;	
	}	

	void FlagVarControl::onMouseDown(app::MouseEvent & event) {
		this->touchedItem = -1;
		this->touchedState = -1;
		onMouseDrag(event);
	}
	
	void FlagVarControl::onMouseDrag(app::MouseEvent & event) {
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
					(*var) |= items[item].key;		// turn ON
				else
					(*var) &= ~(items[item].key);		// turn OFF
				touchedItem = item;
			}
		}
	}
	
	
	//-----------------------------------------------------------------------------
	
	BoolVarControl::BoolVarControl(SimpleGUI *parent, const std::string & name, bool* var, bool defaultValue, int groupId) : Control(parent,name) {
		this->type = Control::BOOL_VAR;
		this->nameOff = name;
		this->var = var;
		this->groupId = groupId;
		*var = defaultValue;
		// ROGER
		this->defaultValue = defaultValue;
		this->lastValue = *var;
		this->asButton = false;
		this->switchButton = false;
		this->dontGoOff = false;
		this->update();
	}	
	
	void BoolVarControl::update() {
		if (this->asButton)
		{
			activeAreaBase = Rectf(0,
								   0,
								   SimpleGUI::buttonSize.x,
								   SimpleGUI::buttonSize.y );
			backArea = Rectf((-SimpleGUI::padding).x,
							 (-SimpleGUI::padding).y,
							 (SimpleGUI::buttonSize + SimpleGUI::padding).x,
							 SimpleGUI::buttonSize.y + ( postgap ? SimpleGUI::padding.y : 0) );
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
		if (!enabled)
			return pos;

		activeArea = activeAreaBase + pos;
		
		gl::color(SimpleGUI::bgColor);
		gl::drawSolidRect(backArea + pos);
		
		gl::color((*var) ? SLIDER_COLOR : SimpleGUI::darkColor);
		gl::drawSolidRect(activeArea);
		
		float x = pos.x + ( asButton ? SimpleGUI::buttonGap.x : SimpleGUI::radioSize.x + SimpleGUI::padding.x*2 );
		gl::enableAlphaBlending();
		gl::drawString( ((*var)?name:nameOff), Vec2f(x, pos.y), (asButton&&(*var) ? SimpleGUI::textColor2 : SimpleGUI::textColor), SimpleGUI::textFont);
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
	
	void BoolVarControl::onMouseDown(app::MouseEvent & event) {
		if ( *var && dontGoOff )
			return;
		if ( switchButton )
			*var = true;
		else
			*var = ! *var;
	}
	
	void BoolVarControl::onMouseUp(app::MouseEvent & event) {
		if ( *var && dontGoOff )
			return;
		if ( switchButton )
			*var = false;
	}
	
	//-----------------------------------------------------------------------------
	
	// ROGER
	ColorVarControl::ColorVarControl(SimpleGUI *parent, const std::string & name, Color* var, Color defaultValue, int colorModel) : Control(parent,name) {
		this->type = Control::COLOR_VAR;
		this->var = var;
		this->varA = NULL;
		this->colorModel = colorModel;
		this->displayValue = false;
		*var = defaultValue;
		activeTrack = 0;
		// ROGER
		this->defaultValue = defaultValue;
		this->lastValue = *var;
		alphaControl = false;
		this->update();
	}
	
	ColorVarControl::ColorVarControl(SimpleGUI *parent, const std::string & name, ColorA* var, ColorA defaultValue, int colorModel) : Control(parent,name) {
		this->type = Control::COLOR_VAR;
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
			if (activeAreas[i].contains(AppBasic::get()->getMousePosMainWindow()))
				channelOver = i;
		return ( channelOver != oldChannel );
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
	void ColorVarControl::setByteValue(int ch, unsigned char b)
	{
		if (channelOver >= 0)
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

	// keya 1-F
	bool ColorVarControl::updateKeyboard(char c)
	{
		if (channelOver >= 0)
		{
			c = toupper(c);
			if ( (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') )
			{
				unsigned char b = this->getByteValue(channelOver);
				char hx[3];
				sprintf(hx,"%2X",b);
				hx[0] = hx[1];
				hx[1] = c;
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
		if (!enabled)
			return pos;
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
		
		// Color Box
		Rectf previewPos = previewArea+pos;
		if (alphaControl)
			gl::color( Color(*varA) );
		else
			gl::color( *var );
		gl::drawSolidRect(previewPos);
		// alpha bar
		if (alphaControl)
		{
			Rectf alphaBar = previewPos;
			alphaBar.y1 = alphaBar.y2 - 1;
			gl::color( Color::black());
			gl::drawSolidRect(alphaBar);
			alphaBar.x2 = alphaBar.x1 + alphaBar.getWidth() * varA->a;
			gl::color( Color::white());
			gl::drawSolidRect(alphaBar);
		}
		
		// Sliders
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
		gl::color(SLIDER_COLOR);
		for (int i = 0 ; i < channelCount ; i++)
		{
			Rectf r = SimpleGUI::getScaledWidthRectf(activeAreas[i], values[i]);
			gl::drawLine(Vec2f(r.x2, r.y1), Vec2f(r.x2, r.y2));				
		}
		if (displayValue)
		{
			gl::enableAlphaBlending();
			for (int i = 0 ; i < channelCount ; i++)
			{
				char hx[3];
				if (alphaControl)
					sprintf(hx,"%02X",(unsigned int)((*varA)[i]*255));
				else
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
	
	
	void ColorVarControl::onMouseDown(app::MouseEvent & event) {
		for (int i = 0 ; i < channelCount ; i++)
		{
			if (activeAreas[i].contains(event.getPos())) {
				activeTrack = i;
				break;
			}
		}
		onMouseDrag(event);
	}
	
	void ColorVarControl::onMouseDrag(app::MouseEvent & event) {
		float value = this->sliderGetMouseDragPos(event.getPos(), activeArea);
		
		if (colorModel == SimpleGUI::RGB) {
			switch (activeTrack) {
				case 0: ( alphaControl ? varA->r : var->r ) = value; break;
				case 1: ( alphaControl ? varA->g : var->g ) = value; break;
				case 2: ( alphaControl ? varA->b : var->b ) = value; break;
				case 3: varA->a = value; break;
			}
		}
		else {
			Vec3f hsv = ( alphaControl ? rgbToHSV(*varA) : rgbToHSV(*var) );
			switch (activeTrack) {
				case 0: hsv.x = value; break;
				case 1: hsv.y = value; break;
				case 2: hsv.z = value; break;
				case 3: varA->a = value; break;
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
	VectorVarControl::VectorVarControl(SimpleGUI *parent, const std::string & name, Vec4f* var, int vc, float min, float max, Vec4f defaultValue) : Control(parent,name) {
		this->type = Control::VECTOR_VAR;
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
			if (activeAreas[i].contains(AppBasic::get()->getMousePosMainWindow()))
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
		if (!enabled)
			return pos;

		activeArea = activeAreaBase + pos;
		for (int i = 0 ; i < vecCount ; i++)
			activeAreas[i] = activeAreasBase[i] + pos;
		
		gl::color(SimpleGUI::bgColor);
		gl::drawSolidRect(backArea+pos);
		
		gl::enableAlphaBlending();
		gl::drawString(name, pos, SimpleGUI::textColor, SimpleGUI::textFont);
		gl::disableAlphaBlending();

		// back
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

		// default line
		gl::color(SimpleGUI::markerColor);
		for (int i = 0 ; i < vecCount ; i++)
		{
			float v = getNormalizedValue(this->displayedValue(defaultValue[i],step),min,max);
			Rectf r = SimpleGUI::getScaledWidthRectf(activeAreas[i], v);
			gl::drawLine(Vec2f(r.x2, r.y1), Vec2f(r.x2, r.y2));
		}

		// value line
		gl::color(SLIDER_COLOR);
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
	
	
	void VectorVarControl::onMouseDown(app::MouseEvent & event) {
		for (int i = 0 ; i < vecCount ; i++)
		{
			if (activeAreas[i].contains(event.getPos())) {
				activeTrack = i;
				break;
			}
		}
		onMouseDrag(event);
	}
	
	void VectorVarControl::onMouseDrag(app::MouseEvent & event) {
		float value = this->sliderGetMouseDragPos(event.getPos(), activeArea);
		if (activeTrack >= 0)
			this->setNormalizedValue(activeTrack,value);
	}
	void VectorVarControl::setNormalizedValue(int vec, float value) {
		float newValue = min + value*(max - min);
		newValue = this->displayedValue(newValue,step);
		if ( newValue != (*var)[vec] )
			(*var)[vec] = newValue;
	}
	
	//-----------------------------------------------------------------------------
	// XY CANVAS CONTROL
	//
	XYVarControl::XYVarControl(SimpleGUI *parent, const std::string & name, Vec2f* var, float min, float max, Vec2f defaultValue) : Control(parent,name) {
		this->type = Control::XY_VAR;
		this->var = var;
		this->vecCount = 2;
		this->over = false;
		this->min = min;
		this->max = max;
		this->setPrecision( 2 );
		this->displayValue = true;
		this->drawAsVector = false;
		*var = defaultValue;
		// ROGER
		this->defaultValue = defaultValue;
		this->lastValue = *var;
		this->update();
	}
	
	void XYVarControl::setPrecision(int p) {
		this->precision = p;
		this->step = (1.0f/pow(10.0f,p));
	}
	
	bool XYVarControl::updateMouse()
	{
		int oldChannel = channelOver;
		channelOver = -1;
		//for (int i = 0 ; i < vecCount ; i++)
		if (activeArea.contains(AppBasic::get()->getMousePosMainWindow()))
			channelOver = 0;
		return ( channelOver != oldChannel );
	}
	
	// keyboard inc/dec
	void XYVarControl::inc(bool shifted)
	{
		this->incdec( step * ( shifted ? 10.0 : 1.0 ) );
	}
	void XYVarControl::dec(bool shifted)
	{
		this->incdec( -step * ( shifted ? 10.0 : 1.0 ) );
	}
	void XYVarControl::incY(bool shifted)
	{
		this->incdecY( step * ( shifted ? 10.0 : 1.0 ) );
	}
	void XYVarControl::decY(bool shifted)
	{
		this->incdecY( -step * ( shifted ? 10.0 : 1.0 ) );
	}
	void XYVarControl::incdec(float step)
	{
		if (channelOver >= 0)
			(*var).x = math<float>::clamp( (*var).x + step, min, max );
	}
	void XYVarControl::incdecY(float step)
	{
		if (channelOver >= 0)
			(*var).y = math<float>::clamp( (*var).y + step, min, max );
	}
	
	void XYVarControl::update() {
		Vec2i thumbSize = SimpleGUI::thumbnailSize;

		Vec2f nameSize = (name.length() ? SimpleGUI::labelSize : Vec2f::zero());
		float gap = ( thumbSize.x < SimpleGUI::sliderSize.x ? (SimpleGUI::sliderSize.x-thumbSize.x)/2.0 : 0 );
		activeAreaBase = Rectf(gap,
							   nameSize.y + SimpleGUI::padding.y,
							   gap + thumbSize.x,
							   nameSize.y + SimpleGUI::padding.y + thumbSize.y );
		backArea = Rectf(
						 (-SimpleGUI::padding).x,
						 (-SimpleGUI::padding).y,
						 (SimpleGUI::sliderSize + SimpleGUI::padding).x,
						 (nameSize + SimpleGUI::padding + thumbSize + SimpleGUI::padding ).y );
		//printf("XYVarControl:: size  %d %d\n",(int)activeAreaBase.getWidth(),(int)activeAreaBase.getHeight());
	}
	
	Vec2f XYVarControl::draw(Vec2f pos) {
		this->lastValue = *var;
		if (!enabled)
			return pos;
		//printf("XY  %.3f / %.3f\n",var->x,var->y);
		activeArea = activeAreaBase + pos;
		
		gl::color(SimpleGUI::bgColor);
		gl::drawSolidRect(backArea+pos);
		
		gl::enableAlphaBlending();
		if (name.length())
			gl::drawString(name, pos, SimpleGUI::textColor, SimpleGUI::textFont);
		gl::disableAlphaBlending();
		
		
		// default
		Vec2f dc = Vec2f( lmap(defaultValue.x,min,max,activeArea.x1,activeArea.x2), lmap(defaultValue.y,max,min,activeArea.y1,activeArea.y2) );
		gl::color( Color::gray(0.2) );
		gl::drawLine( Vec2f( activeArea.x1, dc.y ), Vec2f( activeArea.x2, dc.y ) );
		gl::drawLine( Vec2f( dc.x, activeArea.y1), Vec2f( dc.x, activeArea.y2 ) );

		if (this->isHighlighted())
		{
			// border
			gl::color( SimpleGUI::mouseOverColor * (parentGui->selectedControl == this ? 1.0 : 0.8) );
			gl::drawStrokedRect(activeArea);
		}
		
		// current
		gl::color( SLIDER_COLOR );
		Vec2f c = Vec2f( lmap(var->x,min,max,activeArea.x1,activeArea.x2), lmap(var->y,max,min,activeArea.y1,activeArea.y2) );
		c.x = math<int>::clamp(c.x, activeArea.x1, activeArea.x2);
		c.y = math<int>::clamp(c.y, activeArea.y1, activeArea.y2);
		if ( drawAsVector )
			gl::drawLine( Vec2f( dc.x, dc.y ), Vec2f( c.x, c.y ) );
		else
		{
			gl::drawLine( Vec2f( activeArea.x1, c.y ), Vec2f( activeArea.x2, c.y ) );
			gl::drawLine( Vec2f( c.x, activeArea.y1), Vec2f( c.x, activeArea.y2 ) );
		}

		pos.y += backArea.getHeight() + SimpleGUI::spacing.y;
		return pos;
	}
	
	void XYVarControl::fromString(std::string& strValue) {
		std::vector<std::string> strs;
		boost::split(strs, strValue, boost::is_any_of("\t "));
		for (int i = 0 ; i < vecCount ; i++)
			(*var)[i] = boost::lexical_cast<double>(strs[i]);
	}
	
	std::string XYVarControl::toString() {
		std::stringstream ss;
		ss << var->x << " " << var->y;
		return ss.str();
	}
	
	
	void XYVarControl::onMouseDown(app::MouseEvent & event) {
		onMouseDrag(event);
	}
	
	void XYVarControl::onMouseDrag(app::MouseEvent & event) {
		Vec2f c = event.getPos();
		c.x = math<int>::clamp(c.x, activeArea.x1, activeArea.x2);
		c.y = math<int>::clamp(c.y, activeArea.y1, activeArea.y2);
		*var = Vec2f( lmap(c.x, activeArea.x1, activeArea.x2, min, max),
					 lmap(c.y, activeArea.y1, activeArea.y2, max, min) );
	}
	
	//-----------------------------------------------------------------------------
	// ARCBALL QUATERNION CONTROL
	// from sample project: samples/ImageHeightField
	//
	ArcballVarControl::ArcballVarControl(SimpleGUI *parent, const std::string & name, Vec4f* var, Vec4f defaultValue) : Control(parent,name) {
		this->type = Control::ARCBALL_VAR;
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
		this->onResize();
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
		channelOver = ( activeAreas[0].contains(AppBasic::get()->getMousePosMainWindow()) ? 0 : -1 );
		//printf("ARC CH OVER %d\n",channelOver);
		return ( channelOver != oldChannel );
	}
	
	Vec2f ArcballVarControl::draw(Vec2f pos) {
		this->lastValue = *var;
		if (!enabled)
			return pos;
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
		gl::color( resetting ? SLIDER_COLOR : SimpleGUI::darkColor );
		gl::drawSolidRect(activeAreas[1]);
		gl::color(SimpleGUI::bgColor);
		gl::drawString("x", activeAreas[1].getUpperLeft()+Vec2f(2,-1), SimpleGUI::textColor, SimpleGUI::textFont);

		gl::disableAlphaBlending();

		pos.y += backArea.getHeight() + SimpleGUI::spacing.y;
		return pos;
	}
	
	void ArcballVarControl::onMouseDown(app::MouseEvent & event)
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
	void ArcballVarControl::onMouseDrag(app::MouseEvent & event)
	{
		if (rotating)
		{
			mArcball.mouseDrag( event.getPos() );
			*var = mArcball.getQuat().getVec4();
		}
	}
	void ArcballVarControl::onMouseUp(app::MouseEvent & event)
	{
		rotating = resetting = false;
		mustRefresh = true;
		activeTrack = -1;
	}
	void ArcballVarControl::onResize()
	{
		mArcball.setWindowSize( SGUI_SIZE );
		//mArcball.setRadius( SGUI_HEIGHT / 2.0f );
	}


	//-----------------------------------------------------------------------------
	// TEXTURE CONTROL
	//
	TextureVarControl::TextureVarControl(SimpleGUI *parent, const std::string & name, gl::Texture* var, float refreshRate, bool flipVert) : Control(parent,name) {
		this->type = Control::TEXTURE_VAR;
		this->var = var;
		this->lastValue = *var;
		this->flipVert = flipVert;
		this->refreshRate = refreshRate;		// 0.0 means never refresh
		this->refreshTime = 0.0f;
		this->resized = false;
		this->alpha = true;
		this->dragging = false;
		this->resizeTexture();
	}
	
	void TextureVarControl::resizeTexture()
	{
		Vec2i thumbSize;
		if (*var) {
			texSize.x = var->getWidth();
			texSize.y = var->getHeight();
			thumbSize.y = (int) SimpleGUI::thumbnailSize.y;
			thumbSize.x = (int) SimpleGUI::thumbnailSize.y * var->getAspectRatio();
		}
		else {
			// NO TEXTURE!!
			texSize.x = thumbSize.x = (int) SimpleGUI::thumbnailSize.x;
			texSize.y = thumbSize.y = (int) SimpleGUI::thumbnailSize.y;
		}
		
		Vec2f nameSize = (name.length() ? SimpleGUI::labelSize : Vec2f::zero());
		float gap = ( thumbSize.x < SimpleGUI::sliderSize.x ? (SimpleGUI::sliderSize.x-thumbSize.x)/2.0 : 0 );
		activeAreaBase = Rectf(gap, 
							   nameSize.y + SimpleGUI::padding.y, 
							   gap + thumbSize.x,
							   nameSize.y + SimpleGUI::padding.y + thumbSize.y );
		backArea = Rectf(
						 (-SimpleGUI::padding).x, 
						 (-SimpleGUI::padding).y, 
						 (SimpleGUI::sliderSize + SimpleGUI::padding).x, 
						 (nameSize + SimpleGUI::padding + thumbSize + SimpleGUI::padding ).y );
		//printf("TextureVarControl:: size  %d %d\n",(int)activeAreaBase.getWidth(),(int)activeAreaBase.getHeight());
		resized = true;
	}
	
	bool TextureVarControl::valueHasChanged()
	{
		bool shouldRefresh = ( *var && ( refreshTime == 0.0 || (refreshRate > 0.0 && (getElapsedSeconds() - refreshTime) >= refreshRate)) ) ? true : false;
		if ( *var != lastValue )
			shouldRefresh = true;
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
		if (!enabled)
			return pos;
		
		gl::color(SimpleGUI::bgColor);
		gl::drawSolidRect(backArea+pos);
		
		gl::enableAlphaBlending();
		if (name.length())
			gl::drawString(name, pos, SimpleGUI::textColor, SimpleGUI::textFont);
		
		gl::color(ColorA::white());
		if (*var)
		{
			glPushMatrix();
			gl::transformToFit(var->getBounds(),activeArea);
			if ( ! alpha )
			{
				gl::disableAlphaBlending();
				glColorMask(0,0,0,1);
				//gl::drawSolidRect(activeArea);
				gl::drawSolidRect(var->getBounds());
				glColorMask(1,1,1,0);
			}
			//gl::draw(*var, activeArea);
			gl::draw(*var);
			if ( ! alpha )
				glColorMask(1,1,1,1);
			glPopMatrix();
		}
		else
		{
			gl::drawLine( activeArea.getUpperLeft(), activeArea.getLowerRight() );
			gl::drawLine( activeArea.getLowerLeft(), activeArea.getUpperRight() );
		}
		gl::disableAlphaBlending();
		
		if (parentGui->mouseControl == this) {
			gl::color(Color::white());
			gl::drawStrokedRect(activeArea);
		}
		
		pos.y += backArea.getHeight() + SimpleGUI::spacing.y;
		return pos;
	}

	// ROGER
	void TextureVarControl::onMouseMove(app::MouseEvent & event) {
		//dragging = activeArea.contains(event.getPos());
	}
	bool TextureVarControl::fileDrop(FileDropEvent & event) {
		//dragging = activeArea.contains(event.getPos());
		return false;
	}

	
	//-----------------------------------------------------------------------------
	// LIST CONTROL
	//
	ListVarControl::ListVarControl(SimpleGUI *parent, const std::string & name, int* var, const std::map<int,std::string> &valueLabels) : Control(parent,name) {
		this->type = Control::LIST_VAR;
		this->var = var;
		this->defaultValue = *var;
		this->lastValue = *var;
		this->lastSize = (int) valueLabels.size();
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
		this->lastSize = (int) items.size();
		
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
			gl::color(this->isSelected(i) ? SLIDER_COLOR : SimpleGUI::darkColor);
			gl::drawSolidRect(items[i].activeArea);
		}
		
		// draw items labels
		gl::enableAlphaBlending();
		for (int i = 0 ; i < items.size() ; i++) {
			gl::drawString(items[i].label, 
						   items[i].activeArea.getUpperLeft() + SimpleGUI::buttonGap,
						   this->isSelected(i) ? SimpleGUI::textColor2 : SimpleGUI::textColor,
						   SimpleGUI::textFont);
		}
		gl::disableAlphaBlending();
	}
	
	Vec2f ListVarControl::draw(Vec2f pos) {
		if (!enabled)
			return pos;
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
	
	std::string ListVarControl::getValueLabel(int pos)
	{
		if (pos < 0 || pos >= items.size())
			return "";
		else
			return items[pos].label;
	}

	void ListVarControl::onMouseDown(app::MouseEvent & event) {
		for (int i = 0 ; i < items.size() ; i++) {
			if (items[i].activeArea.contains(event.getPos())) {
				*var = items[i].key;
			}
		}
	}
	
	//-----------------------------------------------------------------------------
	// DROP-DOWN LIST CONTROL
	//
	DropDownVarControl::DropDownVarControl(SimpleGUI *parent, const std::string & name, int* var, const std::map<int,std::string> &valueLabels) : ListVarControl(parent,name,var,valueLabels) {
		this->type = Control::DROP_DOWN_VAR;
		this->dropped = false;
		this->lastDropped = this->dropped;
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
		if (!enabled)
			return pos;
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
		gl::color(SLIDER_COLOR);
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
	
	void DropDownVarControl::onMouseDown(app::MouseEvent & event) {
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
							*var = items[i].key;
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
	
	ButtonControl::ButtonControl(SimpleGUI *parent, const std::string & name, const std::string & name2) : Control(parent,name) {
		this->type = Control::BUTTON;
		this->name2 = name2;
		this->centered = false;
		this->pressed = false;
		this->lastPressed = this->pressed;
		this->lastName2 = name2;
		// ROGER
		this->cbFuncDown = NULL;
		this->cbFuncUp = NULL;
		this->update();
	}
	
	void ButtonControl::update() {
		activeAreaBase = Rectf(0,
							   0,
							   SimpleGUI::buttonSize.x,
							   SimpleGUI::buttonSize.y );
		backArea = Rectf((-SimpleGUI::padding).x,
						 (-SimpleGUI::padding).y,
						 (SimpleGUI::buttonSize + SimpleGUI::padding).x,
						 SimpleGUI::buttonSize.y + ( postgap ? SimpleGUI::padding.y : 0) );
	}
	
	Vec2f ButtonControl::draw(Vec2f pos) {
		lastName2 = name2;
		lastPressed = pressed;
		if (!enabled)
			return pos;
		activeArea = activeAreaBase + pos;
		
		gl::color(SimpleGUI::bgColor);
		gl::drawSolidRect(backArea + pos);
		
		gl::color(locked ? SimpleGUI::lockedColor : (pressed ? SLIDER_COLOR : SimpleGUI::darkColor));
		gl::drawSolidRect(activeArea);
		
		gl::enableAlphaBlending();
		if ( centered )
		{
			Vec2f p = pos + Vec2f( SimpleGUI::buttonSize.x * 0.5f, 0);
			gl::drawStringCentered(name, Vec2f(p), pressed ? SimpleGUI::textColor2 : SimpleGUI::textColor, SimpleGUI::textFont);
		}
		else
		{
			ColorA c = ( pressed ? SimpleGUI::textColor2 : SimpleGUI::textColor );
			gl::drawString(name, pos + SimpleGUI::buttonGap, c, SimpleGUI::textFont);
			if (name2.length())
				gl::drawStringRight(name2, pos + Vec2f(SimpleGUI::buttonSize.x,0) - SimpleGUI::buttonGap/2, c, SimpleGUI::textFont);
		}
		gl::disableAlphaBlending();
		
		pos.y += backArea.getHeight() + SimpleGUI::spacing.y;
		return pos;
	}
	
	void ButtonControl::onMouseDown(app::MouseEvent & event) {
		if (locked)
			return;
		pressed = true;
		if ( cbFuncDown )
		{
			cbFuncDown(event);
			event.setHandled();
		}
	}
	
	void ButtonControl::onMouseUp(app::MouseEvent & event) {
		if (locked)
			return;
		if ( cbFuncUp && pressed )
		{
			cbFuncUp(event);
			event.setHandled();
		}
		pressed = false;
	}
	
	// ROGER
	void ButtonControl::onMouseDrag(app::MouseEvent & event) {
		if (locked)
			return;
		pressed = activeArea.contains(event.getPos());
	}
	
	//-----------------------------------------------------------------------------	
	
	LabelControl::LabelControl(SimpleGUI *parent, const std::string & name, std::string * var, const std::string & defaultValue) : Control(parent,name) {
		this->type = Control::LABEL;
		this->wrap = false;
		this->hideNull = true;
		if ( var ) {
			this->var = var;
			*var = defaultValue;
			this->lastVar = defaultValue;
		}
		else {
			this->var = NULL;
			this->lastVar = "";
		}
		this->update();
	}
	LabelControl * LabelControl::setWrap(bool _b)
	{
		wrap = _b;
		if (_b)
		{
			wrapTexFmt = gl::Texture::Format();
			wrapTexFmt.setMinFilter(GL_NEAREST);
			wrapTexFmt.setMagFilter(GL_NEAREST);
		}
		this->update();
		return this;	// chained setters
	}
	void LabelControl::update() {
		float h = SimpleGUI::labelSize.y;
		if ( wrap && name.length() )
		{
			TextBox tbox = TextBox().size(Vec2i(SimpleGUI::sliderSize.x,TextBox::GROW)).font(SimpleGUI::textFont).text(name);
			tbox.setColor( TEXT_COLOR );
			wrapTex = gl::Texture( tbox.render(), wrapTexFmt );
			h = floor(tbox.measure().y);
		}
		backArea = Rectf(( -SimpleGUI::padding).x,
						 ( -SimpleGUI::padding).y,
						 ( SimpleGUI::sliderSize + SimpleGUI::padding).x,
						 ( h + (postgap ? SimpleGUI::padding.y : 2 ) ) );
		// Hide simple label
		if ( !enabled || (var == NULL && name.length() == 0 && hideNull) )
			backArea = Rectf();
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
		this->update();
		if (var)
			lastVar = (*var);
		if (!enabled)
			return pos;
		
		if ( backArea.getHeight() == 0 )	// hidden
			return pos;
		
		gl::color( bgColor ? bgColor : SimpleGUI::bgColor);
		gl::drawSolidRect(backArea+pos);
		
		gl::enableAlphaBlending();
		if ( name.length() )
		{
			if (wrap)
			{
				gl::color( ColorA::white() );
				gl::draw(wrapTex, pos);
			}
			else
			{
				//printf("name[%s] sz %d\n",name.c_str(),(int)name.length());
				gl::drawString(name, pos, TEXT_COLOR, SimpleGUI::textFont);
				if (var)
					gl::drawStringRight((*var), pos+Vec2f(SimpleGUI::sliderSize.x,0), TEXT_COLOR, SimpleGUI::textFont);
			}
		}
		else if (var)
			gl::drawString((*var), pos, TEXT_COLOR, SimpleGUI::textFont);
		gl::disableAlphaBlending();
		
		pos.y += backArea.getHeight() + SimpleGUI::spacing.y;
		return pos;
	}
	
	//-----------------------------------------------------------------------------		
	
	SeparatorControl::SeparatorControl(SimpleGUI *parent) : Control(parent,name) {
		this->type = Control::SEPARATOR;
		this->name = "separator";
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
		//gl::color(SLIDER_COLOR);
		//gl::drawSolidRect(activeArea);		
		
		pos.y += backArea.getHeight() + SimpleGUI::spacing.y;
		return pos;
	}
	
	//-----------------------------------------------------------------------------

	TabControl::TabControl(SimpleGUI *parent, const std::string & tabName, bool *var, bool defaultValue) : AreaControl(parent,"TabControl") {
		this->type = Control::TAB;
		this->name = tabName;
		this->lastName = tabName;
		this->nameOff = tabName;
		this->selected = false;
		this->blocking = false;
		this->asRadio = false;
		this->defaultSelected = false;
		// Bool
		this->tabId = (int) parentGui->theTabs.size();
		this->var = var;
		this->defaultValue = defaultValue;
		this->lastValue = defaultValue;
		if (var)
			*var = defaultValue;
		this->update();
	}

	void TabControl::update() {
		int gap = SimpleGUI::padding.y;
		backArea = Rectf((-SimpleGUI::padding).x,
						 (-SimpleGUI::padding).y,
						 (SimpleGUI::tabSize + SimpleGUI::padding).x,
						 //(SimpleGUI::tabSize + SimpleGUI::padding).y );
						 SimpleGUI::tabSize.y );
		activeAreaBase = Rectf(0,
							   0,
							   SimpleGUI::tabSize.x,
							   SimpleGUI::tabSize.y );
		// bool button
		int sz = SimpleGUI::tabSize.y - (gap * 2);
		boolAreaBase = Rectf( SimpleGUI::sliderSize.x - gap - sz,
							 gap,
							 SimpleGUI::sliderSize.x - gap,
							 gap + sz );
		boolAreaInBase = Rectf( boolAreaBase.x1 + gap,
							   boolAreaBase.y1 + gap,
							   boolAreaBase.x2 - gap,
							   boolAreaBase.y2 - gap );

	}

	Vec2f TabControl::draw(Vec2f pos) {
		this->lastSelected = selected;
		this->lastSwitchable = switchable;
		if (var)
			this->lastValue = *var;

		activeArea = activeAreaBase + pos;
		boolArea = boolAreaBase + pos;
		boolAreaIn = boolAreaInBase + pos;

		gl::color(SimpleGUI::bgColor);
		gl::drawSolidRect(backArea + pos);

		gl::color( locked ? SimpleGUI::lockedColor : (selected ? SimpleGUI::sliderColor : SimpleGUI::darkColor) );
		gl::drawSolidRect(activeArea);

		gl::enableAlphaBlending();
		gl::drawString( (var?(*var?name:nameOff):name), (pos + SimpleGUI::tabGap), (selected ? SimpleGUI::textColor2 : SimpleGUI::textColor), SimpleGUI::textFont);
		gl::disableAlphaBlending();

		if (var)
		{
			gl::color(SimpleGUI::bgColor);
			if (asRadio)
				gl::drawSolidCircle( boolArea.getCenter(), boolArea.getWidth() * 0.5 );
			else
				gl::drawSolidRect(boolArea);
			if (*var)
			{
				gl::color( locked ? SimpleGUI::lockedColor : (switchable ? SimpleGUI::darkColor : SimpleGUI::sliderColor) );
				if (asRadio)
					gl::drawSolidCircle( boolAreaIn.getCenter(), boolAreaIn.getWidth() * 0.5 );
				else
					gl::drawSolidRect(boolAreaIn);
			}
			else if (switchable)
			{
				gl::color( SimpleGUI::darkColor );
				if (asRadio)
					gl::drawStrokedCircle( boolAreaIn.getCenter(), boolAreaIn.getWidth() * 0.5 );
				else
					gl::drawStrokedRect(boolAreaIn);
			}
		}

		pos.y += backArea.getHeight() + SimpleGUI::spacing.y;
		return pos;
	}

	std::string TabControl::toString() {
		std::stringstream ss;
		if (var)
			ss << *var;
		return ss.str();
	}

	void TabControl::fromString(std::string& strValue) {
		if (var)
		{
			int value = boost::lexical_cast<int>(strValue);
			*var = value ? true : false;
		}
	}

	void TabControl::onMouseDown(app::MouseEvent & event) {
		if (locked)
			return;
		// enable tab
		if (!selected) {
			selected = true;
			parentGui->theTab = this;
		}
		// bool var
		if (var && boolArea.contains(event.getPos()) && !switchable)
			*var = ! *var;
	}

	//-----------------------------------------------------------------------------

	ColumnControl::ColumnControl(SimpleGUI *parent, const std::string & colName) : AreaControl(parent,"ColumnControl") {
		this->tab = NULL;
		this->type = Control::COLUMN;
		this->name = ( colName.length() ? colName : "column" );
	}

	Vec2f ColumnControl::draw(Vec2f pos) {
		this->lastSwitchable = switchable;
		if (enabled)
		{
			pos.x += SimpleGUI::labelSize.x + SimpleGUI::spacing.x + SimpleGUI::padding.x*2;
			pos.y = SimpleGUI::spacing.y + SimpleGUI::padding.y*2;
		}
		return pos;
	}

	//-----------------------------------------------------------------------------

	PanelControl::PanelControl(SimpleGUI *parent, const std::string& panelName) : AreaControl(parent,"PanelControl") {
		this->column = false;
		this->type = Control::PANEL;
		this->name = ( panelName.length() ? panelName : "panel" );
	}

	Vec2f PanelControl::draw(Vec2f pos) {
		this->lastSwitchable = switchable;
		return pos;
	}

	//-----------------------------------------------------------------------------

} } // namespace cinder::sgui
