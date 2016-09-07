//
//  ciConfigGui.cpp
//
//  Created by Roger Sodre on 01/01/2013
//  Copyright 2013 Studio Avante. All rights reserved.
//

#include "ciConfigGui.h"
#include <sys/time.h>

#ifdef QB
#include "qbSource.h"
#endif

namespace cinder {
	
	//
	// CUSTOM CONFIG CLASS
	ciConfigGui::ciConfigGui() : ciConfig()
	{
		mTabCount = 0;
		mGroupCount = 0;

		mGui = new SimpleGUI( app::App::get() );
		//mGui->setName(mAppName+" "+mAppVersion);
		
		//mGui->sliderColor = ColorA(0, 1, 1, 1);		// cyan
		mGui->sliderColor = ColorA(0, 1, 0.8, 1);		// green
		//mGui->sliderColor = ColorA(1, 0.7, 0.7, 1);	// redish
	}
	ciConfigGui::~ciConfigGui()
	{
		for (std::vector<ciConfigGuiBlock*>::iterator it = mBlocks.begin() ; it != mBlocks.end() ; it++)
			delete *it;
		delete mGui;
	}

	void ciConfigGui::draw()
	{
		if ( !this->guiIsVisible() )
			return;

		// Update Blocks
		for (std::vector<ciConfigGuiBlock*>::iterator it = mBlocks.begin() ; it != mBlocks.end() ; it++)
		{
			ciConfigGuiBlock* block = *it;
			block->update();
		}
		mGui->update();

		// Draw GUI
		mGui->draw();
	}
	
	void ciConfigGui::guiSetLimits( int id, float vmin, float vmax )
	{
		if (params[id])
		{
			Control *c = this->guiGetControl(id);
			if ( c )
				c->setLimits( vmin, vmax );
		}
	}

	void ciConfigGui::guiSetName( int id, std::string name )
	{
		if (params[id])
			this->guiGetControl(id)->setName( name );
	}

	void ciConfigGui::guiUpdateValueLabels( int id )
	{
		if (params[id])
		{
			ListVarControl *c = (ListVarControl*) (params[id]->guiControl);
			if (c)
				c->update( params[id]->valueLabels );
		}
	}

	// virtuals
	void ciConfigGui::setCurrentDefault(int def)
	{
		this->ciConfig::setCurrentDefault(def);
		for (int id = 0 ; id < params.size() ; id++)
		{
			ciConfigParam *p = params[id];
			if (p == NULL)
				continue;
			if (p->guiControl == NULL)
				continue;
			if (this->isFloat(id))
			{
				float v = p->vec[0].getInitialValue(def);
				FloatVarControl *c = (FloatVarControl*) p->guiControl;
				c->setDefaultValue(&v);
			}
			else if (this->isInt(id))
			{
				int v = (int) p->vec[0].getInitialValue(def);
				IntVarControl *c = (IntVarControl*) p->guiControl;
				c->setDefaultValue(&v);
			}
			else if (this->isVector(id))
			{
				Vec4f v;
				for (int i = 0 ; i < 4 ; i++)
					v[i] = p->vec[i].getInitialValue(def);
				VectorVarControl *c = (VectorVarControl*) p->guiControl;
				c->setDefaultValue(&v);
			}
		}
	}
	void ciConfigGui::setLimits(int id, float vmin, float vmax)
	{
		this->ciConfig::setLimits( id, vmin, vmax );
		this->guiSetLimits( id, vmin, vmax );
	}



	//////////////////////////////
	//
	// VARIABLE PARAMS
	//
	LabelControl* ciConfigGui::guiAddParam( const std::string & label, std::string * var, bool wrap )
	{
		return mGui->addParam(label, var)->setWrap(wrap);
	}
	BoolVarControl* ciConfigGui::guiAddParam( const std::string & label, bool * var, bool readOnly )
	{
		return (BoolVarControl*) mGui->addParam(label, var, *var)->setReadOnly(readOnly);
	}
	IntVarControl* ciConfigGui::guiAddParam( const std::string & label, int * var, bool readOnly )
	{
		return (IntVarControl*) mGui->addParam(label, var, *var, 1, 0)->setReadOnly(readOnly);
	}
	FloatVarControl* ciConfigGui::guiAddParam( const std::string & label, float * var, int precision, bool readOnly )
	{
		return (FloatVarControl*) mGui->addParam(label, var, *var, 1, 0)->setPrecision(precision)->setReadOnly(readOnly);
	}
	ListVarControl* ciConfigGui::guiAddParamList( const std::string & label, int * var, const std::map<int,std::string> &valueLabels )
	{
		return mGui->addParamList(label, var, valueLabels);
	}
	ListVarControl* ciConfigGui::guiAddParamDropDown( const std::string & label, int * var, const std::map<int,std::string> &valueLabels )
	{
		return mGui->addParamDropDown(label, var, valueLabels);
	}
	TextureVarControl* ciConfigGui::guiAddTexture( const std::string & label, gl::Texture* tex, float refreshRate )
	{
		return mGui->addParam( label, tex, refreshRate );
	}
	
	

