//
//  ciConfigGuiBlocks.cpp
//
//  Created by Roger Sodre on 01/01/2013
//  Copyright 2013 Studio Avante. All rights reserved.
//

#include "ciConfigGuiBlocks.h"
#include "cinder/Utilities.h"
#ifdef BDVJ
#include "lacy.h"
#endif
#include <memory>
#include <algorithm>


/////////////////////////////////////////////////////////////////
//
// ciConfig
//
#ifdef CICONFIG
namespace cinder { namespace sgui {
	ciGuiBlockLoadSave::ciGuiBlockLoadSave( ciConfigGui *cfg ) : ciConfigGuiBlock(cfg)
	{
#if defined( CINDER_MSW )
		std::string cmd = "CTRL+";
#else
		std::string cmd = toString((unsigned char)174);		// (R) = COMMAND SIGN for pf_tempesta_seven_ROGER.ttf
#endif
		//std::string shft = toString((unsigned char)169);	// (C) = SHIFT SIGN for pf_tempesta_seven_ROGER.ttf
		
		cfg->guiAddTab("FILE");
		cfg->guiAddGroup("");
		cfg->guiAddText("PROJECT");
		cfg->guiAddParam("", &mFilename);
		cfg->guiAddSeparator();
		cfg->guiAddButton("Reset",							this, &ciGuiBlockLoadSave::cbReset)->setPostGap(false);
		ButtonControl *bLoad = cfg->guiAddButton("Load Default", cmd+"L", this, &ciGuiBlockLoadSave::cbLoadDefault);
		bLoad->setPostGap(false);
		ButtonControl *bSave = cfg->guiAddButton("Save as Default", cmd+"S", this, &ciGuiBlockLoadSave::cbSaveDefault);
		cfg->guiAddSeparator();
		ButtonControl *bFolder = cfg->guiAddButton("Open Project Folder", this, &ciGuiBlockLoadSave::cbOpenProjectFolder);
		cfg->guiAddSeparator();
		ButtonControl *bImport = cfg->guiAddButton("Import Config...", cmd+"I", this, &ciGuiBlockLoadSave::cbImportConfig);
		bImport->setPostGap(false);
		ButtonControl *bExport = cfg->guiAddButton("Export Config...", cmd+"E", this, &ciGuiBlockLoadSave::cbExportConfig);
#ifdef NO_SAVE
		// Demo: these actions are compiled out (NO_SAVE guards in the callbacks, in
		// ciConfig::onKeyDown and in the save/load/openProjectFolder primitives).
		// Show the buttons locked, keeping their labels, so it is clear they are off.
		bLoad->lock( true );
		bSave->lock( true );
		bFolder->lock( true );
		bImport->lock( true );
		bExport->lock( true );
#endif	// NO_SAVE
	}
	
	void ciGuiBlockLoadSave::update()
	{
		mFilename = mCfg->getDisplayFileName();
	}
	
	//
	// FILE TAB CALLBACKS
	void ciGuiBlockLoadSave::cbReset( ci::app::MouseEvent & event ) {
		mCfg->reset();
		event.setHandled();
	}
	void ciGuiBlockLoadSave::cbLoadDefault( ci::app::MouseEvent & event ) {
#ifndef NO_SAVE
		mCfg->load();
		event.setHandled();
#endif	// NO_SAVE
	}
	void ciGuiBlockLoadSave::cbSaveDefault( ci::app::MouseEvent & event ) {
#ifndef NO_SAVE
		mCfg->save();
		event.setHandled();
#endif	// NO_SAVE
	}
    void ciGuiBlockLoadSave::cbOpenProjectFolder( ci::app::MouseEvent & event ) {
        mCfg->openProjectFolder();
        event.setHandled();
    }
	void ciGuiBlockLoadSave::cbImportConfig( ci::app::MouseEvent & event ) {
#ifndef NO_SAVE
		mCfg->import();
		event.setHandled();
#endif	// NO_SAVE
	}
	void ciGuiBlockLoadSave::cbExportConfig( ci::app::MouseEvent & event ) {
#ifndef NO_SAVE
		mCfg->exportas();
		event.setHandled();
#endif	// NO_SAVE
	}
} } // namespace cinder::sgui
#endif




