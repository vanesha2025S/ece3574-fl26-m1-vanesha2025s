#include "aiws/retrieval_engine.hpp"
#include "aiws/text_processor.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>

namespace aiws {

double RetrievalEngine::canonical_score(double score) {
    // round to 12 decimal places
    return std::round(score * 1e12) / 1e12;
}

std::vector<SearchResult> RetrievalEngine::search(const std::string& query,
                                                  int k,
                                                  const std::vector<Chunk>& chunks,
                                                  const CorpusIndex& index) const {
    // check k
    if (k < 0) {
        throw std::invalid_argument("k cannot be negative");
    }

    if (k == 0) {
        return {};
    }

    // get query terms
    std::vector<std::string> terms = TextProcessor::terms(query);

    if (terms.empty()) {
        return {};
    }

    // remove repeated terms
    std::unordered_set<std::string> unique_terms(terms.begin(), terms.end());

    std::unordered_map<std::size_t, double> scores;
    std::unordered_map<std::size_t, std::size_t> matched;

    std::size_t total_chunks = chunks.size();

    // score each query term
    for (const std::string& term : unique_terms) {
        const std::vector<CorpusIndex::Posting>* term_postings =
            index.postings(term);

        // skip terms not in corpus
        if (term_postings == nullptr) {
            continue;
        }

        std::size_t df = index.document_frequency(term);

        double idf = std::log(
            static_cast<double>(total_chunks + 1) /
            static_cast<double>(df + 1)) + 1.0;

        // score chunks with this term
        for (const CorpusIndex::Posting& posting : *term_postings) {
            double tf = 1.0 + std::log(
                static_cast<double>(posting.frequency));

            scores[posting.chunk_index] += tf * idf;
            matched[posting.chunk_index]++;
        }
    }

    std::vector<SearchResult> results;

    // create search results
    for (const auto& pair : scores) {
        std::size_t chunk_index = pair.first;
        const Chunk& chunk = chunks[chunk_index];

        double coverage = 1.0 + 0.10 *
            static_cast<double>(matched[chunk_index]) /
            static_cast<double>(unique_terms.size());

        double score = canonical_score(pair.second * coverage);

        // create search result
        SearchResult result;
        result.chunk_id = chunk.id;
        result.document_id = chunk.document_id;
        result.chunk_sequence = chunk.sequence;
        result.text = chunk.text;
        result.score = score;
        result.matched_terms = matched[chunk_index];

        results.push_back(result);

    }

    // sort results
    std::sort(results.begin(), results.end(),[&index](const SearchResult& a, const SearchResult& b) {
        if (a.score != b.score) {
            return a.score > b.score;
        }

        std::size_t a_index = index.chunk_index(a.chunk_id);
        std::size_t b_index = index.chunk_index(b.chunk_id);

        if (a_index != b_index) {
            return a_index < b_index;
        }

        return a.chunk_sequence < b.chunk_sequence;
    });

    // keep top k
    if (results.size() > static_cast<std::size_t>(k)) {
        results.resize(static_cast<std::size_t>(k));
    }

    return results;
}

}  // namespace aiws

