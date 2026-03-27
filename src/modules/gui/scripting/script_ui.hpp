#pragma once

#include <string>

namespace eclipse::gui::scripting {

    void drawAISidebar();
    void toggleScriptStudio();
    bool isScriptStudioOpen();
    void drawScriptStudio();

    // State management for UI
    void toggleAISidebar();
    bool isAISidebarOpen();

}
