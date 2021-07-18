#include <iostream>
#include <memory>
#include <unordered_map>
#include <string_view>
#include <set>
#include <string>
#include <utility>
#include <list>
#include <optional>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <vector>

using namespace std;

const double pi = 3.1415926535;
const int r = 6371000;

struct Coordinates {
	double lat = 0.;
	double lon = 0.;

	Coordinates() {}

	Coordinates(double X, double Y) :
		lat(X* pi / 180), lon(Y* pi / 180) {}
};

class RouteManager {
public:
	RouteManager() {}

	unordered_map<string, Coordinates> stops;
	unordered_map<string, set<string>> stopinfo;
	unordered_map<string, pair<char, list<string>>> buses;
};

struct Request;
using RequestHolder = unique_ptr<Request>;

struct Request {
	enum class Type {
		BUS,
		STOP,
		INPUT
	};

	Request(Type type) : type_(type) {}
	static RequestHolder Create(Type type);
	virtual void ParseFrom(string_view input) = 0;
	virtual ~Request() = default;

	const Type type_;
};

const unordered_map<string_view, Request::Type> STR_TO_REQUEST_TYPE = {
	{"Bus", Request::Type::BUS},
	{"Stop", Request::Type::STOP},
	{"In", Request::Type::INPUT}
};

template <typename ResultType>
struct ReadRequest : Request {
	using Request::Request;
	virtual ResultType Process(const RouteManager& manager) const = 0;
};

struct ModifyRequest : Request {
	using Request::Request;
	virtual void Process(RouteManager& manager) = 0;
};

pair<string_view, optional<string_view>> SplitTwoStrict(string_view s, string_view delimiter = " ") {
	const size_t pos = s.find(delimiter);
	if (pos == s.npos) {
		return { s, nullopt };
	}
	else {
		return { s.substr(0, pos), s.substr(pos + delimiter.length()) };
	}
}

pair<string_view, string_view> SplitTwo(string_view s, string_view delimiter = " ") {
	const auto splited = SplitTwoStrict(s, delimiter);
	return { splited.first, splited.second.value_or("") };
}

string_view ReadToken(string_view& s, string_view delimiter = " ") {
	auto splited = SplitTwo(s, delimiter);
	s = splited.second;
	while (splited.first.front() == ' ')
		splited.first.remove_prefix(1);
	while (splited.first.back() == ' ')
		splited.first.remove_suffix(1);
	return splited.first;
}

struct InputReq : ModifyRequest {
	InputReq() : ModifyRequest(Type::INPUT) {}

	void ParseFrom(string_view input) {
		type = string(ReadToken(input));
		name = string(ReadToken(input, ":"));
		if (type == "Bus") {
			auto pos = input.find("-");
			routetype = (pos == input.npos) ? ">" : "-"; //> -
			while (!input.empty()) {
				stops.push_back(string(ReadToken(input, routetype)));
			}
		}
		else if (type == "Stop") {
			x = stof(ReadToken(input, ",").data());
			y = stof(input.data());
		}
	}

	void Process(RouteManager& manager) override {
		if (type == "Bus") {
			char routet = (routetype == "-") ? '-' : '>';
			manager.buses.insert({ name, {routet, stops} });
			for (auto& stop : stops) {
				if (manager.stopinfo.find(stop) == manager.stopinfo.end())
					manager.stopinfo.insert({ stop, {} });
				manager.stopinfo.at(stop).insert(name);
			}
		}
		else if (type == "Stop") {
			manager.stops.insert({ name, Coordinates(x,y) });
			if (manager.stopinfo.find(name) == manager.stopinfo.end())
				manager.stopinfo.insert({ name, {} });
		}
	}

	string type;
	string name;
	double x; double y;
	Coordinates coordinates;
	list<string> stops;
	string routetype;
};

struct BusReq : ReadRequest<string> {
	BusReq() : ReadRequest(Type::BUS) {}

	void ParseFrom(string_view input) {
		while (input.front() == ' ')
			input.remove_prefix(1);
		while (input.back() == ' ')
			input.remove_suffix(1);
		bus_num = string(input);
	}

	int UniqueStops(string bus_num, const RouteManager& manager) const {
		set<string_view> uniq;
		auto start = manager.buses.at(bus_num).second.begin();
		while (start != manager.buses.at(bus_num).second.end()) {
			uniq.insert(*(start++));
		}
		int size = uniq.size();
		//uniq.clear();
		return size;
	}

	int StopsOnRoute(string bus_num, const RouteManager& manager) const {
		int count = manager.buses.at(bus_num).second.size();
		if (manager.buses.at(bus_num).first == '-') {
			count = count * 2 - 1;
		}
		return count;
	}

