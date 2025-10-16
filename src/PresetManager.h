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

#ifndef ELFIN_CONTROLLER_PRESETMANAGER_H
#define ELFIN_CONTROLLER_PRESETMANAGER_H

#include <map>
#include <vector>
#include <utility>
#include <string>
#include <functional>
#include <cstdint>
#include <filesystem/import.h>
#include "sst/jucegui/data/Discrete.h"

#include "configuration.h"

namespace baconpaul::elfin_controller
{
/*
 * Based heaviuy on the six sines preset manager
 */
struct PresetDataBinding;
struct PresetManager
{
    fs::path userPatchesPath;

    // Call with a null host to be read-only
    PresetManager(const fs::path &p)
    {
        loadFactoryPresets();
        userPatchesPath = p;
        rescanUserPresets();
    }
    ~PresetManager() {}

    void loadFactoryPresets();
    void rescanUserPresets();
    void recurseUserPresetFrom(const fs::path &);

    std::string factoryXMLFor(int idx) const;

    static constexpr const char *factoryPath{"resources/content/patch_library"};
    std::map<std::string, std::vector<std::string>> factoryPatchNames;
    std::vector<std::pair<std::string, std::string>> factoryPatchVector;
    std::vector<fs::path> userPatches;

    std::map<fs::path, std::vector<std::pair<fs::path, int32_t>>> userPatchTree;
    std::map<std::string, std::vector<std::pair<std::string, int32_t>>> factoryPatchTree;
};

struct PresetDataBinding : sst::jucegui::data::Discrete
{
    PresetManager &pm;
    PresetDataBinding(PresetManager &p) : pm(p) {}

    std::string getLabel() const override { return "Presets"; }

    std::function<void(int, int, const fs::path &)> onLoad = [](int a, int c, const fs::path &b)
    { ELFLOG("Loading flavor " << a << " from " << b.u8string()); };

    int curr{0};
    bool hasExtra{false};
    std::string extraName{};
    void setExtra(const std::string &s)
    {
        hasExtra = true;
        extraName = s;
    }

    int getValue() const override { return curr; }
    int getDefaultValue() const override { return 0; };
    bool isDirty{false};

    std::string getValueAsStringFor(int i) const override
    {
        if (hasExtra && i < 0)
            return extraName;

        std::string postfix = isDirty ? " *" : "";

        if (i == 0)
            return "Init" + postfix;
        auto fp = i - 1;
        if (fp < pm.factoryPatchVector.size())
        {
            fs::path p{pm.factoryPatchVector[fp].second};
            // p = p / pm.factoryPatchVector[fp].second;
            p = p.replace_extension("");
            return p.u8string() + postfix;
        }
        fp -= pm.factoryPatchVector.size();
        if (fp < pm.userPatches.size())
        {
            auto pt = pm.userPatches[fp];
            pt = pt.replace_extension("");
            return pt.u8string() + postfix;
        }
        return "ERR";
    }
    void setValueFromGUI(const int &f) override
    {
        isDirty = false;
        if (hasExtra)
        {
            hasExtra = false;
        }
        curr = f;
        if (f == 0)
        {
            onLoad(0, 0, {});
            return;
        }
        auto fp = f - 1;
        if (fp < pm.factoryPatchVector.size())
        {
            onLoad(1, fp, {});
        }
        fp -= pm.factoryPatchVector.size();
        if (fp < pm.userPatches.size())
        {
            auto pt = pm.userPatchesPath / pm.userPatches[fp];
            onLoad(2, fp, pt);
        }
    };
    void setValueFromModel(const int &f) override { curr = f; }
    int getMin() const override { return hasExtra ? -1 : 0; }
    int getMax() const override
    {
        return 1 + pm.factoryPatchVector.size() + pm.userPatches.size() - 1 + (hasExtra ? 1 : 0);
    } // last -1 is because inclusive

    void setDirtyState(bool b) { isDirty = b; }

    void setStateForDisplayName(const std::string &s)
    {
        auto q = getValueAsString();
        auto sp = q.find("/");
        if (sp != std::string::npos)
        {
            q = q.substr(sp + 1);
        }

        if (s == "Init")
        {
            setValueFromModel(0);
        }
        else
        {
            bool found{false};
            int idx{1};
            for (const auto &[c, pp] : pm.factoryPatchVector)
            {
                auto p = pp.substr(0, pp.find(".sxsnp"));
                if (p == s)
                {
                    setValueFromModel(idx);
                    found = true;
                    break;
                }
                idx++;
            }
            if (!found)
            {
                for (auto &p : pm.userPatches)
                {
                    auto pn = p.filename().replace_extension("").u8string();
                    if (s == pn)
                    {
                        setValueFromModel(idx);
                        found = true;
                        break;
                    }
                    idx++;
                }
            }
            if (!found)
            {
                setExtra(s);
                setValueFromModel(-1);
            }
        }
    }
};
} // namespace baconpaul::elfin_controller
#endif // PRESETMANAGER_H
