//
//  ciConfig.mm
//
//  Created by Roger Sodre on 08/04/2010
//  Copyright 2011 Studio Avante. All rights reserved.
//


////////////////////////////////////////////////////////////////////////
//
// BASE CONFIG CLASS
//
//  Config class v1.0
//  Manage configuration parameters + save as text file
//  by Roger Sodre
//
//  Install:
//
// - Add addons/ofxMidi
// - Add CoreMIDI.framework
//
//  Usage:
//
// - Inherit Config class
// - Create param enum
// - Initialize parameters
//
//  TODO:
//
// - send string types on sendOsc()
//

#include <boost/algorithm/string.hpp>
#include <exception>
#include <errno.h>
#include "ciConfig.h"
#include "cinder/app/AppBasic.h"
#include "cinder/Utilities.h"

// comment to disable versosity
#ifndef RELEASE
//#define VERBOSE
//#define OSC_VERBOSE
#endif

#define IS_IN_DEFAULTS(f)		( mFolderDefault.length() > 0 && f.find(mFolderDefault) != std::string::npos )

using namespace ci;
using namespace ci::app;
using namespace std;

namespace cinder {
	
	
	ciConfig::ciConfig(ciConfig *_parent)
	{
		// init
		parent = _parent;
		saveTime = "using defaults";
		inPostSetCallback = false;
		postSetCallback_fn = NULL;
		postResetCallback_fn = NULL;
		postLoadCallback_fn = NULL;
		bStarted = false;
		mCurrentDefault = 0;
		mAppName = AppBasic::get()->getAppName();
		mAppVersion = AppBasic::get()->getAppVersion();
		mFolderApp = getPathDirectory( app::getAppPath().string() );
		mFolderBundle = getPathDirectory( App::get()->getResourcePath().string() ) + "Resources/";
		mFolderSave = mFolderApp;
		mFolderAppSupport = AppBasic::get()->getApplicationSupportFolder().string();

		// Save path preference....
#ifdef CINDER_COCOA_TOUCH
		mFolderApp = mFolderBundle = mFolderAppSupport = mFolderSave = getHomeDirectory();
		mFolderList.push_back( mFolderApp );
#else
		mFolderList.push_back( mFolderBundle );
		mFolderList.push_back( mFolderAppSupport );
		mFolderList.push_back( mFolderApp );
#endif
		
#ifdef STANDALONE
		mFolderDefault = mFolderBundle;
#else
		mFolderDefault = mFolderAppSupport;
#endif
		
		// temporary extension, this will be replaced if added extionsions
		fileExt = "cfg";
		this->setFile( mAppName + "." + fileExt );
		
		// Register key events
#ifdef CFG_CATCH_LOOP_EVENTS
		// not for presets
		if (parent == NULL)
		{
			app::getWindow()->connectKeyDown( & ciConfig::onKeyDown, this );
			app::getWindow()->connectFileDrop( & ciConfig::onFileDrop, this );
		}
#endif
		
		// init MIDI
		midiPort = 0;
		midiChannel = 1;
#ifdef CFG_USE_MIDI
		midiHub = NULL;
#endif
		// Do not enable devices for presets (if has parent, it's a preset)
		if (parent == NULL)
		{
#ifdef CFG_USE_MIDI
			midiHub = new midi::Hub();
#endif
			
			// init OSC
#ifdef CFG_USE_OSC
			this->setOscSend(false);
			this->setOscReceive(false);
			this->setOscReceive(true);	// CINDER always receive OSC because have no MIDI
#ifdef CFG_CATCH_LOOP_EVENTS
			ofAddListener(ofEvents.update, this, &ciConfig::oscCallback);
#endif
#endif
		}
		
		// init XML
		//xmlInOut = new XMLInOut(applet);
	}
	ciConfig::~ciConfig()
	{
		// free params
		while ( params.size() > 0 )
		{
			ciConfigParam* p = params.back();
			if (p != NULL)
				delete p;
			params.pop_back();
		}
		
#ifdef CFG_USE_MIDI
		if (midiHub)
			delete midiHub;
#endif
		
#ifdef CFG_USE_OSC
#ifdef CFG_CATCH_LOOP_EVENTS
		ofRemoveListener(ofEvents.update, this, &ciConfig::oscCallback);
#endif
#endif
	}
	
	//
	// Connect MIDI
#ifdef CFG_USE_MIDI
	void ciConfig::connectMidi()
	{
		midiHub->connectAll();
	}
	bool ciConfig::isMidiInConnected()
	{
		return midiHub->isConnected();
	}
#endif
	
