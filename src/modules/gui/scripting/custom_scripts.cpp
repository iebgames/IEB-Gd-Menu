#include "custom_scripts.hpp"
#include <fstream>
#include <Geode/loader/Mod.hpp>
#include <Geode/loader/Log.hpp>
#include <Geode/utils/file.hpp>

namespace eclipse::gui::scripting {

    static CustomScriptManager* s_instance = nullptr;

    CustomScriptManager* CustomScriptManager::get() {
        if (!s_instance) s_instance = new CustomScriptManager();
        return s_instance;
    }

    CustomScriptManager::CustomScriptManager() {
        load();
    }

    std::filesystem::path CustomScriptManager::getFilePath() {
        return geode::Mod::get()->getConfigDir() / "custom_scripts.json";
    }

    void CustomScriptManager::load() {
        auto path = getFilePath();
        auto res = geode::utils::file::readJson(path);
        if (!res) return;

        auto& json = res.unwrap();
        auto arr = json.asArray();
        if (!arr) return;

        m_scripts.clear();
        for (auto const& item : arr.unwrap()) {
            m_scripts.push_back({
                item["name"].as<std::string>().unwrapOr("Untitled"),
                item["code"].as<std::string>().unwrapOr(""),
                item["enabled"].as<bool>().unwrapOr(false)
            });
        }
    }

    void CustomScriptManager::save() {
        std::vector<matjson::Value> arr;
        for (const auto& script : m_scripts) {
            arr.push_back(matjson::makeObject({
                {"name", script.name},
                {"code", script.code},
                {"enabled", script.enabled}
            }));
        }
        matjson::Value root(arr);

        auto data = root.dump();
        auto res = geode::utils::file::writeStringSafe(getFilePath(), data);
        if (res.isErr()) {
            geode::log::error("Failed to save custom scripts: {}", res.unwrapErr());
        }
    }

    void CustomScriptManager::addScript(const std::string& name, const std::string& code) {
        m_scripts.push_back({name, code, false});
        save();
    }

    void CustomScriptManager::updateScript(size_t index, const std::string& name, const std::string& code) {
        if (index >= m_scripts.size()) return;
        bool wasEnabled = m_scripts[index].enabled;
        m_scripts[index] = {name, code, wasEnabled};
        save();
    }

    void CustomScriptManager::deleteScript(size_t index) {
        if (index >= m_scripts.size()) return;
        m_scripts.erase(m_scripts.begin() + index);
        save();
    }

}
