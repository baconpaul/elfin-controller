#include <algorithm>
#include <cmath>

#include <juce_gui_basics/juce_gui_basics.h>
#include "sst/jucegui/style/StyleSheet.h"
#include "ElfinEditor.h"

//==============================================================================
ElfinControllerAudioProcessorEditor::ElfinControllerAudioProcessorEditor(
    ElfinControllerAudioProcessor &p)
    : AudioProcessorEditor(&p), processor(p)
{
    mainPanel = std::make_unique<ElfinMainPanel>(processor);
    addAndMakeVisible(*mainPanel);

    idleTimer = std::make_unique<IdleTimer>(this);
    idleTimer->startTimer(1000 / 60);

    setSize(700, 400);
}

ElfinControllerAudioProcessorEditor::~ElfinControllerAudioProcessorEditor() {}

void ElfinControllerAudioProcessorEditor::idle() {}

void ElfinControllerAudioProcessorEditor::handleAsyncUpdate() {}

void ElfinControllerAudioProcessorEditor::resized() { mainPanel->setBounds(getLocalBounds()); }