/////////////////////////////////////////////////////////////////
//
// QB BLOCKS
//
#ifdef QB
#include "qb.h"
#include "qbSource.h"
namespace cinder { namespace sgui {
	//
	// QB Source Single column
	ciGuiBlockQBSource::ciGuiBlockQBSource( ciConfigGui *cfg, qb::qbSourceSelector * src, const std::string &label, int _cfgSelector, int _cfgTrigger ) : ciConfigGuiBlock(cfg)
	{
		mSource = src;
		mSelectorControl = NULL;
		
		if ( label.length() )
			mCfg->guiAddText(label);
		mTextureControl = mCfg->guiAddTexture( "", &mTexture, 0.1 );
		mName = "<name>";
		mDesc = "<desc>";
		mCfg->guiAddText( "", &mName );		// use label dor desc
		mCfg->guiAddText( "", &mDesc );

		// make selector
		if ( _cfgSelector >= 0 )
		{
			src->useConfigSelector( _cfgSelector, cfg, false );
			mSelectorControl = (IntVarControl*) mCfg->guiAddParam(_cfgSelector, "");
			mSelectorControl->setDisplayValue(false);
		}
		// dummy param
		else
		{
			int min = (int)mCfg->getMin(_cfgSelector);
			int max = (int)mCfg->getMin(_cfgSelector);
			if ( max - min >= 1 )
				mCfg->mGui->addParam(label, mCfg->getPointerInt(_cfgSelector), min, max, mCfg->getInt(_cfgSelector));
		}
		
		// trigger
		if ( _cfgTrigger >= 0 )
			src->useConfigTrigger( _cfgTrigger, cfg );
	}
	void ciGuiBlockQBSource::update()
	{
		//mTextureControl->setRefreshRate( mSource->isPlaying() && _qb.isPlaying() ? 0.1 : 0 );
		mTexture = mSource->getTexture();
		mName = mSource->getName();
		mDesc = mSource->getDesc();
		if (mSelectorControl)	// use label for desc
			mSelectorControl->setName( mDesc );
	}
	
