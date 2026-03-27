#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include <matjson.hpp>

namespace eclipse::gui::scripting {

    struct CustomScript {
        std::string name;
        std::string code;
        bool enabled = false;
    };

    class CustomScriptManager {
    public:
        static CustomScriptManager* get();

        void load();
        void save();

        void addScript(const std::string& name, const std::string& code);
        void updateScript(size_t index, const std::string& name, const std::string& code);
        void deleteScript(size_t index);

        std::vector<CustomScript>& getScripts() { return m_scripts; }

    private:
        CustomScriptManager();
        std::vector<CustomScript> m_scripts;
        std::filesystem::path getFilePath();
    };

}
