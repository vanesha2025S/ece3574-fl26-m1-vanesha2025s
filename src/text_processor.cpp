#include "aiws/text_processor.hpp"

namespace aiws {

std::vector<TokenInfo> TextProcessor::tokenize(const std::string& text) {
    std::vector<TokenInfo> tokens;

    std::string current;
    std::size_t start = 0;
    std::size_t paragraph = 0;

    bool previous_newline = false;
    bool only_spaces = false;

    for (std::size_t i = 0; i < text.size(); i++) {
        char c = text[i];

        // handle crlf as one newline
        if (c == '\r' && i + 1 < text.size() && text[i + 1] == '\n') {
            c = '\n';
            i++;
        }

        bool letter = (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
        bool digit = (c >= '0' && c <= '9');

        if (letter || digit) {
            // start new token
            if (current.empty()) {
                start = i;
            }

            // lowercase letters
            if (c >= 'A' && c <= 'Z') {
                c = static_cast<char>(c - 'A' + 'a');
            }

            current += c;

            previous_newline = false;
            only_spaces = false;
        } else {
            // finish current token
            if (!current.empty()) {
                TokenInfo token;
                token.token = current;
                token.begin = start;
                token.end = i;
                token.paragraph = paragraph;

                tokens.push_back(token);
                current.clear();
            }

            // check paragraph boundary
            if (c == '\n') {
                if (previous_newline || only_spaces) {
                    paragraph++;
                }

                previous_newline = true;
                only_spaces = false;
            } else if ((c == ' ' || c == '\t') && previous_newline) {
                only_spaces = true;
            } else if (c != ' ' && c != '\t') {
                previous_newline = false;
                only_spaces = false;
            }
        }
    }

    // add last token
    if (!current.empty()) {
        TokenInfo token;
        token.token = current;
        token.begin = start;
        token.end = text.size();
        token.paragraph = paragraph;

        tokens.push_back(token);
    }

    return tokens;
}

std::vector<std::string> TextProcessor::terms(const std::string& text) {
    std::vector<TokenInfo> tokens = tokenize(text);
    std::vector<std::string> result;

    for (const TokenInfo& token : tokens) {
        result.push_back(token.token);
    }

    return result;
}

std::string TextProcessor::normalize(const std::string& text) {
    std::vector<std::string> words = terms(text);
    return join(words, 0, words.size());
}

std::string TextProcessor::join(const std::vector<TokenInfo>& tokens,
                                std::size_t begin,
                                std::size_t end) {
    std::string result;

    for (std::size_t i = begin; i < end; i++) {
        if (!result.empty()) {
            result += " ";
        }

        result += tokens[i].token;
    }

    return result;
}

std::string TextProcessor::join(const std::vector<std::string>& tokens,
                                std::size_t begin,
                                std::size_t end) {
    std::string result;

    for (std::size_t i = begin; i < end; i++) {
        if (!result.empty()) {
            result += " ";
        }

        result += tokens[i];
    }

    return result;

}

}  // namespace aiws
