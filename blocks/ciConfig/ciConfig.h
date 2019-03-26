//
//	ciConfig.h
//
//  Created by Roger Sodre on 08/04/2010
//  Copyright 2011 Studio Avante. All rights reserved.
//
#pragma once

////////////////////////////////////////
//
// ciConfig
//
// Originally ofxConfig, made for openFrameworks
// Adapted to work on Cinder by typecasting some OF classes
// QB is for Cinder, so I'll make it a Cinder lib by default
// OF support is now broken, sorry.
//


////////////////////////////////////////
//
// CONFIG FOR CINDER
//
#include "cinder/Cinder.h"
#include "cinder/app/App.h"
#include "cinder/Vector.h"
#include "cinder/Quaternion.h"
#include "cinder/Utilities.h"
#include "cinder/Xml.h"
#include <vector>
#include <map>
#include <string>
#define CFG_CATCH_LOOP_EVENTS
// Cinder Touch
#ifndef CINDER_COCOA_TOUCH
#define CFG_USE_MIDI
//#define CFG_USE_OSC
#endif

//
// Config OSC
#if defined(CFG_USE_OSC) && defined(NO_CFG_USE_OSC)
#undef CFG_USE_OSC
#endif
#ifdef CFG_USE_OSC
#include "OscListener.h"
#include "OscSender.h"
using namespace osc;
typedef osc::Sender ofxOscSender;
typedef osc::Listener ofxOscReceiver;
typedef osc::Message ofxOscMessage;
// OSC Defaults
#define CICFG_OSC_PORT		1122
#define CICFG_OSC_HOST		"localhost"
#endif

//
// MIDI
//
#if defined(CFG_USE_MIDI) && defined(NO_CFG_USE_MIDI)
#undef CFG_USE_MIDI
#endif
#ifdef CFG_USE_MIDI
#include "MidiHub.h"
#endif

//
// MISC DEFINES
// 
#ifndef TWO_PI
#define TWO_PI (M_PI*2.0f)
#endif
#ifndef HALF_PI
#define HALF_PI (M_PI/2.0f)
#endif


namespace cinder {
	

// MIDI defaults
#define MIDI_NONE				-1
#define MIDI_TRIGGER			-2	// The note is actually the value to set
	
#define	CFG_MAX_DATA_LEN		4096
#define FLOAT_VEC				vector<float>

#define CONFIG_PRESET_COUNT		10
#define PRESET_KEY(i)			(i<CONFIG_PRESET_COUNT-1?'1'+i:(i==CONFIG_PRESET_COUNT-1?'0':0))

#define MAX_DEFAULTS			12

//
// Types
enum enumConfigFlags
{
	CFG_FLAG_NONE,
	CFG_FLAG_DROP_DOWN,
	CFG_FLAG_ARCBALL,
	CFG_FLAG_XY,
	CFG_FLAG_XY_VECTOR,
	CFG_FLAG_ALPHA,
	// count
	CFG_FLAG_COUNT
};

	
//
// Types
enum enumConfigTypes
{
	CFG_TYPE_NONE = -1,	
	CFG_TYPE_FLOAT,
	CFG_TYPE_DOUBLE,
	CFG_TYPE_INTEGER,
	CFG_TYPE_LONG,
	CFG_TYPE_BOOLEAN,
	CFG_TYPE_BYTE,
	CFG_TYPE_STRING,
	CFG_TYPE_COLOR,
	CFG_TYPE_VECTOR2,
	CFG_TYPE_VECTOR3,
	CFG_TYPE_VECTOR4,
	CFG_TYPE_COUNT
};

////////////////////////////////////////////////////////////////////////
//
// CONFIG VALUE
//
class ciConfigValue {
public:
	bool	preserveProg;		// priority is prog
	bool	freshness;			// fresh value to read?
	short	midiChannel, midiNote, midiInc, midiDec, midiSwitch;
	char	keyInc[20], keyDec[20], keySwitch[20];
	
	void setup()
	{
		trigger = false;
		freshness = false;
		preserveProg = false;
		midiChannel = 1;
		midiNote = MIDI_NONE;
		midiInc = MIDI_NONE;
		midiDec = MIDI_NONE;
		midiSwitch = MIDI_NONE;
		strcpy( keyInc, "" );
		strcpy( keyDec, "" );
		strcpy( keySwitch, "" );
		this->setLimits(0.0f,1.0f);
		this->set(0.0f);
		this->updateLastValue();
		for (int d = 0 ; d < MAX_DEFAULTS ; d++)
			initialValue[d] = 0.0;
	}
	
	// Setters
	void set(float v)
	{
		//if (value == 0.0 && v != 0)
		if (value < 1.0 && v != 0)		// trigger sensivel
			trigger = true;
		value = v;
		if (min != max)			// avoid div by zero
			prog = (value-min) / (max-min);
		else
			prog = 0.0f;
	}
	void setProg(float p)
	{
		//if (prog == 0.0 && p != 0)
		if (prog < 1.0 && p != 0)		// trigger sensivel
			trigger = true;
		prog = p;
		value = min + ((max-min) * prog);
	}
	void setLimits(float vmin, float vmax)
	{
		min = vmin;
		max = vmax;
		if (preserveProg)
			this->setProg(prog);	// re-calc value
		else
			this->set(value);		// re-calc prog
	}
	void updateLastValue()	{ lastValue = value; }
	void init()				{ for (int d = 0 ; d < MAX_DEFAULTS ; d++) initialValue[d] = value; }
	void setInitialValue(int def, float v)	{ initialValue[def] = v; }

