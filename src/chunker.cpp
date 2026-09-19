#include "aiws/chunker.hpp"
#include "aiws/text_processor.hpp"
#include <stdexcept>

namespace aiws {

Chunker::Chunker(ChunkingPolicy policy) : policy_(policy) {
    if (policy_.max_tokens == 0 || policy_.overlap >= policy_.max_tokens ||
        policy_.paragraph_window > policy_.max_tokens) {
        throw std::invalid_argument("invalid chunking policy");
    }
}

std::vector<Chunk> Chunker::chunk(const Document& document, std::size_t document_order) const {
    // get tokens
    std::vector<TokenInfo> tokens = TextProcessor::tokenize(document.text());
    std::vector<Chunk> chunks;

    // check for empty document
    if (tokens.empty()) {
        return chunks;
    }

    std::size_t start = 0;
    std::size_t sequence = 0;

    // create chunks
    while (start < tokens.size()) {
        std::size_t end = start + policy_.max_tokens;
        // check chunk size
        if (end >= tokens.size()) {
            end = tokens.size();
        } else {
            // look for paragraph boundary
            std::size_t window_start = end - policy_.paragraph_window;

            for (std::size_t i = end; i >= window_start; i--) {
                if(i > start && tokens[i - 1].paragraph != tokens[i].paragraph) {
                    end = i;
                    break;
                }
            }
        }
        // make chunk
        Chunk current;
        current.id = document.id() + "#" + std::to_string(sequence);
        current.document_id = document.id();
        current.document_order = document_order;
        current.sequence = sequence;
        current.text = TextProcessor::join(tokens, start, end);
        current.token_count = end - start;
        current.source_begin = tokens[start].begin;
        current.source_end = tokens[end - 1].end;
        chunks.push_back(current);

        // check for last chunk
        if (end == tokens.size()) {
            break;
        }

        // add overlap
        start = end - policy_.overlap;
        sequence++;
    }

    return chunks;
}

}  // namespace aiws
