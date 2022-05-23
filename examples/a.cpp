#include <imports.hpp>
#include <iostream>
#include <ostream>

std::ostream& operator<< (std::ostream& out, const row& r) {
	for (size_t i = 0; i < r.size(); i++) {
		auto x = r[i];
		if (i) out << '\t';
		if(x.is_bol()) {
			out << (x.as_bol() ? "true" : "false");
		} else if (x.is_i32()) {
			out << x.as_i32();
		} else if (x.is_i64()) {
			out << x.as_i64();
		} else if (x.is_f32()) {
			out << x.as_f32();
		} else if (x.is_f64()) {
			out << x.as_f64();
		} else if (x.is_txt()) {
			out << '"' << x.as_txt() << '"';
		} else {
			out <<"nil";
		}
	}
	return out;
}

std::ostream& operator<< (std::ostream& out, const table& t) {
	for (size_t i = 0; i < t.size(); i++) {
		auto x = t[i];
		if (i) out << '\n';

		if(x.is_bol()) {
			auto y = x.as_bol();
			for (size_t j = 0; j < y.size(); j++) {
				if (j) out << '\t';
				out << (y[j] ? "true" : "false");
			}
		} else if (x.is_i32()) {
			auto y = x.as_i32();
			for (size_t j = 0; j < y.size(); j++) {
				if (j) out << '\t';
				out << y[j];
			}
		} else if (x.is_i64()) {
			auto y = x.as_i64();
			for (size_t j = 0; j < y.size(); j++) {
				if (j) out << '\t';
				out << y[j];
			}
		} else if (x.is_f32()) {
			auto y = x.as_f32();
			for (size_t j = 0; j < y.size(); j++) {
				if (j) out << '\t';
				out << y[j];
			}
		} else if (x.is_f64()) {
			auto y = x.as_f64();
			for (size_t j = 0; j < y.size(); j++) {
				if (j) out << '\t';
				out << y[j];
			}
		} else if (x.is_txt()) {
			auto y = x.as_txt();
			for (size_t j = 0; j < y.size(); j++) {
				if (j) out << '\t';
				out << '"' << y[j] << '"';
			}
		} else {
			out <<"nil";
		}
	}
	return out;
}

int main() {
	{
		// 随机选择一些点
		std::cout << "async_choice_nodes()" << std::endl;

		auto a = std::get<future_table>(async_choice_nodes("player", 2)); // 异步调用，返回future
		auto b = std::get<table>(a.get()); // 同步调用，获得future指向任务的结果
		std::cout << b << std::endl;
	}

	{
		// 获取session状态
		std::cout << "async_query_kv()" << std::endl;
		
		std::string_view keys[] = {"a", "b", "c"};
		auto a = std::get<future_row>(async_query_kv(keys));
		auto b = std::get<row>(a.get());
		std::cout << b << std::endl;
	}
	
	{
		// 获取节点属性
		std::cout << "async_query_node()" << std::endl;

		std::string_view keys[] = {"age", "name"};
		auto a = std::get<future_row>(async_query_node("player102", "player", keys));
		auto b = std::get<row>(a.get());
		std::cout << b << std::endl;
	}

	{
		// 获取节点邻居及属性
		std::cout << "async_query_neighbors()" << std::endl;

		std::string_view keys[] = {"start_year"};
		auto a = std::get<future_table>(async_query_neighbors("player101", "serve", keys));
		auto b = std::get<table>(a.get());
		std::cout << b << std::endl;
	}

	{
		// 选中节点
		std::vector<std::string_view> nids = {"player101", "player102"}; //可以用vector
		select_nodes(nids);
	}

	{
		//选中边
		std::string_view src[] = {"player101", "player101"}; // 可以用原生数组
		std::string_view dst[] = {"player102", "player103"};
		select_edges(src, dst);
	}
	return 0;
}