	// Getters
	float get()							{ return value; }
	float getProg()						{ return prog; }
	float getMin()						{ return min; }
	float getMax()						{ return max; }
	float getLastValue()				{ return lastValue; }
	float getInitialValue(int def=0)	{ return initialValue[def]; }
	bool getTrigger()
	{
		bool t = trigger;
		trigger = false;
		return t;
	}
	
	// Operations
	void invert()
	{
		this->set(max-value);
	}
	void sub(float v=1.0f, bool clamp=true)
	{
		this->set(value-v);
		if (clamp)
			this->clamp();
	}
	void add(float v=1.0f, bool clamp=true)
	{
		this->set(value+v);
		if (clamp)
			this->clamp();
	}
	void clamp()
	{
		if (value < min)
			this->set(min);
		else if (value > max)
			this->set(max);
	}
	
private:
	float	value;
	float	lastValue;						// managed by ciConfig
	float	initialValue[MAX_DEFAULTS];		// managed by ciConfig
	float	prog;
	float	min;
	float	max;
	bool	trigger;		// activated when set from 0 to !0, deactivated when get() is called
};

////////////////////////////////////////////////////////////////////////
//
// CONFIG PARAMETER
//
class ciConfigParam {
public:
	short						pid;
	short						type;
	short						vecCount;
	std::string					name;
	std::map<int,std::string>	valueLabels;
	bool						dummy;			// does not load/save
	bool						editable;
	bool						quater;
	char						flag;
	char						changeGroups;	// group flags
	bool						loaded;			// value was loaded from file?
	bool						changed;		// value changed last frame?
	// Values
	std::string					strval;
	std::string					strvalInitial[MAX_DEFAULTS];
	std::string					strvalLast;
    union {	// share same memory, so vector[0] = value
		struct {
			ciConfigValue value, filler[3];
		};
		ciConfigValue vec[4];
    };
	// for cinder::param
	void			*guiControl;
	float			watchFloat[4];
	bool			watchBool;
	unsigned char	watchByte;
	int				watchInt;
	Color			watchColor;
	ColorA			watchColorA;
	Vec2f			watchVector2;
	Vec4f			watchVector;
	std::string		watchString;
	
	ciConfigParam(short _pid, short _type, const std::string _name)
	{
		pid = _pid;
		type = _type;
		name = _name;
		dummy = false;
		quater = false;
		editable = true;
		guiControl = NULL;
		loaded = false;
		changed = false;
		changeGroups = 0x00;
		flag = 0x00;
		// vector value count
		switch (_type)
		{
			case CFG_TYPE_COLOR:
			case CFG_TYPE_VECTOR4:
				vecCount = 4;
				break;
			case CFG_TYPE_VECTOR3:
				vecCount = 3;
				break;
			case CFG_TYPE_VECTOR2:
				vecCount = 2;
				break;
			default:
				vecCount = 1;
				break;
		}
		// init ciConfigValue here because union members cannot have constructors
		for (int n = 0 ; n < 4 ; n++)
			vec[n].setup();
	}
	
	// init / reset
	void init()
	{
		for (int i = 0 ; i < 4 ; i++)
			vec[i].init();
		strvalLast = strval;
		for (int d = 0 ; d < MAX_DEFAULTS ; d++)
			strvalInitial[d] = strval;
	}

	// return current freshness - automatic UNFRESH
	bool isFresh( bool _unfresh=true )
	{
		bool freshness = (vec[0].freshness || vec[1].freshness || vec[2].freshness || vec[3].freshness);
		if (_unfresh)
			this->unfresh();
		return freshness; 
	}
	void unfresh()
	{
		vec[0].freshness = vec[1].freshness = vec[2].freshness = vec[3].freshness = false; 
	}
	std::string getLastValueString()
	{
		return strvalLast;
	}
	void updateLastValueString()
	{
		strvalLast = strval;
	}

	// return current freshness - automatic UNFRESH
	bool isFloat()			{ return ( type == CFG_TYPE_FLOAT ); }
	bool isBool()			{ return ( type == CFG_TYPE_BOOLEAN ); }
	bool isColor()			{ return ( type == CFG_TYPE_COLOR ); }
	bool isColorVector()	{ return ( type == CFG_TYPE_COLOR || this->isVector() );	}
	bool isVector()			{ return ( type == CFG_TYPE_VECTOR2 || type == CFG_TYPE_VECTOR3 || type == CFG_TYPE_VECTOR4 ); }
	bool isVector2()		{ return ( type == CFG_TYPE_VECTOR2 ); }
	bool isVector3()		{ return ( type == CFG_TYPE_VECTOR3 ); }
	bool isVector4()		{ return ( type == CFG_TYPE_VECTOR4 ); }
	bool isByte()			{ return ( type == CFG_TYPE_BYTE ); }
	bool isInt()			{ return ( type == CFG_TYPE_INTEGER || type == CFG_TYPE_LONG || type == CFG_TYPE_BYTE ||  type == CFG_TYPE_BOOLEAN ); }
	bool isString()			{ return ( type == CFG_TYPE_STRING ); }

