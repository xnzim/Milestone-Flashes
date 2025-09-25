#include <Geode/Geode.hpp>

using namespace geode::prelude;

#include <Geode/modify/PlayLayer.hpp>
class $modify(MilestonePlayLayer, PlayLayer) {
    struct Fields {
        bool milestone25Hit = false;
        bool milestone50Hit = false;
        bool milestone75Hit = false;
        bool milestone90Hit = false;
        bool milestone95Hit = false;
        bool testFlashHit = false;  // Test flash at 5%
        CCLabelBMFont* percentLabel = nullptr;
        ccColor3B originalColor = {255, 255, 255};
        std::string originalText = "";
        float originalScale = 1.0f;  // Store original scale
    };

public:
    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) {
            return false;
        }

        // Reset milestone flags
        m_fields->milestone25Hit = false;
        m_fields->milestone50Hit = false;
        m_fields->milestone75Hit = false;
        m_fields->milestone90Hit = false;
        m_fields->milestone95Hit = false;
        m_fields->testFlashHit = false;
        m_fields->percentLabel = nullptr;

        geode::log::info("Milestone Percentage Flashes mod initialized for level: {}", level ? level->m_levelName : "Unknown");

        return true;
    }

    void onExit() {
        PlayLayer::onExit();
        geode::log::info("🚪 PlayLayer onExit called - mod is shutting down");

        // Custom cleanup if needed
    }

    ~MilestonePlayLayer() {
        geode::log::info("🗑️ MilestonePlayLayer destructor called");
    }

    void setupHasCompleted() {
        PlayLayer::setupHasCompleted();
        geode::log::info("PlayLayer setup completed - trying to find percentage label");
        findPercentageLabel();

        // Note: Will rely on other hooks (postUpdate, updateProgressbar) for now
    }

    void update(float dt) {
        PlayLayer::update(dt);

        // Update hook for milestone checking

        // Get the percentage label if we haven't found it yet
        if (!m_fields->percentLabel) {
            findPercentageLabel();
        }

        if (m_fields->percentLabel) {
            checkMilestones();
        }
    }

    void postUpdate(float dt) {
        PlayLayer::postUpdate(dt);

        // Run milestone checking from postUpdate
        if (m_fields->percentLabel) {
            checkMilestones();
        }
    }

    void updateVisibility(float dt) {
        PlayLayer::updateVisibility(dt);
        // Visibility hook (no logging needed)
    }

    void updateProgressbar() {
        PlayLayer::updateProgressbar();

        // Use progressbar updates as a trigger for milestone checking
        if (m_fields->percentLabel) {
            checkMilestones();
        }
    }


