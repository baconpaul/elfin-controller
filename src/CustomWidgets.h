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

#ifndef ELFIN_CONTROLLER_CUSTOMWIDGETS_H
#define ELFIN_CONTROLLER_CUSTOMWIDGETS_H

#include <juce_gui_basics/juce_gui_basics.h>
#include <memory>
#include <sst/jucegui/components/JogUpDownButton.h>

#include <cmrc/cmrc.hpp>

CMRC_DECLARE(elfin_content);

namespace baconpaul::elfin_controller
{

struct LogoBase : juce::Component
{
    std::unique_ptr<juce::Drawable> logoSVG;

    LogoBase(const std::string &ln)
    {
        try
        {
            auto fs = cmrc::elfin_content::get_filesystem();
            auto f = fs.open("resources/content/logos/" + ln);
            std::string s(f.begin(), f.size());
            auto xml = juce::XmlDocument::parse(s);
            logoSVG = juce::Drawable::createFromSVG(*xml);
        }
        catch (const std::exception &e)
        {
            ELFLOG(e.what());
        }
        setInterceptsMouseClicks(false, false);
    }
};
struct ElfinLogo : LogoBase
{
    ElfinLogo() : LogoBase("The Elfin Controller.svg") { assert(logoSVG); }

    void paint(juce::Graphics &g) override
    {
        if (!logoSVG)
            return;
        // g.fillAll(juce::Colours::darkgrey);
        auto bd = logoSVG->getBounds();
        auto t = juce::AffineTransform();
        auto sc = 0.5;
        t = t.translated(-bd.getX(), -bd.getY());
        t = t.scaled(sc, sc);
        // t = t.translated((getWidth() - bd.getWidth()) / 2, 0);
        logoSVG->draw(g, 1.0, t);
    }
};

struct HideawayLogo : LogoBase
{
    HideawayLogo() : LogoBase("Hideaway Studio.svg") { assert(logoSVG); }

    void paint(juce::Graphics &g) override
    {
        if (!logoSVG)
            return;
        // g.fillAll(juce::Colours::darkblue);
        auto scale = 0.51;
        auto bd = logoSVG->getBounds();
        auto t = juce::AffineTransform();
        t = t.translated(-bd.getX() + getWidth() - 45, -bd.getY() + 8);
        t = t.scaled(scale, scale);
        logoSVG->draw(g, 1.0, t);
    }
};

struct PresetButton : sst::jucegui::components::JogUpDownButton
{
    juce::Rectangle<int> hamburgerButton() const
    {
        auto bx = getLocalBounds().reduced(1);
        return bx.withWidth(bx.getHeight());
    }
    juce::Rectangle<int> diceButton() const
    {
        auto bx = getLocalBounds().reduced(1);
        return bx.withWidth(bx.getHeight()).translated(bx.getHeight(), 0);
    }

    std::function<void()> onDice = []() { ELFLOG("dice"); };
    std::function<void()> onDiceRMB = []() { ELFLOG("dice RMB"); };

    bool isOverControl(const juce::Point<int> &e) const override
    {
        auto pic = sst::jucegui::components::JogUpDownButton::isOverControl(e);
        // we *want* the hamburger to pop the menu right!
        return pic || /* hamburgerButton().contains(e) || */ diceButton().contains(e);
    }

    void mouseDown(const juce::MouseEvent &e) override
    {
        if (e.mods.isPopupMenu() && diceButton().contains(e.position.toInt()) && onDiceRMB)
        {
            onDiceRMB();
        }
        else
        {
            sst::jucegui::components::JogUpDownButton::mouseDown(e);
        }
    }

    void mouseUp(const juce::MouseEvent &event) override
    {
        if (!event.mods.isPopupMenu() && diceButton().contains(event.position.toInt()) && onDice)
        {
            onDice();
        }
        else
        {
            if (!diceButton().contains(event.position.toInt()))
                sst::jucegui::components::JogUpDownButton::mouseUp(event);
        }
    }

    void paint(juce::Graphics &g) override
    {
        auto tx = getColour(Styles::labelcolor);
        auto har = tx;
        if (isHovered)
        {
            har = getColour(Styles::jogbutton_hover);
        }

        sst::jucegui::components::JogUpDownButton::paint(g);
        auto hc = tx;
        if (hamburgerButton().contains(hoverX, getHeight() / 2))
        {
            hc = har;
        }
        sst::jucegui::components::GlyphPainter::paintGlyph(
            g, hamburgerButton(), sst::jucegui::components::GlyphPainter::GlyphType::HAMBURGER, hc);

        hc = tx;
        if (diceButton().contains(hoverX, getHeight() / 2))
        {
            hc = har;
        }
        sst::jucegui::components::GlyphPainter::paintGlyph(
            g, diceButton(), sst::jucegui::components::GlyphPainter::GlyphType::DICE, hc);
    }
};

} // namespace baconpaul::elfin_controller
#endif // CUSTOMWIDGETS_H