	// Cinder Param ingtegration
	float*			getPointer(int i)	{ return &(watchFloat[i]); }
	bool*			getPointerBool()	{ return &watchBool; }
	unsigned char*	getPointerByte()	{ return &watchByte; }
	int*			getPointerInt()		{ return &watchInt; }
	Color*			getPointerColor()	{ return &watchColor; }
	ColorA*			getPointerColorA()	{ return &watchColorA; }
	Vec2f*			getPointerVector2()	{ return &watchVector2; }
	Vec4f*			getPointerVector()	{ return &watchVector; }
	std::string*	getPointerString()	{ return &watchString; }
	// update param pointers
	void updatePointers(int i)
	{
		float val = vec[i].get();
		if (i == 0)
		{
			watchBool = (bool) val;
			watchByte = (unsigned char) val;
			watchInt = (int) val;
			//printf("UPDATE WATCHER 0 watchString [%d] [%s]\n",(int)&watchString,watchString.c_str());
			watchString.assign( strval );
			//printf("UPDATE WATCHER 1 watchString [%d] [%s]\n",(int)&watchString,watchString.c_str());
		}
		watchFloat[i] = val;
		if (i < 2)
			watchVector2[i] = val;
		if (i < 3)
			watchColor[i] = val / 255.0f;
		if (i < 4)
		{
			watchColorA[i] = val / 255.0f;
			watchVector[i] = val;
		}
		//printf("CONFIG updatePointers %s = %.3f\n",name.c_str(),val);
	}
};

////////////////////////////////////////////////////////////////////////
//
// CONFIG CLASS
//
class ciConfig
{
public:
	ciConfig(ciConfig *_parent=NULL);
	~ciConfig();
	
	// main loop
	virtual void update();			// Update state from OSC
	virtual void draw() {}			// Draw GUI
	void onKeyDown( app::KeyEvent & event );
	void onFileDrop( app::FileDropEvent & event );

	// Virtual callbacks
	virtual void postSetCallback(int id, int i) {}		// After a set()
	virtual void preSaveCallback() {}					// Before a save()
	virtual void postLoadCallback() {}					// After a load()
	virtual void setPostLoadCallbackFunction(void(*f)(ciConfig*))			{ postLoadCallback_fn = f; }
	virtual void setPostResetCallbackFunction(void(*f)(ciConfig*))			{ postResetCallback_fn = f; }
	virtual void setPostSetCallbackFunction(void(*f)(ciConfig*,int,int))	{ postSetCallback_fn = f; }
	void(*postLoadCallback_fn)(ciConfig*);
	void(*postResetCallback_fn)(ciConfig*);
	void(*postSetCallback_fn)(ciConfig*,int,int);

	char errmsg[256];
	
	// KEYBOARD
	void resetKeys(int id);
	void swapKeys(int id1, int id2);
	void setKey(int id, std::string key0, std::string key1="", std::string key2="", std::string key3="");
	void setKeyUpDown(int id, std::string up, std::string down);
	void setKeyVectorUpDown(int id, std::string up0, std::string down0, std::string up1, std::string down1);
	void setKeyVectorUpDown(int id, std::string up0, std::string down0, std::string up1, std::string down1, std::string up2, std::string down2);
	void setKeyVectorUpDown(int id, std::string up0, std::string down0, std::string up1, std::string down1, std::string up2, std::string down2, std::string up3, std::string down3);
	void setKeyRGBUpDown(int id, std::string up0, std::string down0, std::string up1, std::string down1, std::string up2, std::string down2);
	void setKeySwitch(int id, std::string key);
	
	// MIDI
	void resetMidi(int id);
	void swapMidi(int id1, int id2);
	void setMidiChannel(int id, int ch);
	void setMidiChannelTrigger(int id, int ch);
	void setMidi(int id, short note, int ch=1);
	void setMidi2(int id, short note0, short note1, int ch=1);
	void setMidi3(int id, short note0, short note1, short note2, int ch=1);
	void setMidi4(int id, short note0, short note1, short note2, short note3, int ch=1);
	void setMidiUpDown(int id, short up, short down, int ch=1);
	void setMidiUpDown2(int id, short up0, short down0, short up1, short down1, int ch=1);
	void setMidiUpDown3(int id, short up0, short down0, short up1, short down1, short up2, short down2, int ch=1);
	void setMidiUpDown4(int id, short up0, short down0, short up1, short down1, short up2, short down2, short up3, short down3, int ch=1);
	void setMidiRGBUpDown(int id, short up0, short down0, short up1, short down1, short up2, short down2, int ch=1);
	void setMidiSwitch(int id, short note, int ch=1);
#ifdef CFG_USE_MIDI
	void connectMidi();
	bool isMidiInConnected();
	//bool isMidiOutConnected() { return midiOut->isConnected(); }
#endif

#ifdef CFG_USE_OSC
	void setOscSend(bool b, int port=CICFG_OSC_PORT);		// Start to send OSC
	void setOscReceive(bool b, int port=CICFG_OSC_PORT);	// Start ro receive OSC
	void pushOsc();											// Seld all params by OSC
#endif
	