	//
	// Update loop
	//
	void ciConfig::update()
	{
		bStarted = true;

		// Watch if Params updated var pointers
		//for (int id = 0 ; id < params.size() ; id++)
		for ( auto iter = params.begin(); iter != params.end(); ++iter )
		{
			//ciConfigParam *p = params[id];
			if ( ciConfigParam *p = *iter )
			{
				int id = (int) (iter - params.begin());
				if (p->isColor())
				{
					for (int i = 0 ; i < 3 ; i++)
					{
						if ( (p->watchColor[i] * 255.0f) != this->get(id, i))
						{
							//printf("WATCH COLOR %d %.3f  old %.3f\n",i,(float)p->watchColor[i],this->get(id, i));
							this->set( id, i, p->watchColor[i] * 255.0f );
						}
					}
					for (int i = 0 ; i < 4 ; i++)
					{
						if ( (p->watchColorA[i] * 255.0f) != this->get(id, i))
						{
							//printf("WATCH COLOR %d %.3f  old %.3f\n",i,(float)p->watchColorA[i],this->get(id, i));
							this->set( id, i, p->watchColorA[i] * 255.0f );
						}
					}
				}
				else if (p->isVector2() && (this->testFlag(id,CFG_FLAG_XY) || this->testFlag(id,CFG_FLAG_XY_VECTOR)))
				{
					for (int i = 0 ; i < 2 ; i++)
						if (p->watchVector2[i] != this->get(id, i))
							this->set( id, i, p->watchVector2[i] );
				}
				else if (p->isVector())
				{
					for (int i = 0 ; i < 4 ; i++)
						if (p->watchVector[i] != this->get(id, i))
							this->set( id, i, p->watchVector[i] );
				}
				else if (p->isBool())
				{
					if (p->watchBool != (bool) this->get(id))
						this->set( id, p->watchBool );
				}
				else if (p->isByte())
				{
					if (p->watchByte != (char) this->get(id))
						this->set( id, p->watchByte );
				}
				else if (p->isInt())
				{
					if (p->watchInt != (int) this->get(id))
						this->set( id, p->watchInt );
				}
				else if (p->isString())
				{
				}
				else
				{
					if (p->watchFloat[0] != this->get(id))
						this->set( id, p->watchFloat[0] );
				}
			}
		}

		//
		// Set changed flags
		changedGroups = 0x00;
		for ( auto iter = params.begin(); iter != params.end(); ++iter )
		{
			if ( ciConfigParam *p = *iter )
			{
				if ( p->changed && p->changeGroups )
					changedGroups |= p->changeGroups;
				p->changed = false;
			}
		}


		//
		// MIDI
#ifdef CFG_USE_MIDI
		midi::Message message;
		while ( midiHub->getNextMessage(&message) )
		{
			int channel = message.channel;
			int note = message.byteOne;
			int val = message.byteTwo;
			this->parseMidiMessage(channel, note, val);
		}
#endif
		
#ifdef CFG_USE_OSC
#ifndef CFG_CATCH_LOOP_EVENTS
		this->oscCallback();
#endif
#endif
	}
	//
	// Keyboard / Presets
	void ciConfig::onKeyDown( app::KeyEvent & event )
	{
		char c = event.getChar();
		//printf("EVENT>> ciConfig::keyDown [%d] [%c]\n",c,c);
		switch( c ) {
#ifndef CICONFIG_DISABLE_RESET
			case 'r':
			case 'R':
				if (event.isMetaDown()) // COMMAND
				{
					this->reset();
					return;
				}
#endif
#ifndef NO_SAVE
			case 'l':
			case 'L':
				if (event.isMetaDown()) // COMMAND
				{
					this->load();
					return;
				}
			case 's':
			case 'S':
				if (event.isMetaDown()) // COMMAND
				{
					//if (event.isShiftDown())
					//	this->exportas();
					//else
					this->save();
					return;
				}
			case 'i':
			case 'I':
				if (event.isMetaDown()) // COMMAND
				{
					this->import();
					return;
				}
			case 'e':
			case 'E':
				if (event.isMetaDown()) // COMMAND
				{
					this->exportas();
					return;
				}
#endif	// NO_SAVE
			case '1' :
			case '2' :
			case '3' :
			case '4' :
			case '5' :
			case '6' :
			case '7' :
			case '8' :
			case '9' :
			case '0' :
				{
					// Save : CONTROL + COMMAND + NUMBER
					if (event.isControlDown() && event.isMetaDown())
					{
						this->save(c);
						//event.setHandled();
						return;
					}
					// Load : COMMAND + NUMBER
					else if (event.isMetaDown())
					{
						this->load(c);
						//event.setHandled();
						return;
					}
					break;
				}
		}
	}
	void ciConfig::onFileDrop(FileDropEvent & event) {
		if ( event.getNumFiles() > 0 )
			if ( this->readFile( event.getFile( 0 ).string() ) )
				event.setHandled();
	}

	
	/////////////////////////////////////////////////////////////////////////
	//
	// KEYBOARD
	//
	//
	// CINDER PARAM keys..
	//	SPACE (or ‘ ‘)
	//	BACKSPACE (or BS)
	//	TAB
	//	CLEAR (or CLR)
	//	RETURN (or RET)
	//	PAUSE
	//	ESCAPE (or ESC)
	//	DELETE (or DEL), INSERT (or INS)
	//	UP, DOWN, RIGHT, LEFT
	//	HOME, END
	//	PGUP, PGDOWN
	//	F1, F2,..., F15
	//
	// Key modifiers can be:
	//	SHIFT
	//	ALT
	//	CTRL
	//
	// Examples:
	//	A, b, #, 5, =, &, F10, BACKSPACE, ALT+A, CTRL+SPACE, SHIFT+F5, ALT+SHIFT+F12
	//
	void ciConfig::resetKeys(int id)
	{
		for (int n = 0 ; n < 4 ; n++)
		{
			strcpy( params[id]->vec[n].keySwitch, "" );
			strcpy( params[id]->vec[n].keyInc, "" );
			strcpy( params[id]->vec[n].keyDec, "" );
		}
	}
	void ciConfig::swapKeys(int id1, int id2)
	{
		char keys[4][3][20];
		for (int n = 0 ; n < 4 ; n++)
		{
			strcpy( keys[n][0], params[id1]->vec[n].keySwitch );
			strcpy( keys[n][1], params[id1]->vec[n].keyInc );
			strcpy( keys[n][2], params[id1]->vec[n].keyDec );
		}
		for (int n = 0 ; n < 4 ; n++)
		{
			strcpy( params[id1]->vec[n].keySwitch, params[id2]->vec[n].keySwitch );
			strcpy( params[id1]->vec[n].keyInc, params[id2]->vec[n].keyInc );
			strcpy( params[id1]->vec[n].keyDec, params[id2]->vec[n].keyDec );
		}
		for (int n = 0 ; n < 4 ; n++)
		{
			strcpy( params[id2]->vec[n].keySwitch, keys[n][0] );
			strcpy( params[id2]->vec[n].keyInc, keys[n][1] );
			strcpy( params[id2]->vec[n].keyDec, keys[n][2] );
		}
	}
	void ciConfig::setKey(int id, string key0, string key1, string key2, string key3)
	{
		strcpy( params[id]->vec[0].keySwitch, key0.c_str() );
		strcpy( params[id]->vec[1].keySwitch, key1.c_str() );
		strcpy( params[id]->vec[2].keySwitch, key2.c_str() );
		strcpy( params[id]->vec[3].keySwitch, key2.c_str() );
	}
	void ciConfig::setKeyUpDown(int id, string up, string down)
	{
		for (int n = 0 ; n < 4 ; n++)
		{
			strcpy( params[id]->vec[n].keyInc, up.c_str() );
			strcpy( params[id]->vec[n].keyDec, down.c_str() );
		}
	}
	void ciConfig::setKeyVectorUpDown(int id, string up0, string down0, string up1, string down1)
	{
		strcpy( params[id]->vec[0].keyInc, up0.c_str() );
		strcpy( params[id]->vec[0].keyDec, down0.c_str() );
		strcpy( params[id]->vec[1].keyInc, up1.c_str() );
		strcpy( params[id]->vec[1].keyDec, down1.c_str() );
	}
	void ciConfig::setKeyVectorUpDown(int id, string up0, string down0, string up1, string down1, string up2, string down2)
	{
		strcpy( params[id]->vec[0].keyInc, up0.c_str() );
		strcpy( params[id]->vec[0].keyDec, down0.c_str() );
		strcpy( params[id]->vec[1].keyInc, up1.c_str() );
		strcpy( params[id]->vec[1].keyDec, down1.c_str() );
		strcpy( params[id]->vec[2].keyInc, up2.c_str() );
		strcpy( params[id]->vec[2].keyDec, down2.c_str() );
	}
	void ciConfig::setKeyVectorUpDown(int id, string up0, string down0, string up1, string down1, string up2, string down2, string up3, string down3)
	{
		strcpy( params[id]->vec[0].keyInc, up0.c_str() );
		strcpy( params[id]->vec[0].keyDec, down0.c_str() );
		strcpy( params[id]->vec[1].keyInc, up1.c_str() );
		strcpy( params[id]->vec[1].keyDec, down1.c_str() );
		strcpy( params[id]->vec[2].keyInc, up2.c_str() );
		strcpy( params[id]->vec[2].keyDec, down2.c_str() );
		strcpy( params[id]->vec[3].keyInc, up3.c_str() );
		strcpy( params[id]->vec[3].keyDec, down3.c_str() );
	}
	void ciConfig::setKeyRGBUpDown(int id, std::string up0, std::string down0, std::string up1, std::string down1, std::string up2, std::string down2)
	{
		this->setKeyVectorUpDown(id,up0,down0,up1,down1,up2,down2);
	}
	
	
	/////////////////////////////////////////////////////////////////////////
	//
	// MIDI
	//
	void ciConfig::resetMidi(int id)
	{
		for (int n = 0 ; n < 4 ; n++)
		{
			params[id]->vec[n].midiNote = MIDI_NONE;
			params[id]->vec[n].midiInc = MIDI_NONE;
			params[id]->vec[n].midiDec = MIDI_NONE;
			params[id]->vec[n].midiSwitch = MIDI_NONE;
		}
	}
	void ciConfig::swapMidi(int id1, int id2)
	{
		int notes[4][5];
		for (int n = 0 ; n < 4 ; n++)
		{
			notes[n][0] = params[id1]->vec[n].midiChannel;
			notes[n][1] = params[id1]->vec[n].midiNote;
			notes[n][2] = params[id1]->vec[n].midiInc;
			notes[n][3] = params[id1]->vec[n].midiDec;
			notes[n][4] = params[id1]->vec[n].midiSwitch;
		}
		for (int n = 0 ; n < 4 ; n++)
		{
			params[id1]->vec[n].midiChannel	= params[id2]->vec[n].midiChannel;
			params[id1]->vec[n].midiNote	= params[id2]->vec[n].midiNote;
			params[id1]->vec[n].midiInc		= params[id2]->vec[n].midiInc;
			params[id1]->vec[n].midiDec	= params[id2]->vec[n].midiDec;
			params[id1]->vec[n].midiSwitch	= params[id2]->vec[n].midiSwitch;
		}
		for (int n = 0 ; n < 4 ; n++)
		{
			params[id2]->vec[n].midiChannel	= notes[n][0];
			params[id2]->vec[n].midiNote	= notes[n][1];
			params[id2]->vec[n].midiInc		= notes[n][2];
			params[id2]->vec[n].midiDec	= notes[n][3];
			params[id2]->vec[n].midiSwitch	= notes[n][4];
		}
	}
	void ciConfig::setMidiChannel(int id, int ch)
	{
		for (int n = 0 ; n < 4 ; n++)
			params[id]->vec[n].midiChannel = ch;
	}
	void ciConfig::setMidiChannelTrigger(int id, int ch)
	{
		this->setMidiChannel(id, ch);
		for (int n = 0 ; n < 4 ; n++)
			params[id]->vec[n].midiNote = MIDI_TRIGGER;
	}
	void ciConfig::setMidi(int id, short note, int ch)
	{
		this->setMidiChannel(id, ch);
		for (int n = 0 ; n < 4 ; n++)
			params[id]->vec[n].midiNote = note;
		// Can I push a value to MIDI device? I don't think so...
		//midiOut->stimulate(midiChannel, note);
		//midiOut->sendControlChange(midiChannel, note, 0);
	}
	void ciConfig::setMidi2(int id, short note0, short note1, int ch)
	{
		this->setMidiChannel(id, ch);
		params[id]->vec[0].midiNote = note0;
		params[id]->vec[1].midiNote = note1;
	}
	void ciConfig::setMidi3(int id, short note0, short note1, short note2, int ch)
	{
		this->setMidiChannel(id, ch);
		params[id]->vec[0].midiNote = note0;
		params[id]->vec[1].midiNote = note1;
		params[id]->vec[2].midiNote = note2;
	}
	void ciConfig::setMidi4(int id, short note0, short note1, short note2, short note3, int ch)
	{
		this->setMidiChannel(id, ch);
		params[id]->vec[0].midiNote = note0;
		params[id]->vec[1].midiNote = note1;
		params[id]->vec[2].midiNote = note2;
		params[id]->vec[3].midiNote = note3;
	}
	void ciConfig::setMidiUpDown(int id, short up, short down, int ch)
	{
		this->setMidiChannel(id, ch);
		for (int n = 0 ; n < 4 ; n++)
		{
			params[id]->vec[n].midiInc = up;
			params[id]->vec[n].midiDec = down;
		}
	}
	void ciConfig::setMidiUpDown2(int id, short up0, short down0, short up1, short down1, int ch)
	{
		this->setMidiChannel(id, ch);
		params[id]->vec[0].midiInc = up0;
		params[id]->vec[0].midiDec = down0;
		params[id]->vec[1].midiInc = up1;
		params[id]->vec[1].midiDec = down1;
	}
	void ciConfig::setMidiUpDown3(int id, short up0, short down0, short up1, short down1, short up2, short down2, int ch)
	{
		this->setMidiChannel(id, ch);
		params[id]->vec[0].midiInc = up0;
		params[id]->vec[0].midiDec = down0;
		params[id]->vec[1].midiInc = up1;
		params[id]->vec[1].midiDec = down1;
		params[id]->vec[2].midiInc = up2;
		params[id]->vec[2].midiDec = down2;
	}
	void ciConfig::setMidiUpDown4(int id, short up0, short down0, short up1, short down1, short up2, short down2, short up3, short down3, int ch)
	{
		this->setMidiChannel(id, ch);
		params[id]->vec[0].midiInc = up0;
		params[id]->vec[0].midiDec = down0;
		params[id]->vec[1].midiInc = up1;
		params[id]->vec[1].midiDec = down1;
		params[id]->vec[2].midiInc = up2;
		params[id]->vec[2].midiDec = down2;
		params[id]->vec[3].midiInc = up3;
		params[id]->vec[3].midiDec = down3;
	}
	void ciConfig::setMidiRGBUpDown(int id, short up0, short down0, short up1, short down1, short up2, short down2, int ch)
	{
		this->setMidiUpDown3(id,up0,down0,up1,down1,up2,down2,ch);
	}
	void ciConfig::setMidiSwitch(int id, short note, int ch)
	{
		this->setMidiChannel(id, ch);
		for (int n = 0 ; n < 4 ; n++)
			params[id]->vec[n].midiSwitch = note;
	}
	//
	// PARSE MIDI message
	// used by OSC as well
	void ciConfig::parseMidiMessage(int channel, int note, int val)
	{
		if ( note < 0 || note > 127 )
		{
			printf("CFG MIDI IN ch[%d] note[%d] = [%d] INVALID NOTE!!!\n",channel,note,val);
			return;
		}
		if ( val < 0 || val > 127 )
		{
			printf("CFG MIDI IN ch[%d] note[%d] = [%d] INVALID VALUE!!!\n",channel,note,val);
			return;
		}
		float newProg = ((float)val / 127.0);
		printf("CFG MIDI IN ch[%d] note[%d] = [%d/%.2f]",channel,note,val,newProg);
		for (int id = 0 ; id < params.size() ; id++)
		{
			if (params[id] == NULL)
				continue;
			ciConfigParam *p = params.at(id);
			for (int i = 0 ; i < 4 ; i++)
			{
				if (p->vec[i].midiChannel != channel)
					continue;
				// The whole channel is triggering a Param
				// The note is actually the value to set
				if (p->vec[i].midiNote == MIDI_TRIGGER)
				{
					this->set(id, i, note);
					printf(" >> CFG id[%d/%d] = [%.6f] (%s)\n",id,i,p->vec[i].get(),p->name.c_str());
					return;
				}
				// Assign MIDI value to param
				else if (p->vec[i].midiNote == note)
				{
					this->setProg(id, i, newProg);
					printf(" >> CFG id[%d/%d] = [%.6f] (%s)\n",id,i,p->vec[i].get(),p->name.c_str());
					return;
				}
				// Increase param
				else if (p->vec[i].midiInc == note)
				{
					if (newProg == 1.0)
					{
						this->inc(id, i, true);
						printf(" >> CFG id[%d/%d] = [%.6f] (%s)\n",id,i,p->vec[i].get(),p->name.c_str());
					}
					else
						printf(" >> ignored\n");
					return;
				}
				// Decrease param
				else if (p->vec[i].midiDec == note)
				{
					if (newProg == 1.0)
					{
						this->dec(id, i, true);
						printf(" >> CFG id[%d/%d] = [%.6f] (%s)\n",id,i,p->vec[i].get(),p->name.c_str());
					}
					else
						printf(" >> ignored\n");
					return;
				}
				// Switch param / inc() in loop
				else if (p->vec[i].midiSwitch == note)
				{
					if (newProg == 1.0)
					{
						if (this->getProg(id, i) < 1.0)
							this->inc(id, i, true);
						else
							this->setToMin(id);
						printf(" >> CFG id[%d/%d] = [%.6f] (%s)\n",id,i,p->vec[i].get(),p->name.c_str());
					}
					else
						printf(" >> ignored\n");
					return;
				}
			}
		}
		printf(" >> unassigned\n");
	}
	