private:
    void findPercentageLabel() {
        static int attemptCount = 0;
        attemptCount++;

        // Attempting to find percentage label

        // Method 1: Direct member access (most reliable)
        if (this->m_percentageLabel) {
            m_fields->percentLabel = this->m_percentageLabel;
            m_fields->originalColor = this->m_percentageLabel->getColor();
            m_fields->originalText = std::string(this->m_percentageLabel->getString());
            m_fields->originalScale = this->m_percentageLabel->getScale();
            geode::log::info("Found percentage label via direct access! Text: '{}', Color: R:{} G:{} B:{}, Scale: {}",
                m_fields->originalText, m_fields->originalColor.r, m_fields->originalColor.g, m_fields->originalColor.b, m_fields->originalScale);

            return;
        }

        // Method 2: Node ID access (using geode.node-ids)
        if (auto percentNode = this->getChildByID("percentage-label")) {
            if (auto labelNode = typeinfo_cast<CCLabelBMFont*>(percentNode)) {
                m_fields->percentLabel = labelNode;
                m_fields->originalColor = labelNode->getColor();
                m_fields->originalScale = labelNode->getScale();
                geode::log::info("Found percentage label via node ID! Color: R:{} G:{} B:{}, Scale: {}",
                    m_fields->originalColor.r, m_fields->originalColor.g, m_fields->originalColor.b, m_fields->originalScale);
                return;
            } else {
                geode::log::warn("Found percentage-label node but it's not a CCLabelBMFont");
            }
        }

        // Method 3: Search through PlayLayer children
        if (auto children = this->getChildren()) {
            CCObject* child;
            CCARRAY_FOREACH(children, child) {
                if (auto label = dynamic_cast<CCLabelBMFont*>(child)) {
                    std::string labelText = label->getString();
                    if (labelText.find("%") != std::string::npos) {
                        m_fields->percentLabel = label;
                        m_fields->originalColor = label->getColor();
                        m_fields->originalScale = label->getScale();
                        geode::log::info("Found percentage label via search! Text: '{}', Color: R:{} G:{} B:{}, Scale: {}",
                            labelText, m_fields->originalColor.r, m_fields->originalColor.g, m_fields->originalColor.b, m_fields->originalScale);
                        return;
                    }
                }
            }
        }

        // Only log warning once if label not found
        if (attemptCount == 10) {
            geode::log::warn("Could not find percentage label after {} attempts", attemptCount);
        }
    }

    void checkMilestones() {
        // Get current progress percentage
        float percentage = getCurrentProgress();

        //// Test flash at 5% to verify system works
        //if (percentage > 5.0f && !m_fields->testFlashHit) {
        //    m_fields->testFlashHit = true;
        //    geode::log::info("🧪 TEST FLASH at {}% progress!", percentage);
        //    testFlash();
        //}

        // Check for milestone hits
        if (percentage > 25.0f && !m_fields->milestone25Hit) {
            m_fields->milestone25Hit = true;
            geode::log::info(" 25% MILESTONE HIT at {}% progress!", percentage);
            flashPercentageLabel();
        } else if (percentage > 50.0f && !m_fields->milestone50Hit) {
            m_fields->milestone50Hit = true;
            geode::log::info(" 50% MILESTONE HIT at {}% progress!", percentage);
            flashPercentageLabel();
        } else if (percentage > 75.0f && !m_fields->milestone75Hit) {
            m_fields->milestone75Hit = true;
            geode::log::info(" 75% MILESTONE HIT at {}% progress!", percentage);
            flashPercentageLabel();
        } else if (percentage > 90.0f && !m_fields->milestone90Hit) {
            m_fields->milestone90Hit = true;
            geode::log::info(" 90% MILESTONE HIT at {}% progress!", percentage);
            flashPercentageLabel();
        } else if (percentage > 95.0f && !m_fields->milestone95Hit) {
            m_fields->milestone95Hit = true;
            geode::log::info(" 95% MILESTONE HIT at {}% progress!", percentage);
            flashPercentageLabel();
        }
    }

    float getCurrentProgress() {
        // Use PlayLayer's built-in method first
        float percentage = this->getCurrentPercent();

        // Fallback calculation if getCurrentPercent() returns 0 or invalid value
        if (percentage <= 0.0f && m_player1 && m_endPortal) {
            float playerX = m_player1->getPositionX();
            float levelLength = m_endPortal->getPositionX();

            if (levelLength > 0.0f) {
                percentage = (playerX / levelLength) * 100.0f;
                percentage = std::max(0.0f, std::min(100.0f, percentage));

                static bool fallbackWarningLogged = false;
                if (!fallbackWarningLogged) {
                    geode::log::debug("Using fallback progress calculation. Player: {}, Level: {}", playerX, levelLength);
                    fallbackWarningLogged = true;
                }
            }
        }

        // Optional: Add occasional progress logging (much less frequent)
        static int debugCount = 0;
        if (debugCount++ % 1800 == 0) {  // Log every ~30 seconds at 60 FPS
            geode::log::debug("Progress: {}%", percentage);
        }

        return percentage;
    }

    void flashPercentageLabel() {
        if (!m_fields->percentLabel) {
            geode::log::error("Cannot flash percentage label - label is null!");
            return;
        }

        // Starting flash animation

        // Stop any existing animations first
        m_fields->percentLabel->stopAllActions();

        // Validate original color - if all zeros, use white as fallback
        if (m_fields->originalColor.r == 0 && m_fields->originalColor.g == 0 && m_fields->originalColor.b == 0) {
            m_fields->originalColor = {255, 255, 255};
            geode::log::warn("Original color was black, using white as fallback");
        }

        // Create highly visible flash animation with scale and color changes using stored original scale
        auto scaleUp = CCScaleTo::create(0.1f, m_fields->originalScale * 1.3f);
        auto scaleDown = CCScaleTo::create(0.1f, m_fields->originalScale);
        auto scaleReset = CCScaleTo::create(0.05f, m_fields->originalScale);  // Final reset
        auto flashRed = CCTintTo::create(0.1f, 255, 50, 50);        // Bright red
        auto flashYellow = CCTintTo::create(0.1f, 255, 255, 0);     // Bright yellow
        auto flashGreen = CCTintTo::create(0.1f, 50, 255, 50);      // Bright green
        auto flashOriginal = CCTintTo::create(0.1f, m_fields->originalColor.r, m_fields->originalColor.g, m_fields->originalColor.b);
        auto delay = CCDelayTime::create(0.05f);

        // Create combined animations that run simultaneously
        auto colorSequence = CCSequence::create(
            flashRed,
            delay,
            flashYellow,
            delay,
            flashGreen,
            delay,
            flashRed,
            delay,
            flashYellow,
            delay,
            flashOriginal,
            nullptr
        );

        auto scaleSequence = CCSequence::create(
            scaleUp,
            scaleDown,
            scaleUp,
            scaleDown,
            scaleUp,
            scaleReset,  // Final guaranteed reset to original scale
            nullptr
        );

        // Run both animations simultaneously
        m_fields->percentLabel->runAction(colorSequence);
        m_fields->percentLabel->runAction(scaleSequence);
    }

    void testFlash() {
        if (!m_fields->percentLabel) {
            geode::log::error("Cannot do test flash - label is null!");
            return;
        }

        // Starting test flash animation

        // Simple quick flash to test if animations work
        m_fields->percentLabel->stopAllActions();

        // Simple color and scale flash without changing text
        auto flashBlue = CCTintTo::create(0.2f, 100, 100, 255);
        auto flashOriginal = CCTintTo::create(0.2f, m_fields->originalColor.r, m_fields->originalColor.g, m_fields->originalColor.b);
        auto scaleUp = CCScaleTo::create(0.1f, m_fields->originalScale * 1.5f);
        auto scaleDown = CCScaleTo::create(0.2f, m_fields->originalScale);

        auto colorSequence = CCSequence::create(flashBlue, flashOriginal, nullptr);
        auto scaleSequence = CCSequence::create(scaleUp, scaleDown, nullptr);

        m_fields->percentLabel->runAction(colorSequence);
        m_fields->percentLabel->runAction(scaleSequence);
    }
};