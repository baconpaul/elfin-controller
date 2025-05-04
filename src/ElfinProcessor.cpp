/*
 * Elfin Controller
 *
 * A small controller plugin for the Elfin 04 Polysynth
 *
 * Copyright 2025, Paul Walker and Various authors, as described in the github
 * transaction log.
 *
 * This source repo is released under the MIT license
 *
 * The source code and license are at https://github.com/baconpaul/elfin-controller
 */
#include <type_traits>

#include "ElfinProcessor.h"
#include "ElfinEditor.h"

#if LINUX
// getCurrentPosition is deprecated in J7
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
namespace baconpaul::elfin_controller
{
//==============================================================================
ElfinControllerAudioProcessor::ElfinControllerAudioProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
    setupConfiguration();

    std::fill(params.begin(), params.end(), nullptr);
    for (const auto &[id, cc] : elfinConfig)
    {
        params[id] = new float_param_t(id, cc.streaming_name, cc.name, 0.5f);
        params[id]->addListener(this);

        addParameter(params[id]);
    }
}

ElfinControllerAudioProcessor::~ElfinControllerAudioProcessor() {}

//==============================================================================
const juce::String ElfinControllerAudioProcessor::getName() const { return JucePlugin_Name; }

bool ElfinControllerAudioProcessor::acceptsMidi() const { return true; }

bool ElfinControllerAudioProcessor::producesMidi() const { return true; }

bool ElfinControllerAudioProcessor::isMidiEffect() const { return true; }

double ElfinControllerAudioProcessor::getTailLengthSeconds() const { return 0.0; }

//==============================================================================
void ElfinControllerAudioProcessor::prepareToPlay(double sr, int samplesPerBlock)
{
    isPlaying = true;
}

void ElfinControllerAudioProcessor::releaseResources() { isPlaying = false; }

void ElfinControllerAudioProcessor::processBlock(juce::AudioBuffer<float> &buffer,
                                                 juce::MidiBuffer &midiMessages)
{
    for (auto &p : params)
    {
        // FIXME atomic compare
        if (p && p->invalid == true)
        {
            p->invalid = false;
            midiMessages.addEvent(juce::MidiMessage::controllerEvent(1, p->desc.midiCC, p->getCC()),
                                  0);
            // ELFLOG(p->desc.name << " to " << p->getCC());
        }
    }

#if 0
    if (sampleCount <= 0 && sampleCount + buffer.getNumSamples() > 0)
    {
        ELFLOG("Note On " << sampleCount);
        midiMessages.addEvent(juce::MidiMessage::noteOn(1, 60, 0.8f), 1);
    }
    else if (sampleCount <= 24000 && sampleCount + buffer.getNumSamples() > 24000)
    {
        ELFLOG("Note Off " << sampleCount);
        midiMessages.addEvent(juce::MidiMessage::noteOff(1, 60, 0.8f), 0);
    }

#endif
    sampleCount += buffer.getNumSamples();
    if (sampleCount > 96000)
    {
        sampleCount -= 96000 + buffer.getNumSamples();
    }
}

//==============================================================================
bool ElfinControllerAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor *ElfinControllerAudioProcessor::createEditor()
{
    rebuildUI = true;
    return new ElfinControllerAudioProcessorEditor(*this);
}

void ElfinControllerAudioProcessor::parameterValueChanged(int parameterIndex, float newValue)
{
    refreshUI = true;
}

//==============================================================================
void ElfinControllerAudioProcessor::getStateInformation(juce::MemoryBlock &destData)
{
    auto s = toXML();
    destData.append(s.c_str(), s.length() + 1);
}

void ElfinControllerAudioProcessor::setStateInformation(const void *data, int sizeInBytes)
{
    auto q = std::string((const char *)data, (size_t)sizeInBytes);
    if (!q.empty())
    {
        fromXML(q);
    }
}

std::string ElfinControllerAudioProcessor::toXML() const
{
    auto doc = juce::XmlElement("elfin");
    doc.setAttribute("version", 1);
    for (auto &p : params)
    {
        auto parX = new juce::XmlElement("param");
        parX->setAttribute("id", p->desc.streaming_name);
        parX->setAttribute("cc", p->desc.midiCC);
        parX->setAttribute("val", p->get());
        parX->setAttribute("ccval", p->getCC());

        doc.addChildElement(parX);
    }

    return doc.toString().toStdString();
}
bool ElfinControllerAudioProcessor::fromXML(const std::string &s)
{
    auto doc = juce::XmlDocument(s);
    if (auto mainElement = doc.getDocumentElement())
    {
        ELFLOG("Got mainElement " << mainElement->getTagName());
        if (mainElement->getTagName() != "elfin")
        {
            ELFLOG("Not elfin");
            return false;
        }
        if (mainElement->getIntAttribute("version", -1) != 1)
        {
            ELFLOG("Not version1");
            return false;
        }

        auto *child = mainElement->getFirstChildElement();

        std::map<std::string, double> valueMap;
        while (child)
        {
            if (child->getTagName() == "param")
            {
                auto sn = child->getStringAttribute("id");
                auto sv = child->getDoubleAttribute("val");
                valueMap[sn.toStdString()] = sv;
            }
            child = child->getNextElement();
        }
        ELFLOG("FIXME : Setvalue notifying host here is odd. Set then notify all instead");
        for (auto p : params)
        {
            auto pos = valueMap.find(p->desc.streaming_name);
            if (pos != valueMap.end())
                p->setValueNotifyingHost(pos->second);
        }
    }
    else
    {
        ELFLOG(doc.getLastParseError());
    }
    return true;
}

} // namespace baconpaul::elfin_controller

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter()
{
    return new baconpaul::elfin_controller::ElfinControllerAudioProcessor();
}