	//////////////////////////////////////
	//
	// QB Source TAB
	//
	ciGuiBlockQBSourceTab::ciGuiBlockQBSourceTab( ciConfigGui *cfg, qb::qbSourceSelector * src, const std::string &label, int _cfgSelector, int _cfgName, int _cfgFileName, int _cfgIgnoreAlpha, std::function<void()> infoBlock, std::function<void()> generatorBlock ) : ciConfigGuiBlock(cfg)
	{
		mSource = src;
		cfgName = _cfgName;
		cfgFileName = _cfgFileName;
		cfgIgnoreAlpha = _cfgIgnoreAlpha;
		mFPS = mWidth = mHeight = 0;
		bMovieEnabled = false;
		bLastMovieEnabled = false;
		bLastMoviePlaying = true;
		bIgnoreAlpha = false;

		// set callbacks
		app::getWindow()->connectFileDrop( & ciGuiBlockQBSourceTab::onFileDrop, this );
		src->useConfigSelector( _cfgSelector, cfg, false );
		src->useConfigName( _cfgName, cfg );

		// COLUMN
		mCfg->guiAddGroup( std::string("> ") + label );
		{
			// Texture
			mTextureControl = mCfg->guiAddTexture( "", &mTexture, 0 );
			
			// Current selected source
			mCfg->guiAddParam( "", &mName )->setImportant();
			//mCfg->guiAddParam( "", &mDesc );
			mCfg->guiAddParam("Type",				&mType );
			mCfg->guiAddParam("Width",				&mWidth, true );
			mCfg->guiAddParam("Height",				&mHeight, true );
			// panel: movie
			mPanelMovie = mCfg->guiAddPanel("mPanelMovie");
			mCfg->guiAddParam("Framerate",			&mFPS, 1, true );
			mCfg->guiAddPanel("");
			cfg->guiAddSeparator();
			mCfg->guiAddParam(cfgIgnoreAlpha,		"Ignore Alpha" );
			// NDI Audio
			cfg->guiAddSeparator();
			mCfg->guiAddText("> AUDIO");
			mCfg->guiAddParamFLoat( QBCFG_AUDIO_VOLUME,	"Volume", 2 )->setFormatAsPercentage();
			mCfg->guiAddParamBool( QBCFG_AUDIO_MUTE, "MUTED" )->setAsButton()->setNameOff("Mute");

			// Custom block
			if (infoBlock != nullptr)
			{
				cfg->guiAddSeparator();
				infoBlock();
			}
		}
		// COLUMN
		mCfg->guiAddGroup( std::string("> TEMPLATES") );
#ifdef BDVJ
		{
			// Same source list, displayed by mode
			// Each mode has its own panel, selecting directly on _cfgSelector
			mCfg->setValueLabels(_cfgSelector,	src->getList());
			mSelectorControl = NULL;
			cfgSelector = _cfgSelector;
			mLastSelected = -1;
			mLastTemplateMode = -1;
			mTemplateMode = TEMPLATE_MODE_GENERATOR;
			for ( int mode = 0 ; mode < TEMPLATE_MODE_COUNT ; mode++ )
				mLastSelectedInMode[mode] = -1;
			std::map<int,std::string> modes;
			modes[TEMPLATE_MODE_GENERATOR] = "Generator";
			modes[TEMPLATE_MODE_TEMPLATES] = "Classic Templates";
			modes[TEMPLATE_MODE_GRIDS] = "Classic Grids";
			modes[TEMPLATE_MODE_PATTERNS] = "Test Pattern";
			modes[TEMPLATE_MODE_PANORAMA] = "360 Panorama";
			mCfg->guiAddParamList( "", &mTemplateMode, modes );
			for ( int mode = 0 ; mode < TEMPLATE_MODE_COUNT ; mode++ )
			{
				size_t first = mCfg->mGui->getControls().size();
				mCfg->guiAddPanel( std::string("mPanelTemplates") + toString(mode) );
				mCfg->guiAddSeparator();
				// Mode header, ex: "> GENERATOR"
				std::string header = modes[mode];
				std::transform( header.begin(), header.end(), header.begin(), ::toupper );
				mCfg->guiAddText( std::string("> ") + header );
				this->addTemplateList( mode );
				if ( mode == TEMPLATE_MODE_GENERATOR && generatorBlock != nullptr )
				{
					mCfg->guiAddSeparator();
					generatorBlock();
				}
				else if ( mode == TEMPLATE_MODE_PATTERNS )
				{
					mCfg->guiAddSeparator();
					mCfg->guiAddText("Test Patterns\nby Paul Bourke", true);
					mCfg->guiAddButton("How to use", this, &ciGuiBlockQBSourceTab::cbDomePatterns);
				}
				mTemplateControls[mode] = std::make_pair( first, mCfg->mGui->getControls().size() );
			}
			mCfg->guiAddPanel("");
		}
#else
		{
			mCfg->setValueLabels(_cfgSelector,	src->getList());
			mSelectorControl = (IntVarControl*) mCfg->guiAddParam( _cfgSelector, "" );
			mSelectorControl->setDisplayValue(false);
		}
#endif
		// COLUMN
		mCfg->guiAddGroup( std::string("> FILE") );
		{
			mCfg->guiAddParamBool("ENABLED",&bMovieEnabled)->setAsButton()->setNameOff("Enable");
#ifdef QTXX
			mCfg->guiAddButton("Load Movie / Image...", this, &ciGuiBlockQBSourceTab::cbLoad)->setPostGap(false);
#else
			mCfg->guiAddButton("Load Image...", this, &ciGuiBlockQBSourceTab::cbLoad)->setPostGap(false);
#endif
			mCfg->guiAddText("(or drop files here)");
			// current movie info
			mCfg->guiAddSeparator();
			mCfg->guiAddParam( "", &mFileName )->setImportant();
			mPanelPlayhead = mCfg->guiAddPanel("mPanelPlayhead");
			mCfg->guiAddParamFLoat(QBCFG_CURRENT_TIME,	"Current Time", 2 )->setFormatAsTimecode()->setReadOnly();
			mCfg->guiAddParam(DUMMY_CURRENT_FRAME,		"Current Frame" );
			mCfg->guiAddParam("Current Framerate",	&mCurrFPS, 1, true );
			// qb play control
			mCfg->guiAddParamFLoat(QBCFG_CURRENT_PROG,	"Playhead", 3 )->setFormatAsPercentage();
			mCfg->guiAddParamBool( QBCFG_PLAYING, "PLAYING" )->setAsButton()->setNameOff("Play");
			mCfg->guiAddButton("Rewind", this, &ciGuiBlockQBSourceTab::cbRewind);
		}
		// COLUMN
#ifdef CFG_BLOCK_SYPHON
		mCfg->guiAddGroup( std::string("> SYPHON") );
		{
			mDirSyphon = new ciGuiBlockSyphonDirectory( cfg, _cfgName );
			_cfg.guiAddBlock( mDirSyphon );
			mCfg->guiAddParam("Client Framerate",	&mCurrFPSSyphon, 1, true );
#ifdef BLENDY_DEMO
			mCfg->guiAddText( "DEMO VERSION, this input is disabled.", true );
#endif
		}
#else
		mDirSyphon = NULL;
#endif
		// COLUMN
#ifdef CFG_BLOCK_SPOUT
		mCurrFPSSpout = 0.0f;
		mCfg->guiAddGroup( std::string("> SPOUT") );
		{
			mDirSpout = new ciGuiBlockSpoutDirectory( cfg, _cfgName );
			_cfg.guiAddBlock( mDirSpout );
			mCfg->guiAddParam("Client Framerate",	&mCurrFPSSpout, 1, true );
#ifdef BLENDY_DEMO
			mCfg->guiAddText( "DEMO VERSION, this input is disabled.", true );
#endif
		}
#endif
		// COLUMN
#ifndef QB_NO_NDI
		mCfg->guiAddGroup( std::string("> NDI") );
		{
			mDirNDI = new ciGuiBlockNDIDirectory( cfg, _cfgName );
			_cfg.guiAddBlock( mDirNDI );
			mCfg->guiAddParam("Client Framerate",	&mCurrFPSNDI, 1, true );
#ifdef BLENDY_DEMO
			mCfg->guiAddText( "DEMO VERSION, this input is disabled.", true );
#endif
		}
#endif
	}
	void ciGuiBlockQBSourceTab::update()
	{
		// get current filename loaded from file
		if ( mCfg->getString(cfgFileName) != mSelectedPath )
		{
			mSelectedPath = mCfg->getString(cfgFileName);
			mFileName = getPathFileName( mSelectedPath );
		}
		
		// Syphopn input loaded from file
		//if ( mSource->getType() == QB_TYPE_SYPHON && ! mDir->isSelected() )
		//	mDir->refresh();
		
#ifdef CFG_BLOCK_SYPHON
		// New Syphon source selected
		if ( mDirSyphon->isFresh() )
		{
			if ( ! mDirSyphon->getCurrentName().empty() )
				mCfg->set( cfgName, std::string("syphon::") + mDirSyphon->getCurrentName() );
		}
#else
		if ( false ) {}		// no Syphon: start of the else-if chain
#endif
#ifdef CFG_BLOCK_SPOUT
		// New Spout source selected
		else if ( mDirSpout->isFresh() )
		{
			if ( ! mDirSpout->getCurrentName().empty() )
				mCfg->set( cfgName, std::string("spout::") + mDirSpout->getCurrentName() );
		}
#endif
		// New NDI source selected
#ifndef QB_NO_NDI
		else if ( mDirNDI->isFresh() )
		{
			if ( ! mDirNDI->getCurrentName().empty() )
				mCfg->set( cfgName, std::string("ndi::") + mDirNDI->getCurrentName() );
		}
#endif
		// New media loaded
		else if ( mSource->isFresh() )
		{
#ifdef CFG_BLOCK_SYPHON
			// Unselect Syphon
			if ( mSource->getType() != QB_SOURCE_SYPHON )
				mDirSyphon->unselect();
#endif
#ifdef CFG_BLOCK_SPOUT
			// Unselect Spout
			if ( mSource->getType() != QB_SOURCE_SPOUT )
				mDirSpout->unselect();
#endif
#ifndef QB_NO_NDI
			if ( mSource->getType() != QB_SOURCE_NDI )
				mDirNDI->unselect();
#endif
			// Adjust QB tiome to movie
			if ( mSource->getType() == QB_SOURCE_MOVIE )
				mCfg->set( QBCFG_RENDER_SECONDS, mSource->getDuration() );
			// Load new file?
			if ( mSelectedPath != mCfg->getString(cfgName) )
				if( fs::exists( mCfg->getString(cfgName) ) )
					this->loadFile( mCfg->getString(cfgName) );
			// Movie trigger
			bMovieEnabled = ( mSelectedPath.compare( mCfg->getString(cfgName) ) == 0 );
		}
		// Triggered movie
		else if ( bMovieEnabled && !mSelectedPath.empty() && mSelectedPath.compare( mCfg->getString(cfgName) ) != 0 )
			this->loadFile( mSelectedPath );
		// don't let trigger movie button OFF
		else if ( !bMovieEnabled && mSelectedPath.compare( mCfg->getString(cfgName) ) == 0 && mSource->getType() == QB_SOURCE_MOVIE )
			bMovieEnabled = true;
		// don't let trigger movie button ON if empty
		else if ( bMovieEnabled && mSelectedPath.empty() )
			bMovieEnabled = false;
		
		// Stopped movie? Remeber if we're playing
		if ( mSource->getType() != QB_SOURCE_MOVIE && _qb.isPlaying() )
		{
			bLastMoviePlaying = _qb.isPlaying();
			_qb.stop();
		}
		// Started movie? Back to old playing state
		if ( bMovieEnabled && bLastMovieEnabled != bMovieEnabled )
			_qb.play( bLastMoviePlaying );
		bLastMovieEnabled = bMovieEnabled;
			

		// Update texture
		if ( mSource->isFresh() || mSource->hasNewFrame() )
		{
			mTextureControl->refresh();
			mTexture = mSource->getTexture();
		}
		
		// Current	
		mType			= mSource->getType();
		mName			= mSource->getName();
		mDesc			= mSource->getDesc();
		mWidth			= mSource->getWidth();
		mHeight			= mSource->getHeight();
		mFPS			= mSource->getFrameRate();
		mCurrFPS		= ( mSource->getType() == QB_SOURCE_MOVIE ? mSource->getCurrentFrameRate() : 0.0 );
		mCurrFPSSyphon	= ( mSource->getType() == QB_SOURCE_SYPHON ? mSource->getCurrentFrameRate() : 0.0 );
		mCurrFPSNDI		= ( mSource->getType() == QB_SOURCE_NDI ? mSource->getCurrentFrameRate() : 0.0 );
#ifdef CFG_BLOCK_SPOUT
		mCurrFPSSpout	= ( mSource->getType() == QB_SOURCE_SPOUT ? mSource->getCurrentFrameRate() : 0.0 );
#endif
		if (mName.empty())
			mName = "< none >";
		if (mFileName.empty())
			mFileName = "< none >";
		
		// hide unused controls
		bool ismov = qb::qbSourceSelector::isMovieFile(mSelectedPath);
		mPanelMovie->enable( ismov );
		mPanelPlayhead->enable( ismov );

#ifdef BDVJ
		// Follow selected template
		int selected = mCfg->getInt( cfgSelector );
		auto selectedIt = mSource->getList().find( selected );
		int selectedMode = ( selectedIt != mSource->getList().end() ? getTemplateMode( selectedIt->second ) : -1 );
		if ( selected != mLastSelected )
		{
			mLastSelected = selected;
			if ( selectedMode >= 0 )
			{
				mLastSelectedInMode[selectedMode] = selected;
				mTemplateMode = selectedMode;
			}
		}
		if ( mTemplateMode != mLastTemplateMode )
		{
			// Mode selected by user: load its last selected (or first) item
			// (not on startup, to keep any loaded file)
			if ( mLastTemplateMode >= 0 && selectedMode != mTemplateMode )
			{
				int item = mLastSelectedInMode[mTemplateMode];
				if ( item < 0 )
					for ( auto it : mSource->getList() )
						if ( getTemplateMode( it.second ) == mTemplateMode ) { item = it.first; break; }
				if ( item >= 0 )
					mCfg->set( cfgSelector, item );
			}
			this->switchTemplateMode();
		}
#endif
		
		// File
		// TODO:: get droppped name, loaded name
		//mFileName = getPathFileName( mCfg->getString( cfgName ) );
	}
	
