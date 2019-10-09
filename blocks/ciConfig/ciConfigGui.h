//
//  ciConfigGui.h
//
//  Created by Roger Sodre on 01/01/2013
//  Copyright 2013 Studio Avante. All rights reserved.
//
// GUI for Cinder based on SimpleGui
// https://github.com/vorg/MowaLibs
// http://forum.libcinder.org/topic/simplegui
//
#pragma once

#include "ciConfig.h"
#include "SimpleGUI.h"

namespace cinder { namespace qb {
	class qbSourceSelector;
} }

using namespace ci::sgui;

namespace cinder {
	
	/////////////////////////////////////////////
	//
	// GUI Blocks
	//
	class ciConfigGui;
	class ciConfigGuiBlock {
	public:
		ciConfigGuiBlock( ciConfigGui *cfg ) : mCfg(cfg), mPanel(NULL) {}
		~ciConfigGuiBlock() {}
		
		PanelControl*	getPanel()	{ return mPanel; }
		
		// virtuals
		virtual void	update()	{}
		
	protected:
		ciConfigGui		*mCfg;
		PanelControl	*mPanel;
	};
	
	
	/////////////////////////////////////////////
	//
	// GUI Config Class
	//
	class ciConfigGui : public ciConfig {
	public:

		ciConfigGui();
		~ciConfigGui();
		
		virtual void draw();
		
		// Setters
		void		guiSetLimits( int id, float vmin, float vmax );
		void		guiSetOffset( const Vec2f & o )		{ mGui->mOffset = o; }
		void		guiSetName( std::string name )		{ mGui->setName(name); }
		void		guiSetName( int id, std::string name );
		void		guiUpdateValueLabels(int id);
		// Getters
		Control*	guiGetControl(int id)			{ return (Control*)( params[id] ? params[id]->guiControl : NULL ); }
		int			guiGetColumnWidth()				{ return mGui->getColumnWidth(); }
		bool		guiHasChanged()					{ return mGui->anythingChanged(); }
		float		guiGetAlpha()					{ return mGui->getAlpha(); }
		Vec2f		guiGetSize()					{ return mGui->getSize(); }
		int			guiGetTabId()					{ return mGui->getTabId(); }
		void		guiSetTab(int t)				{ return mGui->setTab(t); }
		bool		guiIsInteracting(int id)		{ return ( params[id] ? ((Control*)(params[id]->guiControl))->isInteracting() : false ); }
		
		// Wrappers
		void		guiShowHide()					{ if (mGui->isVisible()) this->guiHide(); else this->guiShow(); }
		void		guiHide()						{ mGui->setVisible(false);  }
		void		guiShow( bool b = true )		{ mGui->setVisible(b); }
		bool		guiIsVisible()					{ return mGui->isVisible(); };

		// virtuals
		void		setCurrentDefault(int def);
		void		setLimits(int id, float vmin, float vmax);

		// variable controls
		LabelControl*		guiAddParam( const std::string & label, std::string * var, bool wrap=false );
		BoolVarControl*		guiAddParam( const std::string & label, bool * var, bool readOnly=false );
		IntVarControl*		guiAddParam( const std::string & label, int * var, bool readOnly=false );
		FloatVarControl*	guiAddParam( const std::string & label, float * var, int precision, bool readOnly=false );
		ListVarControl*		guiAddParamList( const std::string & label, int * var, const std::map<int,std::string> &valueLabels );
		ListVarControl*		guiAddParamDropDown( const std::string & label, int * var, const std::map<int,std::string> &valueLabels );
		TextureVarControl*	guiAddTexture( const std::string & label, gl::Texture* tex, float refreshRate=1.0f );
		// typed shortcuts
		BoolVarControl*		guiAddParamBool( const std::string & label, bool * var, bool readOnly=false ) { return (BoolVarControl*)guiAddParam(label, var, readOnly); }
		
		// structure controls
		void				guiAddSeparator();
		LabelControl*		guiAddText( const std::string & label, bool wrap=false );
		TabControl*			guiAddTab( const std::string & label, int id=-1 );
		ColumnControl*		guiAddGroup( const std::string & label="" );
		PanelControl*		guiAddPanel( const std::string & label="" );
		PanelControl*		guiAddBlock( ciConfigGuiBlock *block );

