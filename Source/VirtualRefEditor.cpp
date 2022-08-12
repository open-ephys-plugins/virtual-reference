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


#include "VirtualRefEditor.h"
#include "VirtualRef.h"


VirtualRefEditor::VirtualRefEditor(GenericProcessor* parentNode)
    : VisualizerEditor(parentNode, "Virtual Ref", 190), chanRefCanvas(nullptr)
{	
    canvasSnapshot =  std::make_unique<ImageComponent>("Canvas Snapshot");
    canvasSnapshot->setBounds(20, 30, 160, 95);
    addAndMakeVisible(canvasSnapshot.get());
}

VirtualRefEditor::~VirtualRefEditor()
{
}

Visualizer* VirtualRefEditor::createNewCanvas()
{

    VirtualRef* processor = (VirtualRef*) getProcessor();
	chanRefCanvas = new VirtualRefCanvas(processor);
    return chanRefCanvas;
}

void VirtualRefEditor::saveParametersDialog()
{
    if (!acquisitionIsActive)
    {
        FileChooser fc("Choose the file name...",
                       File::getCurrentWorkingDirectory(),
                       "*",
                       true);

        if (fc.browseForFileToSave(true))
        {
            File fileToSave = fc.getResult();

			XmlElement* xml = new XmlElement("SETTINGS");
			VirtualRef* p = dynamic_cast<VirtualRef*>(getProcessor());
			p->saveCustomParametersToXml(xml);
			if(!xml->writeToFile(fileToSave, String()))
			{
				CoreServices::sendStatusMessage("Couldn't save channel reference data to file.");
			}
			else
			{
				CoreServices::sendStatusMessage("Saved channel reference data to file " + fileToSave.getFullPathName());
			}
			delete xml;
        }
    } else
	{
		CoreServices::sendStatusMessage("Stop acquisition before saving channel references.");
    }
}

void VirtualRefEditor::loadParametersDialog()
{
    if (!acquisitionIsActive)
    {
        FileChooser fc("Choose the file name...",
                       File::getCurrentWorkingDirectory(),
                       "*",
                       true);

        if (fc.browseForFileToOpen())
        {
            File fileToOpen = fc.getResult();

			VirtualRef* p = dynamic_cast<VirtualRef*>(getProcessor());
			auto fileXml = XmlDocument::parse(fileToOpen);
			p->loadCustomParametersFromXml(fileXml.get());
			CoreServices::sendStatusMessage("Loaded channel reference data from file." + fileToOpen.getFullPathName());
        }
    } else
	{
		CoreServices::sendStatusMessage("Stop acquisition before loading channel references.");
    }
}

void VirtualRefEditor::selectedStreamHasChanged()
{
    updateVisualizer();
}

void VirtualRefEditor::setSnapshot(juce::Image& canvasImage)
{
    canvasSnapshot->setImage(canvasImage);
}