	//////////////////////////////////////////////////
	//
	// OSC
	//
#ifdef CFG_USE_OSC
	//
	// Enable/disable OSC
	// Send every param change thu OSC to another app with ciConfig
	void ciConfig::setOscSend(bool b, int port)
	{
		bOscSend = b;
		if (bOscSend)
		{
			bOscReceive = !b;
			this->switchOsc(port);
		}
	}
	//
	// Enable/disable OSC
	// Receive params change from another app with ciConfig
	// Receive MIDI as OSC from MidiAsOsc.app
	void ciConfig::setOscReceive(bool b, int port)
	{
		bOscReceive = b;
		if (bOscReceive)
		{
			bOscSend = !b;
			this->switchOsc(port);
		}
	}
	// Turn OSC on/off
	void ciConfig::switchOsc(int port)
	{
		try {
			if (bOscReceive)
			{
				oscReceiver.setup( port );
				sprintf(errmsg, "OSC RECEIVING PORT %d...",port);
			}
			else if (bOscSend)
			{
				oscSender.setup( CICFG_OSC_HOST, port );
				sprintf(errmsg, "OSC SENDING...");
			}
		} catch (exception &error) {
			sprintf(errmsg, "OSC SETUP FAILED!!!!");
			printf("OSC SETUP FAILED!!!\n");
		}
	}
	//
	// Send params change to another app with ciConfig
	void ciConfig::sendOsc(short id, short i)
	{
		// OSC Receive Disabled?
		if (bOscSend == false)
			return;
		// Do not send string
		if (params[id]->type == CFG_TYPE_STRING)
			return;
		// Send OSC
		std::string addr = "/";
		addr += this->getName(id);
		ofxOscMessage m;
		m.setAddress( addr );
		m.addIntArg( (int)i );
		if (params[id]->vec[i].preserveProg)
		{
			m.addStringArg( "P" );
			m.addFloatArg( this->getProg(id, i) );
		}
		else
		{
			m.addStringArg( "V" );
			m.addFloatArg( this->get(id, i) );
		}
		oscSender.sendMessage( m );
#ifdef OSC_VERBOSE
		printf("OSC OUT >> id[%d/%d] = [%.3f] (%s)\n",id,i,this->getProg(id, i),this->getName(id));
#endif
	}
	//
	// PUSH ALL PARAMS to another app with ciConfig
	void ciConfig::pushOsc()
	{
		for (int id = 0 ; id < params.size() ; id++)
		{
			if (params[id] == NULL)
				continue;
			for (int i = 0 ; i < 4 ; i++)
				sendOsc(id, i);
		}
	}
	//
	// ofxOsc callback
	// - Reads MIDI messages disguised as OSC forwarded from MidiAsOsc.app
	// - Reads PARAMS sent from another app using odxConfig
#ifdef CFG_CATCH_LOOP_EVENTS
	void ciConfig::oscCallback(ofEventArgs &e)
	{
		this->oscCallback();
	}
#endif
	void ciConfig::oscCallback()
	{
		// OSC Receive Disabled?
		if (bOscReceive == false)
			return;
#ifdef OSC_VERBOSE
		sprintf(errmsg, "OSC CALLBACK");
#endif
		//printf("OSC CALLBACK\n");
		// check for waiting messages
		while( oscReceiver.hasWaitingMessages() )
		{
			// get the next message
			ofxOscMessage m;
			if (oscReceiver.getNextMessage( &m ) == false)
				continue;
			
#ifdef OSC_VERBOSE
			sprintf(errmsg, "OSC MSG [%s]", m.getAddress().c_str());
			//printf("OSC MSG [%s] %.2f\n", m.getAddress().c_str(), m.getArgAsFloat( 0 ));
#endif
			// Receive from MisiAsOsc.app
			// Format:	/midi channel note value
			if ( m.getAddress() == "/midi" )
			{
				// both the arguments are int32's
				int channel	= m.getArgAsInt32( 0 );
				int note	= m.getArgAsInt32( 1 );
				float val	= m.getArgAsFloat( 2 );
#ifdef OSC_VERBOSE
				printf("OSC IN: /midi channel[%d] note[%d] value[%f]\n",channel,note,val);
#endif
				this->parseMidiMessage(channel, note, val);
			}
			// Receive from another ciConfig
			// Format:	/PARAM_NAME ix [P|V] value
			else
			{
				const char *name = m.getAddress().substr(1).c_str();
				for (int id = 0 ; id < params.size() ; id++)
				{
					if (params[id] == NULL)
						continue;
					ciConfigParam *p = params.at(id);
					if (name == p->name)
					{
						int i		= 0;
						std::string pv	= "P";
						float val;
						// iPad / ToushOSC
						if (m.getNumArgs() == 1)
							val	= m.getArgAsFloat( 0 );
						// MidiAsOSC
						else
						{
							i	= m.getArgAsInt32( 0 );
							pv	= m.getArgAsString( 1 );
							val	= m.getArgAsFloat( 2 );
						}
						
						if (pv[0] == 'P')
							this->setProg(id, i, val);
						else // value
							this->set(id, i, val);
#ifdef OSC_VERBOSE
						printf("OSC IN >> id[%d/%d] = [%.3f] (%s)\n",id,i,p->vec[i].get(),p->name.c_str());
#endif
						sprintf(errmsg, "OSC TO %s=%.3f", p->name.c_str(), p->vec[i].get());
						return;
					}
				}
			}
		}
		
	}
#endif
	
	
	/////////////////////////////////////////////////////////////////////////
	//
	// PRIVATE
	//
	