	// New parameters
	void add(short id, const std::string name, float val, float vmin=0.0f, float vmax=1.0f);	// Generic = float
	void addFloat(short id, const std::string name, float val, float vmin=0.0f, float vmax=1.0f);
	void addDouble(short id, const std::string name, double val, double vmin=0.0, double vmax=1.0);
	void addInt(short id, const std::string name, int val, int vmin=0, int vmax=1);
	void addLong(short id, const std::string name, long val, long vmin=0, long vmax=1);
	void addBool(short id, const std::string name, bool val);
	void addByte(short id, const std::string name, unsigned char val, unsigned char vmin=0, unsigned char vmax=255);
	void addString(short id, const std::string name, const std::string val);
	void addColor(short id, const std::string name, Color8u p);
	void addColor(short id, const std::string name, float r, float g, float b);
	void addVector2(short id, const std::string name, Vec2f p=Vec2f::zero(), float vmin=0.0f, float vmax=1.0f);
	void addVector2(short id, const std::string name, float x, float y, float vmin=0.0f, float vmax=1.0f);
	void addVector3(short id, const std::string name, Vec3f p=Vec3f::zero(), float vmin=0.0f, float vmax=1.0f);
	void addVector3(short id, const std::string name, float x, float y, float z, float vmin=0.0f, float vmax=1.0f);
	void addVector4(short id, const std::string name, Vec4f p=Vec4f::zero(), float vmin=0.0f, float vmax=1.0f);
	void addVector4(short id, const std::string name, float x, float y, float z, float w, float vmin=0.0f, float vmax=1.0f);
	void addQuater(short id, const std::string name, Vec4f p=Quatf::identity().getVec4()) { this->addVector4(id,name,p,-1.0f,M_TWO_PI); params[id]->quater=true; };

	// Defaults
	void init(int id);
	void setDefault(int def, int id, float v);
	void setDefault(int def, int id, Vec2f v);
	void setDefault(int def, int id, Vec3f v);
	void setDefault(int def, int id, Vec4f v);
	void setDefault(int def, int id, int i, float v);
	void resetDefault(int def);
	void resetDefault(int id, int def);
	void reset()			{ this->resetDefault(0); }
	void reset(int id)		{ this->resetDefault(id,0); }
	virtual void setCurrentDefault(int def)	{ if (def < MAX_DEFAULTS ) mCurrentDefault = def; };

	// Setters
	void updateLastValue(int id, int i)				{ params[id]->vec[i].updateLastValue(); }
	void updateLastValueString(int id)				{ params[id]->updateLastValueString(); }
	void setDummy(int id, bool _e=false)			{ params[id]->dummy = true; params[id]->editable = _e; }
	void setReadOnly(int id, bool _ro=true)			{ params[id]->editable = ! _ro; }
	virtual void setLimits(int id, float vmin, float vmax)	{ this->setLimits(id,0,vmin,vmax); }
	void setLimitsR(int id, float vmin, float vmax) { this->setLimits(id,0,vmin,vmax); }
	void setLimitsG(int id, float vmin, float vmax) { this->setLimits(id,1,vmin,vmax); }
	void setLimitsB(int id, float vmin, float vmax) { this->setLimits(id,2,vmin,vmax); }
	void setLimitsX(int id, float vmin, float vmax) { this->setLimits(id,0,vmin,vmax); }
	void setLimitsY(int id, float vmin, float vmax) { this->setLimits(id,1,vmin,vmax); }
	void setLimitsZ(int id, float vmin, float vmax) { this->setLimits(id,2,vmin,vmax); }
	void setLimitsW(int id, float vmin, float vmax) { this->setLimits(id,3,vmin,vmax); }
	void setLimits(int id, Vec2f vmin, Vec2f vmax);
	void setLimits(int id, Vec3f vmin, Vec3f vmax);
	void setLimits(int id, Vec4f vmin, Vec4f vmax);
	void setLimitsDegrees(int id);
	void setLimitsRadians(int id);
	void setLimitsScreen(int id);
	// Setters
	void preserveProg(int id, bool p=true);
	void setProg(int id, float p)	{ this->setProg(id,0,p); }
	void setProgR(int id, float p)	{ this->setProg(id,0,p); }
	void setProgG(int id, float p)	{ this->setProg(id,1,p); }
	void setProgB(int id, float p)	{ this->setProg(id,2,p); }
	void setProgX(int id, float p)	{ this->setProg(id,0,p); }
	void setProgY(int id, float p)	{ this->setProg(id,1,p); }
	void setProgZ(int id, float p)	{ this->setProg(id,2,p); }
	void setProgW(int id, float p)	{ this->setProg(id,3,p); }
	void setProg(int id, const Vec2f & v)	{ this->setProg(id, v.x, v.y); }
	void setProg(int id, const Vec3f & v)	{ this->setProg(id, v.x, v.y, v.z); }
	void setProg(int id, const Vec4f & v)	{ this->setProg(id, v.x, v.y, v.z, v.w); }
	void setProg(int id, float val0, float val1);							// CFG_TYPE_VECTOR2
	void setProg(int id, float val0, float val1, float val2);				// CFG_TYPE_VECTOR3 / CFG_TYPE_COLOR
	void setProg(int id, float val0, float val1, float val2, float val3);	// CFG_TYPE_VECTOR4
	// Set to min/max
	void setToMin(int id)			{ this->set(id, this->getMin(id)); }
	void setToMax(int id)			{ this->set(id, this->getMax(id)); }
	// Set values (generic = float)
	void set(int id, float val)		{ this->set(id, 0, val); }		// CFG_TYPE_FLOAT (generic)
	void setR(int id, float val)	{ this->set(id, 0, val); }
	void setG(int id, float val)	{ this->set(id, 1, val); }
	void setB(int id, float val)	{ this->set(id, 2, val); }
	void setX(int id, float val)	{ this->set(id, 0, val); }
	void setY(int id, float val)	{ this->set(id, 1, val); }
	void setZ(int id, float val)	{ this->set(id, 2, val); }
	void setW(int id, float val)	{ this->set(id, 3, val); }
	// Set values (specific)
	void set(int id, double val)		{ this->set(id, (float)val); }		// CFG_TYPE_DOUBLE
	void set(int id, short val)			{ this->set(id, (float)val); }		// no type, just cast
	void set(int id, int val)			{ this->set(id, (float)val); }		// CFG_TYPE_INTEGER
	void set(int id, long val)			{ this->set(id, (float)val); }		// CFG_TYPE_LONG
	void set(int id, bool val)			{ this->set(id, (float)( val ? 1.0f : 0.0f )); }	// CFG_TYPE_BOOLEAN
	void set(int id, unsigned char val)	{ this->set(id, (float)val); }		// CFG_TYPE_BYTE
	void set(int id, Color8u c)			{ this->set(id, c.r, c.g, c.b); }	// CFG_TYPE_COLOR
	void set(int id, Color c)			{ this->set(id, c.r*255, c.g*255, c.b*255); }	// CFG_TYPE_COLOR
	void set(int id, Vec2f p)			{ this->set(id, p.x, p.y); }			// CFG_TYPE_VECTOR2
	void set(int id, Vec3f p)			{ this->set(id, p.x, p.y, p.z); }		// CFG_TYPE_VECTOR3
	void set(int id, Vec4f p)			{ this->set(id, p.x, p.y, p.z, p.w); }	// CFG_TYPE_VECTOR4
	void set(int id, const char *val, bool pp=false);						// CFG_TYPE_STRING, or convert to float
	void set(int id, const std::string &val, bool pp=false) { this->set(id,val.c_str(),pp); }		// CFG_TYPE_STRING, or convert to float
	void set(int id, float val0, float val1);							// CFG_TYPE_VECTOR2
	void set(int id, float val0, float val1, float val2);				// CFG_TYPE_VECTOR3 / CFG_TYPE_COLOR
	void set(int id, float val0, float val1, float val2, float val3);	// CFG_TYPE_VECTOR4
	// Value labels
	void clearValueLabels(short id);
	void setValueLabels(short id, const char *val0, ...);
	void setValueLabels(short id, const char labels[][64]);
	void setValueLabels(short id, const std::map<int,std::string> & labels);
	void setValueLabels(short id, const std::vector<std::string> & labels);
	void setValueLabel(short id, int key, std::string label, bool indexToo=false);
	const std::map<int,std::string> & getValueLabels(short id)	{ return params[id]->valueLabels; };
	std::string getValueLabel(short id)							{ return this->getValueLabel( id, this->getInt(id) ); };
	std::string getValueLabel(short id, int key);
	virtual void guiUpdateValueLabels(int id) {}	// auto update GUI

