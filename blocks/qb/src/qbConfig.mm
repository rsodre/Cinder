//
//  qbConfig.cpp
//
//  Created by Roger Sodre on 01/07/2011
//  Copyright 2011 Studio Avante. All rights reserved.
//

#import <Foundation/Foundation.h>
#include <sstream>
#include "qbConfig.h"
#include "qb.h"

using namespace ci;
using namespace ci::sgui;
using namespace sysinfo;

PanelControl *pc3, *pc4, *pc5;

//
// Config Class
qbConfig::qbConfig() : ciConfigGui()
{
	pc3 = pc4 = pc5 = NULL;
	columnQB = columnRender = columnPalette = NULL;
	buttonPlaySwitch = buttonRenderSwitch = NULL;
	panelPaletteColors = panelPaletteReduction = NULL;
	controlRenderTexture = NULL;
}

//
// Refresh GUI from
void qbConfig::setup() {
	// View
	this->addInt(QBCFG_CURRENT_TAB, "QBCFG_CURRENT_TAB", 0, 0, 50 );
	this->addInt(QBCFG_CURRENT_DEFAULT, "QBCFG_CURRENT_DEFAULT", 0, 0, MAX_DEFAULTS );
	// Camera
	this->addFloat(QBCFG_OPACITY, "QBCFG_OPACITY", 1.0f, 0.0f, 1.0f );
	this->addFloat(QBCFG_SCALE, "QBCFG_SCALE", 1.0f, 0.0f, 2.0f );
	this->addVector2(QBCFG_OFFSET, "QBCFG_OFFSET", Vec2f::zero(), -0.5f, 0.5f );
	this->addInt(QBCFG_CAMERA_TYPE, "QBCFG_CAMERA_TYPE", _qb.mDefaultCamera, 0, CAMERA_TYPE_COUNT-1);
	this->addBool(QBCFG_CAMERA_GROUND, "QBCFG_CAMERA_GROUND", false);
	this->addFloat(QBCFG_CAMERA_GROUND_SHIFT, "QBCFG_CAMERA_GROUND_SHIFT", 1.0f, 0.0f, 5.0f);
	this->addFloat(QBCFG_CAMERA_ANGLE, "QBCFG_CAMERA_ANGLE", 0.0f, 0.0f, 90.0f);
	this->addFloat(QBCFG_METRIC_THROW, "QBCFG_METRIC_THROW", 1.0f, 0.1f, 1.2f);
	this->addFloat(QBCFG_PERSPECTIVE_PROG, "QBCFG_PERSPECTIVE_PROG", 1.0f, 0.0f, 1.0f);
	// Animation
	this->addBool(QBCFG_PLAYING, "QBCFG_PLAYING", true);
	this->addBool(QBCFG_PLAY_BACKWARDS, "QBCFG_PLAY_BACKWARDS", false);
	this->addBool(QBCFG_REALTIME_PREVIEW, "QBCFG_REALTIME_PREVIEW", true);
	this->addBool(QBCFG_PREVIEW_DOWNSCALE, "QBCFG_PREVIEW_DOWNSCALE", true);
	this->addBool(QBCFG_PREVIEW_UPSCALE, "QBCFG_PREVIEW_UPSCALE", false);
	this->addFloat(QBCFG_CURRENT_TIME, "QBCFG_CURRENT_TIME", 0.0);
	this->addFloat(QBCFG_CURRENT_PROG, "QBCFG_CURRENT_PROG", 0.0, 0.0, 1.0);
	this->setDummy(QBCFG_CURRENT_PROG);
	this->setReadOnly(QBCFG_CURRENT_PROG,false);
	// Render
	this->addBool(QBCFG_RENDER_OPTIONS, "QBCFG_RENDER_OPTIONS", false);
	this->setDummy(QBCFG_RENDER_OPTIONS);
	this->addBool(QBCFG_PRESERVE_ALPHA, "QBCFG_PRESERVE_ALPHA", false);
	this->addBool(QBCFG_RENDER_PNG_SEQUENCE, "QBCFG_RENDER_PNG_SEQUENCE", false);
	this->addInt(QBCFG_RENDER_FRAMERATE, "QBCFG_RENDER_FRAMERATE", 30, 1, 60);
	this->addFloat(QBCFG_RENDER_QUALITY, "QBCFG_RENDER_QUALITY", 0.75f, 0.5f, 1.0f);
	this->addInt(QBCFG_RENDER_SECONDS, "QBCFG_RENDER_SECONDS", 10, 0, 120);
	this->addInt(QBCFG_RENDER_STILL_SECONDS, "QBCFG_RENDER_STILL_SECONDS", 0, 0, 60);
	this->addBool(QBCFG_FIT_SOURCES_TO_RENDER, "QBCFG_FIT_SOURCES_TO_RENDER", true);
	this->addInt(QBCFG_MODUL8_INPUT, "QBCFG_MODUL8_INPUT", 0, 0, MODUL8_INPUT_COUNT);
	this->addBool(QBCFG_SYPHON_OUTPUT, "QBCFG_SYPHON_OUTPUT", false);
	// PalettePlasma
	this->addBool(QBCFG_PALETTE_FLAG, "QBCFG_PALETTE_FLAG", true );
	this->addInt(QBCFG_PALETTE_NET_STATE, "QBCFG_PALETTE_NET_STATE", 0, 0, PALETTE_NET_COUNT-1 );
	this->addString(QBCFG_PALETTE_NET_HOST, "QBCFG_PALETTE_NET_HOST", "localhost" );
	this->addInt(QBCFG_PALETTE_NET_PORT, "QBCFG_PALETTE_NET_PORT", 11001, 0, 99999 );
	this->addInt(QBCFG_PALETTE_COUNT, "QBCFG_PALETTE_COUNT", QB_PALETTE_COUNT, 1, QB_PALETTE_COUNT );
	this->addFloat(QBCFG_PALETTE_COMPRESS, "QBCFG_PALETTE_COMPRESS", 0.0f );
	this->addFloat(QBCFG_PALETTE_MIN, "QBCFG_PALETTE_MIN", 0.2f, 0.0f, 1.0f );
	this->addFloat(QBCFG_PALETTE_MAX, "QBCFG_PALETTE_MAX", 0.8f, 0.0f, 1.0f );
	this->addColor(QBCFG_PALETTE_1, "QBCFG_PALETTE_1", Color8u::gray((255/4)*4) );
	this->addColor(QBCFG_PALETTE_2, "QBCFG_PALETTE_2", Color8u::gray((255/4)*3) );
	this->addColor(QBCFG_PALETTE_3, "QBCFG_PALETTE_3", Color8u::gray((255/4)*2) );
	this->addColor(QBCFG_PALETTE_4, "QBCFG_PALETTE_4", Color8u::gray((255/4)*1) );
	this->addColor(QBCFG_PALETTE_5, "QBCFG_PALETTE_5", Color8u::gray((255/4)*0) );
	this->addBool(QBCFG_PERLIN_FLAG, "QBCFG_PERLIN_FLAG", true );
	this->addBool(QBCFG_PERLIN_GRAYSCALE, "QBCFG_PERLIN_GRAYSCALE", false );
	this->addInt(QBCFG_PERLIN_OCTAVE, "QBCFG_PERLIN_OCTAVE", 1, 1, 16 );
	this->addFloat(QBCFG_PERLIN_SPEED, "QBCFG_PERLIN_SPEED", 1.0f, 0.0f, 5.0f );
	this->addFloat(QBCFG_PERLIN_FREQ, "QBCFG_PERLIN_FREQ", 5.0f, 0.0f, 10.0f );
	this->addFloat(QBCFG_PERLIN_COMPRESS, "QBCFG_PERLIN_COMPRESS", 0.0f );
	this->addVector2(QBCFG_PERLIN_OFFSET, "QBCFG_PERLIN_OFFSET", Vec2f::zero(), 0.0f, 10.0f );
	this->addVector2(QBCFG_PERLIN_SCALE, "QBCFG_PERLIN_SCALE", Vec2f::one(), 1.0f, 10.0f );
	this->addFloat(QBCFG_PALETTE_REDUCE_TIME, "QBCFG_PALETTE_REDUCE_TIME", 0.0f, 0.0f, 60.0f );
	this->setDummy( QBCFG_PALETTE_REDUCE_TIME );
	
	// Readonly
	this->addString(DUMMY_APP_VERSION, "DUMMY_APP_VERSION", SysInfo::getAppVersionLong());
	this->addString(DUMMY_OS_VERSION, "DUMMY_OS_VERSION", SysInfo::getOsVersion());
	this->addString(DUMMY_GPU_VENDOR, "DUMMY_GPU_VENDOR", SysInfo::getGpuVendor());
	this->addString(DUMMY_GPU_MODEL, "DUMMY_GPU_MODEL", SysInfo::getGpuModel());
	this->addString(DUMMY_GPU_TEX_SIZE, "DUMMY_GPU_TEX_SIZE", SysInfo::getGpuTexSize());
	this->addString(DUMMY_GPU_FBO_WIDTH, "DUMMY_GPU_FBO_WIDTH", SysInfo::getGpuFboWidth());
	this->addString(DUMMY_GPU_FBO_HEIGHT, "DUMMY_GPU_FBO_HEIGHT", SysInfo::getGpuFboHeight());
	this->addString(DUMMY_GL_VERSION, "DUMMY_GL_VERSION", SysInfo::getGlVersion());
	this->addString(DUMMY_GLSL_VERSION, "DUMMY_GLSL_VERSION", SysInfo::getGlslVersion());
	this->addString(DUMMY_RAM_AVAILABLE, "DUMMY_RAM_AVAILABLE", SysInfo::getRam());
	this->addString(DUMMY_PROCESSOR, "DUMMY_PROCESSOR", SysInfo::getProcessor());
	this->addString(DUMMY_MODEL, "DUMMY_MODEL", SysInfo::getCpuModel());
	this->addString(DUMMY_CORES, "DUMMY_CORES", SysInfo::getCpuCores());
	this->addString(DUMMY_GPU_RAM, "DUMMY_GPU_RAM", SysInfo::getGpuRam());
	this->addInt(DUMMY_RENDER_WIDTH, "DUMMY_RENDER_WIDTH", 0);
	this->addInt(DUMMY_RENDER_HEIGHT, "DUMMY_RENDER_HEIGHT", 0);
	this->addInt(DUMMY_QB_WIDTH, "DUMMY_QB_WIDTH", 0);
	this->addInt(DUMMY_QB_HEIGHT, "DUMMY_QB_HEIGHT", 0);
	this->addInt(DUMMY_QB_THROW, "DUMMY_QB_THROW", 0);
	this->addFloat(DUMMY_MOUSE_X, "DUMMY_MOUSE_X", 0, -10000.0, 10000.0);
	this->addFloat(DUMMY_MOUSE_Y, "DUMMY_MOUSE_Y", 0, -10000.0, 10000.0);
	this->addFloat(DUMMY_PREVIEW_SCALE, "DUMMY_PREVIEW_SCALE", 1.0, 0.01, 1000.0);
	this->addFloat(DUMMY_CURRENT_FPS, "DUMMY_CURRENT_FPS", 0.0);
	this->addString(DUMMY_CURRENT_FRAME, "DUMMY_CURRENT_FRAME", "0/0");
	this->addString(DUMMY_RENDER_TIME_REMAINING, "DUMMY_RENDER_TIME_REMAINING", "");
	this->addString(DUMMY_RENDER_TIME_ELAPSED, "DUMMY_RENDER_TIME_ELAPSED", "");
	this->addString(DUMMY_RENDER_TIME_ESTIMATED, "DUMMY_RENDER_TIME_ESTIMATED", "");
	this->addString(DUMMY_RENDER_FRAMES, "DUMMY_RENDER_FRAMES", "");
	this->addString(DUMMY_RENDER_STATUS, "DUMMY_RENDER_STATUS", "");
	//
	this->setDummy(DUMMY_APP_VERSION);
	this->setDummy(DUMMY_OS_VERSION);
	this->setDummy(DUMMY_GPU_VENDOR);
	this->setDummy(DUMMY_GPU_MODEL);
	this->setDummy(DUMMY_GPU_TEX_SIZE);
	this->setDummy(DUMMY_GL_VERSION);
	this->setDummy(DUMMY_GLSL_VERSION);
	this->setDummy(DUMMY_RAM_AVAILABLE);
	this->setDummy(DUMMY_PROCESSOR);
	this->setDummy(DUMMY_MODEL);
	this->setDummy(DUMMY_CORES);
	this->setDummy(DUMMY_GPU_RAM);
	//
	this->setDummy(DUMMY_RENDER_WIDTH);
	this->setDummy(DUMMY_RENDER_HEIGHT);
	this->setDummy(DUMMY_QB_WIDTH);
	this->setDummy(DUMMY_QB_HEIGHT);
	this->setDummy(DUMMY_QB_THROW);
	this->setDummy(DUMMY_MOUSE_X);
	this->setDummy(DUMMY_MOUSE_Y);
	this->setDummy(DUMMY_PREVIEW_SCALE);
	this->setDummy(DUMMY_CURRENT_FPS);
	this->setDummy(DUMMY_CURRENT_FRAME);
	this->setDummy(DUMMY_RENDER_TIME_REMAINING);
	this->setDummy(DUMMY_RENDER_TIME_ELAPSED);
	this->setDummy(DUMMY_RENDER_TIME_ESTIMATED);
	this->setDummy(DUMMY_RENDER_FRAMES);
	this->setDummy(DUMMY_RENDER_STATUS);
	
	
	//
	// Midi
	//
	// sliders
	//this->setMidi(XXXXXXX, 2);
	//this->setMidi(XXXXXXX, 3);
	//this->setMidi(XXXXXXX, 4);
	//this->setMidi(XXXXXXX, 5);
	//this->setMidi(XXXXXXX, 12);
	//this->setMidi(XXXXXXX, 13);
	//
	// knobs
	//this->setMidi(CFG_TRACE, 14);
	//this->setMidi(XXXXXXX, 15);
	//this->setMidi(XXXXXXX, 16);
	//this->setMidi(CFG_TINT_COLOR, 17, 18, 19);
	//this->setMidi(CFG_ADD_COLOR, 20, 21, 22);
	//
	// buttons
	//this->setMidiSwitch(XXXXXXX, 34);
	
	
	//////////////////////////////////////
	//
	// SETUP GUI
	//
	unsigned char ccmd = 174;				// (R) = COMMAND SIGN for pf_tempesta_seven_ROGER.ttf
	std::string cmd = toString(ccmd);
	//this->mGui->thumbnailSize = QBCFG_THUMB_SIZE;
	FloatVarControl *cf;
	ColorVarControl *cc;
	
	/////////////
	//
	// QB TAB
	mGui->setName(_qb.getScreenName());
	tabQB = mGui->addTab("QB");
	
	/////////////
	//
	// QB Column
	//columnQB = (ColumnControl*) this->guiAddGroup("> QB :: "+_qb.getScreenName());
	columnQB = (ColumnControl*) this->guiAddGroup("> QB");
	// Readonly
	this->guiAddParam(DUMMY_RENDER_WIDTH,			"Render Width" );
	this->guiAddParam(DUMMY_RENDER_HEIGHT,			"Render Height" );
	this->guiAddParam(DUMMY_QB_WIDTH,				"Metric Width" );
	this->guiAddParam(DUMMY_QB_HEIGHT,				"Metric Height" );
	this->guiAddParam(DUMMY_QB_THROW,				"Metric Throw" );
	this->guiAddParam(DUMMY_PREVIEW_SCALE,			"Preview Scale", 2 );
	this->guiAddParam(DUMMY_MOUSE_X,				"Mouse X", (_qb.getMetricScale() != 1.0 ? 2 : 0) );
	this->guiAddParam(DUMMY_MOUSE_Y,				"Mouse Y", (_qb.getMetricScale() != 1.0 ? 2 : 0) );
	//this->guiAddParam(DUMMY_GPU,					"GPU" );
	this->guiAddParam(DUMMY_GL_VERSION,				"OpenGL" );
	this->guiAddParam(DUMMY_GLSL_VERSION,			"GLSL" );
	// Load/Save
	this->guiAddSeparator();
	//this->guiAddButton("Load Config", cmd+"L",		(ciConfigGui*)this, (bool(ciConfigGui::*)(ci::app::MouseEvent&)) &qbConfig::cbLoad)->setTriggerUp();
	this->guiAddButton("Load Config", cmd+"L",		this, &qbConfig::cbLoad);
	this->guiAddButton("Save Config", cmd+"S",		this, &qbConfig::cbSave);
	// view params
	this->guiAddSeparator();
	this->guiAddParam(QBCFG_PREVIEW_DOWNSCALE,		"Downscale Preview");
	this->guiAddParam(QBCFG_PREVIEW_UPSCALE,		"Upscale Preview");
	this->guiAddParam(QBCFG_REALTIME_PREVIEW,		"Realtime Preview");
	// Camera
	this->guiAddSeparator();
	this->setValueLabels(QBCFG_CAMERA_TYPE, LABELS_CAMERA_TYPE);
	//this->setFlag(QBCFG_CAMERA_TYPE, CFG_FLAG_DROP_DOWN);
	this->guiAddParam(QBCFG_CAMERA_TYPE,			"> CAMERA" );
	this->guiAddParam(QBCFG_PERSPECTIVE_PROG,		"Perspective Scale", 2 );
	this->guiAddParam(QBCFG_METRIC_THROW,			"Camera Throw", 2 );
	this->guiAddParam(QBCFG_CAMERA_GROUND,			"Camera on Ground" );
	this->guiAddParam(QBCFG_CAMERA_GROUND_SHIFT,	"Ground Shift" );
	this->guiAddParam(QBCFG_CAMERA_ANGLE,			"Camera Angle" );
	this->guiAddParam(QBCFG_SCALE,					"Scale", 2 );
	this->guiAddParam(QBCFG_OFFSET,					"Offset", 3 );
	// Syphon
	panelSyphon = this->mGui->addPanel("panelSyphon");
	this->guiAddSeparator();
	this->guiAddText("> SYPHON");
	this->setValueLabels(QBCFG_MODUL8_INPUT, LABELS_MODUL8_INPUT);
	this->setFlag(QBCFG_MODUL8_INPUT, CFG_FLAG_DROP_DOWN);
	this->guiAddParam(QBCFG_MODUL8_INPUT,			"" );
	this->guiAddParam(QBCFG_SYPHON_OUTPUT,			"Syphon Output" );
	//
	// Info
	this->mGui->addPanel("fps");
	this->guiAddSeparator();
	this->guiAddParam(DUMMY_CURRENT_FPS,			"Current FPS", 1 );

	/////////////
	//
	// ANIMATION + RENDER Column
	//
	// QB Tab
	columnRender = (ColumnControl*) this->guiAddGroup("> QB ANIMATION");
	//
	// Animation
	cf = this->guiAddParamFLoat(QBCFG_CURRENT_TIME,	"Current Time", 2 );
	cf->setFormatAsTimecode()->setReadOnly();
	this->guiAddParam(DUMMY_CURRENT_FRAME,			"Current Frame" );
	// play/pause panels
	this->guiAddSeparator();
	this->guiAddParamFLoat(QBCFG_CURRENT_PROG,	"Playhead", 3 )->setFormatAsPercentage();
	buttonPlaySwitch = this->guiAddButton("Pause", "space",	this, &qbConfig::cbPlaySwitch);
	this->guiAddButton("Rewind", "/",						this, &qbConfig::cbRewind);
	// info
	this->guiAddParam(QBCFG_RENDER_SECONDS,			"Animation Seconds");
	this->guiAddParam(QBCFG_FIT_SOURCES_TO_RENDER,	"Fit Sources Time");
	this->guiAddParam(QBCFG_PLAY_BACKWARDS,			"Play Backwards");
	//
	// Render Options
	this->guiAddSeparator();
	//c = (Control*) this->guiAddParam(QBCFG_RENDER_OPTIONS,	"> RENDER OPTIONS");
	//c->addSwitchPanel("renderOptions");
	this->guiAddText("> RENDER OPTIONS");
	this->guiAddParam(QBCFG_PRESERVE_ALPHA,			"Preserve Alpha");
	this->guiAddParam(QBCFG_RENDER_PNG_SEQUENCE,	"PNG Sequence");
	this->guiAddParam(QBCFG_RENDER_FRAMERATE,		"Render Framerate");
	this->guiAddParamFLoat(QBCFG_RENDER_QUALITY,			"Render Quality", 2)->setFormatAsPercentage();
	//
	// Render
	this->mGui->addPanel("renderControls");
	this->guiAddSeparator();
	this->guiAddText("> RENDER CONTROL");
	this->guiAddButton("Save Screenshot",			this, &qbConfig::cbScreenshot);
	buttonRenderSwitch = this->guiAddButton("Start Render", cmd+"X", this, &qbConfig::cbRenderSwitch);
	this->guiAddButton("Save Render", cmd+"Z",		this, &qbConfig::cbRenderFinish);
	// Render texture
	controlRenderTexture = (TextureVarControl*) this->guiAddTexture("", &mNullTexture, 0);
	this->guiAddParam(DUMMY_RENDER_TIME_ESTIMATED,	"Estimated:");
	this->guiAddParam(DUMMY_RENDER_TIME_ELAPSED,	"Elapsed:");
	this->guiAddParam(DUMMY_RENDER_TIME_REMAINING,	"Remaining:");
	this->guiAddParam(DUMMY_RENDER_FRAMES,			"Frames:");
	this->guiAddParam(DUMMY_RENDER_STATUS,			"Status")->setImportant();

	//
	// QB Palette
	columnPerlinSource = (ColumnControl*) this->guiAddGroup("> QB PERLIN SOURCE");
	//
	// Perlin generator
	this->mGui->addPanel("panel_palette_perlin");
	//this->guiAddParam( QBCFG_PERLIN_FLAG, "> SOURCE PERLIN" );
	controlPaletteTexture = this->guiAddTexture("", &mTexturePalette, 0)->setAlpha(false);
	this->guiAddParam(QBCFG_PERLIN_OCTAVE,		"Octave");
	this->guiAddParam(QBCFG_PERLIN_SPEED,		"Speed", 3);
	this->guiAddParam(QBCFG_PERLIN_FREQ,		"Frequency", 2);
	this->guiAddParam(QBCFG_PERLIN_COMPRESS,	"Compress", 2);
	this->guiAddParam(QBCFG_PERLIN_OFFSET,		"Offset", 1);
	this->guiAddParam(QBCFG_PERLIN_SCALE,		"Scale", 1);
	/*
	 bc = (BoolVarControl*) this->guiAddParam(QBCFG_PALETTE_FLAG, "Enabled");
	 pc = this->mGui->addPanel("panel_palette");
	 pc->column = true;
	 bc->switchPanel( pc );
	 */
#ifdef QB_COLOR_REDUCTION
	Control *c;
	//
	// QB Palette
	columnPalette = (ColumnControl*) this->guiAddGroup("> QB PALETTE");
	//
	// Network
	this->setValueLabels(QBCFG_PALETTE_NET_STATE, LABELS_PALETTE_NET);
	this->setFlag(QBCFG_PALETTE_NET_STATE, CFG_FLAG_DROP_DOWN);
	c = (Control*) this->guiAddParam(QBCFG_PALETTE_NET_STATE, "Network");
	c->addSwitchPanel("panel_palette_net");
	this->guiAddParam(QBCFG_PALETTE_NET_HOST,		"Host");
	this->guiAddParam(QBCFG_PALETTE_NET_PORT,		"IP Port")->setReadOnly();
	//
	// Color Reduction
	panelPaletteReduction = this->mGui->addPanel("panel_palette_reduce");
	this->guiAddSeparator();
	this->guiAddText("> COLOR REDUCTION");
	this->guiAddParam(QBCFG_PALETTE_MIN,			"Min Brightness");
	this->guiAddParam(QBCFG_PALETTE_MAX,			"Max Brightness");
	this->guiAddParam( QBCFG_PALETTE_REDUCE_TIME,	"Reduce Time", 4);
#endif	//QB_COLOR_REDUCTION
	//
	// Palette Colors
	panelPaletteColors = this->mGui->addPanel("panel_palette_colors");
	panelPaletteColors->column = true;
	this->guiAddSeparator();
	this->guiAddText("> COLORS");
	this->guiAddParam(QBCFG_PERLIN_GRAYSCALE,		"Grayscale");
	this->guiAddParam(QBCFG_PALETTE_COUNT,			"Color Count");
	this->guiAddParam(QBCFG_PALETTE_COMPRESS,		"Color Compress");
	cc = (ColorVarControl*) this->guiAddParam(QBCFG_PALETTE_1,	"Color 1");
	cc->displayValue = true;
	// color 2
	this->guiAddButton("vv swap down vv", this, &qbConfig::cbPalette2Switch);
	cc = (ColorVarControl*) this->guiAddParam(QBCFG_PALETTE_2,	"Color 2");
	cc->displayValue = true;
	// color 3
	pc3 = this->mGui->addPanel("panel_palette_3");
	//pc3->column = true;
	this->guiAddButton("vv swap down vv", this, &qbConfig::cbPalette3Switch);
	cc = (ColorVarControl*) this->guiAddParam(QBCFG_PALETTE_3,	"Color 3");
	cc->displayValue = true;
	// color 4
	pc4 = this->mGui->addPanel("panel_palette_4");
	//pc4->column = true;
	this->guiAddButton("vv swap down vv", this, &qbConfig::cbPalette4Switch);
	cc = (ColorVarControl*) this->guiAddParam(QBCFG_PALETTE_4,	"Color 4");
	cc->displayValue = true;
	// color 5
	pc5 = this->mGui->addPanel("panel_palette_5");
	//pc5->column = true;
	this->guiAddButton("vv swap down vv", this, &qbConfig::cbPalette5Switch);
	cc = (ColorVarControl*) this->guiAddParam(QBCFG_PALETTE_5,	"Color 5");
	cc->displayValue = true;
	
	// Read last config
	// TODO:: fazer useFile() funcionar novamente
	std::stringstream os;
	//os << QB_APP_NAME << "_qb.cfg";
	//this->useFile( os.str().c_str() );
	os << QB_APP_NAME << "_qb";
	this->mAppName = os.str();
	this->load();
	//this->makePresets();
}