	//
	// Callbacks
	void ciGuiBlockQBSourceTab::cbLoad( ci::app::MouseEvent & event )
	{
		std::string f = app::getOpenFilePath( mSelectedPath, qb::_qbSourceExt ).string();
		if( fs::exists( f ) )
			this->loadFile(f);
		event.setHandled();
	}
	void ciGuiBlockQBSourceTab::cbPlay( ci::app::MouseEvent & event ) {
		//_qb.playSwitch();
		event.setHandled();
	}
	void ciGuiBlockQBSourceTab::cbRewind( ci::app::MouseEvent & event )
	{
		_qb.rewind();
		event.setHandled();
	}
#ifdef BDVJ
	void ciGuiBlockQBSourceTab::cbDomePatterns( ci::app::MouseEvent & event )
	{
		lacy::Lacy::OpenUrl( "http://paulbourke.net/dome/testpattern/" );
		event.setHandled();
	}
	//
	// Templates
	// Template mode by source name, -1 if not displayed
	int ciGuiBlockQBSourceTab::getTemplateMode( const std::string & name )
	{
		if ( qb::qbSourceSelector::isGenerator( name ) )
			return TEMPLATE_MODE_GENERATOR;
		if ( name.find("_grid") != std::string::npos )
			return TEMPLATE_MODE_GRIDS;
		if ( name.compare(0,5,"DOME_") == 0 )
			return TEMPLATE_MODE_TEMPLATES;
		if ( name.compare(0,8,"pattern_") == 0 )
			return TEMPLATE_MODE_PATTERNS;
		if ( name.compare(0,8,"PANORAMA") == 0 )
			return TEMPLATE_MODE_PANORAMA;
		return -1;
	}
	// Friendly name: "DOME_220_grid_4K.png" > "Dome 220° 4K"
	std::string ciGuiBlockQBSourceTab::getTemplateLabel( const std::string & name )
	{
		if ( qb::qbSourceSelector::isGenerator( name ) )
			return "Fulldome Grid";
		std::string base = getPathFileName( name );
		base = base.substr( 0, base.find_last_of('.') );
		std::vector<std::string> parts = split( base, '_' );
		std::string res = parts.back();
		if ( parts[0] == "DOME" )
		{
			std::string fov = ( parts.size() > 2 && isdigit(parts[1][0]) ? parts[1] : "180" );	// DOME_220_2K / DOME_2K
			return std::string("Dome ") + fov + "\302\260 " + res;
		}
		if ( parts[0] == "pattern" )
			return std::string("Pattern ") + res;
		if ( parts[0] == "PANORAMA" )
			return "Panorama";
		return base;
	}
	// List of templates of a mode, selecting on cfgSelector
	void ciGuiBlockQBSourceTab::addTemplateList( int mode )
	{
		std::map<int,std::string> labels;
		for ( auto it : mSource->getList() )
			if ( getTemplateMode( it.second ) == mode )
				labels[it.first] = getTemplateLabel( it.second );
		if ( ! labels.empty() )
		{
			// A new list snaps a value it does not hold to one of its own items: the lists of the other modes
			// would leave the selector on the last mode (Panorama) when no saved config overrides it
			int * selector = mCfg->getPointerInt( cfgSelector );
			const int selected = *selector;
			mCfg->guiAddParamList( "", selector, labels );
			*selector = selected;
		}
	}
	// Show current mode panels only
	void ciGuiBlockQBSourceTab::switchTemplateMode()
	{
		mLastTemplateMode = mTemplateMode;
		std::vector<Control*> & controls = mCfg->mGui->getControls();
		for ( int mode = 0 ; mode < TEMPLATE_MODE_COUNT ; mode++ )
		{
			bool active = ( mode == mTemplateMode );
			for ( size_t i = mTemplateControls[mode].first ; i < mTemplateControls[mode].second && i < controls.size() ; i++ )
			{
				if ( controls[i]->type == Control::PANEL )
					controls[i]->enable( active );
			}
			// restore switch panels inside the mode
			if ( active )
				for ( size_t i = mTemplateControls[mode].first ; i < mTemplateControls[mode].second && i < controls.size() ; i++ )
					controls[i]->switchPanels();
		}
	}
#endif
	// get dropped file
	void ciGuiBlockQBSourceTab::onFileDrop( ci::app::FileDropEvent & event )
	{
		if ( event.getNumFiles() > 0 )
			if ( this->loadFile( event.getFile( 0 ).string() ) )
				event.setHandled();
	}
	// Generic load
	bool ciGuiBlockQBSourceTab::loadFile( const std::string & f )
	{
		if ( std::find( qb::_qbSourceExt.begin(), qb::_qbSourceExt.end(), getPathExtension(f) ) == qb::_qbSourceExt.end() )
			return false;
		//mSource->play( _qb.isPlaying );
		mCfg->set( cfgName, f );
		mCfg->set( cfgFileName, f );
		mFileName = getPathFileName( f );
		mSelectedPath = f;
		return true;
	}
} } // namespace cinder::sgui
#endif
	



