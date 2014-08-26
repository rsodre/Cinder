//
// GUI for Cinder based on SimpleGui
// https://github.com/vorg/MowaLibs
// http://forum.libcinder.org/topic/simplegui
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

#pragma once

#include <vector>
#include "cinder/app/App.h"
#include "cinder/ArcBall.h"
#include "cinder/Text.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/Fbo.h"
#include "cinder/Camera.h"

using namespace cinder;
using namespace ci::app;

namespace cinder { namespace sgui {
	
	//-----------------------------------------------------------------------------
	
	class Control;
	class FloatVarControl;
	class IntVarControl;
	class ByteVarControl;
	class FlagVarControl;
	class BoolVarControl;
	class ButtonControl;
	class LabelControl;
	class SeparatorControl;
	class ColumnControl;
	class PanelControl;
	class ArcballVarControl;
	class TextureVarControl;
	class ColorVarControl;
	class VectorVarControl;
	class ListVarControl;
	class DropDownVarControl;
	
	//-----------------------------------------------------------------------------
	
	class SimpleGUI {
	private:
		bool enabled;
		Vec2f	mousePos;
		std::vector<Control*> controls;
		
		CallbackId	cbMouseMove;
		CallbackId	cbMouseDown;
		CallbackId	cbMouseUp;
		CallbackId  cbMouseDrag;	
		
		void	init(App* app);	
	public:
		static ColorA darkColor;
		static ColorA lightColor;
		static ColorA bgColor;
		static ColorA textColor;
		static ColorA mouseOverColor;
		static ColorA markerColor;
		static Vec2f spacing;
		static Vec2f padding;
		static Vec2f radioSize;
		static Vec2f sliderSize;
		static Vec2f labelSize;
		static Vec2f buttonSize;
		static Vec2f buttonGap;
		static Vec2f separatorSize;
		static Vec2f thumbnailSize;
		static Font textFont;
		
		enum {
			RGB,
			HSV
		};

		SimpleGUI(App* app);
		~SimpleGUI();
		bool	isSelected() { return selectedControl != NULL; }
		std::vector<Control*>& getControls() { return controls; }	
		
		Control	*getMouseOverControl( Vec2i mousePos );
		bool	onMouseMove(MouseEvent event);
		bool	onMouseDown(MouseEvent event);
		bool	onMouseUp(MouseEvent event);
		bool	onMouseDrag(MouseEvent event);
		
		void	draw();
		void	drawGui();	// FBO
		void	dump();
		void	save(std::string fileName = "");
		void	load(std::string fileName = "");
		
		bool	isEnabled();
		void	setEnabled(bool state);
		
		FloatVarControl* 	addParam(const std::string& paramName, float* var, float min=0, float max=1, float defaultValue = 0);
		IntVarControl*		addParam(const std::string& paramName, int* var, int min=0, int max=1, int defaultValue = 0);
		ByteVarControl*		addParam(const std::string& paramName, unsigned char* var, unsigned char defaultValue = 0);
		FlagVarControl*		addParamFlag(const std::string& paramName, unsigned char* var, int maxf=8, unsigned char defaultValue = 0);
		BoolVarControl*		addParam(const std::string& paramName, bool* var, bool defaultValue = false, int groupId = -1);
		ColorVarControl*	addParam(const std::string& paramName, ColorA* var, ColorA const defaultValue = ColorA(0, 1, 1, 1), int colorModel = RGB);
		ColorVarControl*	addParam(const std::string& paramName, Color* var, Color const defaultValue = ColorA(0, 1, 1, 1), int colorModel = RGB);	// ROGER
		VectorVarControl*	addParam(const std::string& paramName, Vec4f* var, int vecCount, float min=0, float max=1, Vec4f const defaultValue = Vec4f::zero());	// ROGER
		ArcballVarControl*	addParam(const std::string& paramName, Vec4f* var, Vec4f const defaultValue);	// ROGER
		TextureVarControl*	addParam(const std::string& paramName, gl::Texture* var, float scale = 1.0, bool flipVert = false);
		
