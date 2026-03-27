#pragma once

#include <Geode/utils/web.hpp>
#include <string>
#include <vector>
#include <functional>
#include <matjson.hpp>

namespace eclipse::gui::scripting {

    struct Message {
        std::string role; // "user" or "assistant"
        std::string content;
        std::string model;
        std::string rawResponse;
        bool isError = false;
    };

    class AIModule {
    public:
        static AIModule* get();

        void sendMessage(const std::string& prompt, const std::string& model, bool isOpenRouter);
        void clearHistory();

        const std::vector<Message>& getHistory() const { return m_history; }
        
        std::string getGroqKey() const;
        void setGroqKey(const std::string& key);
        
        std::string getOpenRouterKey() const;
        void setOpenRouterKey(const std::string& key);

        const std::vector<std::string>& getGroqModels() const { return m_groqModels; }
        const std::vector<std::string>& getOpenRouterModels() const { return m_openRouterModels; }

        bool isSending() const { return m_isSending; }
        std::string getLastError() const { return m_lastError; }

        // Masking utility
        static std::string maskKey(const std::string& key);

    private:
        AIModule();
        
        std::vector<Message> m_history;
        bool m_isSending = false;
        std::string m_lastError;

        std::vector<std::string> m_groqModels;
        std::vector<std::string> m_openRouterModels;

        void handleResponse(geode::utils::web::WebResponse const& res, const std::string& model);
        void handleError(const std::string& error);
    };

}
