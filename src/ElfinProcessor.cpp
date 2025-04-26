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
    static bool pc{false};
    if (!pc)
    {
        pc = true;
        midiMessages.addEvent(juce::MidiMessage::programChange(1, 5), 0);
    }

    for (auto &p : params)
    {
        // FIXME atomic compare
        if (p && p->invalid == true)
        {
            p->invalid = false;
            midiMessages.addEvent(juce::MidiMessage::controllerEvent(1, p->desc.midiCC, p->getCC()),
                                  0);
        }
    }

#if 1
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
void ElfinControllerAudioProcessor::getStateInformation(juce::MemoryBlock &destData) {}

void ElfinControllerAudioProcessor::setStateInformation(const void *data, int sizeInBytes) {}
} // namespace baconpaul::elfin_controller

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter()
{
    return new baconpaul::elfin_controller::ElfinControllerAudioProcessor();
}