		// ROGER
		LabelControl*		addParam( const std::string &paramName, std::string* var);	// string control
		ListVarControl*		addParamList( const std::string &paramName, int* var, const std::map<int,std::string> &valueLabels );
		ListVarControl*		addParamDropDown( const std::string &paramName, int* var, const std::map<int,std::string> &valueLabels );
		
		ButtonControl*		addButton(const std::string& buttonName);
		LabelControl*		addLabel(const std::string& labelName);
		SeparatorControl*	addSeparator();
		ColumnControl*		addColumn(int x = 0, int y = 0);
		PanelControl*		addPanel(const std::string& panelName="");
		
		static Vec2f		getStringSize(const std::string& str);		
		static Rectf		getScaledWidthRectf(Rectf rect, float scale);
		
		// ROGER
		Control*			getControlByName(const std::string & name);
		int					getColumnWidth()	{ return SimpleGUI::sliderSize.x; };
		bool				hasChanged();
		Vec2f				getSize()			{ return mSize; };
		
		Vec2f				mFinalPosition;
		Vec2f				mSize;
		Vec2f				mOffset;
		
	private:
		// ROGER
		bool		bForceRedraw;
		// FBO
		gl::Fbo		mFbo;
		CallbackId	cbResize;
		CallbackId	cbKeyDown;
		bool		bUsingFbo;
		bool		bShouldResize;
		DropDownVarControl	*droppedList;

		bool		onResize( app::ResizeEvent event );
		bool		onKeyDown( app::KeyEvent event );

	public:
		Control		*mouseControl;		// control with mouse over NOW
		Control		*selectedControl;	// control being manipulated now

	};
	
	//-----------------------------------------------------------------------------
	

	class Control {
	public:
		enum Type {
			FLOAT_VAR,
			INT_VAR,
			BOOL_VAR,
			COLOR_VAR,
			VECTOR_VAR,
			TEXTURE_VAR,
			ARCBALL_VAR,
			LIST_VAR,
			DROP_DOWN_VAR,
			BUTTON,
			LABEL,
			SEPARATOR,
			COLUMN,
			PANEL
		};
		
		// ROGER
		class listVarItem {
		public:
			int				key;
			std::string		label;
			Rectf			activeAreaBase;
			Rectf			activeArea;
		};
		
		
		Vec2f	position;
		ColorA	bgColor;
		Type	type;
		std::string name;
		SimpleGUI *parentGui;
		// ROGER
		bool	mustRefresh;
		bool	readOnly;
		bool	displayValue;
		Vec2f	drawOffset;
		Rectf	backArea;
		Rectf	activeAreaBase;
		Rectf	activeArea;
		Rectf	activeAreasBase[4];
		Rectf	activeAreas[4];
		PanelControl *panelToSwitch;
		Control	*unitControl;
		bool	invertSwitch;
		bool	mouseMoved;
		int		channelOver;

		Control(SimpleGUI *parent);	
		virtual ~Control() {};
		void setBackgroundColor(ColorA color);	
		void notifyUpdateListeners();
		virtual Vec2f draw(Vec2f pos) = 0;
		virtual std::string toString() { return ""; };
		virtual void fromString(std::string& strValue) {};
		virtual void onMouseDown(MouseEvent event) {};
		virtual void onMouseUp(MouseEvent event) {};
		virtual void onMouseDrag(MouseEvent event) {};
		virtual void onResize(ResizeEvent event) {};
		// ROGER
		void setReadOnly(bool b=true);
		void setDisplayValue(bool b=true)			{ displayValue = b; }
		void addSwitchPanel(std::string name)		{ PanelControl *p = parentGui->addPanel(name); this->switchPanel(p); }
		void addSwitchPanelInv(std::string name)	{ PanelControl *p = parentGui->addPanel(name); this->switchPanelInv(p); }
		void switchPanel(PanelControl *p)			{ panelToSwitch = p; invertSwitch = false; }	// Panel to switch ON/OFF with my value
		void switchPanelInv(PanelControl *p)		{ panelToSwitch = p; invertSwitch = true; }		// Panel to switch ON/OFF with my value
		void setUnitControl(Control *c)				{ unitControl = c; }
		bool hasChanged()							{ if (unitControl) if (unitControl->valueHasChanged()) mustRefresh = true; return this->valueHasChanged() || this->mustRefresh; }
		bool isHighlighted(int ch=0)				{ return (channelOver == ch && parentGui->selectedControl == NULL) || (this->isActiveChannel(ch) && parentGui->selectedControl == this); }
		virtual void updateFbo()					{}
		virtual void update()						{}
		virtual void reset()						{}
		virtual bool updateMouse()					{ return false; }		// mouse over requests refresh?
		virtual bool updateKeyboard(char c)			{ return false; }
		virtual bool valueHasChanged()				{ return false; }
		virtual bool hasResized()					{ return false; }
		virtual bool isOn()							{ return true; }		// used to switch panel
		virtual bool keyboardEnabled()				{ return false; }		// keyboard inc/dec
		virtual bool isActiveChannel(int ch)		{ return true; }		// is this the active channel?
		virtual void inc(bool shifted)				{}						// keyboard inc
		virtual void dec(bool shifted)				{}						// keyboard dec
		virtual float getValue()					{ return 0.0; }			// basic float value (when available)
		// draw functions
		void drawBackArea(Rectf a);
	protected:
		float getNormalizedValue(float val, float min, float max) { return (val - min) / (max - min); }
		float displayedValue( float v, float step )	{ return (float)( ((int)(v / step)) * step ); }
	};
	
