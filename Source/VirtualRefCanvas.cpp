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


#include "VirtualRefCanvas.h"
#include "VirtualRefEditor.h"


#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))


VirtualRefCanvas::VirtualRefCanvas(VirtualRef* n) :
    processor(n)
{

    displayViewport = std::make_unique<Viewport>("VirtualRefDisplay");
	display = std::make_unique<VirtualRefDisplay>(processor, this, displayViewport.get());
	displayViewport->setViewedComponent(display.get(), false);
    displayViewport->setScrollBarsShown(true, true);
	addAndMakeVisible(displayViewport.get());

	scrollBarThickness = displayViewport->getScrollBarThickness();
	scrollDistance = 0;

	Font buttonFont("Fira Sans", "SemiBold", 13.0f);

    resetButton = std::make_unique<UtilityButton>("Reset", buttonFont);
    resetButton->setRadius(3.0f);
    resetButton->addListener(this);
    addAndMakeVisible(resetButton.get());

    selectModeButton = std::make_unique<UtilityButton>("Single mode", buttonFont);
	selectModeButton->setTooltip("Allows only one channel per row to be selected at a time");
    selectModeButton->setRadius(4.0f);
    selectModeButton->setClickingTogglesState(true);
    selectModeButton->setToggleState(false, sendNotification);
    selectModeButton->addListener(this);
    addAndMakeVisible(selectModeButton.get());

    saveButton = std::make_unique<UtilityButton>("Save", buttonFont);
    saveButton->setRadius(3.0f);
    saveButton->addListener(this);
    addAndMakeVisible(saveButton.get());

    loadButton = std::make_unique<UtilityButton>("Load", buttonFont);
    loadButton->setRadius(3.0f);
    loadButton->addListener(this);
    addAndMakeVisible(loadButton.get());

    gainSlider = std::make_unique<Slider>("Gain");
	gainSlider->setLookAndFeel(&lnf4);
	gainSlider->setTooltip("Set the global gain value");
	gainSlider->setSliderStyle(Slider::Rotary);
    gainSlider->setRange(0.0f, 2.0f, 0.01f);
    gainSlider->setTextBoxStyle(Slider::TextBoxRight, false, 40, 24);
    gainSlider->setValue(1.0f);
	gainSlider->setColour(Slider::textBoxBackgroundColourId, Colour(211, 211, 211));
	gainSlider->setColour(Slider::textBoxTextColourId , Colour(0, 0, 0));
	gainSlider->setColour(Slider::rotarySliderFillColourId, Colour (240, 179, 12).darker(0.5f));
	gainSlider->setColour(Slider::thumbColourId, Colour (240, 179, 12));
    gainSlider->addListener(this);
    addAndMakeVisible(gainSlider.get());

	Font labelFont("Fira Sans", "SemiBold", 16.0f);

	presetNamesLabel = std::make_unique<Label>("PresetLabel", "Preset:");
	presetNamesLabel->setFont(labelFont);
	addAndMakeVisible(presetNamesLabel.get());

	presetNames.add("None");
	presetNames.add("Common average reference");
	presetNames.add("Avg of other tetrodes");
	presetNames.add("Avg of next tetrode");

    presetNamesBox = std::make_unique<ComboBox>("Presets");
    presetNamesBox->addItemList(presetNames, 1);
    presetNamesBox->setSelectedId(1, sendNotification);
    presetNamesBox->setEditableText(false);
    presetNamesBox->addListener(this);
    addAndMakeVisible(presetNamesBox.get());

	channelCountLabel = std::make_unique<Label>("ChanCountLabel", "No. of channels:");
	channelCountLabel->setFont(labelFont);
	addAndMakeVisible(channelCountLabel.get());

	for (int i=1; i<=8; i++)
	{
		channelCounts.add(String(i*16));
	}

    channelCountBox = std::make_unique<ComboBox>("Channels");
    channelCountBox->addItemList(channelCounts, 1);
    channelCountBox->setSelectedId(1, sendNotification);
    channelCountBox->setEditableText(false);
    channelCountBox->addListener(this);
    addAndMakeVisible(channelCountBox.get());

    update();
}

VirtualRefCanvas::~VirtualRefCanvas()
{
}

void VirtualRefCanvas::beginAnimation()
{
//    startCallbacks();
}

void VirtualRefCanvas::endAnimation()
{
//    stopCallbacks();
}


void VirtualRefCanvas::paint(Graphics& g)
{
    g.fillAll(Colours::grey);
}

