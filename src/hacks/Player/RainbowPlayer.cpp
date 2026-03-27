#include <modules/config/config.hpp>
#include <modules/gui/color.hpp>
#include <modules/gui/gui.hpp>
#include <modules/gui/components/toggle.hpp>
#include <modules/hack/hack.hpp>
#include <utils.hpp>
#include <modules/i18n/translations.hpp>

#include <Geode/modify/GJBaseGameLayer.hpp>

using namespace geode::prelude;

namespace eclipse::hacks::Player {

    class $hack(RainbowPlayer) {
        void init() override {
            auto tab = gui::MenuTab::find("tab.player");

            config::setIfEmpty("player.rainbow.c1", true);
            config::setIfEmpty("player.rainbow.c2", false);
            config::setIfEmpty("player.rainbow.glow", false);
            config::setIfEmpty("player.rainbow.speed", 5.0f);

            tab->addToggle("player.rainbow")
                ->handleKeybinds()
                ->setDescription()
                ->addOptions([](auto options) {
                    options->addToggle("player.rainbow.c1");
                    options->addToggle("player.rainbow.c2");
                    options->addToggle("player.rainbow.glow");
                    options->addInputFloat("player.rainbow.speed", 0.1f, 100.0f, "%.1f");
                });
        }

        [[nodiscard]] const char* getId() const override { return "Rainbow Player"; }
    };

    REGISTER_HACK(RainbowPlayer)

    class $modify(RainbowPlayerLayer, GJBaseGameLayer) {
        void update(float dt) override {
            GJBaseGameLayer::update(dt);

            if (!config::get<bool>("player.rainbow", false)) return;

            auto speed = config::get<float>("player.rainbow.speed", 5.0f);
            auto color = utils::getRainbowColor(speed, 1.0f, 1.0f).toCCColor3B();

            if (m_player1) {
                if (config::get<bool>("player.rainbow.c1", true)) m_player1->setColor(color);
                if (config::get<bool>("player.rainbow.c2", false)) m_player1->setSecondColor(color);
                if (config::get<bool>("player.rainbow.glow", false)) m_player1->m_glowColor = color;
            }

            if (m_player2) {
                if (config::get<bool>("player.rainbow.c1", true)) m_player2->setColor(color);
                if (config::get<bool>("player.rainbow.c2", false)) m_player2->setSecondColor(color);
                if (config::get<bool>("player.rainbow.glow", false)) m_player2->m_glowColor = color;
            }
        }
    };
}