//
// Refresh GUI from
void qbConfig::update() {
	// Call Suuuper
	this->ciConfig::update();

	// buttons
	if ( buttonPlaySwitch )
		buttonPlaySwitch->name = ( _qb.isPlaying() ? "Pause" : "Play" );
	if ( buttonRenderSwitch )
		buttonRenderSwitch->name = ( _renderer.isRendering() ? "Stop Render" : "Start Render" );
	
	// Render Labels
	this->set( DUMMY_RENDER_FRAMES,				_renderer.getFramestring() );
	this->set( DUMMY_RENDER_TIME_REMAINING,		_renderer.getTimeRemainingString() );
	this->set( DUMMY_RENDER_TIME_ELAPSED,		_renderer.getTimeElapsedString() );
	this->set( DUMMY_RENDER_TIME_ESTIMATED,		_renderer.getTimeEstimatedString() );
	this->guiSetName( DUMMY_RENDER_STATUS,		_renderer.getStatusString() );
	this->set( DUMMY_RENDER_STATUS,				_renderer.getProgString() );
}


///////////////////////////////////////////////////////////
//
// BUTTON CALLBACKS
//
// Save/Load
void qbConfig::cbLoad( ci::app::MouseEvent & event ) {
	/*
	std::vector<std::string> extensions;
	extensions.push_back("cfg");
	std::string path = app::getOpenFilePath( app::getAppPath()+"/..", extensions );
	if ( path.length() )
		this->readFile( path.c_str() );
	*/
	this->load();
	event.setHandled();
}
void qbConfig::cbSave( ci::app::MouseEvent & event ) {
	/*
	std::vector<std::string> extensions;
	extensions.push_back("cfg");
	std::string path = app::getSaveFilePath( app::getAppPath()+"/..", extensions );
	if ( path.length() )
		this->saveFile( path.c_str() );
	*/
	this->save();
	event.setHandled();
}

