#include "aiws/context_builder.hpp"
#include "aiws/text_processor.hpp"
namespace aiws {

std::vector<ContextItem> ContextBuilder::build(const std::vector<SearchResult>& ranked,std::size_t token_budget) const {

    std::vector<ContextItem> context;
    std::size_t tokens_used = 0;

    // go through ranked results
    for (const SearchResult& result : ranked){
        // get tokens from result
        std::vector<std::string> terms = TextProcessor::terms(result.text);
        std::size_t tokens_left = token_budget - tokens_used;

        // check if budget is used
        if (tokens_left == 0) {
            break;
        }

        // make context item
        ContextItem item;
        item.chunk_id = result.chunk_id;
        item.document_id = result.document_id;
        item.chunk_sequence = result.chunk_sequence;
        item.score = result.score;
        item.truncated = false;

        // add full result if it fits
        if (terms.size() <= tokens_left) {
            item.text = result.text;
            item.token_count = terms.size();

            context.push_back(item);
            tokens_used += terms.size();
        } else {
            // add part that fits
            item.text = TextProcessor::join(terms, 0, tokens_left);
            item.token_count = tokens_left;
            item.truncated = true;

            context.push_back(item);
            break;
        }
    }

    return context;
}

}  // namespace aiws