/////////////////////////////////////////////////////////////////
//
// SYPHON BLOCK
//
#ifdef CFG_BLOCK_SYPHON
namespace cinder { namespace sgui {
	//
	// Syphon Directory
	//
	syphonServerDirectory _SyphonDirectory;
	ciGuiBlockSyphonDirectory::ciGuiBlockSyphonDirectory( ciConfigGui *cfg, int _cfgName ) : ciConfigGuiBlock(cfg)
	{
		cfgName = _cfgName;
		mListId = mLastId = -1;
		mServerCount = 0;
		bFreshness = false;
		
		// Setup Directory
		_SyphonDirectory.setup();

		// Setup GUI
		mPanel = mCfg->guiAddPanel("ciGuiBlockSyphonDirectory");
		mLabel = cfg->guiAddText("---");	// update() will set
		std::map<int,std::string> valueLabels;
		mListControl = mCfg->guiAddParamList("", &mListId, valueLabels);
#ifdef BLENDY_DEMO
		mListControl->lock( true );		// Demo: live input disabled, server list not selectable
#endif
		this->update();
	}
} } // namespace cinder::sgui
#endif	// CFG_BLOCK_SYPHON

// Server name helpers, also built without Syphon
namespace cinder { namespace sgui {

	std::string ciGuiBlockSyphonDirectory::makeServerDisplayName( std::string appName, std::string textureName )
	{
		std::stringstream servername;
		servername << appName;
		if ( textureName.length() > 0 )
		{
			servername << " | " << textureName;
		}
		return servername.str();
	}