	// Generic flags
	void setFlag(int id, char f)	{ params[id]->flag = f; }
	bool testFlag(int id, char f)	{ return (params[id]->flag == f); }
	char getFlag(int id)			{ return params[id]->flag; }

	// Changed Flags
	void setChangeGroup(char f, int id)		{ if (params[id]) params[id]->changeGroups |= f; }
	bool testChangedGroup(char f)			{ return ( changedGroups & f ); }

	// Getters
	short getParamCount()					{ return paramCount; }
	ciConfigParam* getParamPtr(short i)		{ return params[i]; }
	float getLastValue(int id, int i)		{ return params[id]->vec[i].getLastValue(); }
	std::string getLastValueString(int id)	{ return params[id]->getLastValueString(); }
	const std::string& getName(int id)		{ return params[id]->name; }
	short getType(int id)		{ return params[id]->type; }
	bool isFloat(int id)		{ return params[id]->isFloat(); }
	bool isBool(short id)		{ return params[id]->isBool(); }			// bool type?
	bool isByte(short id)		{ return params[id]->isByte(); }			// byte type?
	bool isInt(short id)		{ return params[id]->isInt(); }				// integer type?
	bool isColor(short id)		{ return params[id]->isColor(); }			// color type?
	bool isVector(short id)		{ return params[id]->isVector(); }			// vector type?
	bool isVector2(short id)	{ return params[id]->isVector2(); }			// vector type?
	bool isVector3(short id)	{ return params[id]->isVector3(); }			// vector type?
	bool isVector4(short id)	{ return params[id]->isVector4(); }			// vector type?
	bool isString(short id)		{ return params[id]->isString(); }			// vector type?
	bool isFresh()				{ bool f=freshness; freshness = false; return f; }			// global freshness - automatic UNFRESH
	bool isFresh(short id, bool _unfresh=true)	{ return params[id]->isFresh(_unfresh); }	// param freshness
	void unfresh()				{ freshness = false; }						// global freshness - automatic UNFRESH
	void unfreshAll()			{ this->unfresh(); for (int id=0;id<params.size();id++) if (params[id]) params[id]->unfresh(); }
	void setLoaded(short id, bool l)	{ params[id]->loaded = l; }
	bool isLoaded(short id)				{ return params[id]->loaded; }
	// pointers
	float* getPointer(int id, int i)		{ return (params[id]->getPointer(i)); }
	float* getPointer(int id)				{ return this->getPointer(id,0); }
	float* getPointerR(int id)				{ return this->getPointer(id,0); }
	float* getPointerG(int id)				{ return this->getPointer(id,1); }
	float* getPointerB(int id)				{ return this->getPointer(id,2); }
	float* getPointerX(int id)				{ return this->getPointer(id,0); }
	float* getPointerY(int id)				{ return this->getPointer(id,1); }
	float* getPointerZ(int id)				{ return this->getPointer(id,2); }
	float* getPointerW(int id)				{ return this->getPointer(id,3); }
	bool* getPointerBool(int id)			{ return (params[id]->getPointerBool()); }
	unsigned char* getPointerByte(int id)	{ return (params[id]->getPointerByte()); }
	int* getPointerInt(int id)				{ return (params[id]->getPointerInt()); }
	Color* getPointerColor(int id)			{ return (params[id]->getPointerColor()); }
	ColorA* getPointerColorA(int id)		{ return (params[id]->getPointerColorA()); }
	Vec2f* getPointerVector2(int id)		{ return (params[id]->getPointerVector2()); }
	Vec4f* getPointerVector(int id)			{ return (params[id]->getPointerVector()); }
	std::string* getPointerString(int id)	{ return (params[id]->getPointerString()); }
	// Get limits
	float getDefault(int id)		{ return this->getDefault(id, 0); }
	float getMin(int id)			{ return this->getMin(id, 0); }
	float getMinR(int id)			{ return this->getMin(id, 0); }
	float getMinG(int id)			{ return this->getMin(id, 1); }
	float getMinB(int id)			{ return this->getMin(id, 2); }
	float getMinX(int id)			{ return this->getMin(id, 0); }
	float getMinY(int id)			{ return this->getMin(id, 1); }
	float getMinZ(int id)			{ return this->getMin(id, 2); }
	float getMinW(int id)			{ return this->getMin(id, 3); }
	float getMax(int id)			{ return this->getMax(id, 0); }
	float getMaxR(int id)			{ return this->getMax(id, 0); }
	float getMaxG(int id)			{ return this->getMax(id, 1); }
	float getMaxB(int id)			{ return this->getMax(id, 2); }
	float getMaxX(int id)			{ return this->getMax(id, 0); }
	float getMaxY(int id)			{ return this->getMax(id, 1); }
	float getMaxZ(int id)			{ return this->getMax(id, 2); }
	float getMaxW(int id)			{ return this->getMax(id, 3); }
	float getProg(int id)			{ return this->getProg(id, 0); }
	float getProgR(int id)			{ return this->getProg(id, 0); }
	float getProgG(int id)			{ return this->getProg(id, 1); }
	float getProgB(int id)			{ return this->getProg(id, 2); }
	float getProgX(int id)			{ return this->getProg(id, 0); }
	float getProgY(int id)			{ return this->getProg(id, 1); }
	float getProgZ(int id)			{ return this->getProg(id, 2); }
	float getProgW(int id)			{ return this->getProg(id, 4); }
	Vec2f getProgVector2(int id)	{ return Vec2f(this->getProgX(id), this->getProgY(id)); }
	Vec3f getProgVector3(int id)	{ return Vec3f(this->getProgX(id), this->getProgY(id), this->getProgZ(id)); }
	Vec4f getProgVector4(int id)	{ return Vec4f(this->getProgX(id), this->getProgY(id), this->getProgZ(id), this->getProgW(id)); }
	// get values (generic = float)
	float get(int id)		{ return this->get(id, 0); }
	float getR(int id)		{ return this->get(id, 0); }
	float getG(int id)		{ return this->get(id, 1); }
	float getB(int id)		{ return this->get(id, 2); }
	float getA(int id)		{ return this->get(id, 3); }
	float getX(int id)		{ return this->get(id, 0); }
	float getY(int id)		{ return this->get(id, 1); }
	float getZ(int id)		{ return this->get(id, 2); }
	float getW(int id)		{ return this->get(id, 3); }
	// get values (specific
	float	getFloat(int id)		{ return (float) (params[id]->value.get()); }
	double	getDouble(int id)		{ return (double) (params[id]->value.get()); }
	short	getShort(int id)		{ return (short) (params[id]->value.get()); }
	int		getInt(int id)			{ return (int) (params[id]->value.get()); }
	long	getLong(int id)			{ return (long) (params[id]->value.get()); }
	bool	getBool(int id)			{ return ( params[id]->value.get() == 0.0 ? false : true ); }
	unsigned char getByte(int id)	{ return (unsigned char) (params[id]->value.get()); }
	Color8u	getColor(int id)		{ return Color8u(this->get(id,0), this->get(id,1),this->get(id,2)); }
	ColorA8u getColorA(int id)		{ return ColorA8u(this->get(id,0), this->get(id,1),this->get(id,2),this->get(id,3)); }
	Vec2f	getVector2(int id)		{ return Vec2f(this->get(id,0), this->get(id,1)); }
	Vec2f	getVector2(int id0, int id1) { return Vec2f(this->get(id0,0), this->get(id1,0)); }
	Vec3f	getVector3(int id)		{ return Vec3f(this->get(id,0), this->get(id,1),this->get(id,2)); }
	Vec3f	getVector3(int id0, int id1, int id2) { return Vec3f(this->get(id0,0), this->get(id1,0),this->get(id2,0)); }
	Vec4f	getVector4(int id)		{ return Vec4f(this->get(id,0), this->get(id,1),this->get(id,2),this->get(id,3)); }
	Vec4f	getVector4(int id0, int id1, int id2, int id3) { return Vec4f(this->get(id0,0), this->get(id1,0),this->get(id2,0),this->get(id3,0)); }
	Quatf	getQuater(int id)		{ return Quatf( this->get(id,3),this->get(id,0),this->get(id,1),this->get(id,2) ); }
	bool	getTrigger(int id)		{ return params[id]->value.getTrigger(); };
	void	getString(int id, char *str, bool raw=false) { strcpy(str, this->getString(id)); }
	const char* getString(int id, bool raw=false);
	// Normalized getters
	float getDegrees(int id)		{ return this->getDegrees(id,0); }
	float getDegreesX(int id)		{ return this->getDegrees(id,0); }
	float getDegreesY(int id)		{ return this->getDegrees(id,1); }
	float getDegreesZ(int id)		{ return this->getDegrees(id,2); }
	float getDegreesW(int id)		{ return this->getDegrees(id,3); }
	Vec2f getDegreesVec2(int id)	{ return Vec2f(this->getDegrees(id,0),this->getDegrees(id,1)); }
	Vec3f getDegreesVec3(int id)	{ return Vec3f(this->getDegrees(id,0),this->getDegrees(id,1),this->getDegrees(id,2)); }
	Vec4f getDegreesVec4(int id)	{ return Vec4f(this->getDegrees(id,0),this->getDegrees(id,1),this->getDegrees(id,2),this->getDegrees(id,3)); }
	float getRadians(int id)		{ return this->getRadians(id,0); }
	float getRadiansX(int id)		{ return this->getRadians(id,0); }
	float getRadiansY(int id)		{ return this->getRadians(id,1); }
	float getRadiansZ(int id)		{ return this->getRadians(id,2); }
	float getRadiansW(int id)		{ return this->getRadians(id,4); }
	Vec2f getRadiansVec2(int id)	{ return Vec2f(this->getRadians(id,0),this->getRadians(id,1)); }
	Vec3f getRadiansVec3(int id)	{ return Vec3f(this->getRadians(id,0),this->getRadians(id,1),this->getRadians(id,2)); }
	Vec4f getRadiansVec4(int id)	{ return Vec4f(this->getRadians(id,0),this->getRadians(id,1),this->getRadians(id,2),this->getRadians(id,3)); }
	
