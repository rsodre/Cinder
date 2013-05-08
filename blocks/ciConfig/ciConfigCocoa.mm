//
//  ciConfig.mm
//
//  Created by Roger Sodre on 08/04/2010
//  Copyright 2011 Studio Avante. All rights reserved.
//

#include "ciConfigCocoa.h"

using namespace cinder;
using namespace ci::app;
using namespace std;

namespace cinder {
	
	/////////////////////////////////////////////////////////////////////
	//
	// COCOA INTEGRATION
	//
	// Cocoa Slider
	void ciConfigCocoa::cocoaSetupSlider(int id, NSSlider *slider)
	{
		[slider setMinValue:this->getMin(id)];
		[slider setMaxValue:this->getMax(id)];
		switch (params[id]->type)
		{
			case CFG_TYPE_FLOAT:
			case CFG_TYPE_DOUBLE:
				[slider setFloatValue:this->get(id)];
				break;
			case CFG_TYPE_INTEGER:
			case CFG_TYPE_LONG:
			case CFG_TYPE_BYTE:
				[slider setIntValue:this->getInt(id)];
				break;
		}
		
	}
	void ciConfigCocoa::cocoaReadSlider(int id, NSSlider *slider)
	{
		this->set(id, [slider floatValue]);
	}
	//
	// Cocoa button
	void ciConfigCocoa::cocoaSetupButton(int id, NSButton *button)
	{
		[button setState: ( this->getBool(id) ? NSOnState : NSOffState )];
	}
	void ciConfigCocoa::cocoaReadButton(int id, NSButton *button)
	{
		this->set(id, (bool)( [button state] == NSOnState ? true : false ));
	}
	//
	// Cocoa Popup (combo box)
	void ciConfigCocoa::cocoaSetupPopup(int id, NSPopUpButton *pop)
	{
		int v = this->getInt(id);
		int min = (int)this->getMin(id);
		int max = (int)this->getMax(id);
		[pop removeAllItems];
		for (int n = min ; n <= max ; n++)
		{
			int ix = (n - min);
			if ( ix < params[id]->valueLabels.size())
				[pop addItemWithTitle:STR2NSS(params[id]->valueLabels[ix].c_str())];
			else
				[pop addItemWithTitle:[NSString stringWithFormat:@"%d",n]];
			if (v == n)
				[pop selectItemAtIndex:ix];
			//printf("GUI POP id(%d) ix %d = %d, current %d\n",id,ix,n,v);
		}
	}
	void ciConfigCocoa::cocoaReadPopup(int id, NSPopUpButton *pop)
	{
		int v = (int)[pop indexOfSelectedItem] + (int)this->getMin(id);
		this->set(id, v);
	}
	//
	// Cocoa Segmented
	void ciConfigCocoa::cocoaSetupSeg(int id, NSSegmentedControl *seg)
	{
		int v = this->getInt(id);
		int min = (int)this->getMin(id);
		int max = (int)this->getMax(id);
		int count = (max-min+1);
		[seg setSegmentCount:count];
		float w = ( ([seg frame].size.width-10) / count );
		for (int n = min ; n <= max ; n++)
		{
			int ix = (n - min);
			[seg setWidth:w forSegment:ix];
			if ( ix < params[id]->valueLabels.size())
				[seg setLabel:STR2NSS(params[id]->valueLabels[ix].c_str()) forSegment:ix];
			else
				[seg setLabel:[NSString stringWithFormat:@"%d",n] forSegment:ix];
			if (v == n)
				[seg setSelectedSegment:ix];
			//printf("GUI SEG id(%d) ix %d = %d, current %d\n",id,ix,n,v);
		}
	}
	void ciConfigCocoa::cocoaReadSeg(int id, NSSegmentedControl *seg)
	{
		int v = (int)[seg selectedSegment] + (int)this->getMin(id);
		this->set(id, v);
	}
	
	
	
} // namespace cinder
