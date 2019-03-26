//
// SimpleGui for Cynder
// Adapted by Roger Sodre
// https://github.com/rsodre/
//
// Based on mowaLibs's SimpleGui
// https://github.com/vorg/MowaLibs
// http://forum.libcinder.org/topic/simplegui
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
#include "cinder/Arcball.h"
#include "cinder/Text.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/Fbo.h"
#include "cinder/Camera.h"
#include "cinder/Timeline.h"

using namespace cinder;

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
	class TabControl;
	class ColumnControl;
	class PanelControl;
	class ArcballVarControl;
	class TextureVarControl;
	class ColorVarControl;
	class VectorVarControl;
	class XYVarControl;
	class ListVarControl;
	class DropDownVarControl;
	
	//-----------------------------------------------------------------------------
	
	class SimpleGUI {
	private:
		Vec2f	mousePos;
		std::vector<Control*> controls;
		
		void	init(app::App* app);
	public:
		static ColorA darkColor;
		static ColorA sliderColor;
		static ColorA lockedColor;
		static ColorA bgColor;
		static ColorA textColor;
		static ColorA textColor2;
		static ColorA mouseOverColor;
		static ColorA markerColor;
		static Vec2f spacing;
		static Vec2f padding;
		static Vec2f radioSize;
		static Vec2f sliderSize;
		static Vec2f labelSize;
		static Vec2f tabSize;
		static Vec2f buttonSize;
		static Vec2f buttonGap;
		static Vec2f tabGap;
		static Vec2f separatorSize;
		static Vec2f thumbnailSize;
		static Font textFont;
		
		enum {
			RGB,
			HSV
		};
		
		enum {
			REFRESH_TAB,
			REFRESH_COLUMN,
			REFRESH_CONTROL,
			REFRESH_NONE,
		};
		
		SimpleGUI(app::App* app);
		~SimpleGUI();
		void	setName(std::string name)		{ appName = name; }
		void	displayFps(bool b=true)			{ bDisplayFps = b; }
		
		bool	isSelected()					{ return selectedControl != NULL; }
		std::vector<Control*>& getControls()	{ return controls; }
		
		Control	*getMouseOverControl( Vec2i mousePos );
		ColumnControl *getMouseOverColumn( Vec2i mousePos );
		
		bool	shouldDrawTabs()		{ return (theTabs.size() > 1); }
		void	update();
		void	draw();
		void	drawGui();	// FBO
		Vec2f	drawControl(Vec2f pos, Control *control);
		Vec2f	drawLabel(Vec2f pos, std::string left, std::string right = "");
		void	dump();
		void	save(std::string fileName = "");
		void	load(std::string fileName = "");
		
		void	setVisible(bool state);
		float	getAlpha()				{ return mAlpha.value(); }
		bool	isVisible()				{ return (mAlpha > 0.0); }

		FloatVarControl* 	addParam(const std::string& paramName, float* var, float min=0, float max=1, float defaultValue = 0);
		IntVarControl*		addParam(const std::string& paramName, int* var, int min=0, int max=1, int defaultValue = 0);
		ByteVarControl*		addParam(const std::string& paramName, unsigned char* var, unsigned char defaultValue = 0);
		FlagVarControl*		addParamFlag(const std::string& paramName, unsigned char* var, int maxf=8, unsigned char defaultValue = 0);
		BoolVarControl*		addParam(const std::string& paramName, bool* var, bool defaultValue = false, int groupId = -1);
		ColorVarControl*	addParam(const std::string& paramName, ColorA* var, ColorA const defaultValue = ColorA(0, 1, 1, 1), int colorModel = RGB);
		ColorVarControl*	addParam(const std::string& paramName, Color* var, Color const defaultValue = ColorA(0, 1, 1, 1), int colorModel = RGB);	// ROGER
		VectorVarControl*	addParam(const std::string& paramName, Vec4f* var, int vecCount, float min=0, float max=1, Vec4f const defaultValue = Vec4f::zero());	// ROGER
		ArcballVarControl*	addParam(const std::string& paramName, Vec4f* var, Vec4f const defaultValue);	// ROGER
		TextureVarControl*	addParam(const std::string& paramName, gl::Texture* var, float refreshRate = 1.0, bool flipVert = false);
		
		// ROGER
		LabelControl*		addParam( const std::string &paramName, std::string* var, const std::string & defaultValue="");	// string control
		ListVarControl*		addParamList( const std::string &paramName, int* var, const std::map<int,std::string> &valueLabels );
		ListVarControl*		addParamDropDown( const std::string &paramName, int* var, const std::map<int,std::string> &valueLabels );
		XYVarControl*		addParamXY(const std::string& paramName, Vec2f* var, float min=-1, float max=1, Vec2f const defaultValue = Vec2f::zero());	// ROGER
		
		ButtonControl*		addButton(const std::string& buttonName, const std::string& valName="");
		LabelControl*		addLabel(const std::string& labelName, bool wrap=false);
		SeparatorControl*	addSeparator();
		TabControl*			addTab(const std::string& tabName, bool *var=NULL, bool defaultValue=false);
		ColumnControl*		addColumn(const std::string& colName="");
		PanelControl*		addPanel(const std::string& panelName="");
		
		static Vec2f		getStringSize(const std::string& str);		
		static Rectf		getScaledWidthRectf(Rectf rect, float scale);
		
		// ROGER
		bool				anythingChanged();
		Control*			getControlByName(const std::string & name);
		int					getColumnWidth()	{ return SimpleGUI::sliderSize.x; }
		Vec2f				getSize()			{ return mSize; }
		int					getTabId();
		void				setTab(int t);
		void				setTab(TabControl *t)	{ theTab = t; }
		
		std::vector<TabControl*>	theTabs;
		TabControl					* theTab;			// The current active tab
		TabControl					* lastTab;			// The last active tab

		Vec2f				mFinalPosition;
		Vec2f				mOffset;
		Vec2f				mSize;
		Rectf				mTabsBounds;

		bool		bForceRedraw;

	private:
		// ROGER
		std::string	appName;
		bool		bBlink;				// blink controls (debug)
		bool		bDisplayFps;
		float		mCurrentFps;
		Anim<float>	mAlpha;
		// FBO
		gl::Fbo		mFbo;
		bool		bUsingFbo;
		bool		bShouldResize;
		DropDownVarControl	*droppedList;

		void		onMouseMove(app::MouseEvent & event);
		void		onMouseDown(app::MouseEvent & event);
		void		onMouseUp(app::MouseEvent & event);
		void		onMouseDrag(app::MouseEvent & event);
		void		onFileDrop(app::FileDropEvent & event);
		void		onKeyDown(app::KeyEvent & event);
		void		onResize();

	public:
		bool			mouseTabs;			// Mouse is over the tabs column?
		ColumnControl	*mouseColumn;		// column with mouse over NOW
		Control			*mouseControl;		// control with mouse over NOW
		Control			*selectedControl;	// control being manipulated now

	};
	
	//-----------------------------------------------------------------------------
	//NEW
	//
	class gControl {
	public:
		gControl( gControl *p ) {
			mParent = p;
		}
		
		void _pushift( gControl *c );
		void _push( gControl *c );
		void _shift( gControl *c );
		
		void _draw( Vec2i & start );
		virtual void _privateDraw( Vec2i & start ) {};
		
		gControl				*mParent;
		std::vector<gControl*>	mChildren;
		
		Area	mArea;
		Area	mAreaActive;
	};
	
	class gBoxFloat : public gControl {
	public:
		gBoxFloat( gControl *p );
		void _privateDraw( Vec2i & start );
	};

	class gControlLabel : public gControl {
	public:
		gControlLabel( gControl *p );
		void _privateDraw( Vec2i & start );
	};

	class gControlSlider : public gControl {
	public:
		gControlSlider( gControl *p );
		void _privateDraw( Vec2i & start );
	};
	//
	//NEW
	//-----------------------------------------------------------------------------
	

	//-----------------------------------------------------------------------------
	
	
	class Control {
	public:
		//NEW
		gControl *gc;
		
		
		enum Type {
			FLOAT_VAR,
			INT_VAR,
			BOOL_VAR,
			COLOR_VAR,
			VECTOR_VAR,
			XY_VAR,
			TEXTURE_VAR,
			ARCBALL_VAR,
			LIST_VAR,
			DROP_DOWN_VAR,
			BUTTON,
			LABEL,
			SEPARATOR,
			TAB,
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
		std::string nameOff;
		std::string lastName;
		std::string suffix;
		SimpleGUI *parentGui;
		// ROGER
		bool	locked;
		bool	visible;
		bool	mustRefresh;
		bool	readOnly;
		bool	important;
		bool	displayValue;
		bool	pregap, postgap;				// Y gap at beginning / end
		bool	slim;
		bool	axisOnDefault;
		bool	axisOnZero;
		bool	axisOnMax;
		Vec2f	drawOffset;
		Rectf	backArea;
		Rectf	activeAreaBase;
		Rectf	activeArea;
		Rectf	activeAreasBase[4];
		Rectf	activeAreas[4];
		Control	*unitControl;
		LabelControl *label;
		std::vector<PanelControl*> panelsToSwitch;		// Panel to switch ON with my value
		std::vector<PanelControl*> panelsToSwitchInv;	// Panel to switch OFF with my value
		bool	mouseMoved;
		int		channelOver;
		Control(SimpleGUI *parent, const std::string & name);
		virtual ~Control() {};
		
		bool enabled, lastEnabled;
		void enable(bool b=true)		{ if (b != enabled) { lastEnabled = enabled; enabled = b; mustRefresh = true; } }
		void disable()					{ this->enable(false); }
		bool isEnabled()				{ return enabled; }
		void lock(bool b=true)			{ if (b != locked) { locked = b; mustRefresh = true; } }
		void switchPanels();

		void setBackgroundColor(ColorA color);
		void notifyUpdateListeners();
		virtual Vec2f draw(Vec2f pos) = 0;
		virtual std::string toString() { return ""; };
		virtual void fromString(std::string& strValue) {};
		virtual void onMouseDown(app::MouseEvent & event) {};
		virtual void onMouseUp(app::MouseEvent & event) {};
		virtual void onMouseDrag(app::MouseEvent & event) {};
		virtual bool fileDrop(app::FileDropEvent & event) { return false; };
		virtual void onResize() {};
		// ROGER
		PanelControl* addSwitchPanel(const std::string & name)		{ panelsToSwitch.push_back( parentGui->addPanel(name) );    this->switchPanels(); return panelsToSwitch.back(); }
		PanelControl* addSwitchPanelInv(const std::string & name)	{ panelsToSwitchInv.push_back( parentGui->addPanel(name) ); this->switchPanels(); return panelsToSwitchInv.back(); }
		void setName(const std::string & newName)	{ if (name != newName) mustRefresh = true; name = newName; }
		void setMustRefresh()						{ mustRefresh = true; }
		bool hasChanged()							{ if (unitControl) if (unitControl->valueHasChanged()) mustRefresh = true; return this->valueHasChanged() || this->mustRefresh; }
		bool controlHasResized()					{ return (this->hasResized() || enabled != lastEnabled); }
		bool isHighlighted(int ch=0)				{ return ( (channelOver == ch && parentGui->selectedControl == NULL) || (this->isActiveChannel(ch) && parentGui->selectedControl == this) ) && ! this->locked; }
		bool isInteracting()						{ return (parentGui->selectedControl == this); }
		void setUnitControl(Control *c)				{ unitControl = c; }
		Control* setReadOnly(bool b=true);			// chained setters
		Control* setImportant(bool b=true);
		Control* setDisplayValue(bool b=true)		{ displayValue = b; this->update(); return this; }	// chained setters
		Control* setPreGap(bool b)					{ pregap = b; this->update(); return this; }		// chained setters
		Control* setPostGap(bool b)					{ postgap = b; this->update(); return this; }		// chained setters
		Control* setNameOff(const std::string & n)	{ nameOff = n; return this; }						// chained setters
		Control* setSlim(bool b=true)				{ slim = b; this->update(); return this; }			// chained setters
		Control* setSuffix(const std::string & s)	{ suffix = s; mustRefresh = true; return this; }
		Control* setAxisOnDefault(bool b=true)		{ axisOnDefault=b; return this; }					// chained setters
		Control* setAxisOnZero(bool b=true)			{ axisOnZero=b; return this; }						// chained setters
		Control* setAxisOnMax(bool b=true)			{ axisOnMax=b; return this; }						// chained setters
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
		virtual void incY(bool shifted)				{ return this->inc(shifted); }	// keyboard inc (Y)
		virtual void decY(bool shifted)				{ return this->dec(shifted); }	// keyboard dec (Y)
		virtual float getValue()					{ return 0.0; }			// basic float value (when available)
		virtual void setDefaultValue(void *v)		{}
		virtual void setLimits(float vmin, float vmax)	{}
		// draw functions
		void drawBackArea(Rectf a);
		// generic float slider
		float sliderGetMouseDragPos(Vec2i pos, Rectf activeArea);
		// statics
		static float getNormalizedValue(float val, float min, float max)	{ return (val - min) / (max - min); }
		static float displayedValue(float v, float step)					{ return roundf(v / step) * step; }
	protected:
	};
	
	//-----------------------------------------------------------------------------
	
	class FloatVarControl : public Control {
	public:	
		float* var;
		float min;
		float max;
		float complement;
		// ROGER
		float defaultValue;
		float lastValue;
		bool formatAsTime;
		bool formatAsTimecode;
		bool displaySign;
		int precision;
		float step;
		bool percentage;
	public:
		FloatVarControl(SimpleGUI *parent, const std::string & name, float* var, float min=0, float max=1, float defaultValue = 0);
		void setNormalizedValue(float value);
		void setComplement(float c);
		Vec2f draw(Vec2f pos);
		std::string toString();
		void fromString(std::string& strValue);
		void onMouseDown(app::MouseEvent & event);	
		void onMouseDrag(app::MouseEvent & event);
		// ROGER
		void update();
		void reset()							{ *var = defaultValue; }
		bool updateMouse();
		FloatVarControl* setPrecision(int p);														// chamed setters
		FloatVarControl* setFormatAsTime(bool b=true)		{ formatAsTime=b; return this; }		// chamed setters
		FloatVarControl* setFormatAsTimecode(bool b=true)	{ formatAsTimecode=b; return this; }	// chamed setters
		FloatVarControl* setFormatAsPercentage(bool b=true)	{ percentage=b; return this; }			// chamed setters
		bool isOn()						{ return ( *var != 0.0f ); }	// used to switch panel
		bool keyboardEnabled()			{ return true; }				// used to inc/dec values
		bool valueHasChanged()			{ return ( this->displayedValue(*var,step) != this->displayedValue(lastValue,step) ); };
		void inc(bool shifted)			{ *var = math<float>::clamp( (*var)+(step*(shifted?10:1)), min, max ); }		// keyboard inc
		void dec(bool shifted)			{ *var = math<float>::clamp( (*var)-(step*(shifted?10:1)), min, max ); }		// keyboard dec
		float getValue()				{ return *var; }
		void setDefaultValue(void *v)	{ defaultValue = *((float*)v); }
		void setLimits(float vmin, float vmax)	{ min = vmin; max = vmax; this->update(); mustRefresh = true; }
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
		void onMouseDown(app::MouseEvent & event);	
		void onMouseDrag(app::MouseEvent & event);	
		// ROGER
		void update();
		void reset()					{ *var = defaultValue; }
		void setStep(int s);
		bool updateMouse();
		bool valueHasChanged()			{ return (*var != lastValue); }
		bool isOn()						{ return (*var != 0); }		// used to switch panel
		bool keyboardEnabled()			{ return true; }			// used to inc/dec values
		void inc(bool shifted)			{ *var = math<int>::clamp( (*var)+(step*(shifted?10:1)), min, max ); }		// keyboard inc
		void dec(bool shifted)			{ *var = math<int>::clamp( (*var)-(step*(shifted?10:1)), min, max ); }		// keyboard dec
		float getValue()				{ return (float)*var; }
		void setDefaultValue(void *v)	{ defaultValue = *((int*)v); }
		void setLimits(float vmin, float vmax)	{ min = vmin; max = vmax; this->update(); mustRefresh = true; }
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
		virtual void onMouseDown(app::MouseEvent & event);
		virtual void onMouseDrag(app::MouseEvent & event);
		// ROGER
		bool displayChar;
		bool displayHex;
		virtual void update();
		bool updateMouse();
		void reset()				{ *var = defaultValue; }
		bool valueHasChanged()		{ return (*var != lastValue); }
		bool isOn()					{ return (*var != 0); }		// used to switch panel
		float getValue()			{ return (float)*var; }
		bool keyboardEnabled()		{ return true; }			// used to inc/dec values
		void inc(bool shifted)		{ *var = math<int>::clamp( (*var)+(shifted?8:1), min, max ); }		// keyboard inc
		void dec(bool shifted)		{ *var = math<int>::clamp( (*var)-(shifted?8:1), min, max ); }		// keyboard dec
		void setDefaultValue(void *v)	{ defaultValue = *((unsigned char*)v); }
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
		void onMouseDown(app::MouseEvent & event);
		void onMouseDrag(app::MouseEvent & event);
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
		bool defaultValue;
		bool lastValue;
	public:
		BoolVarControl(SimpleGUI *parent, const std::string & name, bool* var, bool defaultValue, int groupId);
		Vec2f draw(Vec2f pos);	
		std::string toString();	
		void fromString(std::string& strValue);
		void onMouseDown(app::MouseEvent & event);
		void onMouseUp(app::MouseEvent & event);
		// ROGER
		void update();
		void reset()					{ *var = defaultValue; }
		bool valueHasChanged()			{ return (*var != lastValue); };
		bool isOn()						{ return (*var); }		// used to switch panel
		float getValue()				{ return (float)*var; }
		BoolVarControl* setAsButton(bool b=true)	{ asButton=b; this->update(); return this; }		// chained setters
		BoolVarControl* setAsSwitch(bool b=true)	{ switchButton=b; this->update(); return this; }	// chained setters
		BoolVarControl* setDontGoOff(bool b=true)	{ dontGoOff=b; this->update(); return this; }		// chained setters
		void setDefaultValue(void *v)	{ defaultValue = *((bool*)v); }
		bool asButton;
		bool switchButton;
		bool dontGoOff;
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
		Rectf	previewArea;
	public:
		ColorVarControl(SimpleGUI *parent, const std::string & name, Color* var, Color defaultValue, int colorModel);		// ROGER
		ColorVarControl(SimpleGUI *parent, const std::string & name, ColorA* var, ColorA defaultValue, int colorModel);
		Vec2f draw(Vec2f pos);
		std::string toString();	//saved as "r g b a"
		void fromString(std::string& strValue); //expecting "r g b a"
		void onMouseDown(app::MouseEvent & event);
		void onMouseDrag(app::MouseEvent & event);
		// ROGER
		void update();
		void reset()					{ if (alphaControl) *varA = defaultValueA; else *var = defaultValue; }
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
		void onMouseDown(app::MouseEvent & event);
		void onMouseDrag(app::MouseEvent & event);
		// ROGER
		void update();
		void reset()					{ *var = defaultValue; }
		void setPrecision(int p);
		bool valueHasChanged()			{ return (*var!=lastValue); };
		bool keyboardEnabled()			{ return true; }		// used to inc/dec values
		bool isActiveChannel(int ch)	{ return (activeTrack == ch); }		// is this the active channel?
		void setDefaultValue(void *v)	{ defaultValue = *((Vec4f*)v); }
		bool updateMouse();
		void inc(bool shifted);
		void dec(bool shifted);
		void incdec(float step);
	};
	
	//-----------------------------------------------------------------------------
	
	class XYVarControl : public Control {
	public:
		Vec2f*	var;
		// ROGER
		int		vecCount;
		float	min;
		float	max;
		int		precision;
		float	step;
		bool	over;
		bool	drawAsVector;
		Vec2f	defaultValue;
		Vec2f	lastValue;
		Rectf	previewArea;
	public:
		XYVarControl(SimpleGUI *parent, const std::string & name, Vec2f* var, float min, float max, Vec2f defaultValue);
		Vec2f draw(Vec2f pos);
		std::string toString();	//saved as "x y"
		void setNormalizedValue(int vec, float value);
		void fromString(std::string& strValue); //expecting "r g b a"
		void onMouseDown(app::MouseEvent & event);
		void onMouseDrag(app::MouseEvent & event);
		// ROGER
		void update();
		void reset()						{ *var = defaultValue; }
		void setPrecision(int p);
		bool valueHasChanged()				{ return (*var!=lastValue); };
		bool keyboardEnabled()				{ return true; }		// used to inc/dec values
		bool isActiveChannel(int ch)		{ return true; }		// is this the active channel?
		void setDefaultValue(void *v)		{ defaultValue = *((Vec2f*)v); }
		void setDrawAsVector(bool b=true)	{ drawAsVector = b; }
		bool updateMouse();
		void inc(bool shifted);
		void dec(bool shifted);
		void incY(bool shifted);
		void decY(bool shifted);
		void incdec(float step);
		void incdecY(float step);
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
		void	setDefaultValue(void *v)	{ defaultValue = *((Vec4f*)v); }
		void	onMouseDown(app::MouseEvent & event);
		void	onMouseDrag(app::MouseEvent & event);
		void	onMouseUp(app::MouseEvent & event);
		void	onResize();
	private:
		Vec2i texSize;
	};
	
	//-----------------------------------------------------------------------------
	
	class TextureVarControl : public Control {
	public:
		gl::Texture* var;
		bool flipVert;
		TextureVarControl(SimpleGUI *parent, const std::string & name, gl::Texture* var, float refreshRate, bool flipVert = false);
		Vec2f draw(Vec2f pos);
		void onMouseMove(app::MouseEvent & event);
		bool fileDrop(app::FileDropEvent & event);
		// ROGER
		bool resized;
		double refreshTime;
		void resizeTexture();
		bool valueHasChanged();
		bool hasResized();
		void refresh()								{ refreshTime = 0; }		// force refresh
		bool isOn()									{ return (*var); }			// used to switch panel
		TextureVarControl* setRefreshRate(float s)	{ refreshRate =s; return this; }	// chained setters
		TextureVarControl* setAlpha(bool a)			{ alpha = a; return this; }			// chained setters
		void setTexture(gl::Texture * t)			{ var = t; mustRefresh = true; }
	private:
		bool alpha;
		bool dragging;
		float refreshRate;
		Vec2i texSize;
		gl::Texture lastValue;
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
		std::string getValueLabel(int pos);
		
		virtual void resize();
		virtual Vec2f draw(Vec2f pos);
		virtual void onMouseDown(app::MouseEvent & event);
		virtual bool valueHasChanged()	{ return (*var != lastValue); }
		virtual bool hasResized()		{ return (items.size() != lastSize); }
		bool isOn()						{ return (*var != 0); }		// used to switch panel
	};
	
	class DropDownVarControl : public ListVarControl {
	public:
		DropDownVarControl(SimpleGUI *parent, const std::string & name, int* var, const std::map<int,std::string> &valueLabels);
		void resize();
		Vec2f draw(Vec2f pos);
		void onMouseDown(app::MouseEvent & event);
		bool hasResized()		{ return (this->ListVarControl::hasResized() || lastDropped!=dropped); }
		void open()				{ dropped = true; this->resize(); }
		void close()			{ dropped = false; this->resize(); }
		void setDefaultValue(void *v)	{ defaultValue = *((int*)v); }

		bool dropped;
		bool lastDropped;
		Vec2f dropButtonGap;
		Rectf dropButtonActiveAreaBase, dropButtonActiveArea;
	};
	
	//-----------------------------------------------------------------------------
	
	class ButtonControl : public Control {
	private:
		bool centered;
		bool pressed;
		bool lastPressed;
		// ROGER
		std::function<void(app::MouseEvent&)> cbFuncDown;
		std::function<void(app::MouseEvent&)> cbFuncUp;
		std::string name2;
		std::string lastName2;
	public:
		ButtonControl(SimpleGUI *parent, const std::string & name, const std::string & name2="");
		Vec2f draw(Vec2f pos);
		void update();
		void onMouseDown(app::MouseEvent & event);
		void onMouseUp(app::MouseEvent & event);
		void onMouseDrag(app::MouseEvent & event);
		
		//! Registers a callback for Click events. Returns a unique identifier which can be used as a parameter to unregisterClick().
		void	registerClickDown( std::function<void(app::MouseEvent&)> callback ) {
			cbFuncDown = callback;
		}
		void	registerClickUp( std::function<void(app::MouseEvent&)> callback ) {
			cbFuncUp = callback;
		}
		//! Registers a callback for Click events. Returns a unique identifier which can be used as a parameter to unregisterClick().
		template<typename T>
		void	registerClickDown( T *obj, void(T::*callback)(app::MouseEvent&) ) {
			cbFuncDown = std::bind( callback, obj, std::placeholders::_1 );
		}
		template<typename T>
		void	registerClickUp( T *obj, void(T::*callback)(app::MouseEvent&) ) {
			cbFuncUp = std::bind( callback, obj, std::placeholders::_1 );
		}
		
		// ROGER
		ButtonControl* setCentered(bool b=true)		{ centered = b; return this; }		// chained setters
		bool valueHasChanged()						{ return (pressed!=lastPressed || name2.compare(lastName2)!=0); };
		bool isOn()									{ return (pressed); }		// used to switch panel

	};
	
	//-----------------------------------------------------------------------------
	
	class LabelControl : public Control {
	public:
		LabelControl(SimpleGUI *parent, const std::string & name, std::string * var=NULL, const std::string & defaultValue="");
		void update();
		void setText(const std::string& text);
		Vec2f draw(Vec2f pos);
		LabelControl * setWrap(bool _b);	// chained setters
		bool valueHasChanged();
		// ROGER
		std::string * var;
		std::string lastVar;
		gl::Texture wrapTex;
		gl::Texture::Format wrapTexFmt;
		bool wrap;
		bool hideNull;
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
		AreaControl(SimpleGUI *parent, const std::string & name) : switchable(false), lastSwitchable(false), Control(parent,name) {}
		bool valueHasChanged()			{ return (switchable != lastSwitchable); }
		bool hasResized()				{ return this->valueHasChanged(); }
		void setSwitchable(bool b=true)	{ switchable = b; }
		Rectf backArea;
	protected:
		bool switchable;
		bool lastSwitchable;
	};
	
	//-----------------------------------------------------------------------------
	
	class TabControl : public AreaControl {
	public:
		// Bool
		bool* var;
		bool defaultValue;
		bool lastValue;
		bool selected;
		bool lastSelected;
		bool defaultSelected;
		bool blocking;
		bool asRadio;
		int tabId;
		Rectf boolAreaBase;
		Rectf boolArea;
		Rectf boolAreaInBase;
		Rectf boolAreaIn;
	public:
		TabControl(SimpleGUI *parent, const std::string & tabName, bool *var=NULL, bool def=false);
		Vec2f draw(Vec2f pos);
		void onMouseDown(app::MouseEvent & event);
		std::string toString();
		void fromString(std::string& strValue);
		void update();
		void select(bool b=true)		{ selected = b; }
		void reset()					{ selected = defaultSelected; if (var) *var = defaultValue; }
		bool valueHasChanged()			{ return (var ? (*var != lastValue) : false ) || lastSelected != selected || this->AreaControl::valueHasChanged(); }
		bool hasResized()				{ return false; };
		float getValue()				{ return (float)*var; }
		void setBlocking(bool b=true)	{ if (b != blocking) { blocking = b ; parentGui->bForceRedraw = true; } }
		void setRadio(bool b=true)		{ asRadio = b ; parentGui->bForceRedraw = true; }
	};
	
	//-----------------------------------------------------------------------------
	
	class ColumnControl : public AreaControl {
	public:
		TabControl* tab;
		ColumnControl(SimpleGUI *parent, const std::string & colbName);
		Vec2f draw(Vec2f pos);
	};
	
	//-----------------------------------------------------------------------------
	
	class PanelControl : public AreaControl {
	public:
		PanelControl(SimpleGUI *parent, const std::string & panelName="");
		Vec2f draw(Vec2f pos);
		PanelControl* setColumn(bool b=true)	{ column=b; return this; }
		bool column;		// affects whole column
	};
	
	
	
	//-----------------------------------------------------------------------------
	
} //namespace sgui
} //namespace cinder