		ButtonControl*		guiAddButton(const std::string & label )
		{
			return mGui->addButton(label);
		}
		ButtonControl*		guiAddButton(const std::string & label, std::function<void (app::MouseEvent&)> callbackUp )
		{
			ButtonControl *c = mGui->addButton(label);
			c->registerClickUp( callbackUp );
			return c;
		}
		ButtonControl*		guiAddButton(const std::string & label, std::function<void (app::MouseEvent&)> callbackDown, std::function<void (app::MouseEvent&)> callbackUp )
		{
			ButtonControl *c = mGui->addButton(label);
			c->registerClickDown( callbackDown );
			c->registerClickUp( callbackUp );
			return c;
		}
		ButtonControl*		guiAddButton(const std::string & label, const std::string & label2, std::function<void (app::MouseEvent&)> callbackUp )
		{
			ButtonControl *c = mGui->addButton(label,label2);
			c->registerClickUp( callbackUp );
			return c;
		}
		ButtonControl*		guiAddButton(const std::string & label, const std::string & label2, std::function<void (app::MouseEvent&)> callbackDown, std::function<void (app::MouseEvent&)> callbackUp )
		{
			ButtonControl *c = mGui->addButton(label,label2);
			c->registerClickDown( callbackDown );
			c->registerClickUp( callbackUp );
			return c;
		}
		template<typename T>
		ButtonControl*		guiAddButton(const std::string & label, T *obj, void (T::*callbackUp)(app::MouseEvent&) )
		{
			ButtonControl *c = mGui->addButton(label);
			c->registerClickUp( obj, callbackUp );
			return c;
		}
		template<typename T>
		ButtonControl*		guiAddButton(const std::string & label, T *obj, void (T::*callbackDown)(app::MouseEvent&), void (T::*callbackUp)(app::MouseEvent&) )
		{
			ButtonControl *c = mGui->addButton(label);
			c->registerClickDown( obj, callbackDown );
			c->registerClickUp( obj, callbackUp );
			return c;
		}
		template<typename T>
		ButtonControl*		guiAddButton(const std::string & label, const std::string & label2, T *obj, void (T::*callbackUp)(app::MouseEvent&) )
		{
			ButtonControl *c = mGui->addButton(label,label2);
			c->registerClickUp( obj, callbackUp );
			return c;
		}
		template<typename T>
		ButtonControl*		guiAddButton(const std::string & label, const std::string & label2, T *obj, void (T::*callbackDown)(app::MouseEvent&), void (T::*callbackUp)(app::MouseEvent&) )
		{
			ButtonControl *c = mGui->addButton(label,label2);
			c->registerClickDown( obj, callbackDown );
			c->registerClickUp( obj, callbackUp );
			return c;
		}


		//
		// ciConfig Controls
		Control*	guiAddParam(int id, const std::string & label="", int precision=-1)		{ return this->guiAddParamPrivate(id, -1, label, precision); }	// -1 = auto
		Control*	guiAddParamX(int id, const std::string & label="", int precision=-1)	{ return this->guiAddParamPrivate(id,  0, label, precision); }
		Control*	guiAddParamY(int id, const std::string & label="", int precision=-1)	{ return this->guiAddParamPrivate(id,  1, label, precision); }
		Control*	guiAddParamZ(int id, const std::string & label="", int precision=-1)	{ return this->guiAddParamPrivate(id,  2, label, precision); }
		// typed shortcuts
		BoolVarControl*		guiAddParamBool(int id, const std::string & label="") { return (BoolVarControl*)guiAddParam(id,label); }
		FloatVarControl*	guiAddParamFLoat(int id, const std::string & label="", int precision=-1) { return (FloatVarControl*)guiAddParam(id,label,precision); }

		// SimpleGui Params
		SimpleGUI* mGui;
		
	protected:
		
		Control*	guiAddParamPrivate(int id, int i, const std::string & label, int precision);
		
		int			mTabCount;
		int			mGroupCount;
		
		// Blocks
		std::vector<ciConfigGuiBlock*>	mBlocks;
		
	};
	
	
	
	
} // namespace cinder




