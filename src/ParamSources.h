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

#ifndef ELFIN_CONTROLLER_PARAMSOURCES_H
#define ELFIN_CONTROLLER_PARAMSOURCES_H

#include "ElfinProcessor.h"

#include <string>
#include <sst/jucegui/data/Continuous.h>
#include <sst/jucegui/data/Discrete.h>

namespace baconpaul::elfin_controller
{

struct ParamSource : sst::jucegui::data::Continuous
{
    ElfinControllerAudioProcessor::float_param_t *par{nullptr};
    ElfinMainPanel &panel;

    ParamSource(ElfinControllerAudioProcessor::float_param_t *v, ElfinMainPanel &m) : panel(m)
    {
        par = v;
    }

    std::string getLabel() const override { return par->desc.label; }
    float getValue() const override { return par->get(); }
    bool isBipolar() const override { return par->desc.isBipolar; }
    void setValueFromGUI(const float &f) override
    {
        if (par->getCC() != par->getCCForFloat(f))
        {
            par->setValueNotifyingHost(f);
            panel.updateToolTip(par);
        }
    }
    void setValueFromModel(const float &f) override {}
    float getDefaultValue() const override { return par->getFloatForCC(par->desc.midiCCDefault); }
    float getMin() const override { return 0; }
    float getMax() const override { return 1; }
};

struct DiscreteParamSource : sst::jucegui::data::Discrete
{
    ElfinControllerAudioProcessor::float_param_t *par{nullptr};
    ElfinMainPanel &panel;
    DiscreteParamSource(ElfinControllerAudioProcessor::float_param_t *v, ElfinMainPanel &p)
        : panel(p)
    {
        par = v;
    }

    std::string getLabel() const override { return par->desc.label; }
    int cacheCC{-1}, cacheLookup{-1};
    int getValue() const override
    {
        auto v = par->getCC();
        int idx{0};
        for (auto &d : par->desc.discreteRanges)
        {
            if (v >= d.from && v <= d.to)
                return idx;
            idx++;
        }
        return 0;
    }
    void setValueFromGUI(const int &i) override
    {
        // poly/uni needs an all notes off to avoid hw device
        // wedging voiuce mgr
        if (par->desc.streaming_name == "poly_uni")
        {
            panel.processor.sendAllNotesOff = true;
        }
        auto rng = par->desc.discreteRanges[i];
        auto mid = (rng.from + rng.to) / 2;
        auto f = par->getFloatForCC(mid);
        par->setValueNotifyingHost(f);
    }
    void setValueFromModel(const int &f) override {}
    int getDefaultValue() const override { return 2; }
    int getMin() const override { return 0; }
    int getMax() const override { return par->desc.discreteRanges.size() - 1; }
    std::string getValueAsStringFor(int i) const override
    {
        return par->desc.discreteRanges[i].label;
    }
};

} // namespace baconpaul::elfin_controller
#endif // PARAMSOURCES_H