// Rewind Animation
void qbConfig::cbRewind( ci::app::MouseEvent & event ) {
	_qb.rewind();
	event.setHandled();
}

// Play/Stop
void qbConfig::cbPlaySwitch( ci::app::MouseEvent & event ) {
	_qb.playSwitch();
	event.setHandled();
}

// Render
void qbConfig::cbRenderSwitch( ci::app::MouseEvent & event ) {
	_renderer.startstop();
	event.setHandled();
}
void qbConfig::cbRenderFinish( ci::app::MouseEvent & event ) {
	_renderer.stop();
	_renderer.finish();
	event.setHandled();
}

// Screenshot
void qbConfig::cbScreenshot( ci::app::MouseEvent & event ) {
	_renderer.takeScreenshot();
	event.setHandled();
}

// Palette control
void qbConfig::cbPalette2Switch( ci::app::MouseEvent & event ) {
	this->paletteSwitchColors( QBCFG_PALETTE_1, QBCFG_PALETTE_2 );
	event.setHandled();
}
void qbConfig::cbPalette3Switch( ci::app::MouseEvent & event ) {
	this->paletteSwitchColors( QBCFG_PALETTE_2, QBCFG_PALETTE_3 );
	event.setHandled();
}
void qbConfig::cbPalette4Switch( ci::app::MouseEvent & event ) {
	this->paletteSwitchColors( QBCFG_PALETTE_3, QBCFG_PALETTE_4 );
	event.setHandled();
}
void qbConfig::cbPalette5Switch( ci::app::MouseEvent & event ) {
	this->paletteSwitchColors( QBCFG_PALETTE_4, QBCFG_PALETTE_5 );
	event.setHandled();
}
void qbConfig::paletteSwitchColors( int cfg1, int cfg2 ) {
	Color8u c1 = this->getColor( cfg1 );
	Color8u c2 = this->getColor( cfg2 );
	this->set( cfg1, c2 );
	this->set( cfg2, c1 );
}


