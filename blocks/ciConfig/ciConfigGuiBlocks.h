//
//  ciConfigGuiBlocks.h
//
//  Created by Roger Sodre on 01/01/2013
//  Copyright 2013 Studio Avante. All rights reserved.
//
#pragma once

#include "ciConfigGui.h"

#define CICONFIG
//#define QB
#define SYPHON


/////////////////////////////////
//
// QB
//
#ifdef QB
namespace cinder { namespace sgui {
	//
	// Lacy Auth block
	class ciGuiBlockQBSource : public ciConfigGuiBlock {
	public:
		ciGuiBlockQBSource( ciConfigGui *cfg, qb::qbSourceSelector * src, const std::string &label, int cfgIdSelector, int cfgIdTrigger=-1 );
		~ciGuiBlockQBSource() {}
		
		// virtuals
		void update();
		
	protected:
		qb::qbSourceSelector	* mSource;
		TextureVarControl		* mTextureControl;
		IntVarControl			* mSelectorControl;
		gl::Texture				mTexture;
		std::string				mName;
		std::string				mDesc;
	};
} } // namespace cinder::sgui
#endif



/////////////////////////////////
//
// ciConfig
//
#ifdef CICONFIG
#include "ciConfig.h"
namespace cinder { namespace sgui {
	//
	// Lacy Auth block
	class ciGuiBlockLoadSave : public ciConfigGuiBlock {
	public:
		ciGuiBlockLoadSave( ciConfigGui *cfg );
		~ciGuiBlockLoadSave() {}
		
		// virtuals
		void update();
		
	protected:
		std::string mFilename;

		void cbReset( ci::app::MouseEvent & event );
		void cbLoadDefault( ci::app::MouseEvent & event );
		void cbSaveDefault( ci::app::MouseEvent & event );
		void cbLoadFile( ci::app::MouseEvent & event );
		void cbSaveFile( ci::app::MouseEvent & event );
	};
} } // namespace cinder::sgui
#endif




/////////////////////////////////
//
// Syphon
//
#ifdef SYPHON
#include "cinderSyphon.h"
namespace cinder { namespace sgui {
	//
	// Syphon Client block
	extern syphonServerDirectory _SyphonDirectory;
	class ciGuiBlockSyphon : public ciConfigGuiBlock {
	public:
		ciGuiBlockSyphon( ciConfigGui *cfg, int nameId, int enabledId=-1, bool showInfo=true );
		~ciGuiBlockSyphon() {}
		
		void exclude( const std::string & name)	{ mExclude = name; }
		
		syphonClient * getClient()				{ return &mClient; }
		
		// virtuals
		void update();
		
	protected:
		syphonClient		mClient;
		ListVarControl *	mListControl;
		LabelControl *		mLabel;
		
		int			mCfgNameId;
		int			mCfgEnabledId;
		int			mListId;			// List id watcher
		int			mLastId;			// Last id selected
		std::string	mExclude;
		float		mClientFPS;
		int			mClientWidth;
		int			mClientHeight;
		int			mServerCount;
	};
} } // namespace cinder::sgui
#endif
	