void VirtualRefCanvas::refresh()
{
    repaint();
}

void VirtualRefCanvas::refreshState()
{
	update();
    resized();
}

void VirtualRefCanvas::resized()
{
    displayViewport->setBounds(20, 20, getWidth() - 40, getHeight()-90);
    resetButton->setBounds(10, getHeight()-60, 100, 20);
	selectModeButton->setBounds(110, getHeight()-60, 100, 20);
	loadButton->setBounds(10, getHeight()-30, 100, 20);
	saveButton->setBounds(110, getHeight()-30, 100, 20);

	gainSlider->setBounds(220, getHeight()-75, 140, 80);

	channelCountLabel->setBounds(380, getHeight()-30, 120, 20);
	channelCountBox->setBounds(500, getHeight()-30, 200, 20);
	presetNamesLabel->setBounds(380, getHeight()-60, 120, 20);
	presetNamesBox->setBounds(500, getHeight()-60, 200, 20);
}

void VirtualRefCanvas::update()
{
	display->update();
	gainSlider->setValue(processor->getGlobalGain());
}

void VirtualRefCanvas::mouseDown(const MouseEvent& event)
{

}

void VirtualRefCanvas::buttonClicked(Button* b)
{
	UtilityButton *button = dynamic_cast<UtilityButton*>(b);

	if (button == resetButton.get())
	{
		display->reset();
	}
	else if (button == selectModeButton.get())
	{
		display->setEnableSingleSelectionMode(button->getToggleState());
		display->grabKeyboardFocus();
	}
	else if (button == loadButton.get())
	{
		VirtualRefEditor* editor = dynamic_cast<VirtualRefEditor*>(processor->getEditor());
		editor->loadParametersDialog();
	}
	else if (button == saveButton.get())
	{
		VirtualRefEditor* editor = dynamic_cast<VirtualRefEditor*>(processor->getEditor());
		editor->saveParametersDialog();
	}
}

void VirtualRefCanvas::comboBoxChanged(ComboBox* cb)
{
	if (cb == presetNamesBox.get() || cb == channelCountBox.get())
	{
		String presetName = presetNames[presetNamesBox->getSelectedId()-1];
		String s = channelCounts[channelCountBox->getSelectedId()-1];
		int numChannels = s.getIntValue();
		display->applyPreset(presetName, numChannels);
	}
}

void VirtualRefCanvas::sliderValueChanged(Slider* slider)
{
    if (slider == gainSlider.get())
	{
		processor->setGlobalGain(gainSlider->getValue());
	}
}



// ----------------------------------------------------------------

VirtualRefDisplay::VirtualRefDisplay(VirtualRef* n, VirtualRefCanvas* c, Viewport* v, bool selectMode) :
    processor(n), canvas(c), viewport(v), nChannelsBefore(-1), singleSelectMode(selectMode)
{
	addKeyListener(this);
	drawTable();
}

VirtualRefDisplay::~VirtualRefDisplay()
{
}

