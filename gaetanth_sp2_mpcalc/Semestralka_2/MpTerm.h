#pragma once
#include "MpInt.h"
#include <iostream>
#include <queue>
#include <string>
#include <unordered_map>
#include <sstream>
#include <stdexcept>
#include <cctype>
#include <vector>
#include <regex>

template <std::size_t Precision>
class MPTerm final {  // Added 'final' to prevent inheritance
private:
    using MpType = MpInt<Precision>;

    std::deque<MpType> history;
    static constexpr std::size_t HistorySize = 5;

    // Tokenize input string to separate numbers and operators
    std::vector<std::string> tokenizeInput(const std::string& input) const {  // Added const
        std::vector<std::string> tokens;
        std::string current;

        for (char ch : input) {
            if (std::isdigit(ch) || ch == '$' || (ch == '-' && current.empty())) {
                current += ch;
            }
            else if (ch == '+' || ch == '-' || ch == '*' || ch == '/' || ch == '!') {
                if (!current.empty()) {
                    tokens.push_back(current);
                    current.clear();
                }
                tokens.emplace_back(1, ch);
            }
            else if (!std::isspace(ch)) {
                throw std::invalid_argument("Invalid character in input: " + std::string(1, ch));
            }
        }

        if (!current.empty()) {
            tokens.push_back(current);
        }

        return tokens;
    }

    // Parse placeholder or number
    MpType parseInput(const std::string& input) const {  // Added const
        if (input[0] == '$') {
            int index = input[1] - '1';
            if (index >= 0 && index < static_cast<int>(history.size())) {
                return history.at(index);
            }
            std::cerr << "Invalid history index: " << input << std::endl;
            throw std::out_of_range("Invalid history index");
        }
        return MpType::from_string(input);
    }

    // Store result in history
    void storeResult(const MpType& result) {
        if (history.size() == HistorySize) {
            history.pop_back();
        }
        history.push_front(result);
    }

    // Print history
    void printHistory() const {  // Added const
        for (std::size_t i = 0; i < history.size(); ++i) {
            std::cout << "$" << (i + 1) << ": " << history[i].to_string() << std::endl;
        }
    }

public:
    // Move assignment operator
    MPTerm& operator=(MPTerm&&) noexcept = default;

    // Process single command
    bool processCommand(const std::string& line) {  // Can't be const (modifies history)
        if (line.empty()) return true;

        if (line == "exit") {
            return false;
        }
        if (line == "bank") {
            printHistory();
            return true;
        }

        try {
            // Handle factorial operation
            if (line.find('!') != std::string::npos) {
                const size_t pos = line.find('!');
                const std::string num = line.substr(0, pos);
                const MpType value = parseInput(num);

                if (!value.fits_in_uint32()) {
                    throw std::overflow_error("Input too large for factorial");
                }

                const uint32_t n = value.to_uint32();
                const MpType result = MpType::factorial(n);
                storeResult(result);
                std::cout << "$1 = " << result.to_string() << std::endl;
                return true;
            }

            // Handle arithmetic operations
            const auto tokens = tokenizeInput(line);
            if (tokens.size() != 3) {
                throw std::invalid_argument("Invalid input format");
            }

            MpType result = parseInput(tokens[0]);
            for (size_t i = 1; i < tokens.size(); i += 2) {
                const std::string& op = tokens[i];
                const MpType rhs = parseInput(tokens[i + 1]);

                if (op == "+") result = result + rhs;
                else if (op == "-") result = result - rhs;
                else if (op == "*") result = result * rhs;
                else if (op == "/") result = result / rhs;
                else throw std::invalid_argument("Invalid operator: " + op);
            }

            storeResult(result);
            std::cout << "$1 = " << result.to_string() << std::endl;
            return true;
        }
        catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
            return true;
        }
    }

    // Main run loop
    void run() {
        std::cout << "To exit type 'exit', to show history type 'bank'" << std::endl;
        std::string line;
        while (true) {
            std::cout << ">> ";
            if (!std::getline(std::cin, line)) break;
            if (!processCommand(line)) break;
        }
    }
};

// Mode selector
void runMode(const int mode) {  // Added const
    if (mode == 1) {
        std::cout << "MPCalc - unlimited precision mode" << std::endl;
        MPTerm<MpInt<0>::Unlimited> unlimited_terminal;
        unlimited_terminal.run();
    }
    else if (mode == 2) {
        std::cout << "MPCalc - 32-byte precision mode" << std::endl;
        MPTerm<32> limited_terminal;
        limited_terminal.run();
    }
    else if (mode == 3) {
        std::cout << "MPCalc - demo mode" << std::endl;

        const std::vector<std::string> demoScript = {  // Added const
            "100+100",
            "5+1+1",
            "$1+$1",
            "bank",
            "100!",
            "exit"
        };

        MPTerm<80> demo_terminal;

        for (const auto& cmd : demoScript) {  // Added const reference
            // Show command with delay
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            std::cout << ">> " << cmd << std::endl;

            // Process command with delay
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
            if (!demo_terminal.processCommand(cmd)) break;
        }
    }
    else {
        std::cerr << "Invalid parameter. Use 1 (unlimited), 2 (32-byte), or 3 (demo)" << std::endl;
    }
}