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

#ifndef ELFIN_CONTROLLER_ELFINPROCESSOR_H
#define ELFIN_CONTROLLER_ELFINPROCESSOR_H

#include "juce_audio_processors/juce_audio_processors.h"
#include "configuration.h"
#include <vector>
#include <map>

namespace baconpaul::elfin_controller
{
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

    std::atomic<bool> refreshUI{false}, rebuildUI{false};
    std::atomic<bool> sendAllNotesOff{false};

    //==============================================================================
    struct ElfinParam : juce::AudioParameterFloat
    {
        ElfinControl control;
        ElfinDescription desc;
        ElfinParam(ElfinControl c, juce::String sname, juce::String name, float def)
            : control(c),
              juce::AudioParameterFloat({sname, 1}, name,
                                        juce::NormalisableRange<float>(0.0, 1.0, 0.001), def)
        {
            desc = elfinConfig.at(control);
        }

        static float getFloatForCC(int cc) { return std::clamp(cc / 127.0, 0., 1.); }
        int getCCForFloat(float f)
        {
            return std::clamp((int)std::round(convertTo0to1(f) * 127), 0, 127);
        }
        int getCC() { return getCCForFloat(get()); }
        std::atomic<bool> invalid{false};

      protected:
        void valueChanged(float newValue) override { invalid = true; }
    };
    typedef ElfinParam float_param_t;
    std::array<float_param_t *, nElfinParams> params{};
    std::map<int, float_param_t *> paramsByCC;
    int sampleGap{0};
    static constexpr int maxMessagesPerSample{3};
    std::atomic<int> midiGapMultiplier{2};

    juce::AudioParameterBool *bypassParam{nullptr};
    juce::AudioProcessorParameter *getBypassParameter() const override { return bypassParam; }

    std::string toXML() const;
    bool fromXML(const std::string &s);
    bool fromSYX(const std::vector<uint8_t> &s);
    void randomizePatch();
    void applyPostPatchChangeConstraints();

    std::unique_ptr<juce::PropertiesFile> properties;

  public:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ElfinControllerAudioProcessor)
};
} // namespace baconpaul::elfin_controller

#endif // SURGE_SRC_SURGE_FX_SURGEFXPROCESSOR_H