	// store new param
	void ciConfig::pushParam(int id, ciConfigParam* p)
	{
		// id is always the vector index
		// so we fill the vector with NULLs until it has <id> elements
		while (params.size() <= id)
			params.push_back(NULL);
		// replace param
		params.erase(params.begin()+id);
		params.insert(params.begin()+id, p);
	}
	// Is it a number?
	bool ciConfig::isNumber(int id)
	{
		switch (params[id]->type)
		{
			case CFG_TYPE_FLOAT:
			case CFG_TYPE_DOUBLE:
			case CFG_TYPE_INTEGER:
			case CFG_TYPE_LONG:
			case CFG_TYPE_BYTE:
			case CFG_TYPE_COLOR:
			case CFG_TYPE_VECTOR2:
			case CFG_TYPE_VECTOR3:
			case CFG_TYPE_VECTOR4:
				return true;
				break;
		}
		return false;
	}
	// Is it a vector?
	bool ciConfig::isVector(int id)
	{
		switch (params[id]->type)
		{
			case CFG_TYPE_COLOR:
			case CFG_TYPE_VECTOR2:
			case CFG_TYPE_VECTOR3:
			case CFG_TYPE_VECTOR4:
				return true;
				break;
		}
		return false;
	}
	
	
	/////////////////////////////////////////////////////////////////////////
	//
	// INITTERS
	//
	// generic (float)
	void ciConfig::add(short id, const string name, float val, float vmin, float vmax)
	{
		this->addFloat(id, name, val);
	}
	// specific
	void ciConfig::addFloat(short id, const string name, float val, float vmin, float vmax)
	{
		this->pushParam(id, new ciConfigParam(id, CFG_TYPE_FLOAT, name));
		this->setLimits(id, vmin, vmax);
		this->set(id, val);
		this->init(id);
	}
	void ciConfig::addDouble(short id, const string name, double val, double vmin, double vmax)
	{
		this->pushParam(id, new ciConfigParam(id, CFG_TYPE_DOUBLE, name));
		this->setLimits(id, (float)vmin, (float)vmax);
		this->set(id, val);
		this->init(id);
	}
	void ciConfig::addInt(short id, const string name, int val, int vmin, int vmax)
	{
		this->pushParam(id, new ciConfigParam(id, CFG_TYPE_INTEGER, name));
		this->setLimits(id, (float)vmin, (float)vmax);
		this->set(id, val);
		this->init(id);
	}
	void ciConfig::addLong(short id, const string name, long val, long vmin, long vmax)
	{
		this->pushParam(id, new ciConfigParam(id, CFG_TYPE_LONG, name));
		this->setLimits(id, (float)vmin, (float)vmax);
		this->set(id, val);
		this->init(id);
	}
	void ciConfig::addBool(short id, const string name, bool val)
	{
		this->pushParam(id, new ciConfigParam(id, CFG_TYPE_BOOLEAN, name));
		this->setLimits(id, 0.0f, 1.0f);
		this->set(id, val );
		this->init(id);
	}
	void ciConfig::addByte(short id, const string name, unsigned char val, unsigned char vmin, unsigned char vmax)
	{
		this->pushParam(id, new ciConfigParam(id, CFG_TYPE_BYTE, name));
		this->setLimits(id, (float)vmin, (float)vmax);
		this->set(id, (float) val);
		this->init(id);
	}
	void ciConfig::addString(short id, const string name, const string val)
	{
		this->pushParam(id, new ciConfigParam(id, CFG_TYPE_STRING, name));
		this->set(id, val);
		this->init(id);
	}
	void ciConfig::addColor(short id, const string name, Color8u c)
	{
		this->addColor(id, name, c.r, c.g, c.b);
	}
	void ciConfig::addColor(short id, const string name, float r, float g, float b)
	{
		this->pushParam(id, new ciConfigParam(id, CFG_TYPE_COLOR, name));
		this->setLimitsR(id, 0.0f, 255.0f);
		this->setLimitsG(id, 0.0f, 255.0f);
		this->setLimitsB(id, 0.0f, 255.0f);
		this->set(id, r, g, b, 255.0f);
		this->init(id);
	}
	void ciConfig::addVector2(short id, const string name, Vec2f p, float vmin, float vmax)
	{
		this->addVector2(id, name, p.x, p.y, vmin, vmax);
	}
	void ciConfig::addVector2(short id, const string name, float x, float y, float vmin, float vmax)
	{
		this->pushParam(id, new ciConfigParam(id, CFG_TYPE_VECTOR2, name));
		this->setLimitsX(id, vmin, vmax);
		this->setLimitsY(id, vmin, vmax);
		this->set(id, x, y);
		this->init(id);
	}
	void ciConfig::addVector3(short id, const string name, Vec3f p, float vmin, float vmax)
	{
		this->addVector3(id, name, p.x, p.y, p.z, vmin, vmax);
	}
	void ciConfig::addVector3(short id, const string name, float x, float y, float z, float vmin, float vmax)
	{
		this->pushParam(id, new ciConfigParam(id, CFG_TYPE_VECTOR3, name));
		this->setLimitsX(id, vmin, vmax);
		this->setLimitsY(id, vmin, vmax);
		this->setLimitsZ(id, vmin, vmax);
		this->set(id, x, y, z);
		this->init(id);
	}
	void ciConfig::addVector4(short id, const string name, Vec4f p, float vmin, float vmax)
	{
		this->addVector4(id, name, p.x, p.y, p.z, p.w, vmin, vmax);
	}
	void ciConfig::addVector4(short id, const string name, float x, float y, float z, float w, float vmin, float vmax)
	{
		this->pushParam(id, new ciConfigParam(id, CFG_TYPE_VECTOR4, name));
		this->setLimitsX(id, vmin, vmax);
		this->setLimitsY(id, vmin, vmax);
		this->setLimitsZ(id, vmin, vmax);
		this->setLimitsW(id, vmin, vmax);
		this->set(id, x, y, z, w);
		this->init(id);
	}
	// reset to initial values
	void ciConfig::init(int id)
	{
		if (params[id] != NULL)
			params[id]->init();
	}
	void ciConfig::setDefault(int def, int id, float v)
	{
		for (int i = 0 ; i < 4 ; i++)
			this->setDefault(def, id, i, v);
	}
	void ciConfig::setDefault(int def, int id, Vec2f v )
	{
		this->setDefault(def, id, 0, v[0]);
		this->setDefault(def, id, 1, v[1]);
	}
	void ciConfig::setDefault(int def, int id, Vec3f v )
	{
		this->setDefault(def, id, 0, v[0]);
		this->setDefault(def, id, 1, v[1]);
		this->setDefault(def, id, 2, v[2]);
	}
	void ciConfig::setDefault(int def, int id, Vec4f v )
	{
		this->setDefault(def, id, 0, v[0]);
		this->setDefault(def, id, 1, v[1]);
		this->setDefault(def, id, 2, v[2]);
		this->setDefault(def, id, 3, v[3]);
	}
	void ciConfig::setDefault(int def, int id, int i, float v)
	{
		ciConfigParam *p = params[id];
		p->vec[i].setInitialValue(def,v);
	}
	void ciConfig::resetDefault(int def)
	{
		if (def >= MAX_DEFAULTS)
		{
			printf("INVALID CONFIG DEFAULT %d\n",def);
			return;
		}
		for ( short id = 0 ; id < params.size() ; id++ )
			this->resetDefault(id, def);
		mDisplayFileName = "< reset >";
		if ( postResetCallback_fn )
			postResetCallback_fn(this);
		// virtual to update gui
		this->setCurrentDefault(def);
	}
	void ciConfig::resetDefault(int id, int def)
	{
		ciConfigParam *p = params[id];
		if (p != NULL)
		{
			for (int i = 0 ; i < 4 ; i++)
			{
				if (this->isString(id))
					p->strval = p->strvalInitial[def];
				else
					this->set( id, i, p->vec[i].getInitialValue(def) );
				this->setLoaded(id, false);
			}
		}
	}
	
	
	/////////////////////////////////////////////////////////////////////////
	//
	// SETTERS
	//
	void ciConfig::setLimits(int id, Vec2f vmin, Vec2f vmax)
	{
		this->setLimitsX(id, vmin.x, vmax.x);
		this->setLimitsY(id, vmin.y, vmax.y);
	}
	void ciConfig::setLimits(int id, Vec3f vmin, Vec3f vmax)
	{
		this->setLimitsX(id, vmin.x, vmax.x);
		this->setLimitsY(id, vmin.y, vmax.y);
		this->setLimitsZ(id, vmin.z, vmax.z);
	}
	void ciConfig::setLimits(int id, Vec4f vmin, Vec4f vmax)
	{
		this->setLimitsX(id, vmin.x, vmax.x);
		this->setLimitsY(id, vmin.y, vmax.y);
		this->setLimitsZ(id, vmin.z, vmax.z);
		this->setLimitsW(id, vmin.w, vmax.w);
	}
	void ciConfig::setLimitsDegrees(int id)
	{
		this->setLimitsX(id, 0, 360);
		this->setLimitsY(id, 0, 360);
		this->setLimitsZ(id, 0, 360);
		this->setLimitsW(id, 0, 360);
	}
	void ciConfig::setLimitsRadians(int id)
	{
		this->setLimitsX(id, 0, TWO_PI);
		this->setLimitsY(id, 0, TWO_PI);
		this->setLimitsZ(id, 0, TWO_PI);
		this->setLimitsW(id, 0, TWO_PI);
	}
	void ciConfig::setLimitsScreen(int id)
	{
		this->setLimitsX(id, 0, getWindowWidth());
		this->setLimitsY(id, 0, getWindowHeight());
		this->setLimitsZ(id, 0, 0);
		this->setLimitsW(id, 0, 0);
	}
	//
	// Called after any set
	void ciConfig::post_set(int id)
	{
		this->post_set( id, 0, false);
		this->post_set( id, 1, false);
		this->post_set( id, 2, true);
		this->post_set( id, 3, true);
	}
	void ciConfig::post_set(int id, int i, bool doCB)
	{
		ciConfigParam *p = params[id];
		// update Param pointers
		p->updatePointers(i);
		// Set freshness
		bool f = false;
		switch (p->type)
		{
			case CFG_TYPE_FLOAT:
			case CFG_TYPE_DOUBLE:
			case CFG_TYPE_COLOR:
			case CFG_TYPE_VECTOR2:
			case CFG_TYPE_VECTOR3:
			case CFG_TYPE_VECTOR4:
				if ( this->get(id, i) != this->getLastValue(id, i) )
					f = true;
				break;
			case CFG_TYPE_INTEGER:
			case CFG_TYPE_LONG:
			case CFG_TYPE_BOOLEAN:
			case CFG_TYPE_BYTE:
				if ( (int) this->get(id, i) != (int) this->getLastValue(id, i) )
					f = true;
				break;
			case CFG_TYPE_STRING:
				if ( this->getLastValueString(id).compare( this->getString(id) ) != 0 )
					f = true;
				break;
			default:
				f = true;
				break;
		}
		// If fresh...
		if (f)
		{
			// Mark file as read
			//if ( ! p->dummy )
			//	printf("BREAKPOINT\n");
			//if ( p->name.compare( "PROJ0_LENS_SHIFT_H" ) == 0 )
			//	printf("BREAKPOINT\n");

			if (mDisplayFileName[0] != '*')
				mDisplayFileName.insert(0, "*");
			// set as fresh
			freshness = true;
			p->changed = true;
			p->vec[i].freshness = true;
			// save current value
			if (p->isString())
				this->updateLastValueString(id);
			else
				this->updateLastValue(id, i);
			// Virtual callback
			if ( bStarted && !inPostSetCallback && doCB )
			{
				inPostSetCallback = true;
				if ( postSetCallback_fn )
					postSetCallback_fn(this, id, i);
				//else
				this->postSetCallback(id, i);
				inPostSetCallback = false;
			}
#ifdef CFG_USE_OSC
			// Send OSC
			this->sendOsc(id, i);
#endif
		}
	}
	//
	// PROG
	//
	// Preserve Prog: prog is priority
	// Prog will be saved and read form file instead of value
	void ciConfig::preserveProg(int id, bool b)
	{
		params[id]->vec[0].preserveProg = b;
		params[id]->vec[1].preserveProg = b;
		params[id]->vec[2].preserveProg = b;
		params[id]->vec[3].preserveProg = b;
	}
	// Set Prog
	void ciConfig::setProg(int id, int i, float p)		// Private
	{
		params[id]->vec[i].setProg(p);
		this->post_set(id, i);
	}
	void ciConfig::setProg(int id, float val0, float val1)
	{
		params[id]->vec[0].setProg(val0);
		params[id]->vec[1].setProg(val1);
		this->post_set(id);
	}
	void ciConfig::setProg(int id, float val0, float val1, float val2)
	{
		params[id]->vec[0].setProg(val0);
		params[id]->vec[1].setProg(val1);
		params[id]->vec[2].setProg(val2);
		this->post_set(id);
	}
	void ciConfig::setProg(int id, float val0, float val1, float val2, float val3)
	{
		params[id]->vec[0].setProg(val0);
		params[id]->vec[1].setProg(val1);
		params[id]->vec[2].setProg(val2);
		params[id]->vec[3].setProg(val3);
		this->post_set(id);
	}
	// generic (float)
	// val1, val2 will be used only on CFG_TYPE_VECTORX and CFG_TYPE_COLOR
	void ciConfig::set(int id, int i, float val)		// Private
	{
		ciConfigParam *p = params[id];
		switch (p->type)
		{
			case CFG_TYPE_FLOAT:
			case CFG_TYPE_DOUBLE:
			case CFG_TYPE_INTEGER:
			case CFG_TYPE_LONG:
			case CFG_TYPE_BOOLEAN:
			case CFG_TYPE_BYTE:
			case CFG_TYPE_COLOR:
			case CFG_TYPE_VECTOR2:
			case CFG_TYPE_VECTOR3:
			case CFG_TYPE_VECTOR4:
				p->vec[i].set(val);
				break;
			case CFG_TYPE_STRING:
			{
				std::ostringstream os;
				os << val;
				p->strval = os.str();
				break;
			}
			default:
				printf("Config.set(int) ERROR invalid id[%d] type[%d] val[%d]\n",id,p->type,(int)val);
				return;
				break;
		}
		this->post_set(id, i);
	}
	// If CFG_TYPE_STRING, just set
	// Else, convert to float
	// pp = Prog?
	void ciConfig::set(int id, const char *val, bool pp)
	{
		// Set string!
		if (this->isString(id))
		{
			params[id]->strval = string( val );
			params[id]->updatePointers(0);
			this->post_set(id, 0);
		}
		// Set vector values
		// val = "number,number,number"
		else if (this->isVector(id))
		{
			std::vector<std::string> splits;
			boost::split(splits, val, boost::is_any_of(","));
			std::string vals[4];
			for (int n = 0 ; n < splits.size() ; n++)
				vals[n] = splits[n];
			if (pp)
				this->setProg(id, atof(vals[0].c_str()), atof(vals[1].c_str()), atof(vals[2].c_str()), atof(vals[3].c_str()));
			else
				this->set(id, atof(vals[0].c_str()), atof(vals[1].c_str()), atof(vals[2].c_str()), atof(vals[3].c_str()));
		}
		// defualt: float
		else if (pp)
			this->setProg(id, 0, val);
		else
			this->set(id, 0, val);
	}
	void ciConfig::set(int id, float val0, float val1)
	{
		ciConfigParam *p = params[id];
		p->vec[0].set(val0);
		p->vec[1].set(val1);
		this->post_set(id);
	}
	void ciConfig::set(int id, float val0, float val1, float val2)
	{
		ciConfigParam *p = params[id];
		p->vec[0].set(val0);
		p->vec[1].set(val1);
		p->vec[2].set(val2);
		this->post_set(id);
	}
	void ciConfig::set(int id, float val0, float val1, float val2, float val3)
	{
		ciConfigParam *p = params[id];
		p->vec[0].set(val0);
		p->vec[1].set(val1);
		p->vec[2].set(val2);
		p->vec[3].set(val3);
		this->post_set(id);
	}
	//
	// Vaule labels for radio buttons
	// Works for CFG_TYPE_INTEGER
	// TERMINATE WITH NULL
	// example: this->setValueLabels(THE_SHAPE, "Sphere", "Cube", NULL);
	void ciConfig::setValueLabels(short id, const char *val0, ...)
	{
		ciConfigParam *param = params[id];
		va_list args;
		va_start (args, val0);
		char *p = (char*) val0;			// get first
		for (int key = 0 ; p ; key++)
		{
			param->valueLabels[key] = string(p);
			p = va_arg (args, char *);	// next
			//printf("label %s\n",p);
		}
		va_end (args);
		// virtual
		this->guiUpdateValueLabels(id);
	}
	// accept labels like this...
	//	char labels[][64] = 
	//	{
	//		"640 x 480",
	//		"1024 x 768",
	//		"1280 x 1024",
	//		"1920 x 1080",
	//		NULL
	//	};
	void ciConfig::updateLimitsByValueLabels(short id)
	{
		ciConfigParam *param = params[id];
		if (param->valueLabels.size() == 0)
			this->setLimits(id, 0, 0);
		else
		{
			std::map<int,std::string>::const_iterator it_min = param->valueLabels.begin();
			std::map<int,std::string>::const_reverse_iterator it_max = param->valueLabels.rbegin();
			this->setLimits(id, it_min->first, it_max->first);
		}
	}
	void ciConfig::clearValueLabels(short id)
	{
		params[id]->valueLabels.clear();
		this->updateLimitsByValueLabels(id);
		// virtual
		this->guiUpdateValueLabels(id);
	}
	void ciConfig::setValueLabels(short id, const char labels[][64])
	{
		for (int key = 0 ; labels[key][0] != '\0' ; key++)
			params[id]->valueLabels[key] = string(labels[key]);
		this->updateLimitsByValueLabels(id);
		// virtual
		this->guiUpdateValueLabels(id);
	}
	void ciConfig::setValueLabels(short id, const std::map<int,std::string> & labels)
	{
		std::map<int,std::string>::const_iterator it;
		for ( it = labels.begin() ; it != labels.end(); it++ )
		{
			int key = (it->first);
			std::string name = (it->second);
			params[id]->valueLabels[key] = name;
		}
		this->updateLimitsByValueLabels(id);
		// virtual
		this->guiUpdateValueLabels(id);
	}
	void ciConfig::setValueLabels(short id, const std::vector<std::string> & labels)
	{
		int count = 0;
		for ( auto it = labels.begin() ; it != labels.end(); it++, count++ )
			params[id]->valueLabels[count] = *it;
		this->updateLimitsByValueLabels(id);
		// virtual
		this->guiUpdateValueLabels(id);
	}
	// SETTER / GETER
	void ciConfig::setValueLabel(short id, int key, string label, bool indexToo)
	{
		//printf("SET LABEL [%d] = [%s]\n",key,label.c_str());
		ciConfigParam *param = params[id];
		if (indexToo)
		{
			std::ostringstream os;
			os << key << ": " << label;
			param->valueLabels[key] = os.str();
		}
		else
			param->valueLabels[key] = label;
		this->updateLimitsByValueLabels(id);
		// virtual
		this->guiUpdateValueLabels(id);
	}
	std::string ciConfig::getValueLabel(short id, int key)
	{
		ciConfigParam *param = params[id];
		std::string str = param->valueLabels[key];
		if (str.length())
			return str;
		else
			return this->getString(id);
	}
	
	
	/////////////////////////////////////////////////////////////////////////
	//
	// OPERATIONS
	//
	// Invert a value
	void ciConfig::invert(int id, short i)		// Private
	{
		params[id]->vec[i].invert();
		this->post_set(id, i);
	}
	void ciConfig::invert(int id)
	{
		if (this->isVector(id))
		{
			this->invert(id, 0);
			this->invert(id, 1);
			this->invert(id, 2);
			this->invert(id, 3);
		}
		else
			this->invert(id, 0);
	}
	//
	// Subtract a value
	void ciConfig::sub(int id, int i, float val, bool clamp)	// Private
	{
		params[id]->vec[i].sub(val, clamp);
		this->post_set(id, i);
	}
	void ciConfig::sub(int id, float val, bool clamp)
	{
		if (this->isVector(id))
		{
			this->sub(id, 0, val, clamp);
			this->sub(id, 1, val, clamp);
			this->sub(id, 2, val, clamp);
			this->sub(id, 3, val, clamp);
		}
		else
			this->sub(id, 0, val, clamp);
	}
	//
	// Add  a value
	void ciConfig::add(int id, int i, float val, bool clamp)	// Private
	{
		params[id]->vec[i].add(val, clamp);
		this->post_set(id, i);
	}
	void ciConfig::add(int id, float val, bool clamp)
	{
		if (this->isVector(id))
		{
			this->add(id, 0, val, clamp);
			this->add(id, 1, val, clamp);
			this->add(id, 2, val, clamp);
			this->add(id, 3, val, clamp);
		}
		else
			this->add(id, 0, val, clamp);
	}
	// inc + loop para o primeiro
	void ciConfig::incLoop(int id)
	{
		this->inc(id, false);	// inc, NO clamp
		if ( this->getInt(id) > this->getMax(id) )
			this->setToMin(id);
	}
	
	
	