//
// Render Texture
void qbConfig::setRenderTexture( gl::Texture * frame )
{
	if ( controlRenderTexture )
		controlRenderTexture->setTexture( frame ? frame : &mNullTexture );
}


//
// ciConfig CALLBACK
void qbConfig::postSetCallback(int id, int i)
{
	switch (id)
	{
		case QBCFG_CURRENT_DEFAULT:
			this->setCurrentDefault( this->getInt(QBCFG_CURRENT_DEFAULT) );
			break;
		case QBCFG_CAMERA_TYPE:
			_qb.setCameraType( this->get(QBCFG_CAMERA_TYPE) );
			break;
		case QBCFG_CAMERA_GROUND:
		case QBCFG_CAMERA_GROUND_SHIFT:
		case QBCFG_CAMERA_ANGLE:
		case QBCFG_METRIC_THROW:
		case QBCFG_PERSPECTIVE_PROG:
			_qb.resizeCameras();
			break;
		case QBCFG_PREVIEW_DOWNSCALE:
		case QBCFG_PREVIEW_UPSCALE:
			_qb.resizePreview();
			break;
		case QBCFG_PALETTE_COUNT:
			/*
			if ( pc3 )
				pc3->enabled = ( this->getInt(QBCFG_PALETTE_COUNT) < 3 ? false : true );
			if ( pc4 )
				pc4->enabled = ( this->getInt(QBCFG_PALETTE_COUNT) < 4 ? false : true );
			if ( pc5 )
				pc5->enabled = ( this->getInt(QBCFG_PALETTE_COUNT) < 5 ? false : true );
			 */
			break;
		case QBCFG_PALETTE_NET_STATE:
			_palette.setupOsc();
			break;
		case QBCFG_PALETTE_1:
			_palette.sendOsc(0);
			break;
		case QBCFG_PALETTE_2:
			_palette.sendOsc(1);
			break;
		case QBCFG_PALETTE_3:
			_palette.sendOsc(2);
			break;
		case QBCFG_PALETTE_4:
			_palette.sendOsc(3);
			break;
		case QBCFG_PALETTE_5:
			_palette.sendOsc(4);
			break;

	}
}
