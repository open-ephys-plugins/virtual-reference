/*
------------------------------------------------------------------

This file is part of the Open Ephys GUI
Copyright (C) 2014 Open Ephys

------------------------------------------------------------------

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef __VIRTUALREFCANVAS_H__
#define __VIRTUALREFCANVAS_H__

#include <EditorHeaders.h>
#include <VisualizerWindowHeaders.h>
#include <VisualizerEditorHeaders.h>

#include "VirtualRef.h"

class ElectrodeTableButton;
class VirtualRefDisplay;

class VirtualRefCanvas : public Visualizer,
						 public Button::Listener,
						 public ComboBox::Listener,
						 public Slider::Listener
{
public:

    /** Constructor */
	VirtualRefCanvas(VirtualRef* n);
    
	/** Destructor */
	~VirtualRefCanvas();

	/** Fills background */
	void paint(Graphics& g) override;

    /** Renders the Visualizer on each animation callback cycle */
	void refresh() override;

	void beginAnimation() override;
    void endAnimation() override;

    /** Called when the Visualizer's tab becomes visible after being hidden */
	void refreshState() override;

	/** Sets bounds of sub-components*/
	void resized() override;

	/** Called when the Visualizer is first created, and optionally when
		the parameters of the underlying processor are changed */
	void update();

    /** Respond to button clicks*/
	void buttonClicked(Button* button) override;

	bool updateNeeded = false;

	/** Respond to combo box changes*/
	void comboBoxChanged(ComboBox* cb) override;

	/** Respond to slider changes*/
	void sliderValueChanged(Slider* slider) override;

private:

	std::unique_ptr<VirtualRefDisplay> display;
	VirtualRef* processor;
	std::unique_ptr<Viewport> displayViewport;

	std::unique_ptr<ComboBox> presetNamesBox;
	std::unique_ptr<Label> presetNamesLabel;
	std::unique_ptr<ComboBox> channelCountBox;
	std::unique_ptr<Label> channelCountLabel;
	StringArray presetNames;
	StringArray channelCounts;

	LookAndFeel_V4 lnf4;

	std::unique_ptr<UtilityButton> resetButton;
	std::unique_ptr<UtilityButton> selectModeButton;
	std::unique_ptr<UtilityButton> saveButton;
	std::unique_ptr<UtilityButton> loadButton;
	std::unique_ptr<Slider> gainSlider;

	OwnedArray<ElectrodeTableButton> electrodeButtons;

	int scrollBarThickness;
	int scrollDistance;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VirtualRefCanvas);

};


class ElectrodeTableButton : public ElectrodeButton
{
public:

	ElectrodeTableButton(int index, int rowIndex_, int colIndex_) : ElectrodeButton(index), rowIndex(rowIndex_), colIndex(colIndex_)
	{
	}

	int getRowIndex()
	{
		return rowIndex;
	}

	int getColIndex()
	{
		return colIndex;
	}

private:

	int rowIndex;
	int colIndex;

};


class CarButton : public ElectrodeButton
{
public:

	CarButton(String label_, int channelIndex_) : ElectrodeButton(channelIndex_)
	{
		setButtonText(label_);
	}
};


class VirtualRefDisplay : public Component, public Button::Listener, public KeyListener
{
public:

    /** Constructor */
	VirtualRefDisplay(VirtualRef*, VirtualRefCanvas*, Viewport*, bool selectMode = false);
    
	/** Destructor */
	~VirtualRefDisplay();

    void paint(Graphics& g);

	/** Updates the reference matrix view*/
	void update();

	/** Respond to combo box changes*/
	void buttonClicked(Button* button);

	bool keyPressed(const KeyPress &key, Component *originatingComponent);

	/** Draws the Electrode Button table from the reference matrix */
	void drawTable();

	/** Reset's the table channel states */
	void reset();

	/** Enable single channel selection mode */
	void setEnableSingleSelectionMode(bool b);

	/** Apply a preset from the list */
	void applyPreset(String name, int numChannels);

private:

    int nChannelsBefore;
	bool singleSelectMode;
	int selectedRow;
	int selectedColumn;

	VirtualRef* processor;
	VirtualRefCanvas* canvas;
    Viewport* viewport;

	OwnedArray<ElectrodeTableButton> electrodeButtons;
	OwnedArray<CarButton> carButtons;
	OwnedArray<Label> rowLabels;
	OwnedArray<Label> headerLabels;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VirtualRefDisplay);

};


#endif // __VIRTUALREFCANVAS_H__

