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

#ifndef ELFIN_CONTROLLER_ELFINPROCESSOR_H
#define ELFIN_CONTROLLER_ELFINPROCESSOR_H

#include "juce_audio_processors/juce_audio_processors.h"

template <typename T, int Capacity = 4096> class LockFreeQueue
{
  public:
    LockFreeQueue() : fifo(Capacity) {}

    void push(const T &item)
    {
        int start1, size1, start2, size2;
        fifo.prepareToWrite(1, start1, size1, start2, size2);

        if (size1 > 0)
        {
            buffer[start1] = item;
        }

        fifo.finishedWrite(size1);
    }

    bool pop(T &item)
    {
        int start1, size1, start2, size2;
        fifo.prepareToRead(1, start1, size1, start2, size2);

        if (size1 > 0)
        {
            item = buffer[start1];
        }

        fifo.finishedRead(size1);

        return size1 > 0;
    }

  private:
    juce::AbstractFifo fifo;
    std::array<T, Capacity> buffer;
};

//==============================================================================
/**
 */
class ElfinControllerAudioProcessor : public juce::AudioProcessor,
                                      public juce::AudioProcessorParameter::Listener,
                                      public juce::AsyncUpdater
{
  public:
    static constexpr int nAWParams{10};

    //==============================================================================
    ElfinControllerAudioProcessor();
    ~ElfinControllerAudioProcessor();

    //==============================================================================
    std::atomic<bool> isPlaying{false};
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    void processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &) override;

    //==============================================================================
    juce::AudioProcessorEditor *createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    void getStateInformation(juce::MemoryBlock &destData) override;
    void setStateInformation(const void *data, int sizeInBytes) override;

    void parameterValueChanged(int parameterIndex, float newValue) override;
    void parameterGestureChanged(int parameterIndex, bool isStarting) override {}
    void handleAsyncUpdate() override {}

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int index) override {}
    const juce::String getProgramName(int index) override { return "Init"; }
    void changeProgramName(int index, const juce::String &newName) override {}

    struct ResetTypeMsg
    {
        int type; // -1 for type, otherwise param
        int32_t toIndex;
        float value;
    };
    LockFreeQueue<ResetTypeMsg> resetType;

    int64_t sampleCount{0};

    std::atomic<bool> refreshUI{false}, rebuildUI{false};

    //==============================================================================
    typedef juce::AudioParameterFloat float_param_t;

    juce::AudioParameterBool *bypassParam{nullptr};
    juce::AudioProcessorParameter *getBypassParameter() const override { return bypassParam; }

    std::unique_ptr<juce::PropertiesFile> properties;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ElfinControllerAudioProcessor)
};

#endif // SURGE_SRC_SURGE_FX_SURGEFXPROCESSOR_H
