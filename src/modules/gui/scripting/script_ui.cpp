#include "script_ui.hpp"
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include "ai_module.hpp"
#include "custom_scripts.hpp"
#include <modules/gui/theming/manager.hpp>
#include <modules/i18n/translations.hpp>

namespace eclipse::gui::scripting {

    static bool s_sidebarOpen = false;
    static std::string s_promptBuffer;
    static int s_selectedModelGroq = 5; // Default llama
    static int s_selectedModelOR = 0;
    static bool s_useOpenRouter = false;
    static bool s_showMoreDetails = false;

    // Editor state
    static bool s_editorOpen = false;
    static int s_editingIndex = -1;
    static std::string s_editorName;
    static std::string s_editorCode;

    void toggleAISidebar() { s_sidebarOpen = !s_sidebarOpen; }
    bool isAISidebarOpen() { return s_sidebarOpen; }

    void drawAISidebar() {
        if (!s_sidebarOpen) return;

        auto& io = ImGui::GetIO();
        float scale = io.DisplaySize.x / 1920.f;
        float width = 450.f * scale;

        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
        ImGui::SetNextWindowSize(io.DisplaySize, ImGuiCond_Always);

        if (ImGui::Begin("IEB AI Script Maker", &s_sidebarOpen, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse)) {
            
            // Model Selection
            ImGui::Text("AI Model Setup");
            ImGui::Checkbox("Use OpenRouter", &s_useOpenRouter);

            auto& ai = *AIModule::get();
            if (s_useOpenRouter) {
                if (ImGui::BeginCombo("Model", ai.getOpenRouterModels()[s_selectedModelOR].c_str())) {
                    for (int i = 0; i < ai.getOpenRouterModels().size(); i++) {
                        if (ImGui::Selectable(ai.getOpenRouterModels()[i].c_str(), s_selectedModelOR == i)) {
                            s_selectedModelOR = i;
                        }
                    }
                    ImGui::EndCombo();
                }
            } else {
                if (ImGui::BeginCombo("Model", ai.getGroqModels()[s_selectedModelGroq].c_str())) {
                    for (int i = 0; i < ai.getGroqModels().size(); i++) {
                        if (ImGui::Selectable(ai.getGroqModels()[i].c_str(), s_selectedModelGroq == i)) {
                            s_selectedModelGroq = i;
                        }
                    }
                    ImGui::EndCombo();
                }
            }

            ImGui::Separator();

            // API Keys (Masked)
            static bool s_showKeys = false;
            ImGui::Checkbox("Show Keys", &s_showKeys);
            
            if (s_useOpenRouter) {
                std::string key = ai.getOpenRouterKey();
                if (s_showKeys) {
                    if (ImGui::InputText("OR Key", &key, ImGuiInputTextFlags_Password)) ai.setOpenRouterKey(key);
                } else {
                    std::string masked = AIModule::maskKey(key);
                    ImGui::Text("OR Key: %s", masked.c_str());
                }
            } else {
                std::string key = ai.getGroqKey();
                if (s_showKeys) {
                    if (ImGui::InputText("Groq Key", &key, ImGuiInputTextFlags_Password)) ai.setGroqKey(key);
                } else {
                    std::string masked = AIModule::maskKey(key);
                    ImGui::Text("Groq Key: %s", masked.c_str());
                }
            }

            ImGui::Separator();

            // Chat History
            ImGui::BeginChild("ChatHistory", ImVec2(0, -ImGui::GetFrameHeightWithSpacing() * 4), true);
            for (const auto& msg : ai.getHistory()) {
                if (msg.role == "system") continue; // Hide system prompt from user

                if (msg.role == "user") {
                    ImGui::TextColored(ImVec4(0.4f, 0.7f, 1.0f, 1.0f), "[You]:");
                    ImGui::TextWrapped("%s", msg.content.c_str());
                } else {
                    ImGui::TextColored(ImVec4(0.1f, 1.0f, 0.4f, 1.0f), "[AI (%s)]:", msg.model.c_str());
                    if (msg.isError) {
                        ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), "%s", msg.content.c_str());
                    } else {
                        ImGui::TextWrapped("%s", msg.content.c_str());
                        
                        if (s_showMoreDetails) {
                            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.7f, 0.7f, 1.0f));
                            ImGui::SeparatorText("Debug / Raw Details");
                            std::string label = "Raw Response##" + std::to_string((size_t)&msg);
                            ImGui::InputTextMultiline(label.c_str(), (char*)msg.rawResponse.c_str(), msg.rawResponse.size(), ImVec2(-1, 100), ImGuiInputTextFlags_ReadOnly);
                            ImGui::PopStyleColor();
                        }
                    }
                }
                ImGui::Separator();
            }
            if (ai.isSending()) {
                ImGui::TextDisabled("AI is thinking...");
            }
            if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) ImGui::SetScrollHereY(1.0f);
            ImGui::EndChild();

            // Details Toggle
            ImGui::Checkbox("Show More Details", &s_showMoreDetails);
            
            // Input Area
            ImGui::PushItemWidth(-1);
            if (ImGui::InputTextWithHint("##prompt", "Explain what your script should do...", &s_promptBuffer, ImGuiInputTextFlags_EnterReturnsTrue)) {
                if (!s_promptBuffer.empty() && !ai.isSending()) {
                    std::string model = s_useOpenRouter ? ai.getOpenRouterModels()[s_selectedModelOR] : ai.getGroqModels()[s_selectedModelGroq];
                    ai.sendMessage(s_promptBuffer, model, s_useOpenRouter);
                    s_promptBuffer.clear();
                }
            }
            ImGui::PopItemWidth();

            if (ImGui::Button("Send", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, 0))) {
                if (!s_promptBuffer.empty() && !ai.isSending()) {
                    std::string model = s_useOpenRouter ? ai.getOpenRouterModels()[s_selectedModelOR] : ai.getGroqModels()[s_selectedModelGroq];
                    ai.sendMessage(s_promptBuffer, model, s_useOpenRouter);
                    s_promptBuffer.clear();
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Clear Chat", ImVec2(-FLT_MIN, 0))) {
                ai.clearHistory();
            }
        }
        ImGui::End();
    }

    static bool s_studioOpen = false;

    void toggleScriptStudio() { s_studioOpen = !s_studioOpen; }
    bool isScriptStudioOpen() { return s_studioOpen; }

    void drawScriptStudio() {
        if (!s_studioOpen) return;

        auto& io = ImGui::GetIO();
        auto& manager = *CustomScriptManager::get();

        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
        ImGui::SetNextWindowSize(io.DisplaySize, ImGuiCond_Always);

        if (ImGui::Begin("IEB Script Studio", &s_studioOpen, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse)) {
            auto& scripts = manager.getScripts();

            ImGui::Text("Custom Scripts Portfolio");
            if (ImGui::Button("Create New Script", ImVec2(200.f, 0))) {
                s_editorOpen = true;
                s_editingIndex = -1;
                s_editorName = "New Script";
                s_editorCode = "";
            }

            ImGui::SeparatorText(eclipse::i18n::get("custom.script-studio.title").view().data());
            
            ImGui::BeginChild("ScriptsList", ImVec2(0, io.DisplaySize.y - 150), true);
            for (size_t i = 0; i < scripts.size(); i++) {
                ImGui::PushID(i);
                
                // Styling to match native hacks
                float itemWidth = ImGui::GetContentRegionAvail().x;
                ImGui::BeginGroup();
                
                if (ImGui::Checkbox("##enable", &scripts[i].enabled)) {
                    manager.save();
                }
                ImGui::SameLine();
                
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2.0f);
                ImGui::Text("%s", scripts[i].name.c_str());
                
                ImGui::SameLine(itemWidth - 120);
                if (ImGui::Button("Edit", ImVec2(55, 0))) {
                    s_editorOpen = true;
                    s_editingIndex = i;
                    s_editorName = scripts[i].name;
                    s_editorCode = scripts[i].code;
                }
                ImGui::SameLine();
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 0.6f));
                if (ImGui::Button("X", ImVec2(30, 0))) {
                    manager.deleteScript(i);
                }
                ImGui::PopStyleColor();
                
                ImGui::EndGroup();
                ImGui::Separator();
                
                ImGui::PopID();
            }
            ImGui::EndChild();
        }
        ImGui::End();

        // Editor Modal
        if (s_editorOpen) {
            ImGui::OpenPopup("Script Editor");
        }

        ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x * 0.8f, io.DisplaySize.y * 0.8f), ImGuiCond_Appearing);
        if (ImGui::BeginPopupModal("Script Editor", &s_editorOpen)) {
            ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Language: JavaScript (Geode/Cocos API)");
            
            ImGui::PushItemWidth(-1);
            ImGui::InputTextWithHint("##name", "Script Name...", &s_editorName);
            ImGui::InputTextMultiline("##code", &s_editorCode, ImVec2(-1, ImGui::GetContentRegionAvail().y - 40));
            ImGui::PopItemWidth();

            if (ImGui::Button("Save", ImVec2(120, 0))) {
                if (s_editingIndex == -1) {
                    manager.addScript(s_editorName, s_editorCode);
                } else {
                    manager.updateScript(s_editingIndex, s_editorName, s_editorCode);
                }
                s_editorOpen = false;
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                s_editorOpen = false;
            }
            ImGui::EndPopup();
        }
    }

}
