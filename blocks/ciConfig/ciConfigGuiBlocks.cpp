//
//  ciConfigGuiBlocks.cpp
//
//  Created by Roger Sodre on 01/01/2013
//  Copyright 2013 Studio Avante. All rights reserved.
//

#include "ciConfigGuiBlocks.h"

	
/////////////////////////////////////////////////////////////////
//
// QB BLOCKS
//
#ifdef QB
#include "qb.h"
#include "qbSource.h"
namespace cinder { namespace sgui {
	ciGuiBlockQBSource::ciGuiBlockQBSource( ciConfigGui *cfg, qb::qbSourceSelector * src, const std::string &label, int cfgIdSelector, int cfgIdTrigger ) : ciConfigGuiBlock(cfg)
	{
		mSource = src;
		mSelectorControl = NULL;
		
		if ( label.length() )
			mConfig->guiAddText(label);
		mTextureControl = mConfig->guiAddTexture( "", &mTexture, 0.1 );
		mConfig->guiAddText( "", &mName );		// use label dor desc
		
		// make selector
		if ( cfgIdSelector >= 0 )
		{
			src->useConfigSelector( cfgIdSelector, cfg, false );
			mSelectorControl = (IntVarControl*) mConfig->guiAddParam(cfgIdSelector, "");
			mSelectorControl->setDisplayValue(false);
		}
		// dummy param
		else
		{
			mConfig->guiAddText( "", &mDesc );
			int min = (int)mConfig->getMin(cfgIdSelector);
			int max = (int)mConfig->getMin(cfgIdSelector);
			if ( max - min >= 1 )
				mConfig->mGui->addParam(label, mConfig->getPointerInt(cfgIdSelector), min, max, mConfig->getInt(cfgIdSelector));
		}
		
		// trigger
		if ( cfgIdTrigger >= 0 )
			src->useConfigTrigger( cfgIdTrigger, cfg );
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

} } // namespace cinder::sgui
#endif
	
	
	

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
		cfg->guiAddButton("Reset",							this, &ciGuiBlockLoadSave::cbReset)->setTriggerUp()->setPostGap(false);
		cfg->guiAddButton("Load Default",		cmd+"L",	this, &ciGuiBlockLoadSave::cbLoadDefault)->setTriggerUp()->setPostGap(false);
		cfg->guiAddButton("Save as Default",	cmd+"S",	this, &ciGuiBlockLoadSave::cbSaveDefault)->setTriggerUp();
		cfg->guiAddSeparator();
		cfg->guiAddButton("Import Config...",	cmd+"I",	this, &ciGuiBlockLoadSave::cbLoadFile)->setTriggerUp()->setPostGap(false);
		cfg->guiAddButton("Export Config...",	cmd+"E",	this, &ciGuiBlockLoadSave::cbSaveFile)->setTriggerUp();
	}
	
	void ciGuiBlockLoadSave::update()
	{
		mFilename = mConfig->getDisplayFileName();
	}
	
	//
	// FILE TAB CALLBACKS
	bool ciGuiBlockLoadSave::cbReset( ci::app::MouseEvent event ) {
		mConfig->reset();
		return false;
	}
	bool ciGuiBlockLoadSave::cbLoadDefault( ci::app::MouseEvent event ) {
		mConfig->load();
		return false;
	}
	bool ciGuiBlockLoadSave::cbSaveDefault( ci::app::MouseEvent event ) {
		mConfig->save();
		return false;
	}
	bool ciGuiBlockLoadSave::cbLoadFile( ci::app::MouseEvent event ) {
		return mConfig->import();
	}
	bool ciGuiBlockLoadSave::cbSaveFile( ci::app::MouseEvent event ) {
		return mConfig->exportas();
	}
} } // namespace cinder::sgui
#endif




/////////////////////////////////////////////////////////////////
//
// SYPHON BLOCK
//
#ifdef SYPHON
namespace cinder { namespace sgui {
	syphonServerDirectory _SyphonDirectory;
	ciGuiBlockSyphon::ciGuiBlockSyphon( ciConfigGui *cfg, int nameId, int enabledId, bool showInfo ) : ciConfigGuiBlock(cfg)
	{
		mCfgNameId = nameId;
		mCfgEnabledId = enabledId;
		mListId = mLastId = -1;
		mServerCount = 0;
		
		// Swtup Syphon
		_SyphonDirectory.setup();
		mClient.setup();
		mClient.setApplicationName( mConfig->getString( mCfgNameId ) );
		mClient.setServerName("");
		mClient.update();

		// Setup GUI
		mPanel = mConfig->guiAddPanel("ciGuiBlockSyphon");
		mLabel = cfg->guiAddText("---");	// update() will set
		std::map<int,std::string> valueLabels;
		mListControl = mConfig->guiAddParamList("", &mListId, valueLabels);
		if ( showInfo )
		{
			mConfig->guiAddParam("Width",		&mClientWidth, true );
			mConfig->guiAddParam("Height",		&mClientHeight, true );
			mConfig->guiAddParam("Framerate",	&mClientFPS, 1, true );
		}
		
		this->update();
	}
	
	void ciGuiBlockSyphon::update()
	{
		//printf("SY BLOCK [%s]  fresh %d\n",mConfig->getString(mCfgNameId),mConfig->isFresh(mCfgNameId));
		if ( _SyphonDirectory.hasChanged() || mConfig->isFresh(mCfgNameId) )
		{
			// Current Server we're using
			std::string current = mConfig->getString(mCfgNameId);
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
			mListId = -1;
			for (int i = 0 ; i < mListControl->items.size() ; i++ ) {
				if ( current.compare( mListControl->getValueLabel(i) ) == 0 )
				{
					mListId = i;
					break;
				}
			}
		}

		// New Syphon source selected
		// Checo newSyphonSource porque se os indices de SYPHON_SOURCE sao dinamicos
		if ( mLastId != mListId )
		{
			if (mListId >= 0)
			{
				std::string label = mListControl->getValueLabel( mListId );
				mConfig->set( mCfgNameId, label );
				mClient.setApplicationName( label );
				mClient.update();
			}
			else
				mClient.setApplicationName( "thisisnotasyphonserver" );
			mLastId = mListId;
		}
		
		// Update client
		if (mListId >= 0 && mConfig->getBool(mCfgEnabledId) )
			mClient.update();

		// Refresh Data
		mLabel->setName( mServerCount == 0 ? "No Available Clients!" : "Available Clients..." );
		mClientFPS = mClient.getCurrentFrameRate();
		mClientWidth = mClient.getWidth();
		mClientHeight = mClient.getHeight();
	}
} } // namespace cinder::sgui
#endif
	
	
