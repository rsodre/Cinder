//
//  ciConfigGuiBlocks.h
//
//  Created by Roger Sodre on 01/01/2013
//  Copyright 2013 Studio Avante. All rights reserved.
//
#pragma once

// Allow Syphon params
#define CFG_BLOCK_SYPHON
#ifndef QB_NO_NDI
#define CFG_BLOCK_NDI
#endif

#include "ciConfigGui.h"
#include "cinder/app/AppBasic.h"
#ifdef CFG_BLOCK_NDI
#include "CinderNDIFinder.h"
#endif
#include <functional>

#define CICONFIG
//#define QB


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
		void cbImportConfig( ci::app::MouseEvent & event );
		void cbExportConfig( ci::app::MouseEvent & event );
	};
} } // namespace cinder::sgui
#endif




/////////////////////////////////
//
// QB
//
#ifdef QB
namespace cinder { namespace sgui {
	//
	// QB Source Single column
	class ciGuiBlockQBSource : public ciConfigGuiBlock {
	public:
		ciGuiBlockQBSource( ciConfigGui *cfg, qb::qbSourceSelector * src, const std::string &label, int _cfgSelector, int _cfgTrigger=-1 );
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
	//
	// QB Source TAB
	class ciGuiBlockSyphonDirectory;
	class ciGuiBlockNDIDirectory;
	class ciGuiBlockQBSourceTab : public ciConfigGuiBlock {
	public:
		ciGuiBlockQBSourceTab( ciConfigGui *cfg, qb::qbSourceSelector * src, const std::string &label, int _cfgSelector, int _cfgName, int _cfgFileName, int _cfgIgnoreAlpha=-1, std::function<void()> infoBlock=nullptr );
		~ciGuiBlockQBSourceTab() {}
		
		// virtuals
		void update();

	protected:
		ciGuiBlockSyphonDirectory	* mDirSyphon;
		ciGuiBlockNDIDirectory		* mDirNDI;
		qb::qbSourceSelector		* mSource;
		TextureVarControl			* mTextureControl;
		IntVarControl				* mSelectorControl;
		PanelControl				* mPanelMovie;
		PanelControl				* mPanelPlayhead;

		int					cfgName;
		int					cfgFileName;
		int					cfgIgnoreAlpha;
		std::string			mSelectedPath;
		bool				bTemplateEnabled;
		bool				bMovieEnabled;
		bool				bSyphonEnabled;
		bool				bLastMovieEnabled;
		bool				bLastMoviePlaying;
		bool				bIgnoreAlpha;
		int					iii;

		// gui
		gl::Texture			mTexture;
		std::string			mType;
		std::string			mName;
		std::string			mDesc;
		std::string			mFileName;
		int					mWidth;
		int					mHeight;
		float				mFPS;
		float				mCurrFPS, mCurrFPSSyphon, mCurrFPSNDI;

		// callbacks
		void cbLoad( ci::app::MouseEvent & event );
		void cbPlay( ci::app::MouseEvent & event );
		void cbRewind( ci::app::MouseEvent & event );
#ifdef BDVJ
		void cbDomePatterns( ci::app::MouseEvent & event );
#endif
		void onFileDrop( ci::app::FileDropEvent & event );
		bool loadFile( const std::string & f );
	};
} } // namespace cinder::sgui
#endif



/////////////////////////////////
//
// Syphon
//
#ifdef CFG_BLOCK_SYPHON
#include "cinderSyphon.h"
namespace cinder { namespace sgui {
	//
	// Syphon Directory block
	extern syphonServerDirectory _SyphonDirectory;
	class ciGuiBlockSyphonDirectory : public ciConfigGuiBlock {
	public:
		ciGuiBlockSyphonDirectory( ciConfigGui *cfg, int _cfgName );
		~ciGuiBlockSyphonDirectory() {}
		
		void exclude( const std::string & name )	{ mExclude = name; }
		void unselect()								{ mListId = -1; }
		void refresh();
		
		bool isFresh()					{ return bFreshness; }
		std::string getCurrentName()	{ return mCurrentName; }
		bool isSelected()				{ return (mListId >= 0); }
		
		// virtuals
		void update();
		
	private:

		void reselect( const std::string & name );

		ListVarControl *	mListControl;
		LabelControl *		mLabel;
		
		int				cfgName;
		int				mListId, mLastId;	// List id watcher
		bool			bFreshness;			// selected/unselected new
		int				mServerCount;
		std::string		mExclude;
		std::string		mCurrentName;
	};
	
	//
	// Syphon Client block
	class ciGuiBlockSyphon : public ciConfigGuiBlock {
	public:
		ciGuiBlockSyphon( ciConfigGui *cfg, int _cfgName, int _cfgEnabled=-1 );
		~ciGuiBlockSyphon() {}
		
		void exclude( const std::string & name )	{ mDirBlock->exclude(name); }
		
		// virtuals
		void update();
		
	private:
		
		ciGuiBlockSyphonDirectory	*mDirBlock;
//		syphonClient			mClient;

		int			cfgEnabled;
		float		mClientFPS;
		int			mClientWidth;
		int			mClientHeight;
	};
} } // namespace cinder::sgui
#endif




/////////////////////////////////
//
// NDI
//
#ifdef CFG_BLOCK_NDI
//#include "cinderSyphon.h"
namespace cinder { namespace sgui {
	//
	// NDI Directory block
	class ciGuiBlockNDIDirectory : public ciConfigGuiBlock {
	public:
		ciGuiBlockNDIDirectory( ciConfigGui *cfg, int _cfgName );
		~ciGuiBlockNDIDirectory();
		
		void exclude( const std::string & name )	{ mExclude = name; }
		void unselect()								{ mListId = -1; }
		void refresh();
		
		bool isFresh()					{ return bFreshness; }
		std::string getCurrentName()	{ return mCurrentName; }
		bool isSelected()				{ return (mListId >= 0); }
		
		// virtuals
		void update();
		
	private:
		
		void reselect( const std::string & name );
		
		ListVarControl *	mListControl;
		LabelControl *		mLabel;
		
		int				cfgName;
		int				mListId, mLastId;	// List id watcher
		bool			bFreshness;			// selected/unselected new
		int				mServerCount;
		std::string		mExclude;
		std::string		mCurrentName;
		bool			bDirChanged;
		
		ci::signals::connection mNDISourceAdded;
		ci::signals::connection mNDISourceRemoved;
		
		void sourceAdded( const NDISource& source );
		void sourceRemoved( const std::string sourceName );
	};
	
	//
	// NDI Client block
	class ciGuiBlockNDI : public ciConfigGuiBlock {
	public:
		ciGuiBlockNDI( ciConfigGui *cfg, int _cfgName, int _cfgEnabled=-1 );
		~ciGuiBlockNDI() {}
		
		void exclude( const std::string & name )	{ mDirBlock->exclude(name); }
		
		// virtuals
		void update();
		
	private:
		
		ciGuiBlockNDIDirectory*	mDirBlock;
//		syphonClient			mClient;

		int			cfgEnabled;
		float		mClientFPS;
		int			mClientWidth;
		int			mClientHeight;
	};
} } // namespace cinder::sgui
#endif