	////////////////////////////////////////////////////////
	//
	// STRUCTURE CONTROLS
	//
	void ciConfigGui::guiAddSeparator()
	{
		mGui->addSeparator();
	}
	LabelControl* ciConfigGui::guiAddText( const std::string & label, bool wrap )
	{
		return mGui->addLabel(label)->setWrap(wrap);
	}
	TabControl* ciConfigGui::guiAddTab( const std::string & _name, int id )
	{
		TabControl * c;
		if ( id >= 0 )
			c = mGui->addTab( _name, this->getPointerBool(id), this->getBool(id));
		else
			c = mGui->addTab( _name );
		mTabCount++;
		return c;
	}
	ColumnControl* ciConfigGui::guiAddGroup( const std::string & _name )
	{
		ColumnControl * c = mGui->addColumn();
		if ( _name.length() )
			c->label = mGui->addLabel(_name);
		mGroupCount++;
		return c;
	}
	PanelControl* ciConfigGui::guiAddPanel( const std::string & _name )
	{
		return mGui->addPanel(_name);
	}
	
	//////////////////////////////
	//
	// ciConfig PARAMS
	//
	Control* ciConfigGui::guiAddParamPrivate( int id, int i, const std::string & label, int precision )
	{
		ciConfigParam *p = params[id];
		if ( p == NULL )
			return NULL;
		
		Control *c = NULL;
		
		if (i >= 0)
		{
			c = mGui->addParam(label, this->getPointer(id,i), this->getMin(id,i), this->getMax(id,i), this->get(id,i));
			c->setDisplayValue();
			if (precision >= 0)
				((FloatVarControl*)c)->setPrecision( precision );
		}
		else if (p->valueLabels.size() > 0)
		{
			if ( this->testFlag(id,CFG_FLAG_DROP_DOWN) )
				c = mGui->addParamDropDown(label, this->getPointerInt(id), p->valueLabels );
			else
				c = mGui->addParamList(label, this->getPointerInt(id), p->valueLabels );
		}
		else if (this->isColor(id))
		{
			if ( this->testFlag(id,CFG_FLAG_ALPHA) )
				c = mGui->addParam(label, this->getPointerColorA(id), this->getColorA(id) );
			else
				c = mGui->addParam(label, this->getPointerColor(id), Color( this->getColor(id) ));
		}
		else if (this->isVector(id))
		{
			if ( this->isVector4(id) && ( p->quater || this->testFlag(id,CFG_FLAG_ARCBALL)) )
				c = mGui->addParam(label, this->getPointerVector(id), 4, this->getMin(id), this->getMax(id), this->getVector4(id) );
			else if ( this->isVector2(id) && (this->testFlag(id,CFG_FLAG_XY) || this->testFlag(id,CFG_FLAG_XY_VECTOR)) )
			{
				c = mGui->addParamXY(label, this->getPointerVector2(id), this->getMin(id), this->getMax(id), this->getVector2(id) );
				if ( this->testFlag(id,CFG_FLAG_XY_VECTOR) )
					((XYVarControl*)c)->setDrawAsVector();
				if (precision >= 0)
					((XYVarControl*)c)->setPrecision( precision );
			}
			else
			{
				c = mGui->addParam(label, this->getPointerVector(id), this->getVectorCount(id), this->getMin(id), this->getMax(id), this->getVector4(id) );
				if (precision >= 0)
					((VectorVarControl*)c)->setPrecision( precision );
			}
		}
		else if (this->isBool(id))
			c = mGui->addParam(label, this->getPointerBool(id), this->getBool(id));
		else if (this->isString(id))
			c = mGui->addParam(label, this->getPointerString(id), this->getString(id));
		else if (this->isByte(id))
		{
			c = mGui->addParamFlag(label, this->getPointerByte(id), (precision==-1?8:precision), this->getByte(id));
			((IntVarControl*)c)->setDisplayValue();
		}
		else if (this->isInt(id))
		{
			c = mGui->addParam(label, this->getPointerInt(id), (int)this->getMin(id), (int)this->getMax(id), this->getInt(id));
			c->setDisplayValue();
		}
		else
		{
			c = mGui->addParam(label, this->getPointer(id), this->getMin(id), this->getMax(id), this->get(id));
			c->setDisplayValue();
			if (precision >= 0)
				((FloatVarControl*)c)->setPrecision( precision );
		}
		// TODO...
		//options << "key='" << p->vec[i].keySwitch << "' ";
		//options << "keyincr='" << p->vec[i].keyInc << "' ";
		//options << "keydecr='" << p->vec[i].keyDec << "' ";
		
		// readonly
		if ( ! p->editable )
			c->setReadOnly( true );

		// Save reference
		p->guiControl = (void*) c;
		
		return c;
	}
	
	
	//////////////////////////////
	//
	// GUI Blocks
	//
	PanelControl* ciConfigGui::guiAddBlock( ciConfigGuiBlock *block )
	{
		mBlocks.push_back(block);
		return block->getPanel();
	}

} // namespace cinder



