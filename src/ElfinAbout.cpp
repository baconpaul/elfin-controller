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

#include "ElfinAbout.h"
#include <sst/plugininfra/version_information.h>
#include <fstream>

namespace baconpaul::elfin_controller
{
void ElfinAbout::paint(juce::Graphics &g)
{
    g.fillAll(juce::Colours::black.withAlpha(0.4f));

    auto bxBnd = getLocalBounds().reduced(70).withTrimmedBottom(80);
    g.setColour(juce::Colours::black);
    g.fillRect(bxBnd);
    g.setColour(juce::Colours::white);
    g.drawRect(bxBnd);

    g.setColour(juce::Colour(0xFF, 0x90, 0x00));
    g.setFont(juce::FontOptions(30));
    auto txRec = bxBnd.reduced(8, 4);
    g.drawText("Elfin Controller", txRec, juce::Justification::centredTop);
    txRec = txRec.withTrimmedTop(33);

    g.setFont(juce::FontOptions(18));
    g.setColour(juce::Colour(0xE0, 0xE0, 0xE0));
    g.drawText(std::string() + "Version : " +
                   sst::plugininfra::VersionInformation::git_implied_display_version + " / " +
                   sst::plugininfra::VersionInformation::git_commit_hash,
               txRec, juce::Justification::centredTop);
    txRec = txRec.withTrimmedTop(50);

    g.setFont(juce::FontOptions(12));
    std::vector<std::string> msg = {
        "Copyright 2025 Paul Walker (baconpaul) and other authors as described in the Git "
        "Transaction Log",
        "Elfin Controller source code is released under the MIT license.",
        "Elfin Controller binaries are released under the Gnu General Public license 3 or later.",
        "Find the source and licenses at https://github.com/baconpaul/elfin-controller",
        "Built with JUCE. https://juce.com",
        "Built with a collection of libraries from the Surge Synth Team. "
        "https://surge-synth-team.org",
        "Built with the Free Audio clap-juce-extensions. https://github.com/free-audio/",
        std::string("This version built ") + sst::plugininfra::VersionInformation::build_date +
            " " + sst::plugininfra::VersionInformation::build_time + " with " +
            sst::plugininfra::VersionInformation::cmake_compiler};
    for (const auto &m : msg)
    {
        g.drawText(m, txRec, juce::Justification::topLeft);
        txRec = txRec.withTrimmedTop(14);
    }

    txRec = txRec.withTrimmedTop(50);
    g.setFont(juce::FontOptions(18));
    g.setColour(juce::Colour(0xE0, 0xE0, 0xE0));
    g.drawText("Click anywhere to close", txRec, juce::Justification::centredTop);
}
} // namespace baconpaul::elfin_controller