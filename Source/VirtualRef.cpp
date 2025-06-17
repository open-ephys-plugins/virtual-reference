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

#include "VirtualRef.h"
#include "VirtualRefEditor.h"
#include <stdio.h>

#define MIN(a, b) (((a) < (b)) ? (a) : (b))

VirtualRef::VirtualRef()
    : GenericProcessor ("Virtual Ref"),
      channelBuffer (1, BUFFER_SIZE),
      avgBuffer (1, BUFFER_SIZE),
      globalGain (1.0f)
{
}

VirtualRef::~VirtualRef()
{
}

AudioProcessorEditor* VirtualRef::createEditor()
{
    editor = std::make_unique<VirtualRefEditor> (this);
    return editor.get();
}

void VirtualRef::updateSettings()
{
    for (auto stream : getDataStreams())
    {
        int numChannels = (stream->getChannelCount() > 128) ? 128 : stream->getChannelCount();

        refMatMap.emplace (stream->getStreamId(), std::make_unique<ReferenceMatrix> (numChannels));

        if (editor != nullptr)
        {
            editor->updateVisualizer();
        }
    }
}

void VirtualRef::process (AudioBuffer<float>& buffer)
{
    // loop through the streams
    for (auto stream : getDataStreams())
    {
        if ((*stream)["enable_stream"])
        {
            float* ref;
            float refGain;
            int numRefs;
            int numChan = refMatMap[stream->getStreamId()]->getNumberOfChannels();

            channelBuffer = buffer;

            for (int i = 0; i < numChan; i++)
            {
                avgBuffer.clear();

                ref = refMatMap[stream->getStreamId()]->getChannel (i);
                numRefs = 0;
                for (int j = 0; j < numChan; j++)
                {
                    if (ref[j] > 0)
                    {
                        numRefs++;
                    }
                }

                for (int j = 0; j < numChan; j++)
                {
                    if (ref[j] > 0)
                    {
                        refGain = 1.0f / float (numRefs);
                        int globalChanIndex = stream->getContinuousChannels()[j]->getGlobalIndex();

                        avgBuffer.addFrom (0,
                                           0,
                                           channelBuffer,
                                           globalChanIndex,
                                           0,
                                           channelBuffer.getNumSamples(),
                                           refGain);
                    }
                }

                if (numRefs > 0)
                {
                    int globalChanIndex = stream->getContinuousChannels()[i]->getGlobalIndex();

                    buffer.addFrom (globalChanIndex, // destChannel
                                    0, // destStartSample
                                    avgBuffer, // source
                                    0, // sourceChannel
                                    0, // sourceStartSample
                                    buffer.getNumSamples(), // numSamples
                                    -1.0f * globalGain); // global gain to apply
                }
            }
        }
    }
}

ReferenceMatrix* VirtualRef::getReferenceMatrix()
{
    return refMatMap[getEditor()->getCurrentStream()].get();
}

void VirtualRef::setGlobalGain (float value)
{
    globalGain = value;
}

float VirtualRef::getGlobalGain()
{
    return globalGain;
}

void VirtualRef::saveCustomParametersToXml (XmlElement* xml)
{
    xml->setAttribute ("Type", "VirtualRef");
    xml->setAttribute ("GlobalGain", getGlobalGain());

    for (auto stream : getDataStreams())
    {
        uint16 streamID = stream->getStreamId();
        XmlElement* streamXml = xml->createNewChildElement ("STREAM");
        streamXml->setAttribute ("ID", streamID);

        int numChannels = refMatMap[streamID]->getNumberOfChannels();

        for (int i = 0; i < numChannels; i++)
        {
            float* ref = refMatMap[streamID]->getChannel (i);

            XmlElement* channelXml = streamXml->createNewChildElement ("CHANNEL");
            channelXml->setAttribute ("Index", i + 1);
            for (int j = 0; j < numChannels; j++)
            {
                if (ref[j] > 0)
                {
                    XmlElement* refXml = channelXml->createNewChildElement ("REFERENCE");
                    refXml->setAttribute ("Index", j + 1);
                    refXml->setAttribute ("Value", ref[j]);
                }
            }
        }
    }
}