	//-----------------------------------------------------------------------------
	
	class FloatVarControl : public Control {
	public:	
		float* var;
		float min;
		float max;
		// ROGER
		float defaultValue;
		float lastValue;
		bool formatAsTime;
		bool axisOnDefault;
		int precision;
		float step;
		bool percentage;
	public:
		FloatVarControl(SimpleGUI *parent, const std::string & name, float* var, float min=0, float max=1, float defaultValue = 0);
		void setNormalizedValue(float value);
		Vec2f draw(Vec2f pos);
		std::string toString();
		void fromString(std::string& strValue);
		void onMouseDown(MouseEvent event);	
		void onMouseDrag(MouseEvent event);
		// ROGER
		void update();
		void reset()						{ *var = defaultValue; }
		bool updateMouse();
		void setPrecision(int p);
		void setFormatAsTime(bool b=true)	{ formatAsTime=b; }
		void setAxisOnDefault(bool b=true)	{ axisOnDefault=b; }
		bool isOn()							{ return ( *var ); }		// used to switch panel
		bool keyboardEnabled()				{ return true; }		// used to inc/dec values
		bool valueHasChanged()				{ return ( this->displayedValue(*var,step) != this->displayedValue(lastValue,step) ); };
		void inc(bool shifted)				{ *var = math<float>::clamp( (*var)+(step*(shifted?10:1)), min, max ); }		// keyboard inc
		void dec(bool shifted)				{ *var = math<float>::clamp( (*var)-(step*(shifted?10:1)), min, max ); }		// keyboard dec
		float getValue()					{ return *var; }
	private:
	};
	
	//-----------------------------------------------------------------------------
	
	class IntVarControl : public Control {
	public:
		int* var;
		int min;
		int max;
		// ROGER
		int defaultValue;
		int lastValue;
		int step;
		std::vector<listVarItem> items;

	public:
		IntVarControl(SimpleGUI *parent, const std::string & name, int* var, int min=0, int max=1, int defaultValue = 0);
		void setNormalizedValue(float value);
		Vec2f draw(Vec2f pos);
		std::string toString();	
		void fromString(std::string& strValue);
		void onMouseDown(MouseEvent event);	
		void onMouseDrag(MouseEvent event);	
		// ROGER
		void update();
		void reset()				{ *var = defaultValue; }
		void setStep(int s);
		bool updateMouse();
		bool valueHasChanged()		{ return (*var != lastValue); }
		bool isOn()					{ return (*var); }		// used to switch panel
		bool keyboardEnabled()		{ return true; }		// used to inc/dec values
		void inc(bool shifted)		{ *var = math<int>::clamp( (*var)+(step*(shifted?10:1)), min, max ); }		// keyboard inc
		void dec(bool shifted)		{ *var = math<int>::clamp( (*var)-(step*(shifted?10:1)), min, max ); }		// keyboard dec
		float getValue()			{ return (float)*var; }
	};
	
