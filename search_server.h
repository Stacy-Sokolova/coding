#pragma once

#include <istream>
#include <ostream>
#include <set>
#include <list>
#include <vector>
#include <deque>
#include <map>
#include <string>
#include <utility>
#include <future>
#include <unordered_map>
#include <string_view>
using namespace std;

template <typename T>
class Synchronized {
public:
	explicit Synchronized(T initial = T())
		: value(move(initial))
	{
	}
	struct Access {
		lock_guard<mutex> guard;
		T& ref_to_index;
	};
	Access GetAccess() {
		return { lock_guard(m), value };
	}
private:
	T value;
	mutex m;
};

class InvertedIndex {
public:

	InvertedIndex() : z(0) {
		docs.reserve(50001);
	}
	void Add(const string& document);
	const vector< pair<size_t, size_t>>& Lookup(string_view word) const;

	size_t docs_size = 0;
private:
	unordered_map<string_view, vector<pair<size_t, size_t>>> index;
	vector<string> docs;
	vector<pair<size_t, size_t>> z;
};

class SearchServer {
public:
	SearchServer() = default;
	explicit SearchServer(istream& document_input);
	void UpdateDocumentBase(istream& document_input);
	void AddQueriesStream(istream& query_input, ostream& search_results_output);

private:
	Synchronized<InvertedIndex> index;
	vector<future<void>> f;
};