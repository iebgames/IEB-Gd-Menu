#pragma once
#include "base-component.hpp"
#include <functional>

namespace eclipse::gui {
    class CustomComponent : public Component {
    public:
        explicit CustomComponent(std::string id, std::function<void()> drawCallback) : m_id(std::move(id)), m_callback(std::move(drawCallback)) {
            m_type = ComponentType::Custom;
        }

        void onUpdate() override {}
        
        [[nodiscard]] const std::string& getId() const override { return m_id; }
        [[nodiscard]] const std::string& getTitle() const override { return m_id; }

        void draw() { if (m_callback) m_callback(); }

    private:
        std::string m_id;
        std::function<void()> m_callback;
    };
}
