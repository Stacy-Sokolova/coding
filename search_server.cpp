#include "search_server.h"
#include "iterator_range.h"
#include "parse.h"

#include <algorithm>
#include <iterator>
#include <sstream>
#include <iostream>
#include <deque>
#include <unordered_map>
#include <unordered_set>
#include <array>

vector<string_view> SplitIntoWords(string_view s) {
	//istringstream words_input(line);
	//return { istream_iterator<string>(words_input), istream_iterator<string>() };
	//return SplitBy(Strip(s), ' ');
	vector<string_view> result;
	while (!s.empty()) {

		while (!s.empty() && isspace(s.front())) {
			s.remove_prefix(1);
		}

		if (s.empty()) break;

		size_t pos = s.find(' ');
		result.push_back(s.substr(0, pos));
		s.remove_prefix(pos != s.npos ? pos + 1 : s.size());
	}
	return result;
}

SearchServer::SearchServer(istream& document_input) {
	UpdateDocumentBase(document_input);
}

void UpdateDocumentBaseAsynk(istream& document_input, Synchronized<InvertedIndex>& sync_index) {
	InvertedIndex new_index;
	for (string current_document; getline(document_input, current_document); ) {
		new_index.Add(move(current_document));
	}
	auto access = sync_index.GetAccess();
	//swap(access.ref_to_index, new_index);
	access.ref_to_index = move(new_index);
}

void SearchServer::UpdateDocumentBase(istream& document_input) {
	InvertedIndex new_index;
	for (string current_document; getline(document_input, current_document); ) {
		new_index.Add(move(current_document));
	}
	auto access = index.GetAccess();
	access.ref_to_index = move(new_index);
	//swap(access.ref_to_index, new_index);
	//future<void>(async(UpdateDocumentBaseAsynk, ref(document_input), ref(index)));
}

void AddQueriedAsynk(istream& query_input, ostream& search_results_output, Synchronized<InvertedIndex>& sync_index) {
	size_t size = sync_index.GetAccess().ref_to_index.docs_size;
	vector<pair<size_t, size_t> > search_results;
	search_results.reserve(size);

	for (string current_query; getline(query_input, current_query); )
	{
		search_results.assign(size, { 0, 0 });
		vector<string_view> words = SplitIntoWords(move(current_query));
		{

			auto access = sync_index.GetAccess();

			for (const auto& word : words) {
				for (auto& [docid, count] : access.ref_to_index.Lookup(word)) {
					auto hit = search_results[docid].second;
					search_results[docid] = { docid, count + hit };
				}
			}
		}

		auto part_end = search_results.begin() + min(static_cast<size_t>(5), search_results.size());

		partial_sort(
			search_results.begin(), part_end,
			search_results.end(),
			[](const pair< size_t, size_t>& lhs, const pair<size_t, size_t>& rhs) {
				int64_t lhs_docid = lhs.first;
				auto lhs_hit_count = lhs.second;
				int64_t rhs_docid = rhs.first;
				auto rhs_hit_count = rhs.second;
				return make_pair(move(lhs_hit_count), -move(lhs_docid)) >
					make_pair(move(rhs_hit_count), -move(rhs_docid));
			}
		);

		search_results_output << current_query << ':';
		for (auto [docid, hitcount] : Head(search_results, 5)) {
			if (hitcount != 0) {
				search_results_output << " {"
					<< "docid: " << docid << ", "
					<< "hitcount: " << hitcount << '}';
			}
		}
		search_results_output << '\n';

		search_results.clear();
	}
}

void SearchServer::AddQueriesStream(
	istream& query_input, ostream& search_results_output
) {
	f.push_back(async(launch::async, AddQueriedAsynk, ref(query_input), ref(search_results_output), ref(index)));
}

void InvertedIndex::Add(const string& document) {
	const size_t docid = docs_size;
	docs_size++;
	docs.push_back(document);
	map<string_view, size_t> tempData;

	for (string_view word : SplitIntoWords(docs.back())) {
		tempData[word]++;
	}

	for (const auto& i : tempData) {
		index[i.first].push_back({ docid,i.second });
	}
}

const vector< pair<size_t, size_t>>& InvertedIndex::Lookup(string_view word) const {
	auto itf = index.find(word);
	auto ite = index.end();

	return itf != ite ? itf->second : z;
}