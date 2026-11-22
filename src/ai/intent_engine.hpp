#pragma once
// zephyr-terminal / ai / intent_engine.hpp
//
// Natural language intent-to-command engine. Translates English queries
// into robust, safe shell commands with safety analysis and explanations.

#include "risk_classifier.hpp"

#include <string>
#include <vector>

namespace zephyr::ai {

struct IntentResult {
    std::string prompt;
    std::string generated_command;
    std::string explanation;
    RiskLevel risk = RiskLevel::Low;
    std::string risk_reason;
    float confidence = 0.9f;
    std::vector<std::string> alternatives;
};

class IntentEngine {
public:
    IntentEngine();

    IntentResult translate(const std::string& query, const std::string& cwd = ".") const;

    // Helper formatting for CLI
    static std::string format_card(const IntentResult& result);

private:
    RiskClassifier risk_classifier_;
};

} // namespace zephyr::ai