	/////////////////////////////////////////////////////////////////////////
	//
	// GETTERS
	//
	// Get generic value (float)
	float ciConfig::get(int id, int i)		// Private
	{
		ciConfigParam *p = params[id];
		if ( p == NULL )
		{
			//printf("config GET NULL [%d,%d]\n",id,i);
			return 0.0;
		}
		switch (p->type)
		{
			case CFG_TYPE_FLOAT:
			case CFG_TYPE_DOUBLE:
			case CFG_TYPE_INTEGER:
			case CFG_TYPE_LONG:
			case CFG_TYPE_BOOLEAN:
			case CFG_TYPE_BYTE:
			case CFG_TYPE_COLOR:
			case CFG_TYPE_VECTOR2:
			case CFG_TYPE_VECTOR3:
			case CFG_TYPE_VECTOR4:
				return p->vec[i].get();
				break;
			case CFG_TYPE_STRING:
				return atof(p->strval.c_str());
				break;
			default:
				printf("Config.get(int) ERROR invalid id[%d] type[%d]\n",id,p->type);
				return 0.0f;
				break;
		}
	}
	const char* ciConfig::getString(int id, bool raw)
	{
		ciConfigParam *param = params[id];
		// String type: no conversion
		if (param->type == CFG_TYPE_STRING)
			return (param->strval).c_str();
		// Init string
		param->strval = "";
		// Convert to string...
		bool pp = param->vec[0].preserveProg;
		for (int i = 0 ; i < this->getVectorCount(id) ; i++)
		{
			char val[16];
			if (i > 0)
				param->strval += ",";
			// prog value
			if (pp)
				sprintf(val,"%f",this->getProg(id,i));
			// normal value
			else
			{
				// Format value
				switch(param->type)
				{
					case CFG_TYPE_INTEGER:
						if (raw)
							sprintf(val,"%d",(int)this->get(id));
						else
						{
							//int v = (int)this->get(id,i) - (int)this->getMin(id,i);
							std::string l = param->valueLabels[this->getInt(id)];
							if (l.length())
								return l.c_str();
							else
								sprintf(val,"%d",(int)this->get(id));
						}
						break;
					case CFG_TYPE_COLOR:
						sprintf(val,"%d",(int)this->get(id,i));
						break;
					case CFG_TYPE_BYTE:
						if (raw)
							sprintf(val,"%d",(int)this->get(id,i));
						else
							sprintf(val,"%c",(int)this->get(id,i));
						break;
					case CFG_TYPE_LONG:
						sprintf(val,"%ld",(long)this->get(id,i));
						break;
					case CFG_TYPE_BOOLEAN:
						if (raw)
							sprintf(val,"%d",(this->get(id,i) == 0.0 ? 0 : 1));
						else
							sprintf(val,"%s",(this->get(id,i) == 0.0 ? "OFF" : "ON"));
						break;
					default:
						sprintf(val,"%f",this->get(id,i));
						break;
				}
			}
			param->strval += val;
		}
		// return string
		return (param->strval).c_str();
	}
	//
	// Normalized getters
	float ciConfig::getDegrees(int id, int i)
	{
		float v = this->get(id, i);
		while (v < 0.0f)
			v += 360.0f;
		while (v >= 360.0f)
			v -= 360.0f;
		return v;
	}
	float ciConfig::getRadians(int id, int i)
	{
		float v = this->get(id, i);
		while (v < 0.0f)
			v += TWO_PI;
		while (v >= TWO_PI)
			v -= TWO_PI;
		return v;
	}
	
	
	
	
	/////////////////////////////////////////////////////////////////////////
	//
	// NEW SAVE / READ
	//
	// Set the default file extension
	void ciConfig::setFileExtension(const std::string e)
	{
		// Change current filename extension
		if ( mCurrentFileName.length() )
			mCurrentFileName.replace( mCurrentFileName.size()-fileExt.length(), fileExt.length(), e );
		// set current file extension
		fileExt = e;
	}
	//
	// Sets default file and load it
	// OBS: Nao faz mais muito sentido ter isso...
	void ciConfig::setFile(const std::string & f)
	{
		if (f[0] == '/')
			mCurrentFileName = f;
		else
			mCurrentFileName = mFolderSave + "/" + f;
	}
	int ciConfig::useFile(const std::string & f, const std::string & path)
	{
		if ( path.length() )
			this->setFile( path + "/" + f );
		else
			this->setFile( f );
		return this->readFile( f );
	}
	//
	// Open / Save from picking a file
	bool ciConfig::import()
	{
		std::vector<std::string> extensions;
		extensions.push_back( fileExt );
		std::string path = app::getOpenFilePath( mFolderSave, extensions ).string();
		if ( path.length() )
		{
			mFolderSave = path;
			if ( this->readFile( path.c_str() ) > 0 )
				this->setFile( path.c_str() );
			return true;
		}
		return false;
	}
	bool ciConfig::exportas()
	{
		std::vector<std::string> extensions;
		extensions.push_back( fileExt );
		std::string path = app::getSaveFilePath( mFolderSave, extensions ).string();
		if ( path.length() )
		{
			mFolderSave = path;
			if ( this->saveFile( path.c_str() ) > 0 )
				this->setFile( path.c_str() );
			return true;
		}
		return false;
	}
	//
	// Load default file
	int ciConfig::load(char preset)
	{
		for ( int n = 0 ; n < mFolderList.size() ; n++ )
		{
			std::string f = this->makeFileName( mFolderList[n], preset );
			if( fs::exists( f ) )
				if ( int ps = this->readFile( f, preset ) > 0 )
					return ps;
		}
		return 0;
	}
	//
	// Save default file
	int ciConfig::save(char preset)
	{
		std::string f = this->makeFileName( mFolderDefault, preset );
		return this->saveFile( f, preset );
	}
	//
	// Preset info
	bool ciConfig::presetExist(char preset)
	{
		XmlTree doc;
		if ( ! this->getPresetXml(preset, doc) )
			return false;
		// Get preset
		std::string nodeName = "config/" + this->makeXmlNodeName( preset );
		return doc.hasChild( nodeName );
	}
	std::string ciConfig::presetTimestamp(char preset)
	{
		XmlTree doc;
		if ( this->getPresetXml(preset, doc) )
		{
			// Get preset
			std::string nodeName = "config/" + this->makeXmlNodeName( preset );
			if ( doc.hasChild( nodeName ) )
			{
				XmlTree node = doc.getChild( nodeName );
				return node.getAttribute("SAVE_TIME");
			}
		}
		return "";
	}
	//
	// Make names
	std::string ciConfig::makeFileName(const std::string & path, char preset)
	{
		// WARNING :: BLENDY VJ IS USING THIS...
		//std::string f = path + "/" + mAppName;
		std::string f = path + "/DEFAULT";
// Commenting to save all presets on the same file
//		if ( preset )
//			f += std::string("_preset") + preset;
		f += "." + fileExt;
		return f;
	}
	std::string ciConfig::makeXmlNodeName(char preset)
	{
		return ( preset ? std::string("preset_") + preset : "default" );
	}
	bool ciConfig::getPresetXml(char preset, XmlTree & doc)
	{
		std::string f = this->makeFileName( mFolderDefault, preset );
		return this->getPresetXml( f, doc );
	}
	bool ciConfig::getPresetXml(std::string f, XmlTree & doc)
	{
		if ( ! fs::exists( f ) )
			return false;
		try {
			doc = XmlTree( loadFile( f ) );
		} catch (std::exception e) {
			console() << "---------------------------------------------" << std::endl;
			console() << " XML exception: " << e.what() << std::endl;
			console() << "---------------------------------------------" << std::endl;
			return false;
		}
		return true;
	}

	
	/////////////////////////////////////////////////////////////////////////
	//
	// SAVE / READ config to file
	//
	// SAVE to file on data folder
	// Return saved params
	int ciConfig::readFile(const std::string & f, char preset)
	{
		XmlTree doc;
		if ( ! this->getPresetXml(f, doc) )
		{
			// Not XML! try old format...
			return readFile_old( f, preset );
		}

		//
		// Read node "config"
		XmlTree node;
		std::string nodeName = "config/" + this->makeXmlNodeName( preset );
		if ( ! doc.hasChild( nodeName ) )
		{
			printf("-------------- READ XML ERROR! node not found [%s]\n",nodeName.c_str());
			return 0;
		}
		node = doc.getChild( nodeName );
		
		// reset load status
		this->reset();

		for( XmlTree::Iter child = node.begin(); child != node.end(); ++child )
		{
			std::string name = child->getTag();
			//std::string value = child->getValue();
			std::string value = child->getAttribute("value");
			bool pp = ( child->hasAttribute("prog") ? (child->getAttribute("prog").getValue() == "true") : false );
			
			//
			// Find param
			// Read params
			for (int pid = 0 ; pid < params.size() ; pid++ )
			{
				ciConfigParam *param = params[pid];
				if (param == NULL)
					continue;
				if ( name == param->name && ! param->dummy )
				{
#ifdef VERBOSE
					printf ("READ XML param %d: %s = %s\n",pid,name.c_str(),value.c_str());
#endif
					this->preserveProg( pid, pp );
					this->set(pid, value.c_str(), pp);
					this->setLoaded(pid, true);
					break;
				}
			}
		}
		
		
#ifdef VERBOSE
		//console() << doc << std::endl;
#endif

		//
		// SAVE!!!
//		doc.write( writeFile( f ) );
		printf("-------------- READ [%s]\n",f.c_str());
		printf("-------------- READ %d params OK!\n",(int)params.size());
		sprintf(errmsg, "READ [%s]", f.c_str());

		if ( postLoadCallback_fn )
			postLoadCallback_fn(this);

		// remember file
		this->setDisplayFilename( f, preset );

		// return param count
		return (int) params.size();
	}
	//
	// SAVE to file on data folder
	// Return saved params
	int ciConfig::saveFile(const std::string & f, char preset)
	{
		XmlTree doc;
		if ( ! this->getPresetXml(f, doc) )
			printf("-------------- SAVE XML ERROR! NOT XML!\n");
		
		// TODO:: TEST IF LOADED A XML!!!
		
		if ( doc.getNodeType() != XmlTree::NODE_DOCUMENT )
			doc = XmlTree::createDoc();

		// Get Nodes
		XmlTree rootNode_new, node_new;
		XmlTree * rootNode, * node;		// must be pointer because of getChild()
		std::string nodeName = this->makeXmlNodeName( preset );
		if ( doc.hasChild( "config" ) )
			rootNode = &doc.getChild( "config");
		else
		{
			rootNode_new = XmlTree( "config", "" );
			rootNode = &rootNode_new;
		}
		if ( rootNode->hasChild( nodeName ) )
			node = &rootNode->getChild( nodeName );
		else
		{
			node_new = XmlTree( nodeName, "" );
			node = &node_new;
		}

		// Save date
		time_t now;
		time ( &now );
		std::string date = string( ctime(&now) );
		date[date.length()-1] = '\0';	// removes '\n'
		node->setAttribute( "SAVE_TIME", date );
		
		// Save params
		for ( int id = 0 ; id < params.size() ; id++ )
		{
			ciConfigParam *param = params[id];
			if ( param == NULL )
				continue;
			//if ( param->dummy )
			//	continue;
			std::string name = this->getName(id);
			std::string value = this->getString(id, true);
			// Update ttribute
			XmlTree p_new;
			XmlTree * p;		// must be pointer necause of getChild()
			if ( node->hasChild( name ) )
				p = &node->getChild( name );
			else
			{
				p_new = XmlTree( name, "" );
				p = &p_new;
			}
			p->setAttribute( "value", value );
			// prog?
			if ( param->dummy )
				p->setAttribute( "dummy", "true" );
			// prog?
			if ( param->vec[0].preserveProg )
				p->setAttribute( "prog", "true" );
			// New?
			if ( ! p->hasParent() )
				node->push_back( *p );
		}

		// New Xml?
		if ( ! node->hasParent() )
			rootNode->push_back( *node );
		if ( ! rootNode->hasParent() )
			doc.push_back( *rootNode );

#ifdef VERBOSE
		console() << doc << std::endl;
#endif

		//
		// SAVE FILE!!
		doc.write( writeFile( f ) );
		printf("-------------- SAVE [%s]\n",f.c_str());
		printf("-------------- SAVED %d params OK!\n",(int)params.size());
		sprintf(errmsg, "SAVED [%s]", f.c_str());
		
		// remember file
		this->setDisplayFilename( f, preset );
		
		// return param count
		return (int) params.size();
 
	}
	//
	// Set the display filename from a saved/loaded file
	void ciConfig::setDisplayFilename(const std::string & f, char preset)
	{
		if ( IS_IN_DEFAULTS(f) )
		{
			if (preset)
				mDisplayFileName = std::string("< snapshot ") + preset + " >";
			else
				mDisplayFileName = "< default >";
		}
		else
		{
			std::string ext = getPathExtension( f );
			std::string ff = getPathFileName(f);
			ff = ff.substr(0,ff.length()-ext.length()-1);
			mDisplayFileName = "[ " + ff + " ]";
		}
	}



	
	
