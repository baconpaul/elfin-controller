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
    }
};
struct ElfinLogo : LogoBase
{
    ElfinLogo() : LogoBase("The Elfin Logo.svg") { assert(logoSVG); }

    void paint(juce::Graphics &g) override
    {
        if (!logoSVG)
            return;

        auto bd = logoSVG->getBounds();
        auto t = juce::AffineTransform();
        auto sc = 0.55;
        t = t.translated(0, -bd.getY());
        t = t.scaled(sc, sc);
        t = t.translated(sc *(getWidth() - bd.getWidth()) / 2 + 5, 0);
        logoSVG->draw(g, 1.0, t);
    }
};

struct HideawayLogo : LogoBase
{
    HideawayLogo() : LogoBase("Hideaway logo.svg") { assert(logoSVG); }

    void paint(juce::Graphics &g) override
    {
        if (!logoSVG)
            return;
        auto scale = 0.45;
        auto bd = logoSVG->getBounds();
        auto t = juce::AffineTransform();
        t = t.translated(0, -bd.getY());
        t = t.scaled(scale, scale);
        t = t.translated((getWidth() - scale * bd.getWidth()) / 2, 0);
        logoSVG->draw(g, 1.0, t);
    }
};

} // namespace baconpaul::elfin_controller
#endif // CUSTOMWIDGETS_H