	//
	// Operations
	void invert(int id);
	void invertR(int id)	{ this->invert(id, 0); }
	void invertG(int id)	{ this->invert(id, 1); }
	void invertB(int id)	{ this->invert(id, 2); }
	void invertX(int id)	{ this->invert(id, 0); }
	void invertY(int id)	{ this->invert(id, 1); }
	void invertZ(int id)	{ this->invert(id, 2); }
	void invertW(int id)	{ this->invert(id, 3); }
	void sub(int id, float val, bool clamp=true);
	void subR(int id, float val, bool clamp=true) { this->sub(id, 0, val, clamp); }
	void subG(int id, float val, bool clamp=true) { this->sub(id, 1, val, clamp); }
	void subB(int id, float val, bool clamp=true) { this->sub(id, 2, val, clamp); }
	void subX(int id, float val, bool clamp=true) { this->sub(id, 0, val, clamp); }
	void subY(int id, float val, bool clamp=true) { this->sub(id, 1, val, clamp); }
	void subZ(int id, float val, bool clamp=true) { this->sub(id, 2, val, clamp); }
	void subW(int id, float val, bool clamp=true) { this->sub(id, 3, val, clamp); }
	void add(int id, float val, bool clamp=true);
	void addR(int id, float val, bool clamp=true) { this->add(id, 0, val, clamp); }
	void addG(int id, float val, bool clamp=true) { this->add(id, 1, val, clamp); }
	void addB(int id, float val, bool clamp=true) { this->add(id, 2, val, clamp); }
	void addX(int id, float val, bool clamp=true) { this->add(id, 0, val, clamp); }
	void addY(int id, float val, bool clamp=true) { this->add(id, 1, val, clamp); }
	void addZ(int id, float val, bool clamp=true) { this->add(id, 2, val, clamp); }
	void addW(int id, float val, bool clamp=true) { this->add(id, 3, val, clamp); }
	void dec(int id, bool clamp=true)	{ this->dec(id, 0, clamp); }
	void decR(int id, bool clamp=true)	{ this->dec(id, 0, clamp); }
	void decG(int id, bool clamp=true)	{ this->dec(id, 1, clamp); }
	void decB(int id, bool clamp=true)	{ this->dec(id, 2, clamp); }
	void decX(int id, bool clamp=true)	{ this->dec(id, 0, clamp); }
	void decY(int id, bool clamp=true)	{ this->dec(id, 1, clamp); }
	void decZ(int id, bool clamp=true)	{ this->dec(id, 2, clamp); }
	void decW(int id, bool clamp=true)	{ this->dec(id, 3, clamp); }
	void inc(int id, bool clamp=true)	{ this->inc(id, 0, clamp); }
	void incR(int id, bool clamp=true)	{ this->inc(id, 0, clamp); }
	void incG(int id, bool clamp=true)	{ this->inc(id, 1, clamp); }
	void incB(int id, bool clamp=true)	{ this->inc(id, 2, clamp); }
	void incX(int id, bool clamp=true)	{ this->inc(id, 0, clamp); }
	void incY(int id, bool clamp=true)	{ this->inc(id, 1, clamp); }
	void incZ(int id, bool clamp=true)	{ this->inc(id, 2, clamp); }
	void incW(int id, bool clamp=true)	{ this->inc(id, 3, clamp); }
	void incLoop(int id);
	
