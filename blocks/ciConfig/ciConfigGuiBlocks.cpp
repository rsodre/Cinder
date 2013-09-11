//
//  ciConfigGuiBlocks.cpp
//
//  Created by Roger Sodre on 01/01/2013
//  Copyright 2013 Studio Avante. All rights reserved.
//

#include "ciConfigGuiBlocks.h"
#include "cinder/Utilities.h"


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
		mCfg->load();
		event.setHandled();
	}
	void ciGuiBlockLoadSave::cbSaveDefault( ci::app::MouseEvent & event ) {
		mCfg->save();
		event.setHandled();
	}
	void ciGuiBlockLoadSave::cbImportConfig( ci::app::MouseEvent & event ) {
		mCfg->import();
		event.setHandled();
	}
	void ciGuiBlockLoadSave::cbExportConfig( ci::app::MouseEvent & event ) {
		mCfg->exportas();
		event.setHandled();
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
		mCfg->guiAddText( "", &mName );		// use label dor desc
		
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
			mCfg->guiAddText( "", &mDesc );
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
	ciGuiBlockQBSourceTab::ciGuiBlockQBSourceTab( ciConfigGui *cfg, qb::qbSourceSelector * src, const std::string &label, int _cfgSelector, int _cfgName, int _cfgFileName ) : ciConfigGuiBlock(cfg)
	{
		mSource = src;
		cfgName = _cfgName;
		cfgFileName = _cfgFileName;
		mFPS = mWidth = mHeight = 0;
		bMovieEnabled = false;
		bLastMovieEnabled = false;
		bLastMoviePlaying = true;
		
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
		}
		// COLUMN
		mCfg->guiAddGroup( std::string("> FILE") );
		{
			mCfg->guiAddParamBool("ENABLED",&bMovieEnabled)->setAsButton()->setNameOff("Enable");
			mCfg->guiAddButton("Load Movie / Image...", this, &ciGuiBlockQBSourceTab::cbLoad)->setPostGap(false);
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
			mDir = new ciGuiBlockSyphonDirectory( cfg, _cfgName );
			_cfg.guiAddBlock( mDir );
			mCfg->guiAddParam("Client Framerate",	&mCurrFPSSyphon, 1, true );
		}
		// COLUMN
		mCfg->guiAddGroup( std::string("> TEMPLATES") );
		{
			mCfg->setValueLabels(_cfgSelector,	src->getList());
			mSelectorControl = (IntVarControl*) mCfg->guiAddParam( _cfgSelector, "" );
			mSelectorControl->setDisplayValue(false);
		}
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
		if ( mDir->isFresh() )
		{
			if ( ! mDir->getCurrentName().empty() )
				mCfg->set( cfgName, std::string("syphon::") + mDir->getCurrentName() );
		}
		// New media loaded
		else if ( mSource->isFresh() )
		{
			// Unselect Syphon
			if ( mSource->getType() == QB_SOURCE_SYPHON )
				;//mDir->refresh();
			else
				mDir->unselect();
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
	// get dropped file
	void ciGuiBlockQBSourceTab::onFileDrop( ci::app::FileDropEvent & event )
	{
		if ( event.getNumFiles() > 0 )
		{
			this->loadFile( event.getFile( 0 ).string() );
			event.setHandled();
		}
	}
	// Generic load
	void ciGuiBlockQBSourceTab::loadFile( const std::string & f )
	{
		if ( std::find( qb::_qbSourceExt.begin(), qb::_qbSourceExt.end(), getPathExtension(f) ) == qb::_qbSourceExt.end() )
			return;
		//mSource->play( _qb.isPlaying );
		mCfg->set( cfgName, f );
		mCfg->set( cfgFileName, f );
		mFileName = getPathFileName( f );
		mSelectedPath = f;
	}
} } // namespace cinder::sgui
#endif
	
	



/////////////////////////////////////////////////////////////////
//
// SYPHON BLOCK
//
#ifdef SYPHON
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
			mServerCount = valueLabels.size();
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
		mDir = new ciGuiBlockSyphonDirectory( cfg, _cfgName );
		mCfg->guiAddBlock( mDir );
		
		// Setup Client
		mClient.setup();
		mClient.setApplicationName( mCfg->getString( _cfgName ) );
		mClient.setServerName("");
		mClient.update();
		
		// Setup GUI
		mCfg->guiAddParam("Width",		&mClientWidth, true );
		mCfg->guiAddParam("Height",		&mClientHeight, true );
		mCfg->guiAddParam("Framerate",	&mClientFPS, 1, true );

		this->update();
	}
	
	void ciGuiBlockSyphon::update()
	{
		// New Syphon source selected
		if ( mDir->isFresh() )
			mClient.setApplicationName( mDir->getCurrentName() );
		
		// Update client
		if ( ! mDir->getCurrentName().empty() && mCfg->getBool(cfgEnabled) )
			mClient.update();
		
		// Refresh Data
		mClientFPS = mClient.getCurrentFrameRate();
		mClientWidth = mClient.getWidth();
		mClientHeight = mClient.getHeight();
	}
} } // namespace cinder::sgui
#endif
	
	
