#include "aiws/processing_core.hpp"

#include <iostream>
#include <stdexcept>
#include <string>

namespace {

int failures = 0;

void check(bool condition, const std::string& message) {
    if (!condition) {
        ++failures;
        std::cerr << "FAIL: " << message << '\n';
    }
}

}

int main() {
    using namespace aiws;

    // test lowercase
    check(ProcessingCore::normalize("TEST-Case_123") == "test case 123","normalization with separators");

    // test empty input
    check(ProcessingCore::normalize("").empty(), "empty normalization");

    Workspace ws;
    ws.add_document(Document{"d1", "One", "apple banana apple"});
    ProcessingCore core;
    core.rebuild(ws);

    // test missing term
    check(core.document_frequency("orange") == 0,"missing term has zero document frequency");

    // test missing chunk
    check(core.term_frequency("apple", "missing") == 0, "missing chunk has zero term frequency");

    // test multiple term input
    bool term_threw = false;

    try {
        (void)core.document_frequency("apple banana");
    }
    catch (const std::invalid_argument&) {
        term_threw = true;
    }

    check(term_threw,"multiple terms throw invalid argument");

    // test k equals zero
    auto zero_results = core.search("apple", 0);

    check(zero_results.empty(), "zero k returns no results");

    // test empty query
    auto empty_results = core.search("!!!", 10);

    check(empty_results.empty(),"empty query returns no results");

    // test repeated query terms
    auto first = core.search("apple", 10);
    auto repeated = core.search("apple apple apple", 10);

    check(!first.empty() && !repeated.empty() && first[0].score == repeated[0].score, "repeated query terms do not change score");

    // test zero context budget
    auto zero_context = core.build_context("apple", 10, 0);

    check(zero_context.empty(),"zero token budget returns no context");

    // test exact context budget
    auto exact_context = core.build_context("apple", 10, 3);

    check(exact_context.size() == 1 && exact_context[0].token_count == 3 &&!exact_context[0].truncated,"exact token budget keeps full chunk");

    // test rebuild replaces old corpus
    Workspace new_ws;
    new_ws.add_document(Document{"d2", "Two", "orange grape"});

    core.rebuild(new_ws);

    check(core.document_frequency("apple") == 0,"rebuild removes old corpus");

    check(core.document_frequency("orange") == 1,"rebuild adds new corpus");

    // test duplicate document ids
    Workspace duplicate_ws;
    duplicate_ws.add_document(Document{"same", "One", "alpha"});
    duplicate_ws.add_document(Document{"same", "Two", "beta"});

    bool duplicate_threw = false;

    try {
        core.rebuild(duplicate_ws);
    }
    catch (const std::invalid_argument&) {
        duplicate_threw = true;
    }

    check(duplicate_threw,"duplicate document ids throw invalid argument");

    // make sure old corpus is still there
    check(core.document_frequency("orange") == 1,"failed rebuild keeps old corpus");

    if (failures == 0) {
        std::cout << "All student tests passed.\n";
        return 0;
    }

    std::cerr << failures << " student test(s) failed.\n";
    return 1;
}