	std::pair<std::string,std::string> ciGuiBlockSyphonDirectory::splitServerDisplayName( std::string displayName )
	{
		std::string appName;
		std::string textureName;
		const std::size_t pos = displayName.find( " | " );
		if( pos == std::string::npos )
		{
			appName = displayName;
		} else {
			appName = displayName.substr( 0, pos );
			textureName = displayName.substr( pos + 3 );
		}
		return std::make_pair( appName, textureName );
	}
} } // namespace cinder::sgui

#ifdef CFG_BLOCK_SYPHON
namespace cinder { namespace sgui {

	void ciGuiBlockSyphonDirectory::update()
	{
		bFreshness = false;
		
		// Current Server we're using
		std::string current = mCfg->getString(cfgName);
		if ( current.compare(0,8,"syphon::") == 0 )		// remove qbSource prefix
			current = current.substr(8);
		//else
		//	current = "";
		
		// Changed?
		if ( _SyphonDirectory.hasChanged() || mCfg->isFresh(cfgName) )
		{
			//std::string current = ( mListId >= 0 ? mListControl->getValueLabel( mListId ) : "" );
			// Make new Syphon list
			int key = 0;
			std::map<int,std::string> valueLabels;
			for (int i = 0 ; i < _SyphonDirectory.getServerCount() ; i++)
			{
				std::string servername = makeServerDisplayName( _SyphonDirectory.getServer(i).getAppName(), _SyphonDirectory.getServer(i).getName() );
				// no feedback!!
				if ( servername.compare( mExclude ) == 0 )
					continue;
				// Set label
				valueLabels[key++] = servername;
			}
			// Update list
			mListControl->update( valueLabels );
			mServerCount = (int) valueLabels.size();
			// Unselect / Select
			this->reselect( current );
		}
		
		// New Syphon source selected
		// Checo newSyphonSource porque se os indices de SYPHON_SOURCE sao dinamicos
		if ( _SyphonDirectory.hasChanged() || mLastId != mListId )
		{
			if (mListId >= 0)
			{
				mCurrentName = mListControl->getValueLabel( mListId );
				mCfg->set( cfgName, mCurrentName );
			}
			else
				mCurrentName = "";
			this->reselect( mCurrentName );
			bFreshness = true;
		}
		mLastId = mListId;

		// Refresh Data
		mLabel->setName( mServerCount == 0 ? "No Available Clients!" : "Available Clients..." );
	}

	void ciGuiBlockSyphonDirectory::reselect( const std::string & name )
	{
		mListId = -1;
		for (int i = 0 ; i < mListControl->items.size() ; i++ ) {
			if ( name.compare( mListControl->getValueLabel(i) ) == 0 )
			{
				mListId = i;
				break;
			}
		}
	}
	

	//////////////////////////////////////
	//
	// Syphon Client
	//
	ciGuiBlockSyphon::ciGuiBlockSyphon( ciConfigGui *cfg, int _cfgName, int _cfgEnabled ) : ciConfigGuiBlock(cfg)
	{
		cfgEnabled = _cfgEnabled;
		
		// Setup Directory
		mDirBlock = new ciGuiBlockSyphonDirectory( cfg, _cfgName );
		mCfg->guiAddBlock( mDirBlock );
		
//		// Setup Client
//		mClient.setup();
//		mClient.setApplicationName( mCfg->getString( _cfgName ) );
//		mClient.setServerName("");
//		mClient.update();
//
//		// Setup GUI
//		mCfg->guiAddParam("Width",		&mClientWidth, true );
//		mCfg->guiAddParam("Height",		&mClientHeight, true );
//		mCfg->guiAddParam("Framerate",	&mClientFPS, 1, true );

		this->update();
	}
	