	//-----------------------------------------------------------------------------
	// ROGER
	class ByteVarControl : public Control {
	public:
		unsigned char* var;
		unsigned char min;
		unsigned char max;
		unsigned char defaultValue;
		unsigned char lastValue;
		
	public:
		ByteVarControl(SimpleGUI *parent, const std::string & name, unsigned char* var, unsigned char defaultValue = 0);
		virtual void setNormalizedValue(float value);
		virtual Vec2f draw(Vec2f pos);
		virtual std::string toString();
		virtual void fromString(std::string& strValue);
		virtual void onMouseDown(MouseEvent event);
		virtual void onMouseDrag(MouseEvent event);
		// ROGER
		bool displayChar;
		bool displayHex;
		virtual void update();
		void reset()				{ *var = defaultValue; }
		bool valueHasChanged()		{ return (*var != lastValue); }
		bool isOn()					{ return (*var); }		// used to switch panel
		float getValue()			{ return (float)*var; }
	};
	
	//-----------------------------------------------------------------------------
	// ROGER
	class FlagVarControl : public ByteVarControl {
	private:
		std::vector<listVarItem> items;
		std::string flagChars;
		int maxFlags;
		int touchedItem;
		int touchedState;

	public:
		FlagVarControl(SimpleGUI *parent, const std::string & name, unsigned char* var, int max=8, unsigned char defaultValue = 0);
		void onMouseDown(MouseEvent event);
		void onMouseDrag(MouseEvent event);
		void update();
		void reset()								{ *var = defaultValue; }
		Vec2f draw(Vec2f pos);
		void setMaxFlags(int maxf);
		void setFlagChars(const std::string & cs)	{ flagChars = cs; };
	};
	
	//-----------------------------------------------------------------------------
	
	class BoolVarControl : public Control {
	public:
		bool* var;
		int groupId;
		// ROGER
		std::string nameOff;
		bool defaultValue;
		bool lastValue;
	public:
		BoolVarControl(SimpleGUI *parent, const std::string & name, bool* var, bool defaultValue, int groupId);
		Vec2f draw(Vec2f pos);	
		std::string toString();	
		void fromString(std::string& strValue);
		void onMouseDown(MouseEvent event);
		// ROGER
		void update();
		void reset()					{ *var = defaultValue; }
		bool valueHasChanged()			{ return (*var != lastValue); };
		bool isOn()						{ return (*var); }		// used to switch panel
		void setAsButton(bool b=true)	{ asButton=b; this->update(); }
		bool asButton;
		float getValue()				{ return (float)*var; }
	};
	
	//-----------------------------------------------------------------------------
	
	class ColorVarControl : public Control {
	public:
		Color*	var;
		ColorA* varA;
		int		activeTrack;
		int		colorModel;
		// ROGER
		Color	defaultValue;
		Color	lastValue;
		ColorA	defaultValueA;
		ColorA	lastValueA;
		bool	alphaControl;
		int		channelCount;
		bool	displayHex;
		Rectf	previewArea;
	public:
		ColorVarControl(SimpleGUI *parent, const std::string & name, Color* var, Color defaultValue, int colorModel);		// ROGER
		ColorVarControl(SimpleGUI *parent, const std::string & name, ColorA* var, ColorA defaultValue, int colorModel);
		Vec2f draw(Vec2f pos);
		std::string toString();	//saved as "r g b a"
		void fromString(std::string& strValue); //expecting "r g b a"
		void onMouseDown(MouseEvent event);	
		void onMouseDrag(MouseEvent event);
		// ROGER
		void update();
		void reset()					{ *var = defaultValue; }
		bool updateMouse();
		bool updateKeyboard(char c);
		bool valueHasChanged()			{ return ( alphaControl ? (*varA!=lastValueA) : (*var!=lastValue) ); };
		bool keyboardEnabled()			{ return true; }					// used to inc/dec values
		bool isActiveChannel(int ch)	{ return (activeTrack == ch); }		// is this the active channel?
		void inc(bool shifted);
		void dec(bool shifted);
		void incdec(int step);
		void setByteValue(int ch, unsigned char b);
		unsigned char getByteValue(int ch);

	};
	
