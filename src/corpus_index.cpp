#include "aiws/corpus_index.hpp"
#include "aiws/text_processor.hpp"


#include <stdexcept>
#include <unordered_map>
namespace aiws {

CorpusIndex::CorpusIndex(const std::vector<Chunk>& chunks) {
    build(chunks);
}

void CorpusIndex::build(const std::vector<Chunk>& chunks) {
    // clear old index
    postings_.clear();
    chunk_by_id_.clear();

    // go through chunks
    for (std::size_t i = 0; i < chunks.size(); i++) {
        chunk_by_id_[chunks[i].id] = i;

        // get terms
        std::vector<std::string> terms = TextProcessor::terms(chunks[i].text);
        std::unordered_map<std::string, std::size_t> frequencies;

        // count each term
        for (const std::string& term : terms) {
            frequencies[term]++;
        }

        // add terms to index
        for (const auto& pair : frequencies) {
            Posting posting;
            posting.chunk_index = i;
            posting.frequency = pair.second;
            postings_[pair.first].push_back(posting);
        }
    }
}

std::size_t CorpusIndex::document_frequency(
    const std::string& normalized_term) const noexcept {

    // find term
    auto it = postings_.find(normalized_term);

    if (it == postings_.end()) {
        return 0;
    }

    // number of chunks with term
    return it->second.size();
}

std::size_t CorpusIndex::term_frequency(
    const std::string& normalized_term,
    const std::string& chunk_id) const noexcept {

    // find term
    auto it = postings_.find(normalized_term);

    if (it == postings_.end()) {
        return 0;
    }

    // find chunk
    auto chunk_it = chunk_by_id_.find(chunk_id);

    if (chunk_it == chunk_by_id_.end()) {
        return 0;
    }

    // find frequency in chunk
    for (const Posting& posting : it->second) {
        if (posting.chunk_index == chunk_it->second) {
            return posting.frequency;
        }
    }

    return 0;
}

const std::vector<CorpusIndex::Posting>* CorpusIndex::postings(
    const std::string& normalized_term) const noexcept {

    // find term
    auto it = postings_.find(normalized_term);

    if (it == postings_.end()) {
        return nullptr;
    }

    return &it->second;
}

const Chunk* CorpusIndex::find_chunk(
    const std::vector<Chunk>& chunks,
    const std::string& chunk_id) const noexcept {

    // find chunk id
    auto it = chunk_by_id_.find(chunk_id);

    if (it == chunk_by_id_.end()) {
        return nullptr;
    }

    // check index
    if (it->second >= chunks.size()) {
        return nullptr;
    }

    return &chunks[it->second];
}

std::size_t CorpusIndex::chunk_index(const std::string& chunk_id) const {
    // find chunk id
    auto it = chunk_by_id_.find(chunk_id);

    if (it == chunk_by_id_.end()) {
        throw std::out_of_range("chunk not found");
    }

    return it->second;
}

}  // namespace aiws
