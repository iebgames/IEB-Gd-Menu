#include "ai_module.hpp"
#include <modules/config/config.hpp>
#include <Geode/loader/Log.hpp>
#include <Geode/utils/async.hpp>
#include <fmt/format.h>
#include <thread>
#include "custom_scripts.hpp"

namespace eclipse::gui::scripting {

    static AIModule* s_instance = nullptr;

    AIModule* AIModule::get() {
        if (!s_instance) s_instance = new AIModule();
        return s_instance;
    }

    AIModule::AIModule() {
        m_groqModels = {
            "qwen/qwen3-32b",
            "deepseek-r1-distill-llama-70b",
            "gemma2-9b-it",
            "groq/compound",
            "groq/compound-mini",
            "llama-3.1-8b-instant",
            "llama-3.3-70b-versatile",
            "meta-llama/llama-4-maverick-17b-128e-instruct",
            "meta-llama/llama-4-scout-17b-16e-instruct",
            "meta-llama/llama-guard-4-12b",
            "meta-llama/llama-prompt-guard-2-22m",
            "meta-llama/llama-prompt-guard-2-86m",
            "moonshotai/kimi-k2-instruct",
            "moonshotai/kimi-k2-instruct-0905",
            "openai/gpt-oss-120b"
        };

        m_openRouterModels = {
            "openrouter/polaris-alpha",
            "nvidia/nemotron-nano-12b-v2-vl:free",
            "minimax/minimax-m2:free",
            "alibaba/tongyi-deepresearch-30b-a3b:free",
            "meituan/longcat-flash-chat:free",
            "nvidia/nemotron-nano-9b-v2:free",
            "deepseek/deepseek-chat-v3.1:free",
            "openai/gpt-oss-20b:free",
            "z-ai/glm-4.5-air:free",
            "qwen/qwen3-coder:free",
            "moonshotai/kimi-k2:free",
            "cognitivecomputations/dolphin-mistral-24b-venice-edition:free",
            "google/gemma-3n-e2b-it:free",
            "tngtech/deepseek-r1t2-chimera:free",
            "mistralai/mistral-small-3.2-24b-instruct:free",
            "deepseek/deepseek-r1-0528-qwen3-8b:free",
            "deepseek/deepseek-r1-0528:free",
            "google/gemma-3n-e4b-it:free",
            "meta-llama/llama-3.3-8b-instruct:free",
            "qwen/qwen3-4b:free",
            "qwen/qwen3-30b-a3b:free",
            "qwen/qwen3-14b:free",
            "qwen/qwen3-235b-a22b:free",
            "tngtech/deepseek-r1t-chimera:free",
            "microsoft/mai-ds-r1:free",
            "arliai/qwq-32b-arliai-rpr-v1:free",
            "agentica-org/deepcoder-14b-preview:free",
            "meta-llama/llama-4-maverick:free",
            "meta-llama/llama-4-scout:free",
            "qwen/qwen2.5-vl-32b-instruct:free",
            "deepseek/deepseek-chat-v3-0324:free",
            "mistralai/mistral-small-3.1-24b-instruct:free",
            "google/gemma-3-4b-it:free",
            "google/gemma-3-12b-it:free",
            "google/gemma-3-27b-it:free",
            "mistralai/mistral-small-24b-instruct-2501:free",
            "deepseek/deepseek-r1-distill-llama-70b:free",
            "deepseek/deepseek-r1:free",
            "google/gemini-2.0-flash-exp:free",
            "meta-llama/llama-3.3-70b-instruct:free",
            "qwen/qwen-2.5-coder-32b-instruct:free",
            "meta-llama/llama-3.2-3b-instruct:free",
            "qwen/qwen-2.5-72b-instruct:free",
            "nousresearch/hermes-3-llama-3.1-405b:free",
            "mistralai/mistral-nemo:free",
            "mistralai/mistral-7b-instruct:free"
        };
    }

    std::string AIModule::getGroqKey() const {
        return config::get<std::string>("ai.groq-key", "");
    }

    void AIModule::setGroqKey(const std::string& key) {
        config::set("ai.groq-key", key);
    }

    std::string AIModule::getOpenRouterKey() const {
        return config::get<std::string>("ai.openrouter-key", "");
    }

    void AIModule::setOpenRouterKey(const std::string& key) {
        config::set("ai.openrouter-key", key);
    }

    std::string AIModule::maskKey(const std::string& key) {
        if (key.length() <= 8) return "****";
        return key.substr(0, 5) + "**********" + key.substr(key.length() - 4);
    }

