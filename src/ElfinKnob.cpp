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

#include "ElfinKnob.h"
#include "UIConstants.h"
// This is super gross
#include "../libs/sst/sst-jucegui/src/sst/jucegui/components/KnobPainter.hxx"

namespace baconpaul::elfin_controller
{
namespace jcmp = sst::jucegui::components;

void ElfinKnob::paint(juce::Graphics &g)
{
    jcmp::Knob::paint(g);
    return;

    jcmp::knobPainterNoBody(g, this, continuous());

    auto b = getLocalBounds();
    auto knobarea = b.withHeight(b.getWidth());

    auto circle = [knobarea](float r) -> juce::Path
    {
        auto region = knobarea.toFloat().reduced(r);
        auto p = juce::Path();
        p.startNewSubPath(region.getCentreX(), region.getY());
        p.addArc(region.getX(), region.getY(), region.getWidth(), region.getHeight(), 0,
                 2 * juce::MathConstants<float>::pi);
        p.closeSubPath();
        return p;
    };

    auto c = juce::Colour(0xA0, 0xA0, 0x90);
    auto graded = juce::ColourGradient::vertical(c.brighter(0.2), knobarea.getY(), c.darker(0.3),
                                                 knobarea.getBottom());

    int r0 = 8;
    g.setColour(juce::Colours::black);
    g.fillPath(circle(r0));
    g.setGradientFill(graded);
    g.fillPath(circle(r0 + 1));
    auto ci = juce::Colour(0x20, 0x20, 0x20);
    auto gradedo = juce::ColourGradient::vertical(ci.darker(0.2), knobarea.getY(), ci.brighter(0.3),
                                                  knobarea.getBottom());

    g.setGradientFill(gradedo);
    g.fillPath(circle(r0 + 3));

    auto handleAngle = [knobarea](float v) -> float
    {
        float dPath = 0.2;
        float dAng = juce::MathConstants<float>::pi * (1 - dPath);
        float pt = dAng * (2 * v - 1);
        return pt;
    };

    g.saveState();
    g.addTransform(juce::AffineTransform()
                       .translated(-knobarea.getWidth() / 2, -knobarea.getHeight() / 2)
                       .rotated(handleAngle(continuous()->getValue01()))
                       .translated(knobarea.getWidth() / 2, knobarea.getHeight() / 2));

    auto hanWidth = 2.f;
    auto hanLen = 8.f;
    auto hanRect =
        juce::Rectangle<float>(knobarea.getWidth() / 2.f - hanWidth / 2.f, r0, hanWidth, hanLen);
    g.setColour(juce::Colour(0xE0, 0xE0, 0xA0));
    g.fillRect(hanRect);
    g.restoreState();
}

} // namespace baconpaul::elfin_controller