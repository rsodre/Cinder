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


/////////////////////////////////////////////////////////////////
//
// ciConfig
//
#ifdef CICONFIG
namespace cinder { namespace sgui {
	ciGuiBlockLoadSave::ciGuiBlockLoadSave( ciConfigGui *cfg ) : ciConfigGuiBlock(cfg)
	{
		std::string cmd = toString((unsigned char)174);		// (R) = COMMAND SIGN for pf_tempesta_seven_ROGER.ttf
		//std::string shft = toString((unsigned char)169);	// (C) = SHIFT SIGN for pf_tempesta_seven_ROGER.ttf
		
		cfg->guiAddTab("FILE");
		cfg->guiAddGroup("");
		cfg->guiAddParam("", &mFilename);
		cfg->guiAddSeparator();
		cfg->guiAddButton("Reset",							this, &ciGuiBlockLoadSave::cbReset)->setPostGap(false);
		cfg->guiAddButton("Load Default",		cmd+"L",	this, &ciGuiBlockLoadSave::cbLoadDefault)->setPostGap(false);
		cfg->guiAddButton("Save as Default",	cmd+"S",	this, &ciGuiBlockLoadSave::cbSaveDefault);
		cfg->guiAddSeparator();
		cfg->guiAddButton("Import Config...",	cmd+"I",	this, &ciGuiBlockLoadSave::cbImportConfig)->setPostGap(false);
		cfg->guiAddButton("Export Config...",	cmd+"E",	this, &ciGuiBlockLoadSave::cbExportConfig);
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
	ciGuiBlockQBSourceTab::ciGuiBlockQBSourceTab( ciConfigGui *cfg, qb::qbSourceSelector * src, const std::string &label, int _cfgSelector, int _cfgName, int _cfgFileName, int _cfgIgnoreAlpha, std::function<void()> infoBlock ) : ciConfigGuiBlock(cfg)
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
		{
			mCfg->setValueLabels(_cfgSelector,	src->getList());
			mSelectorControl = (IntVarControl*) mCfg->guiAddParam( _cfgSelector, "" );
			mSelectorControl->setDisplayValue(false);
#ifdef BDVJ
			mCfg->guiAddSeparator();
			mCfg->guiAddText("Test Patterns\nby Paul Bourke", true);
			mCfg->guiAddButton("How to use", this, &ciGuiBlockQBSourceTab::cbDomePatterns);
#endif
		}
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
		mCfg->guiAddGroup( std::string("> SYPHON") );
		{
			mDirSyphon = new ciGuiBlockSyphonDirectory( cfg, _cfgName );
			_cfg.guiAddBlock( mDirSyphon );
			mCfg->guiAddParam("Client Framerate",	&mCurrFPSSyphon, 1, true );
		}
		// COLUMN
#ifndef QB_NO_NDI
		mCfg->guiAddGroup( std::string("> NDI") );
		{
			mDirNDI = new ciGuiBlockNDIDirectory( cfg, _cfgName );
			_cfg.guiAddBlock( mDirNDI );
			mCfg->guiAddParam("Client Framerate",	&mCurrFPSNDI, 1, true );
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
		
		// New Syphon source selected
		if ( mDirSyphon->isFresh() )
		{
			if ( ! mDirSyphon->getCurrentName().empty() )
				mCfg->set( cfgName, std::string("syphon::") + mDirSyphon->getCurrentName() );
		}
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
			// Unselect Syphon
			if ( mSource->getType() != QB_SOURCE_SYPHON )
				mDirSyphon->unselect();
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
		if (mName.empty())
			mName = "< none >";
		if (mFileName.empty())
			mFileName = "< none >";
		
		// hide unused controls
		bool ismov = qb::qbSourceSelector::isMovieFile(mSelectedPath);
		mPanelMovie->enable( ismov );
		mPanelPlayhead->enable( ismov );
		
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
		this->update();
	}
	
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
				std::string servername = _SyphonDirectory.getServer(i).getAppName();
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
		if ( current.compare(0,8,"ndi::") == 0 )		// remove qbSource prefix
			current = current.substr(8);
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

