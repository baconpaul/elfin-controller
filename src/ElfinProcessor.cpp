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