	//-----------------------------------------------------------------------------

	class VectorVarControl : public Control {
	public:
		Vec4f*	var;
		int		activeTrack;
		// ROGER
		int		vecCount;
		float	min;
		float	max;
		int		precision;
		float	step;
		Vec4f	defaultValue;
		Vec4f	lastValue;
		Rectf	previewArea;
	public:
		VectorVarControl(SimpleGUI *parent, const std::string & name, Vec4f* var, int vc, float min, float max, Vec4f defaultValue);
		Vec2f draw(Vec2f pos);
		std::string toString();	//saved as "r g b a"
		void setNormalizedValue(int vec, float value);
		void fromString(std::string& strValue); //expecting "r g b a"
		void onMouseDown(MouseEvent event);	
		void onMouseDrag(MouseEvent event);
		// ROGER
		void update();
		void reset()					{ *var = defaultValue; }
		void setPrecision(int p);
		bool valueHasChanged()			{ return (*var!=lastValue); };
		bool keyboardEnabled()			{ return true; }		// used to inc/dec values
		bool isActiveChannel(int ch)	{ return (activeTrack == ch); }		// is this the active channel?
		bool updateMouse();
		void inc(bool shifted);
		void dec(bool shifted);
		void incdec(float step);
	};
	
	//-----------------------------------------------------------------------------
	
	class ArcballVarControl : public Control {
	public:
		ArcballVarControl(SimpleGUI *parent, const std::string & name, Vec4f* var, Vec4f defaultValue);
		Vec2f draw(Vec2f pos);
		Vec4f*		var;
		Vec4f		defaultValue;
		Vec4f		lastValue;
		gl::Fbo		fbo;
		CameraPersp cam;
		Arcball		mArcball;
		Vec2i		dragStart;
		Vec3f		cameraScale;
		bool		rotating, resetting;
		int			activeTrack;
		void	updateFbo();
		bool	updateMouse();
		bool	valueHasChanged()			{ return (*var!=lastValue); };
		bool	keyboardEnabled()			{ return true; }					// used to inc/dec values
		bool	isActiveChannel(int ch)		{ return (activeTrack == ch); }		// is this the active channel?
		void	setCameraScale(Vec3f s)		{ cameraScale = s; }
		void	onMouseDown(MouseEvent event);
		void	onMouseDrag(MouseEvent event);
		void	onMouseUp(MouseEvent event);
		void	onResize( app::ResizeEvent event );
	private:
		Vec2i texSize;
	};
	
	//-----------------------------------------------------------------------------
	
	class TextureVarControl : public Control {
	public:
		gl::Texture* var;
		float scale;
		bool flipVert;
		TextureVarControl(SimpleGUI *parent, const std::string & name, gl::Texture* var, float scale, bool flipVert = false);
		Vec2f draw(Vec2f pos);
		// ROGER
		bool resized;
		float refreshRate;
		double refreshTime;
		void resizeTexture();
		bool valueHasChanged();
		bool hasResized();
		void refresh()			{ refreshTime = 0; }	// force refresh
		bool isOn()				{ return (*var); }		// used to switch panel
	private:
		Vec2i texSize;
	};
	
	
	//-----------------------------------------------------------------------------
	// ROGER
	
	class ListVarControl : public Control {
	public:
		int* var;
		std::vector<listVarItem> items;
		int defaultValue;
		int lastValue;
		int lastSize;
	public:
		ListVarControl(SimpleGUI *parent, const std::string & name, int* var, const std::map<int,std::string> &valueLabels);
		std::string toString();
		void fromString(std::string& strValue);
		void drawHeader(Vec2f pos);
		void drawList(Vec2f pos);
		void update(const std::map<int,std::string> &valueLabels);
		bool isSelected(int i) { return ( items[i].key == *var ); }
		
		virtual void resize();
		virtual Vec2f draw(Vec2f pos);
		virtual void onMouseDown(MouseEvent event);
		virtual bool valueHasChanged()	{ return (*var != lastValue); }
		virtual bool hasResized()		{ return (items.size() != lastSize); }
		bool isOn()						{ return (*var); }		// used to switch panel
	};
	
