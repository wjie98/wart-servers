#include <imports.hpp>
#include <iostream>

int main() {
	auto a = std::get<future_table>(async_choice_nodes("player", 2));
	auto b = std::get<table>(a.get());
	for (size_t i = 0; i < b.size(); i++) {
		auto view = b[i].as_txt();
		for (size_t j = 0; j < view.size(); j++) {
			std::cout << view[j] << std::endl;
		}
	}

	std::string_view key1 = "a";
	std::string_view key2 = "b";
	std::string_view key3 = "c";
	std::vector<std::string_view> keys = {key1, key2, key3};
	auto c = std::get<future_row>(async_query_kv(keys));
	auto d = std::get<row>(c.get());
	for (size_t i = 0; i < d.size(); i++) {
		auto view = d[i];
		std::cout << view.is_txt() << std::endl;
	}

	std::vector<int64_t> src = {1, 2, 3};
	std::vector<int64_t> dst = {2, 3, 1};
	select_edges(src, dst);
	return 0;
}
