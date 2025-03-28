#include <algorithm>
#include <cmath>

#include <juce_gui_basics/juce_gui_basics.h>
#include "ElfinEditor.h"

//==============================================================================
ElfinControllerAudioProcessorEditor::ElfinControllerAudioProcessorEditor(
    ElfinControllerAudioProcessor &p)
    : AudioProcessorEditor(&p), processor(p)
{
    idleTimer = std::make_unique<IdleTimer>(this);
    idleTimer->startTimer(1000 / 60);

    setSize(700, 400);
}

ElfinControllerAudioProcessorEditor::~ElfinControllerAudioProcessorEditor() {}

void ElfinControllerAudioProcessorEditor::idle() {}

void ElfinControllerAudioProcessorEditor::handleAsyncUpdate() {}

void ElfinControllerAudioProcessorEditor::resized() {}

void ElfinControllerAudioProcessorEditor::paint(juce::Graphics &g)
{
    auto b = getLocalBounds();
    auto gr = juce::ColourGradient(juce::Colours::blue, {0.f, 0.f}, juce::Colours::green,
                                   {0.f, 1.f * getHeight()}, false);
    g.setGradientFill(gr);
    g.fillAll();
}
