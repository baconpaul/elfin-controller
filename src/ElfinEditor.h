#ifndef ElfinController_EDITOR_H
#define ElfinController_EDITOR_H

#include "ElfinProcessor.h"
#include "juce_gui_basics/juce_gui_basics.h"
#include "ElfinMainPanel.h"

class ElfinControllerAudioProcessorEditor : public juce::AudioProcessorEditor, juce::AsyncUpdater
{
  public:
    ElfinControllerAudioProcessorEditor(ElfinControllerAudioProcessor &);
    ~ElfinControllerAudioProcessorEditor();

    std::unique_ptr<ElfinMainPanel> mainPanel;
    //==============================================================================
    void resized() override;
    // bool keyPressed(const juce::KeyPress &) override;

    virtual void handleAsyncUpdate() override;

    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    ElfinControllerAudioProcessor &processor;

    static constexpr int baseWidth = 600, baseHeight = 600;

    struct IdleTimer : juce::Timer
    {
        IdleTimer(ElfinControllerAudioProcessorEditor *ed) : ed(ed) {}
        ~IdleTimer() = default;
        void timerCallback() override { ed->idle(); }
        ElfinControllerAudioProcessorEditor *ed;
    };
    void idle();
    std::unique_ptr<IdleTimer> idleTimer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ElfinControllerAudioProcessorEditor)
};

#endif // SURGE_SRC_SURGE_FX_SURGEFXEDITOR_H
