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

#ifndef __VIRTUALREF_H__
#define __VIRTUALREF_H__

#include <ProcessorHeaders.h>

#define BUFFER_SIZE 1024

class ReferenceMatrix;

/**

  VirtualRef

  Allows the user to select arbitrary (virtual) reference channels for each 
  recording channel.

  @see GenericProcessor

*/

class VirtualRef : public GenericProcessor

{
public:
    /** Constructor */
    VirtualRef();

    /** Destructor */
    ~VirtualRef();

    /** Applys average reference gain from all the selected channels for each input channel*/
    void process (AudioBuffer<float>& buffer);

    /** Create custom editor*/
    AudioProcessorEditor* createEditor();

    /** Called whenever the signal chain is altered. */
    void updateSettings();

    /** Gets the reference matrix for current stream */
    ReferenceMatrix* getReferenceMatrix();

    /** Sets the global gain value */
    void setGlobalGain (float value);

    /** Gets the global gain value */
    float getGlobalGain();

    /** Saves all custom parameters */
    void saveCustomParametersToXml (XmlElement* parentElement);

    /** Loads all custom parameters */
    void loadCustomParametersFromXml (XmlElement* customParamsXml);

private:
    std::map<String, std::unique_ptr<ReferenceMatrix>> refMatMap;
    AudioBuffer<float> channelBuffer;
    AudioBuffer<float> avgBuffer;
    float globalGain;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VirtualRef);
};

/**

  Reference matrix

  Each row indicates the selected reference channels for each channel.

  1: selected
  0: not selected

  TODO allow values between 0 and 1 to set the gain of each reference channel

  @see VirtualRef

*/

class ReferenceMatrix
{
public:
    /** Constructor */
    ReferenceMatrix (int nChan);

    /** Destructor */
    ~ReferenceMatrix();

    /** Set the number of channels for the matrix*/
    void setNumberOfChannels (int n);

    /** Get the number of channels for the matrix*/
    int getNumberOfChannels();

    /** Called when the input channels have changed*/
    void update();

    /** Sets the value for the specified row and column */
    void setValue (int rowIndex, int colIndex, float value);

    /** Gets the value for the specified row and column */
    float getValue (int rowIndex, int colIndex);

    /** Gets the channel value for the specified index */
    float* getChannel (int index);

    /** Checks if all the reference channels are active for the given input channel index */
    bool allChannelReferencesActive (int index);

    /** Sets the value for all reference channels */
    void setAll (float value);

    /** Sets the value for all reference channels up to the max channel number*/
    void setAll (float value, int maxChan);

    /** Clears the reference channel matrix */
    void clear();

    /** Prints the matrix values*/
    void print();

private:
    int nChannels;
    int nChannelsBefore;
    float* values;
};

#endif //__VIRTUALREF_H__
