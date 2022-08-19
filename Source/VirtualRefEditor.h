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

#ifndef __VIRTUALREFEDITOR_H__
#define __VIRTUALREFEDITOR_H__

#include <EditorHeaders.h>
#include <VisualizerWindowHeaders.h>
#include <VisualizerEditorHeaders.h>


#include "VirtualRefCanvas.h"

/** 
  * Draws the preview image and adds a border to it
*/
class PreviewImageComponent : public Component
{
public:
    /** Constructor */
    PreviewImageComponent(const String& name);

    /** Destructor */
    ~PreviewImageComponent();

    void paint(Graphics& g) override;

    void resized() override;

    /** Sets the ImageComponent's image */
    void setImage(juce::Image& img);

private:
    std::unique_ptr<ImageComponent> canvasImageComponent;
};

/**

  User interface for the virtual reference plugin.

  @see VirtualRef

*/
class VirtualRefEditor : public VisualizerEditor,
                         public DragAndDropContainer
{
public:

    /** Constructor*/
    VirtualRefEditor(GenericProcessor* parentNode);

    /** Destructor */
    virtual ~VirtualRefEditor();

	/** Creates the Virtual Reference matrix settings interface*/
    Visualizer* createNewCanvas();

    /** Save reference matrix in a custom location*/
    void saveParametersDialog();

    /** Load reference matrix from a custom location*/
    void loadParametersDialog();

    /** Upadte visualizer when selected stream changes*/
    void selectedStreamHasChanged() override;

    /** Sets the canvas preview image for the editor*/ 
    void setSnapshot(juce::Image& canvasImage);

private:

	VirtualRefCanvas* chanRefCanvas;

    std::unique_ptr<PreviewImageComponent> canvasSnapshot;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VirtualRefEditor);

};

#endif  // __VIRTUALREFEDITOR_H__