void VirtualRefDisplay::drawTable()
{
	int nChannels = processor->getReferenceMatrix()->getNumberOfChannels();

	if (nChannels != nChannelsBefore)
	{
		/* Move these values somewhere else? */
		int xOffset = 50;
		int yOffset = 1;
		int cellWidth = 19;
		int cellHeight = 15;
		int vSpace = 1;
		int headerHeight = 20;
		int carWidth = 35;

		int totalWidth = xOffset + carWidth + nChannels * cellWidth;
		int totalHeigth = yOffset + headerHeight + nChannels * (cellHeight + vSpace);

		headerLabels.clear();
		rowLabels.clear();
		electrodeButtons.clear();
		carButtons.clear();

		/* Create header */
		Font font("Fira Sans", "Bold", 14.0f);

		Label* header1 = new Label("headerCol1", "Channel");
		header1->setJustificationType(Justification::horizontallyCentred);
		header1->setBounds(0, yOffset, xOffset, headerHeight);
		header1->setFont(font);
		addAndMakeVisible(header1);

		headerLabels.add(header1);

		Label* header2 = new Label("headerCol2", "");
		header2->setJustificationType(Justification::horizontallyCentred);
		header2->setBounds(xOffset, yOffset, carWidth, headerHeight);
		header2->setFont(font);
		addAndMakeVisible(header2);

		headerLabels.add(header2);

		Label* header3 = new Label("headerCol3", "Reference(s)");
		header3->setJustificationType(Justification::horizontallyCentred);
		header3->setBounds(xOffset + carWidth, yOffset, totalWidth - xOffset - carWidth, headerHeight);
		header3->setFont(font);
		addAndMakeVisible(header3);

		headerLabels.add(header3);

		/* Add button matrix */
		for (int i=0; i<nChannels; i++)
		{
			/* Row label */
			Label* label = new Label(String::formatted("row_label_%d", i), String::formatted("%d", i+1));
			label->setJustificationType(Justification::horizontallyCentred);
			label->setBounds(0, yOffset + headerHeight + i*(cellHeight + vSpace), xOffset, cellHeight);
			label->setFont(font);
			addAndMakeVisible(label);

			rowLabels.add(label);

			/* Button to select all channels as reference (aka common average reference) */
			CarButton* cb = new CarButton("all", i);
			cb->setToggleState(false, dontSendNotification);
			cb->setBounds(xOffset + 5, yOffset + headerHeight + i*(cellHeight + vSpace), carWidth - 10, cellHeight);
			cb->addListener(this);
			cb->setRadioGroupId(0);
			addAndMakeVisible(cb);

			carButtons.add(cb);

			for (int j=0; j<nChannels; j++)
			{
				bool state = processor->getReferenceMatrix()->getValue(i, j) > 0;

				ElectrodeTableButton* button = new ElectrodeTableButton(j+1, i, j);
				button->setToggleState(state, dontSendNotification);
				button->setRadioGroupId(0);
				button->setBounds(xOffset + carWidth + j*cellWidth, yOffset + headerHeight + i*(cellHeight + vSpace), cellWidth, cellHeight);
				button->addListener(this);
				addAndMakeVisible(button);

				electrodeButtons.add(button);
			}
		}

		setSize(totalWidth, totalHeigth);
		setBounds(0, 0, totalWidth, totalHeigth);
		nChannelsBefore = nChannels;
	}
	else
	{
		/* Only update toggle states of buttons */
		int numRefs = 0;
		for (int i=0; i<nChannels; i++)
		{
			for (int j=0; j<nChannels; j++)
			{
				bool state = processor->getReferenceMatrix()->getValue(i, j) > 0;

				if(state)
					numRefs++;

				ElectrodeTableButton* button = electrodeButtons[i*nChannels + j];
				button->setToggleState(state, dontSendNotification);
			}

			if(numRefs == nChannels)
				carButtons[i]->setToggleState(true, dontSendNotification);
			else
				carButtons[i]->setToggleState(false, dontSendNotification);
			
			numRefs = 0;
		}
	}

	// Update Editor snapshot image
	Image refImage = createComponentSnapshot(Rectangle<int>(0, 0, getWidth(), getHeight()));
	refImage.rescaled(160, 95, Graphics::highResamplingQuality);
	VirtualRefEditor* editor = dynamic_cast<VirtualRefEditor*>(processor->getEditor());
	editor->setSnapshot(refImage);
}


void VirtualRefDisplay::update()
{
	drawTable();
}

void VirtualRefDisplay::reset()
{
	processor->getReferenceMatrix()->clear();

	for(auto button : carButtons)
		button->setToggleState(false, dontSendNotification);

	update();
}

void VirtualRefDisplay::setEnableSingleSelectionMode(bool mode)
{
	singleSelectMode = mode;
	
	if(mode)
	{
		for(auto button : carButtons)
		{
			button->setToggleState(false, sendNotification);
			button->setEnabled(false);
		}
	}
	else
	{
		for(auto button : carButtons)
			button->setEnabled(true);
	}
}

void VirtualRefDisplay::paint(Graphics& g)
{
	g.fillAll(Colours::grey);
}

void VirtualRefDisplay::resized()
{
}

void VirtualRefDisplay::mouseDown(const MouseEvent& event)
{

}

void VirtualRefDisplay::buttonEvent(Button* button)
{
}

void VirtualRefDisplay::buttonClicked(Button* b)
{
	
	selectedRow = -1;
	selectedColumn = -1;

	if (b->getButtonText().startsWith("all"))
	{
		CarButton* button = dynamic_cast<CarButton*>(b);
		int channelIndex = button->getChannelNum();

		float* chan = processor->getReferenceMatrix()->getChannel(channelIndex);

		float value;
		button->getToggleState() ? value = 1 : value = 0;
			
		for (int i=0; i<processor->getReferenceMatrix()->getNumberOfChannels(); i++)
		{
			chan[i] = value;
		}
	}
	else
	{
		ElectrodeTableButton* button = dynamic_cast<ElectrodeTableButton*>(b);

		int rowIndex = button->getRowIndex();
		int colIndex = button->getColIndex();
		bool state = button->getToggleState();

		if (singleSelectMode)
		{
			int nChannels = processor->getReferenceMatrix()->getNumberOfChannels();
			for (int i=0; i<nChannels; i++)
			{
				processor->getReferenceMatrix()->setValue(rowIndex, i, 0);
			}
			processor->getReferenceMatrix()->setValue(rowIndex, colIndex, 1.);

			selectedRow = rowIndex;
			selectedColumn = colIndex;
		}
		else
		{
			processor->getReferenceMatrix()->setValue(rowIndex, colIndex, (float)state);
		}
	}

	update();
}