void VirtualRef::loadCustomParametersFromXml (XmlElement* customParamsXml)
{
    float globGain = (float) customParamsXml->getDoubleAttribute ("GlobalGain", 1.0f);
    setGlobalGain (globGain);

    for (auto streamXml : customParamsXml->getChildWithTagNameIterator ("STREAM"))
    {
        uint16 streamID = streamXml->getIntAttribute ("ID");
        LOGD ("Stream ID: ", streamID);

        if (streamID == 0 || refMatMap.find (streamID) == refMatMap.end())
            continue;

        refMatMap[streamID]->clear();

        for (auto channelXml : streamXml->getChildWithTagNameIterator ("CHANNEL"))
        {
            int channelIndex = channelXml->getIntAttribute ("Index");

            for (auto refXml : channelXml->getChildWithTagNameIterator ("REFERENCE"))
            {
                int refIndex = refXml->getIntAttribute ("Index");
                float gain = (float) refXml->getDoubleAttribute ("Value");
                refMatMap[streamID]->setValue (channelIndex - 1, refIndex - 1, gain);
            }
        }
    }

    getEditor()->updateVisualizer();
}

/* -----------------------------------------------------------------
ReferenceMatrix
----------------------------------------------------------------- */

ReferenceMatrix::ReferenceMatrix (int nChan)
{
    nChannels = nChan;
    nChannelsBefore = -1;
    values = nullptr;
    update();
}

ReferenceMatrix::~ReferenceMatrix()
{
    if (values != nullptr)
        delete[] values;
}

void ReferenceMatrix::setNumberOfChannels (int n)
{
    nChannels = n;
    update();
}

int ReferenceMatrix::getNumberOfChannels()
{
    return nChannels;
}

void ReferenceMatrix::update()
{
    if (nChannels != nChannelsBefore)
    {
        if (values != nullptr)
            delete[] values;

        values = new float[nChannels * nChannels];
        for (int i = 0; i < nChannels * nChannels; i++)
            values[i] = 0;

        nChannelsBefore = nChannels;
    }
}

void ReferenceMatrix::setValue (int rowIndex, int colIndex, float value)
{
    if (rowIndex >= 0 && rowIndex < nChannels && colIndex >= 0 && colIndex < nChannels)
    {
        values[rowIndex * nChannels + colIndex] = value;
    }
    else
    {
        std::cout << "RefMatrix::setValue INDEX OUT OF BOUNDS! (rowIndex=" << rowIndex << ", colIndex=" << colIndex << ")" << std::endl;
    }
}

float ReferenceMatrix::getValue (int rowIndex, int colIndex)
{
    float value = -1;
    if (rowIndex >= 0 && rowIndex < nChannels && colIndex >= 0 && colIndex < nChannels)
    {
        value = values[rowIndex * nChannels + colIndex];
    }

    return value;
}

float* ReferenceMatrix::getChannel (int index)
{
    if (index >= 0 && index < nChannels)
        return &values[index * nChannels];
    else
        return nullptr;
}

bool ReferenceMatrix::allChannelReferencesActive (int index)
{
    float* chan = getChannel (index);

    int nActive = 0;

    if (chan != nullptr)
    {
        for (int i = 0; i < nChannels; i++)
        {
            if (chan[i] > 0)
            {
                nActive++;
            }
        }
    }

    return nActive == nChannels;
}

void ReferenceMatrix::setAll (float value)
{
    if (values != nullptr)
    {
        for (int i = 0; i < nChannels; i++)
        {
            for (int j = 0; j < nChannels; j++)
            {
                values[i * nChannels + j] = value;
            }
        }
    }
}

void ReferenceMatrix::setAll (float value, int maxChan)
{
    if (values != nullptr)
    {
        maxChan = MIN (nChannels, maxChan);
        for (int i = 0; i < maxChan; i++)
        {
            for (int j = 0; j < maxChan; j++)
            {
                values[i * nChannels + j] = value;
            }
        }
    }
}

void ReferenceMatrix::clear()
{
    if (values != nullptr)
    {
        for (int i = 0; i < nChannels; i++)
        {
            for (int j = 0; j < nChannels; j++)
            {
                values[i * nChannels + j] = 0;
            }
        }
    }
}

void ReferenceMatrix::print()
{
    for (int i = 0; i < nChannels; i++)
    {
        float* chan = getChannel (i);
        for (int j = 0; j < nChannels; j++)
        {
            std::cout << chan[j] << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}