	void ciGuiBlockSyphon::update()
	{
//		// New Syphon source selected
//		if ( mDirBlock->isFresh() )
//			mClient.setApplicationName( mDirBlock->getCurrentName() );
//
//		// Update client
//		if ( ! mDirBlock->getCurrentName().empty() && mCfg->getBool(cfgEnabled) )
//		{
//			mClient.update();
//			//printf("SyphonClient.update() > ciGuiBlockSyphon\n");
//		}
//
//		// Refresh Data
//		mClientFPS = mClient.getCurrentFrameRate();
//		mClientWidth = mClient.getWidth();
//		mClientHeight = mClient.getHeight();
	}
} } // namespace cinder::sgui
#endif
	
	


/////////////////////////////////////////////////////////////////
//
// SPOUT BLOCK (Windows, same behaviour as the Syphon directory)
//
#ifdef CFG_BLOCK_SPOUT
namespace cinder { namespace sgui {
	spoutServerDirectory _SpoutDirectory;
	ciGuiBlockSpoutDirectory::ciGuiBlockSpoutDirectory( ciConfigGui *cfg, int _cfgName ) : ciConfigGuiBlock(cfg)
	{
		cfgName = _cfgName;
		mListId = mLastId = -1;
		mServerCount = 0;
		bFreshness = false;

		// Setup Directory
		_SpoutDirectory.setup();

		// Setup GUI
		mPanel = mCfg->guiAddPanel("ciGuiBlockSpoutDirectory");
		mLabel = cfg->guiAddText("---");	// update() will set
		std::map<int,std::string> valueLabels;
		mListControl = mCfg->guiAddParamList("", &mListId, valueLabels);
#ifdef BLENDY_DEMO
		mListControl->lock( true );		// Demo: live input disabled, server list not selectable
#endif
		this->update();
	}

	void ciGuiBlockSpoutDirectory::update()
	{
		bFreshness = false;

		// Current Server we're using
		std::string current = mCfg->getString(cfgName);
		if ( current.compare(0,7,"spout::") == 0 )		// remove qbSource prefix
			current = current.substr(7);

		// Changed? (hasChanged() is true once per change, as with Syphon)
		bool changed = _SpoutDirectory.hasChanged();
		if ( changed || mCfg->isFresh(cfgName) )
		{
			// Make new Spout list
			int key = 0;
			std::map<int,std::string> valueLabels;
			for (int i = 0 ; i < _SpoutDirectory.getServerCount() ; i++)
			{
				std::string servername = ciGuiBlockSyphonDirectory::makeServerDisplayName( _SpoutDirectory.getServer(i).getAppName(), _SpoutDirectory.getServer(i).getName() );
				// no feedback!!
				if ( servername.compare( mExclude ) == 0 )
					continue;
				// Set label
				valueLabels[key++] = servername;
			}
			// Update list
			mListControl->update( valueLabels );
			mServerCount = (int) valueLabels.size();
			// Unselect / Select
			this->reselect( current );
		}

		// New Spout source selected
		// (the Syphon block re-checks the directory here; its hasChanged() was already consumed above, same result)
		if ( mLastId != mListId )
		{
			if (mListId >= 0)
			{
				mCurrentName = mListControl->getValueLabel( mListId );
				mCfg->set( cfgName, mCurrentName );
			}
			else
				mCurrentName = "";
			this->reselect( mCurrentName );
			bFreshness = true;
		}
		mLastId = mListId;

		// Refresh Data
		mLabel->setName( mServerCount == 0 ? "No Available Clients!" : "Available Clients..." );
	}

	void ciGuiBlockSpoutDirectory::reselect( const std::string & name )
	{
		mListId = -1;
		for (int i = 0 ; i < mListControl->items.size() ; i++ ) {
			if ( name.compare( mListControl->getValueLabel(i) ) == 0 )
			{
				mListId = i;
				break;
			}
		}
	}
} } // namespace cinder::sgui
#endif	// CFG_BLOCK_SPOUT




/////////////////////////////////////////////////////////////////
//
// NDI BLOCK
//
#ifdef CFG_BLOCK_NDI
namespace cinder { namespace sgui {
	//
	// NDI Directory
	//
	CinderNDIFinderPtr _NDIDirectory;
	ciGuiBlockNDIDirectory::ciGuiBlockNDIDirectory( ciConfigGui *cfg, int _cfgName ) : ciConfigGuiBlock(cfg)
	{
		cfgName = _cfgName;
		mListId = mLastId = -1;
		mServerCount = 0;
		bFreshness = false;
		bDirChanged = false;
		
		// Setup Directory / Create the NDI finder
		CinderNDIFinder::Description finderDscr;
		_NDIDirectory = std::make_unique<CinderNDIFinder>( finderDscr );
		
		mNDISourceAdded = _NDIDirectory->getSignalNDISourceAdded().connect( std::bind( &ciGuiBlockNDIDirectory::sourceAdded, this, std::placeholders::_1 ) );
		mNDISourceRemoved = _NDIDirectory->getSignalNDISourceRemoved().connect( std::bind( &ciGuiBlockNDIDirectory::sourceRemoved, this, std::placeholders::_1 ) );

		// Setup GUI
		mPanel = mCfg->guiAddPanel("ciGuiBlockNDIDirectory");
		cfg->guiAddText("(EXPERIMENTAL)");
		mLabel = cfg->guiAddText("---");	// update() will set
		std::map<int,std::string> valueLabels;
		mListControl = mCfg->guiAddParamList("", &mListId, valueLabels);
#ifdef BLENDY_DEMO
		mListControl->lock( true );		// Demo: live input disabled, server list not selectable
#endif
		this->update();
	}
	
