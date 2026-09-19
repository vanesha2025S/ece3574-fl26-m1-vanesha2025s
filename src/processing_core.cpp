#include "aiws/processing_core.hpp"
#include "aiws/chunker.hpp"
#include "aiws/context_builder.hpp"
#include "aiws/corpus_index.hpp"
#include "aiws/retrieval_engine.hpp"
#include "aiws/text_processor.hpp"

#include <stdexcept>
#include <unordered_set>
namespace aiws {

struct ProcessingCore::Impl {
    std::vector<Chunk> chunks;
    CorpusIndex index;
    RetrievalEngine retrieval;
    ContextBuilder context_builder;
};

ProcessingCore::ProcessingCore() : impl_(std::make_unique<Impl>()) { }

ProcessingCore::~ProcessingCore() = default;

ProcessingCore::ProcessingCore(ProcessingCore&&) noexcept = default;

ProcessingCore& ProcessingCore::operator=(ProcessingCore&&) noexcept = default;

std::string ProcessingCore::normalize(const std::string& text) {
    return TextProcessor::normalize(text);
}

void ProcessingCore::rebuild(const Workspace& workspace) {
    // check document ids
    std::unordered_set<std::string> ids;

    for (const Document& document : workspace.documents()) {
        if (!ids.insert(document.id()).second) {
            throw std::invalid_argument("duplicate document id");
        }
    }

    // make new chunks
    std::vector<Chunk> new_chunks;
    Chunker chunker;

    for (std::size_t i = 0; i < workspace.documents().size(); i++) {
        std::vector<Chunk> document_chunks =
            chunker.chunk(workspace.documents()[i], i);

        new_chunks.insert(new_chunks.end(),document_chunks.begin(),document_chunks.end());
    }

    // make new index
    CorpusIndex new_index(new_chunks);

    // replace old corpus
    impl_->chunks = std::move(new_chunks);
    impl_->index = std::move(new_index);
}

const std::vector<Chunk>& ProcessingCore::chunks() const noexcept {
    // return stored chunks
    return impl_->chunks;
}

std::size_t ProcessingCore::chunk_count() const noexcept {
    // return number of chunks
    return impl_->chunks.size();
}

std::size_t ProcessingCore::document_frequency(const std::string& term) const {
    // normalize term
    std::vector<std::string> terms = TextProcessor::terms(term);

    if (terms.empty()) {
        return 0;
    }

    if (terms.size() > 1) {
        throw std::invalid_argument("term must contain one token");
    }

    return impl_->index.document_frequency(terms[0]);
}

std::size_t ProcessingCore::term_frequency(const std::string& term,
                                           const std::string& chunk_id) const {
    // normalize term
    std::vector<std::string> terms = TextProcessor::terms(term);

    if (terms.empty()) {
        return 0;
    }

    if (terms.size() > 1) {
        throw std::invalid_argument("term must contain one token");
    }

    return impl_->index.term_frequency(terms[0], chunk_id);
}

std::vector<SearchResult> ProcessingCore::search(const std::string& query, int k) const {
    // search corpus
    return impl_->retrieval.search(query, k, impl_->chunks, impl_->index);
}

std::vector<ContextItem> ProcessingCore::build_context(const std::string& query,
                                                       int k,
                                                       std::size_t token_budget) const {
    // get ranked results
    std::vector<SearchResult> ranked = search(query, k);

    // build context
    return impl_->context_builder.build(ranked, token_budget);
}

}  // namespace aiws
