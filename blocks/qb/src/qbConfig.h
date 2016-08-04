//
//	qbConfig.h
//
//  Created by Roger Sodre on 01/07/2011
//	Copyright 2011 Studio Avante. All rights reserved.
//
#pragma once

#include "ciConfigGui.h"

#define QB_PALETTE_COUNT			5

//////////////////////////////
//
// Config IDs
//
enum enumQBCfg
{
	QBCFG_NONE = -1,
	// ciConfig
	QBCFG_CURRENT_TAB,
	QBCFG_CURRENT_DEFAULT,
	// Camera
	QBCFG_CAMERA_TYPE,
	QBCFG_CAMERA_GROUND,
	QBCFG_CAMERA_GROUND_SHIFT,
	QBCFG_METRIC_THROW,
	QBCFG_PERSPECTIVE_PROG,
	QBCFG_OPACITY,
	QBCFG_SCALE,
	QBCFG_OFFSET,
	// Animation
	QBCFG_PLAYING,
	QBCFG_PLAY_BACKWARDS,
	QBCFG_REALTIME_PREVIEW,
	QBCFG_PREVIEW_DOWNSCALE,
	QBCFG_PREVIEW_UPSCALE,
	QBCFG_CURRENT_TIME,
	QBCFG_CURRENT_PROG,
	// Render
	QBCFG_RENDER_OPTIONS,
	QBCFG_PRESERVE_ALPHA,
	QBCFG_RENDER_PNG_SEQUENCE,
	QBCFG_RENDER_FRAMERATE,
	QBCFG_RENDER_QUALITY,
	QBCFG_RENDER_SECONDS,
	QBCFG_RENDER_STILL_SECONDS,
	QBCFG_FIT_SOURCES_TO_RENDER,
	QBCFG_MODUL8_INPUT,
	QBCFG_SYPHON_OUTPUT,
	// Palette
	QBCFG_PALETTE_FLAG,
	QBCFG_PALETTE_COUNT,
	QBCFG_PALETTE_MIN,
	QBCFG_PALETTE_MAX,
	QBCFG_PALETTE_COMPRESS,
	QBCFG_PALETTE_1,
	QBCFG_PALETTE_2,
	QBCFG_PALETTE_3,
	QBCFG_PALETTE_4,
	QBCFG_PALETTE_5,
	QBCFG_PERLIN_FLAG,
	QBCFG_PERLIN_GRAYSCALE,
	QBCFG_PERLIN_OCTAVE,
	QBCFG_PERLIN_SPEED,
	QBCFG_PERLIN_FREQ,
	QBCFG_PERLIN_OFFSET,
	QBCFG_PERLIN_SCALE,
	QBCFG_PERLIN_COMPRESS,
	QBCFG_PERLIN_TEXTURE,
	QBCFG_PALETTE_NET_STATE,
	QBCFG_PALETTE_NET_HOST,
	QBCFG_PALETTE_NET_PORT,
	QBCFG_PALETTE_REDUCE_TIME,
	//
	// READ ONLY
	DUMMY_APP_VERSION,
	DUMMY_OS_VERSION,
	DUMMY_GPU_VENDOR,
	DUMMY_GPU_MODEL,
	DUMMY_GPU_TEX_SIZE,
	DUMMY_GPU_FBO_WIDTH,
	DUMMY_GPU_FBO_HEIGHT,
	DUMMY_GL_VERSION,
	DUMMY_GLSL_VERSION,
	DUMMY_RAM_AVAILABLE,
	DUMMY_PROCESSOR,
	DUMMY_MODEL,
	DUMMY_CORES,
	DUMMY_GPU_RAM,
	DUMMY_RENDER_WIDTH,
	DUMMY_RENDER_HEIGHT,
	DUMMY_QB_WIDTH,
	DUMMY_QB_HEIGHT,
	DUMMY_QB_THROW,
	DUMMY_MOUSE_X,
	DUMMY_MOUSE_Y,
	DUMMY_PREVIEW_SCALE,
	DUMMY_CURRENT_FPS,
	DUMMY_CURRENT_FRAME,
	DUMMY_RENDER_STATUS,
	DUMMY_RENDER_FRAMES,
	DUMMY_RENDER_TIME_REMAINING,
	DUMMY_RENDER_TIME_ELAPSED,
	DUMMY_RENDER_TIME_ESTIMATED,
	// # of params
	QBCFG_COUNT
};

//
// Camera Types
enum enum_qbModul8Inputs {
	MODUL8_OFF,
	MODUL8_BELOW,
	MODUL8_ABOVE,
	// count
	MODUL8_INPUT_COUNT
};
#define LABELS_MODUL8_INPUT \
"Modul8 OFF",\
"Modul8 Below",\
"Modul8 Above",\
/*"Dome",*/\
NULL

//
// Camera Types
enum enum_qbCameraType {
	CAMERA_TYPE_ORTHO,
	CAMERA_TYPE_PERSP,
	CAMERA_TYPE_STEREO,
	//CAMERA_TYPE_FISHEYE,
	// count
	CAMERA_TYPE_COUNT
};
#define LABELS_CAMERA_TYPE \
"Orthographic",\
"Perspective",\
"Stereo",\
/*"Dome",*/\
NULL

//
// Palette Networking
enum enum_qbPaletteNetworking {
	PALETTE_NET_OFF = 0,
	PALETTE_NET_SEND,
	PALETTE_NET_RECEIVE,
	// count
	PALETTE_NET_COUNT
};
#define LABELS_PALETTE_NET \
"OFF",\
"Send",\
"Receive",\
NULL



//
// Config Class
class qbConfig : public ci::ciConfigGui {
public:
	qbConfig();
	~qbConfig() {}

	void	setup();
	void	update();
	
	void	enableQBGui( bool e=true )		{ columnRender->enable(); columnPalette->enable(); }
	void	setRenderTexture( gl::Texture * frame );
	
	int		getCurrentTab()					{ return this->getInt( QBCFG_CURRENT_TAB ); }

	// tabs
	sgui::TabControl			* tabQB;
	// columns
	sgui::ColumnControl			* columnQB;
	sgui::ColumnControl			* columnRender;
	sgui::ColumnControl			* columnPerlinSource;
	sgui::ColumnControl			* columnPalette;
	// panels
	sgui::PanelControl			* panelSyphon;
	sgui::PanelControl			* panelPaletteColors;
	sgui::PanelControl			* panelPaletteReduction;
	// controls
	sgui::ButtonControl			* buttonPlaySwitch;
	sgui::ButtonControl			* buttonRenderSwitch;
	sgui::TextureVarControl		* controlRenderTexture;
	sgui::TextureVarControl		* controlPaletteTexture;
	// gui textures
	gl::Texture					mNullTexture;
	gl::Texture					mTexturePalette;
	
private:
	
	void	cbLoad( ci::app::MouseEvent & event );
	void	cbSave( ci::app::MouseEvent & event );
	void	cbPlaySwitch( ci::app::MouseEvent & event );
	void	cbRewind( ci::app::MouseEvent & event );
	void	cbRenderSwitch( ci::app::MouseEvent & event );
	void	cbRenderFinish( ci::app::MouseEvent & event );
	void	cbScreenshot( ci::app::MouseEvent & event );
	void	cbPalette2Switch( ci::app::MouseEvent & event );
	void	cbPalette3Switch( ci::app::MouseEvent & event );
	void	cbPalette4Switch( ci::app::MouseEvent & event );
	void	cbPalette5Switch( ci::app::MouseEvent & event );
	void	paletteSwitchColors( int cfg1, int cfg2 );

	void	postSetCallback(int id, int i);
	
};

