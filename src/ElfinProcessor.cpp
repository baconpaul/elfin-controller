/*
 * Elfin Controller
 *
 * A small controller plugin for the Elfin 04 Polysynth
 *
 * Copyright 2024-2025, Paul Walker and Various authors, as described in the github
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

//==============================================================================
ElfinControllerAudioProcessor::ElfinControllerAudioProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
    for (int i=0; i<128; ++i)
        lastCCValue[i] = -1;
    params[0] = new juce::AudioParameterFloat("elf_cutoff", "Cutoff", 0.0f, 1.0f, 0.5f);
    addParameter(params[0]);
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

    if (params[0]->get() != lastCCValue[0])
    {
        ELFLOG("Cutoff changed to " << params[0]->get());
        lastCCValue[0] = params[0]->get();
        midiMessages.addEvent(juce::MidiMessage::controllerEvent(1, 16, 127 * params[0]->get()), 0);
    }
    if (sampleCount <= 0 && sampleCount + buffer.getNumSamples() > 0)
    {
        ELFLOG("Note On " << sampleCount);
        auto r = rand() % 127;
        midiMessages.addEvent(juce::MidiMessage::controllerEvent(1, 21, r), 0);
        midiMessages.addEvent(juce::MidiMessage::noteOn(1, 60, 0.8f), 1);
    }
    else if (sampleCount <= 24000 && sampleCount + buffer.getNumSamples() > 24000)
    {
        ELFLOG("Note Off " << sampleCount);
        midiMessages.addEvent(juce::MidiMessage::noteOff(1, 60, 0.8f), 0);
    }
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

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter()
{
    return new ElfinControllerAudioProcessor();
}