    void AIModule::sendMessage(const std::string& prompt, const std::string& model, bool isOpenRouter) {
        if (m_isSending) return;

        m_isSending = true;
        m_lastError = "";

        // Add system prompt if empty
        if (m_history.empty()) {
            m_history.push_back({
                "system",
                "You are IEB AI, an advanced scripting assistant for 'IEB Menu' (a Geometry Dash mod menu by IEB Studio).\n"
                "Your goal is to write JavaScript scripts using the IEB Custom Script API to add new features for the user.\n"
                "RESPONSE FORMAT (STRICT): You MUST output your response in this EXACT format. Do not include markdown code blocks (```).\n"
                "showtouser: <a brief explanation for the user in English, unless they spoke to you in another language>\n"
                "code: <the raw JavaScript code only>\n"
                "name: <a short descriptive name for the script>\n",
                model, ""
            });
        }

        // Add user message to history
        m_history.push_back({"user", prompt, model, ""});

        std::string url = isOpenRouter ? "https://openrouter.ai/api/v1/chat/completions" : "https://api.groq.com/openai/v1/chat/completions";
        std::string apiKey = isOpenRouter ? getOpenRouterKey() : getGroqKey();

        auto payload = matjson::makeObject({
            {"model", model}
        });
        
        std::vector<matjson::Value> messages;
        for (const auto& msg : m_history) {
            messages.push_back(matjson::makeObject({
                {"role", msg.role},
                {"content", msg.content}
            }));
        }
        payload["messages"] = messages;

        geode::utils::web::WebRequest req;
        req.header("Authorization", fmt::format("Bearer {}", apiKey))
           .header("Content-Type", "application/json")
           .header("HTTP-Referer", "https://ieb-ai.local") // OpenRouter requirement
           .header("X-Title", "IEB AI") // OpenRouter requirement
           .bodyJSON(payload);
           
        std::thread([this, req, url, model]() mutable {
            auto res = req.postSync(url);
            geode::queueInMainThread([this, res = std::move(res), model]() {
                handleResponse(res, model);
            });
        }).detach();
    }

    void AIModule::handleResponse(geode::utils::web::WebResponse const& res, const std::string& model) {
        m_isSending = false;
        if (!res.ok()) {
            handleError(fmt::format("API Error ({}): {}", res.code(), res.string().unwrapOr("Unknown error")));
            return;
        }

        auto jsonRes = res.json();
        if (!jsonRes) {
            handleError("Failed to parse JSON response");
            return;
        }

        auto val = jsonRes.unwrap();
        
        // OpenAI format: choices[0].message.content
        try {
            std::string content = val["choices"][0]["message"]["content"].asString().unwrapOr("");
            std::string raw = val.dump();
            std::string showToUser = content;

            // Attempt to parse structured response
            size_t showIdx = content.find("showtouser:");
            size_t codeIdx = content.find("code:");
            size_t nameIdx = content.find("name:");

            if (showIdx != std::string::npos && codeIdx != std::string::npos && nameIdx != std::string::npos) {
                showIdx += 11;
                showToUser = content.substr(showIdx, codeIdx - showIdx);
                
                codeIdx += 5;
                std::string codeStr = content.substr(codeIdx, nameIdx - codeIdx);
                
                nameIdx += 5;
                std::string nameStr = content.substr(nameIdx);

                // Simple trim implementation
                auto trim = [](std::string& s) {
                    s.erase(0, s.find_first_not_of(" \n\r\t"));
                    s.erase(s.find_last_not_of(" \n\r\t") + 1);
                };
                trim(showToUser);
                trim(codeStr);
                trim(nameStr);

                // Strip markdown backticks if AI inadvertently added them
                if (codeStr.starts_with("```")) {
                    size_t nl = codeStr.find('\n');
                    if (nl != std::string::npos) codeStr = codeStr.substr(nl + 1);
                }
                if (codeStr.ends_with("```")) {
                    codeStr = codeStr.substr(0, codeStr.length() - 3);
                }
                trim(codeStr);

                if (!nameStr.empty() && !codeStr.empty()) {
                    CustomScriptManager::get()->addScript(nameStr, codeStr);
                    geode::log::info("IEB AI auto-saved script: {}", nameStr);
                }
            }

            m_history.push_back({"assistant", showToUser, model, raw});
        } catch (const std::exception& e) {
            handleError(fmt::format("Failed to extract content: {}", e.what()));
        }
    }

    void AIModule::handleError(const std::string& error) {
        m_lastError = error;
        m_history.push_back({"assistant", fmt::format("Error: {}", error), "", "", true});
        geode::log::error("AI Error: {}", error);
    }

    void AIModule::clearHistory() {
        m_history.clear();
    }

}