	void ciGuiBlockNDIDirectory::sourceAdded( const NDISource& source )
	{
		std::cout << "NDI source added: " << source.p_ndi_name <<std::endl;
		bDirChanged = true;
	}
	
	void ciGuiBlockNDIDirectory::sourceRemoved( std::string sourceName )
	{
		std::cout << "NDI source removed: " << sourceName <<std::endl;
		bDirChanged = true;
	}

	ciGuiBlockNDIDirectory::~ciGuiBlockNDIDirectory()
	{
		if( _NDIDirectory )
		{
			mNDISourceAdded.disconnect();
			mNDISourceRemoved.disconnect();
		}
	}

	void ciGuiBlockNDIDirectory::update()
	{
		bFreshness = false;
		
		// Current Server we're using
		std::string current = mCfg->getString(cfgName);
		if ( current.compare(0,5,"ndi::") == 0 )		// remove qbSource prefix
			current = current.substr(5);
		//else
		//	current = "";
		
		// Changed?
		if ( bDirChanged || mCfg->isFresh(cfgName) )
		{
			//std::string current = ( mListId >= 0 ? mListControl->getValueLabel( mListId ) : "" );
			// Make new NDI list
			int key = 0;
			std::map<int,std::string> valueLabels;
			for (auto i = 0u ; i < _NDIDirectory->getSourcesCount() ; i++)
			{
				std::string servername = _NDIDirectory->getSourceName(i);
				// no feedback!!
				if ( servername.compare( mExclude ) == 0 )
					continue;
				// Set label
				valueLabels[key++] = servername;
			}
			// Update list
			mListControl->update( valueLabels );
			mServerCount = (int) valueLabels.size();
			// Unselect / Select
			this->reselect( current );
		}
		
		// New NDI source selected
		// Checo newNDISource porque se os indices de NDI_SOURCE sao dinamicos
		if ( bDirChanged || mLastId != mListId )
		{
			if (mListId >= 0)
			{
				mCurrentName = mListControl->getValueLabel( mListId );
				mCfg->set( cfgName, mCurrentName );
			}
			else
				mCurrentName = "";
			this->reselect( mCurrentName );
			bFreshness = true;
		}
		
		mLastId = mListId;
		bDirChanged = false;
		
		// Refresh Data
		mLabel->setName( mServerCount == 0 ? "No Available Clients!" : "Available Clients..." );
	}
	
	void ciGuiBlockNDIDirectory::reselect( const std::string & name )
	{
		mListId = -1;
		for (int i = 0 ; i < mListControl->items.size() ; i++ ) {
			if ( name.compare( mListControl->getValueLabel(i) ) == 0 )
			{
				mListId = i;
				break;
			}
		}
	}
	
	
	//////////////////////////////////////
	//
	// NDI Client
	//
	ciGuiBlockNDI::ciGuiBlockNDI( ciConfigGui *cfg, int _cfgName, int _cfgEnabled ) : ciConfigGuiBlock(cfg)
	{
		cfgEnabled = _cfgEnabled;
		
		// Setup Directory
		mDirBlock = new ciGuiBlockNDIDirectory( cfg, _cfgName );
		mCfg->guiAddBlock( mDirBlock );
		
//		// Setup Client
//		mClient.setup();
//		mClient.setApplicationName( mCfg->getString( _cfgName ) );
//		mClient.setServerName("");
//		mClient.update();
//
//		// Setup GUI
//		mCfg->guiAddParam("Width",		&mClientWidth, true );
//		mCfg->guiAddParam("Height",		&mClientHeight, true );
//		mCfg->guiAddParam("Framerate",	&mClientFPS, 1, true );
		
		this->update();
	}
	
	void ciGuiBlockNDI::update()
	{
//		// New NDI source selected
//		if ( mDirBlock->isFresh() )
//			mClient.setApplicationName( mDirBlock->getCurrentName() );
//
//		// Update client
//		if ( ! mDirBlock->getCurrentName().empty() && mCfg->getBool(cfgEnabled) )
//		{
//			mClient.update();
//			//printf("SyphonClient.update() > ciGuiBlockNDI\n");
//		}
//
//		// Refresh Data
//		mClientFPS = mClient.getCurrentFrameRate();
//		mClientWidth = mClient.getWidth();
//		mClientHeight = mClient.getHeight();
	}
} } // namespace cinder::sgui
#endif

