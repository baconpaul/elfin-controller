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
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
    setupConfiguration();

    std::fill(params.begin(), params.end(), nullptr);
    for (const auto &[id, cc] : elfinConfig)
    {
        auto def = float_param_t::getFloatForCC(cc.midiCCDefault);
        params[id] = new float_param_t(id, cc.streaming_name, cc.name, def);
        params[id]->addListener(this);
        paramsByCC[cc.midiCC] = params[id];

        // In the standalone, force a send on startup
        if (wrapperType == juce::AudioProcessor::WrapperType::wrapperType_Standalone)
        {
            params[id]->invalid = true;
        }

        addParameter(params[id]);
    }

    if (wrapperType == juce::AudioProcessor::WrapperType::wrapperType_Standalone)
    {
        sendAllNotesOff = true;
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
    sampleGap = std::ceil(sr / 48000 * 4);
}

void ElfinControllerAudioProcessor::releaseResources() { isPlaying = false; }

void ElfinControllerAudioProcessor::processBlock(juce::AudioBuffer<float> &buffer,
                                                 juce::MidiBuffer &midiMessages)
{
    int midiTimeForParams{0};
    int numSamples = buffer.getNumSamples();

    bool anoOn{true}, anoDone{false};
    if (sendAllNotesOff.compare_exchange_strong(anoOn, anoDone))
    {
        midiTimeForParams = std::min(sampleGap, numSamples - 1);
        midiMessages.addEvent(juce::MidiMessage::controllerEvent(1, 123, 0), 0);
    }
    int ct{0};
    for (auto &p : params)
    {
        bool inOn{true}, onDone{false};
        if (p && p->invalid.compare_exchange_strong(inOn, onDone))
        {
            midiMessages.addEvent(juce::MidiMessage::controllerEvent(1, p->desc.midiCC, p->getCC()),
                                  midiTimeForParams);
            if (ct == maxMessagesPerSample)
            {
                ct = 0;
                midiTimeForParams += midiGapMultiplier * sampleGap;
            }
            ct++;
            if (midiTimeForParams >= numSamples)
            {
                break;
            }
        }
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
        parX->setAttribute("ccval", p->getCC());

        doc.addChildElement(parX);
    }

    return doc.toString().toStdString();
}
bool ElfinControllerAudioProcessor::fromXML(const std::string &s)
{
    sendAllNotesOff = true;
    auto doc = juce::XmlDocument(s);
    if (auto mainElement = doc.getDocumentElement())
    {
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

        std::map<std::string, int> valueMap;
        while (child)
        {
            if (child->getTagName() == "param")
            {
                auto sn = child->getStringAttribute("id");
                auto sc = child->getIntAttribute("ccval");
                valueMap[sn.toStdString()] = sc;
            }
            child = child->getNextElement();
        }
        for (auto p : params)
        {
            auto pos = valueMap.find(p->desc.streaming_name);
            if (pos != valueMap.end())
                p->setValueNotifyingHost(p->getFloatForCC(pos->second));
        }
    }
    else
    {
        ELFLOG(doc.getLastParseError());
    }
    return true;
}

bool ElfinControllerAudioProcessor::fromSYX(const std::vector<uint8_t> &d)
{
    if (d.size() != 108)
    {
        ELFLOG("Mis-sized sysex data");
        return false;
    }
    sendAllNotesOff = true;
    for (auto i = 0; i < d.size(); i += 3)
    {
        if (d[i] != 0xb0)
        {
            ELFLOG("Non-control-byte at " << i);
            return false;
        }
        int cc = d[i + 1];
        int va = d[i + 2];
        auto pit = paramsByCC.find(cc);
        if (pit != paramsByCC.end())
        {
            pit->second->setValueNotifyingHost(pit->second->getFloatForCC(va));
        }
        else
        {
            ELFLOG("Unable to map param " << cc << " val=" << va);
        }
    }
    return true;
}

void ElfinControllerAudioProcessor::randomizePatch()
{
    for (auto p : params)
    {
        p->setValueNotifyingHost(p->getFloatForCC(rand() % 128));
    }
}

} // namespace baconpaul::elfin_controller

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter()
{
    return new baconpaul::elfin_controller::ElfinControllerAudioProcessor();
}