	//
	// READ / SAVE to file
	const std::string getAppName()			{ return mAppName; }
	const std::string getAppPath()			{ return mFolderApp; }
	const std::string getCurrentFileName()	{ return mCurrentFileName; }
	const std::string getDisplayFileName()	{ return mDisplayFileName; }
	const std::string getFileExtension()	{ return fileExt; }
	const std::string getSaveTime()			{ return saveTime; }
	// Shortcuts
	void setFileExtension(const std::string e);
	bool import();
	bool exportas();
	void setFile(const std::string & filename);
	virtual int useFile(const std::string & filename, const std::string & path="");
	virtual int load()			{ return this->load(NULL); }
	virtual int save()			{ return this->save(NULL); }
	int load(char preset);
	int save(char preset);
	bool presetExist(char preset);
	std::string presetTimestamp(char preset);
	bool getPresetXml(char preset, XmlTree & doc);
	bool getPresetXml(std::string f, XmlTree & doc);

	std::string		mAppName;				// The app name
	
protected:
	// Parameter index
	ciConfig		*parent;				// reference to parent config
	std::vector<ciConfigParam*>	params;
	std::string		mAppVersion;			// The app version (plist)
	std::string		mFolderApp;				// The app folder
	std::string		mFolderBundle;			// File bundle
	std::string		mFolderAppSupport;		// Library/ApplicationSupport/{TARGET_NAME}>/
	std::string		mFolderDefault;			// Main default save folder
	std::string		mFolderSave;			// For save/load GUI controls
	std::string		fileExt;
	std::string		mCurrentFileName;		// Full path of the file being used
	std::string		mDisplayFileName;		// Short filename for display
	std::string		saveTime;
	int				mCurrentDefault;
	short			paramCount;
	bool			freshness;				// global freshness (anything fresh?)
	char			changedGroups;			// changed group flags
	bool			inPostSetCallback;		// flag not to fall into a callback loop
	bool			bStarted;
	std::vector<std::string>	mFolderList;		// The save path folders, in order of preference
	std::vector<std::string>	mFileList;			// Actual files to be read, in order of preference
	