void VirtualRefDisplay::applyPreset(String name, int numChannels)
{
	ReferenceMatrix* refMat = processor->getReferenceMatrix();
	int nChannels = refMat->getNumberOfChannels();

	if (name.equalsIgnoreCase("Other tetrode electrodes"))
	{
		nChannels = MIN(nChannels, numChannels);
		refMat->clear();

		int nTetrodes = nChannels / 4;
		for (int i=0; i<nTetrodes; i++)
		{
			for (int j=0; j<4; j++)
			{
				int channelIndex = i*4 + j;
				for (int k=0; k<4; k++)
				{
					if (j != k)
					{
						refMat->setValue(channelIndex, i*4 + k, 1);
					}
				}
			}
		}

		drawTable();
	}
	else if (name.equalsIgnoreCase("All tetrode electrodes"))
	{
		nChannels = MIN(nChannels, numChannels);
		refMat->clear();

		int nTetrodes = nChannels / 4;
		for (int i=0; i<nTetrodes; i++)
		{
			for (int j=0; j<4; j++)
			{
				int channelIndex = i*4 + j;
				for (int k=0; k<4; k++)
				{
					refMat->setValue(channelIndex, i*4 + k, 1);
				}
			}
		}

		drawTable();
	}
	else if (name.equalsIgnoreCase("Common average reference"))
	{
		nChannels = MIN(nChannels, numChannels);
		refMat->clear();
		for (int i=0; i<nChannels; i++)
		{
			for (int j=0; j<nChannels; j++)
			{
				refMat->setValue(i, j, 1);
			}
		}

		drawTable();
	}
	else if (name.equalsIgnoreCase("Avg of other tetrodes"))
	{
		nChannels = MIN(nChannels, numChannels);
		refMat->clear();

		/* Activate all channels and deselect channels at the same tetrode */
		int nTetrodes = nChannels / 4;
		refMat->setAll(1, nChannels);
		for (int i=0; i<nTetrodes; i++)
		{
			for (int j=0; j<4; j++)
			{
				int channelIndex = i*4 + j;
				for (int k=0; k<4; k++)
				{
					refMat->setValue(channelIndex, i*4 + k, 0);
				}
			}
		}

		drawTable();
	}
	else if (name.equalsIgnoreCase("Avg of next tetrode"))
	{
		nChannels = MIN(nChannels, numChannels);
		refMat->clear();

		int nTetrodes = nChannels / 4;

		for (int i=0; i<nTetrodes; i++)
		{
			for (int j=0; j<4; j++)
			{
				for (int k=0; k<4; k++)
				{
					if (i < nTetrodes - 1)
					{
						refMat->setValue(i*4+j, (i+1)*4+k, 1);
					}
					else
					{
						refMat->setValue(i*4+j, (i-1)*4+k, 1);
					}
				}
			}
		}

		drawTable();
	}
}

/*
	using keyboard in single channel mode not implemented yet!
*/
bool VirtualRefDisplay::keyPressed(const KeyPress &key, Component *originatingComponent)
{
//	std::cout << "VirtualRefDisplay::keyPressed key code = " << key.getKeyCode() << " | description = " << key.getTextDescription() << " | singleSelectMode = " << singleSelectMode << std::endl;
	if (singleSelectMode)
	{
		if (selectedRow > -1 && selectedColumn > -1)
		{
			int nChannels = processor->getReferenceMatrix()->getNumberOfChannels();
			if (key.getTextDescription().compare("cursor left") == 0 && selectedColumn > 0)
			{
//				std::cout << ":::Move left:::" << std::endl;
                return true;
			}
			else if (key.getTextDescription().compare("cursor right") == 0 && selectedColumn < nChannels - 1)
			{
//				std::cout << ":::Move right:::" << std::endl;
                return true;
			}
		}
	}
    return false;
}

