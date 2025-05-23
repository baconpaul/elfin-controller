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

    // layer one a light sort of inner gugter
    int currRad = 8;
    float alpha{1.f};
    if (!isEnabled())
        alpha = 0.5f;

    {
        auto ci = juce::Colour(0xA0, 0xA0, 0xA0).withAlpha(alpha);
        auto gradedo = juce::ColourGradient::vertical(ci.brighter(0.2), knobarea.getY(),
                                                      ci.darker(0.3), knobarea.getBottom());
        g.setGradientFill(gradedo);
        g.fillPath(circle(currRad));
    }

    // Next layer is ridgey knobs
    currRad += 2.5;
    {
        auto p = juce::Path();
        auto region = knobarea.toFloat().reduced(currRad);
        auto cx = region.getCentreX();
        auto cy = region.getCentreY();

        auto numSpikes = 13;
        auto dAng = 2.0 * juce::MathConstants<float>::pi / numSpikes;
        auto inRad = region.getWidth() / 2 - 1;
        auto outRad = region.getWidth() / 2 + 1;
        for (int i = 0; i < numSpikes; ++i)
        {
            auto x0 = cx + outRad * std::cos(dAng * i);
            auto y0 = cy + outRad * std::sin(dAng * i);

            auto x1 = cx + inRad * std::cos(dAng * (i + 0.5));
            auto y1 = cy + inRad * std::sin(dAng * (i + 0.5));
            if (i == 0)
                p.startNewSubPath(x0, y0);
            else
                p.lineTo(x0, y0);
            p.lineTo(x1, y1);
        }
        p.closeSubPath();

        juce::Graphics::ScopedSaveState ss(g);
        g.addTransform(
            juce::AffineTransform()
                .translated(-cx, -cy)
                .rotated(continuous()->getValue01() * 2 * juce::MathConstants<float>::pi * 0.8 +
                         juce::MathConstants<float>::pi * 0.7)
                .translated(cx, cy));
        auto ci = juce::Colour(0x22, 0x22, 0x22).withAlpha(alpha);
        g.setColour(ci);
        g.fillPath(p);
        g.setColour(juce::Colours::black.withAlpha(alpha));
        g.strokePath(p, juce::PathStrokeType(1));
    }

    // Then a fixed inner circle
    currRad += 3;
    {
        auto ci = juce::Colour(0x20, 0x20, 0x20).withAlpha(alpha);
        auto gradedo = juce::ColourGradient::vertical(ci.brighter(0.3), knobarea.getY(),
                                                      ci.darker(0.2), knobarea.getBottom());

        g.setGradientFill(gradedo);
        g.fillPath(circle(currRad));
        g.setColour(juce::Colour(0x20, 0x20, 0x20).withAlpha(alpha));
        g.strokePath(circle(currRad), juce::PathStrokeType(1));
    }

    // And finally a handle
    {
        auto region = knobarea.toFloat().reduced(currRad);
        auto cx = region.getCentreX();
        auto cy = region.getCentreY();

        juce::Graphics::ScopedSaveState ss(g);
        g.addTransform(
            juce::AffineTransform()
                .translated(-cx, -cy)
                .rotated(continuous()->getValue01() * 2 * juce::MathConstants<float>::pi * 0.8 -
                         juce::MathConstants<float>::pi * 0.3)
                .translated(cx, cy));
        if (isHovered)
            g.setColour(juce::Colours::white);
        else
            g.setColour(juce::Colour(0x90, 0x90, 0x90));
        g.drawLine(cx - 5, cy, region.getX(), cy);
    }
}

} // namespace baconpaul::elfin_controller