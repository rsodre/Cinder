//
//  ciConfigGuiSimple.h
//
//  Created by Roger Sodre on 08/04/2010
//  Copyright 2010 Studio Avante. All rights reserved.
//
#pragma once

#include "ciConfigGuiBase.h"
#include "SimpleGUI.h"

namespace cinder {
	
	//
	// GUI for Cinder based on SimpleGui
	// https://github.com/vorg/MowaLibs
	// http://forum.libcinder.org/topic/simplegui
	//
	
	//
	// Config Clas
	class ciConfigGuiSimple : public ciConfigGuiBase {
	public:
		using ciConfigGuiBase::guiAddParam;		// for virtual polyphormism

		ciConfigGuiSimple();
		~ciConfigGuiSimple();
		
		// Virtuals
		void	guiSetOffset( const Vec2f & o )		{ mGui->mOffset = o; }
		int		guiGetColumnWidth()					{ return mGui->getColumnWidth(); }
		void	guiAddGroup( std::string _name );
		void*	guiAddPanel();
		void	guiAddSeparator();
		void*	guiAddText( std::string text );
		mowa::sgui::ButtonControl*	guiAddButton(const std::string &label, ciConfigGuiBase *obj, bool (ciConfigGuiBase::*callback)(ci::app::MouseEvent) );
		//template<typename T> CallbackId guiAddButton(const std::string &label, T *obj, bool (T::*callback)(MouseEvent) );
		void	guiUpdateValueLabels(int id);
		void	guiShow( bool b = true )			{ mGui->setEnabled(b); };
		bool	guiIsVisible()						{ return mGui->isEnabled(); };
		bool	guiHasChanged()						{ return mGui->hasChanged(); };
		Vec2f	guiGetSize()						{ return mGui->getSize(); };
		void	drawGui();
		
		// Generic params
		void*	guiAddParam(int *val, const std::string &label, bool readOnly=false);
		void*	guiAddParam(float *val, const std::string &label, int precision, bool readOnly=false);

		
	protected:
		
		// Virtuals
		void*	guiAddParamPrivate(int id, int i, const std::string &label, int precision);
		void	guiSetLabel(int id, int i, const std::string &label);
		
		// SimpleGui Params
		mowa::sgui::SimpleGUI::SimpleGUI* mGui;
		
		
	};
	
	
} // cinder::