	// MIDI
	int			midiChannel;
	int			midiPort;
#ifdef CFG_USE_MIDI
	midi::Hub	*midiHub;
#endif
	// used by OSC as well
	void parseMidiMessage(int channel, int note, int val);
	
	// OSC
#ifdef CFG_USE_OSC
	bool			bOscSend;
	bool			bOscReceive;
	ofxOscSender	oscSender;
	ofxOscReceiver	oscReceiver;
	void switchOsc(int port);			// Switch OSC SEND<>RECEIVE
	void sendOsc(short id, short i);	// Sends one message by OSC
	void oscCallback();
#ifdef CFG_CATCH_LOOP_EVENTS
	void oscCallback(ofEventArgs &e);
#endif
#endif
	
	//
	// Methods
	int saveFile(const std::string & filename, char preset=0);
	int readFile(const std::string & filename, char preset=0);
	int readFile_old(const std::string & filename, char preset=0);
	void setDisplayFilename(const std::string & f, char preset=0);
	std::string makeFileName(const std::string & path, char preset=0);
	std::string makeXmlNodeName(char preset=0);
	void pushParam(int id, ciConfigParam* p);
	bool isNumber(int id);
	bool isVector(int id);
	int getVectorCount(int id)			{ return params[id]->vecCount; };

	// Channel access
	// i=channel(RGB)/axis(XYZ)
	void updateLimitsByValueLabels(short id);
	void setLimits(int id, int i, float vmin, float vmax)	{ params[id]->vec[i].setLimits(vmin,vmax); }
	void setProg(int id, int i, float val);
	void setProg(int id, int i, const char *val)			{ this->setProg(id, i, (float)atof(val)); }
	void set(int id, int i, float val);
	void set(int id, int i, const char *val)				{ this->set(id, i, (float)atof(val)); }
	void post_set(int id);
	void post_set(int id, int i, bool doCB=true);
	float get(int id, int i);
	float getDefault(int id, int i)		{ return params[id]->vec[i].getInitialValue(); };
	float getMin(int id, int i)			{ return params[id]->vec[i].getMin(); };
	float getMax(int id, int i)			{ return params[id]->vec[i].getMax(); };
	float getProg(int id, int i)		{ return params[id]->vec[i].getProg(); };
	float getDegrees(int id, int i);
	float getRadians(int id, int i);
	// Operators
	void invert(int id, short i);
	void sub(int id, int i, float val, bool clamp);
	void add(int id, int i, float val, bool clamp);
	void dec(int id, int i, bool clamp)		{ this->sub(id, i, 1.0f, clamp); }
	void inc(int id, int i, bool clamp)		{ this->add(id, i, 1.0f, clamp); }

	
};


} // namespace cinder