	class DropDownVarControl : public ListVarControl {
	public:
		DropDownVarControl(SimpleGUI *parent, const std::string & name, int* var, const std::map<int,std::string> &valueLabels);
		void resize();
		Vec2f draw(Vec2f pos);
		void onMouseDown(MouseEvent event);
		bool hasResized()		{ return (this->ListVarControl::hasResized() || lastDropped!=dropped); }
		void open()				{ dropped = true; this->resize(); }
		void close()			{ dropped = false; this->resize(); }
		
		bool dropped;
		bool lastDropped;
		Vec2f dropButtonGap;
		Rectf dropButtonActiveAreaBase, dropButtonActiveArea;
	};
	
	//-----------------------------------------------------------------------------
	
	class ButtonControl : public Control {
	private:
		bool triggerUp;
		bool pressed;
		bool lastPressed;
		CallbackMgr<bool (MouseEvent)>		callbacksClick;
		CallbackId							callbackId;
		// ROGER
		std::string lastName;
	public:
		ButtonControl(SimpleGUI *parent, const std::string & name);
		Vec2f draw(Vec2f pos);
		void onMouseDown(MouseEvent event);
		void onMouseUp(MouseEvent event);
		void onMouseDrag(MouseEvent event);
		
		//! Registers a callback for Click events. Returns a unique identifier which can be used as a parameter to unregisterClick().
		void	registerClick( std::function<bool (MouseEvent)> callback ) { callbackId = callbacksClick.registerCb( callback ); }
		//! Registers a callback for Click events. Returns a unique identifier which can be used as a parameter to unregisterClick().
		template<typename T>
		void	registerClick( T *obj, bool (T::*callback)(MouseEvent) ) { callbackId = callbacksClick.registerCb( std::bind1st( std::mem_fun( callback ), obj ) ); }
		//! Unregisters a callback for Click events.
		void	unregisterClick() { callbacksClick.unregisterCb( callbackId ); callbackId = 0; }
		
		void fireClick();
		
		// ROGER
		void setTriggerUp(bool b=true)		{ triggerUp = b; }
		bool valueHasChanged()				{ return (pressed!=lastPressed || name.compare(lastName)!=0); };
		bool isOn()							{ return (pressed); }		// used to switch panel

	};
	
	//-----------------------------------------------------------------------------
	
	class LabelControl : public Control {
	public:
		LabelControl(SimpleGUI *parent, const std::string & name);
		LabelControl(SimpleGUI *parent, const std::string & name, std::string * var);
		void setup();
		void setText(const std::string& text);
		Vec2f draw(Vec2f pos);	
		// ROGER
		std::string * var;
		std::string lastName;
		std::string lastVar;
		bool valueHasChanged();
	};
	
	//-----------------------------------------------------------------------------
	
	class SeparatorControl : public Control {
	public:
		SeparatorControl(SimpleGUI *parent);
		Vec2f draw(Vec2f pos);	
	};
	
	//-----------------------------------------------------------------------------
	
	// ROGER
	class AreaControl : public Control {
	public:
		AreaControl(SimpleGUI *parent) : enabled(true), lastEnabled(true), Control(parent) {}
		bool enabled;
		bool lastEnabled;
		bool valueHasChanged()		{ return (enabled != lastEnabled); };
		bool hasResized()			{ return this->valueHasChanged(); };
		void enable(bool e=true)	{ enabled = e; }
		void disable()				{ enabled = false; }
		bool isEnabled()			{ return enabled; }
	};
	
	//-----------------------------------------------------------------------------

	class ColumnControl : public AreaControl {
	public:
		int x;
		int y;
		ColumnControl(SimpleGUI *parent, int x = 0, int y = 0);
		Vec2f draw(Vec2f pos);	
	}; 
	
	//-----------------------------------------------------------------------------
	
	class PanelControl : public AreaControl {
	public:
		PanelControl(SimpleGUI *parent, const std::string& panelName="");
		Vec2f draw(Vec2f pos);
		bool column;		// affects whole column
	};
	
	
	
	//-----------------------------------------------------------------------------
	
} //namespace sgui
} //namespace cinder