	/////////////////////////////////////////////////////////////////////
	//
	// OLD FILE FORMAT!!!
	//
	// Save comments
	//fputs("#\n",fp);
	//fputs("# ciConfig file\n",fp);
	//fputs("#\n",fp);
	//fputs("# Format:\n",fp);
	//fputs("#\tPARAM_NAME[.channel]:[P]value\n",fp);
	//fputs("# Examples:\n",fp);
	//fputs("#\tPARAM_VALUE:10.0\t(parameter with value)\n",fp);
	//fputs("#\tPARAM_PROG:P1.0\t\t(parameter with prog: from 0.0(vmin) to 1.0(vmax))\n",fp);
	//fputs("#\tPARAM_VECTOR2:150.0,150.0\t(vector: x,y)\n",fp);
	//fputs("#\tPARAM_VECTOR2:P0.5,0.5\t\t(vector with prog: x,y)\n",fp);
	//fputs("#\tPARAM_VECTOR3:150.0,150.0,0.0\t(vector: x,y,z)\n",fp);
	//fputs("#\tPARAM_VECTOR3:P0.5,0.5,0.0\t\t(vector with prog: x,y,z)\n",fp);
	//fputs("#\tPARAM_VECTOR4:150.0,150.0,0.0\t(vector: x,y,z,w)\n",fp);
	//fputs("#\tPARAM_VECTOR4:P0.5,0.5,0.0,0.0\t\t(vector with prog: x,y,z,w)\n",fp);
	//fputs("#\tPARAM_COLOR:255,255,255\t\t\t(color: r,g,b)\n",fp);
	//fputs("#\tPARAM_COLOR:P0.0,0.5,1.0\t\t(color with prog: r,g,b)\n",fp);
	//fputs("#\n",fp);
	int ciConfig::readFile_old(const std::string & f, char preset)
	{
		// Open file
		FILE *fp = fopen (f.c_str(),"r");
		if (fp == NULL)
		{
			printf("ERROR reading config file [%s] errno [%d/%s]\n",f.c_str(),errno,strerror(errno));
			sprintf(errmsg, "READ ERROR! [%s]", f.c_str());
			return 0;
		}
		printf("-------------- READ config [%s]...\n",f.c_str());
		
		// Here we go!
		bStarted = true;
		int pid;
		bool pp;						// is prog?
		char *p;						// aux pointer
		char data[CFG_MAX_DATA_LEN];	// full line
		std::string key;				// param key
		std::string val;				// param value
		
		// reset load status
		for (pid = 0 ; pid < params.size() ; pid++ )
			if(params[pid] != nullptr)
				this->setLoaded(pid, false);
		
		// Read file
		for (int n = 0 ; fgets(data, sizeof(data) ,fp) != NULL ; n++)
		{
			// Comment?
			if (data[0] == '#')
				continue;
			// erase '\n';
			data[strlen(data)-1] = '\0';
			// find separator
			p = strchr(data,':');
			if (p == NULL)
				continue;
			// get val
			if ( (pp = (*(p+1)=='P')) == true)
				val = string(p+2);
			else
				val = string(p+1);
			// get key
			*p = '\0';
			key = data;
			
			// Read params
			for (pid = 0 ; pid < params.size() ; pid++ )
			{
				ciConfigParam *param = params[pid];
				if (param == NULL)
					continue;
				if ( key == param->name )
				{
#ifdef VERBOSE
					printf ("READ CFG param %d: %s = %s\n",pid,key.c_str(),val.c_str());
#endif
					this->preserveProg(pid, pp);
					this->set(pid, val.c_str(), pp);
					this->setLoaded(pid, true);
					break;
				}
			}
			
#ifdef VERBOSE
			// finish output
			if (pid == params.size() && key != "SAVE_TIME")
				printf ("  !!!!!!! param do not exist!\n");
			else
				printf ("\n");
#endif
			
			// Is saved time?
			if (key == "SAVE_TIME")
				saveTime = val;
		}
		// Virtual Callback
		this->postLoadCallback();
		
		// Close file
		fclose (fp);
		printf("-------------- READ OK! %d params\n",(int)params.size());
		sprintf(errmsg, "READ [%s]", f.c_str());
		
		// remember file
		this->setDisplayFilename( f, preset );
		
		// return param count
		return (int) params.size();
	}

	
	
	
	
} // namespace cinder
