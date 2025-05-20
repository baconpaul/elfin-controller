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

#include "PresetManager.h"
#include "sst/plugininfra/strnatcmp.h"
#include <cmrc/cmrc.hpp>

CMRC_DECLARE(elfin_content);

namespace baconpaul::elfin_controller
{
void PresetManager::loadFactoryPresets()
{
    ELFLOG("Loading factory presets");
    try
    {
        auto fs = cmrc::elfin_content::get_filesystem();
        cmrc::directory_iterator dir = fs.iterate_directory(factoryPath);
        for (const auto &q : dir)
        {
            std::vector<std::string> ents;

            if (q.is_directory())
            {
                for (const auto &patch :
                     fs.iterate_directory(std::string() + factoryPath + "/" + q.filename()))
                {
                    ents.push_back(patch.filename());
                }

                std::sort(ents.begin(), ents.end(),
                          [](const auto &a, const auto &b)
                          { return strnatcasecmp(a.c_str(), b.c_str()) < 0; });
                factoryPatchNames[q.filename()] = ents;
            }
        }

        factoryPatchVector.clear();
        for (const auto &[c, st] : factoryPatchNames)
        {
            for (const auto &pn : st)
            {
                factoryPatchVector.emplace_back(c, pn);
            }
        }

        int32_t pidx = (int32_t)(1);
        for (auto &[path, pname] : factoryPatchVector)
        {
            factoryPatchTree[path].push_back({pname, pidx});
            pidx++;
        }
    }
    catch (const std::exception &e)
    {
        ELFLOG(e.what());
    }
}

std::string PresetManager::factoryXMLFor(int idx) const
{
    try
    {
        auto fs = cmrc::elfin_content::get_filesystem();
        auto [path, name] = factoryPatchVector[idx];
        auto f = fs.open(std::string() + factoryPath + "/" + path + "/" + name);
        return std::string(f.begin(), f.end());
    }
    catch (const std::exception &e)
    {
        ELFLOG(e.what());
    }
    return "";
}

void PresetManager::recurseUserPresetFrom(const fs::path &p)
{
    if (fs::is_directory(p))
    {
        for (auto &el : fs::directory_iterator(p))
        {
            auto elp = el.path();
            if (elp.filename() == "." || elp.filename() == "..")
            {
                continue;
            }
            if (fs::is_directory(elp))
            {
                recurseUserPresetFrom(elp);
            }
            else if (fs::is_regular_file(elp) &&
                     (elp.extension() == ".elfin" || elp.extension() == ".syx"))
            {
                auto pushP = elp.lexically_relative(userPatchesPath);
                userPatches.push_back(pushP);
            }
        }
    }
}

void PresetManager::rescanUserPresets()
{
    userPatches.clear();
    try
    {
        recurseUserPresetFrom(userPatchesPath);
        std::sort(userPatches.begin(), userPatches.end(),
                  [](const fs::path &a, const fs::path &b)
                  {
                      auto appe = a.parent_path().empty();
                      auto bppe = b.parent_path().empty();

                      if (appe && bppe)
                      {
                          return strnatcasecmp(a.filename().u8string().c_str(),
                                               b.filename().u8string().c_str()) < 0;
                      }
                      else if (appe)
                      {
                          return true;
                      }
                      else if (bppe)
                      {
                          return false;
                      }
                      else
                      {
                          return a < b;
                      }
                  });
    }
    catch (fs::filesystem_error &)
    {
    }

    int32_t pidx = (int32_t)(1 + factoryPatchVector.size());
    userPatchTree.clear();
    for (auto &p : userPatches)
    {
        userPatchTree[p.parent_path()].push_back({p, pidx});
        pidx++;
    }

#if 0
    for (const auto &[k, v] : userPatchTree)
    {
        ELFLOG("K/V = '" << k.u8string() << "'");
        for (const auto &p : v)
        {
            ELFLOG("    - " << p.first.u8string() << " / " << p.second);
        }
    }
#endif
}

} // namespace baconpaul::elfin_controller