	string Process(const RouteManager& manager) const override {
		ostringstream out_stream;
		auto pos = manager.buses.find(bus_num);
		if (pos == manager.buses.end()) {
			out_stream << "Bus " << bus_num << ": not found\n";
		}
		else {
			int count = StopsOnRoute(bus_num, manager);
			int uniq = UniqueStops(bus_num, manager);

			double length = 0.;
			auto start = manager.buses.at(bus_num).second.begin();
			while (start != prev(manager.buses.at(bus_num).second.end())) {
				const auto& rhs = manager.stops.at(*start); start++;
				const auto& lhs = manager.stops.at(*start);
				length += acos(sin(lhs.lat) * sin(rhs.lat) +
					cos(lhs.lat) * cos(rhs.lat) *
					cos(fabs(lhs.lon - rhs.lon))
				) * r;
			}

			if (manager.buses.at(bus_num).first == '-') {
				length *= 2;
			}

			out_stream << "Bus " << bus_num << ": " <<
				count << " stops on route, " <<
				uniq << " unique stops, " <<
				setprecision(6) << length << " route length\n";
		}
		return out_stream.str();
	}

	string bus_num;
};

struct StopReq : ReadRequest<string> {
	StopReq() : ReadRequest(Type::STOP) {}

	void ParseFrom(string_view input) {
		while (input.front() == ' ')
			input.remove_prefix(1);
		while (input.back() == ' ')
			input.remove_suffix(1);
		stop = string(input);
	}

	string Process(const RouteManager& manager) const override {
		ostringstream out_stream;
		if (manager.stopinfo.find(stop) == manager.stopinfo.end())
			out_stream << "Stop " << stop << ": not found\n";
		else {
			if (manager.stopinfo.at(stop).empty())
				out_stream << "Stop " << stop << ": no buses\n";
			else {
				out_stream << "Stop " << stop << ": buses ";
				auto it = manager.stopinfo.at(stop).begin();
				while (it != prev(manager.stopinfo.at(stop).end()))
					out_stream << *(it++) << " ";
				out_stream << *it << "\n";
			}
		}
		return out_stream.str();
	}

	string stop;
};

RequestHolder Request::Create(Request::Type type) {
	switch (type) {
	case Request::Type::BUS:
		return make_unique<BusReq>();
	case Request::Type::STOP:
		return make_unique<StopReq>();
	case Request::Type::INPUT:
		return make_unique<InputReq>();
	default:
		return nullptr;
	}
}

template <typename Number>
Number ReadNumberOnLine(istream& stream) {
	Number number;
	stream >> number;
	string dummy;
	getline(stream, dummy);
	return number;
}

optional<Request::Type> ConvertRequestTypeFromString(string_view type_str) {
	if (const auto it = STR_TO_REQUEST_TYPE.find(type_str);
		it != STR_TO_REQUEST_TYPE.end()) {
		return it->second;
	}
	else {
		return nullopt;
	}
}

RequestHolder ParseRequest(string_view request_str) {
	const auto request_type = ConvertRequestTypeFromString(ReadToken(request_str));
	if (!request_type) {
		return nullptr;
	}
	RequestHolder request = Request::Create(*request_type);
	if (request) {
		request->ParseFrom(request_str);
	};
	return request;
}

vector<RequestHolder> ReadRequests(istream& in = cin) {
	const size_t modifyrequest_count = ReadNumberOnLine<size_t>(in);

	vector<RequestHolder> requests;
	//requests.resize(modifyrequest_count);

	for (size_t i = 0; i < modifyrequest_count; ++i) {
		string request_str;
		getline(in, request_str);
		const auto request_type = Request::Type::INPUT;
		RequestHolder request = Request::Create(request_type);
		request->ParseFrom(request_str);
		requests.push_back(move(request));
	}

	const size_t request_count = ReadNumberOnLine<size_t>(in);
	//requests.resize(modifyrequest_count + request_count);

	for (size_t i = 0; i < request_count; ++i) {
		string request_str;
		getline(in, request_str);
		if (auto request = ParseRequest(request_str)) {
			requests.push_back(move(request));
		}
	}

	return requests;
}

vector<string> ProcessRequests(const vector<RequestHolder>& requests) {
	vector<string> responses;
	RouteManager manager;
	for (const auto& request_holder : requests) {
		if (request_holder->type_ == Request::Type::BUS) {
			const auto& request = static_cast<const BusReq&>(*request_holder);
			responses.push_back(request.Process(manager));
		}
		if (request_holder->type_ == Request::Type::STOP) {
			const auto& request = static_cast<const StopReq&>(*request_holder);
			responses.push_back(request.Process(manager));
		}
		if (request_holder->type_ == Request::Type::INPUT) {
			auto& request = static_cast<InputReq&>(*request_holder);
			request.Process(manager);
		}
	}
	return responses;
}

void PrintResponses(const vector<string>& responses, ostream& out = cout) {
	for (const string res : responses)
		out << res;
}

int main() {
	const auto requests = ReadRequests();
	const auto responses = ProcessRequests(requests);
	PrintResponses(responses);

	return 0;
}