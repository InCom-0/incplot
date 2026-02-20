#pragma once

#include <cctype>
#include <string>
#include <string_view>
#include <utility>


namespace incom::terminal_plot {

namespace detail {

inline char to_lower_ascii(char c) {
	return static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
}

inline std::string to_lower_ascii_copy(std::string_view sv) {
	std::string out;
	out.reserve(sv.size());
	for (char c : sv) { out.push_back(to_lower_ascii(c)); }
	return out;
}

inline bool is_hex_digit(char c) {
	return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f');
}

inline bool validate_percent_encoding(std::string_view sv) {
	for (std::size_t i = 0; i < sv.size(); ++i) {
		if (sv[i] != '%') { continue; }
		if (i + 2 >= sv.size()) { return false; }
		if (!is_hex_digit(sv[i + 1]) || !is_hex_digit(sv[i + 2])) { return false; }
		i += 2;
	}
	return true;
}

inline bool parse_port(std::string_view sv, unsigned short& out_port) {
	if (sv.empty()) {
		out_port = 0;
		return true;
	}

	std::size_t i = 0;
	bool negative = false;
	if (sv[i] == '+' || sv[i] == '-') {
		negative = (sv[i] == '-');
		++i;
	}
	if (i >= sv.size()) { return false; }

	int value = 0;
	for (; i < sv.size(); ++i) {
		const char c = sv[i];
		if (c < '0' || c > '9') { return false; }
		value = (value * 10) + static_cast<int>(c - '0');
		if (value > 65535) { break; }
	}

	if (i != sv.size()) { return false; }
	if (negative) { value = -value; }

	if (value > 0 && value < 65536) {
		out_port = static_cast<unsigned short>(value);
		return true;
	}
	return false;
}

inline bool parse_host_and_port(std::string_view part, unsigned short& out_port) {
	if (part.empty()) { return true; }

	std::size_t i = 0;
	if (part[i] == '[') {
		++i;
		while (i < part.size() && part[i] != ']') { ++i; }
		if (i == part.size()) { return false; }
		++i;
	} else {
		while (i < part.size() && part[i] != ':') { ++i; }
	}

	if (i < part.size() && part[i] == ':') {
		++i;
		return parse_port(part.substr(i), out_port);
	}

	out_port = 0;
	return true;
}

inline bool parse_authority(std::string_view authority, unsigned short& out_port) {
	std::string_view part = authority;
	std::size_t user_info_pos = std::string_view::npos;
	for (std::size_t i = 0; i < authority.size(); ++i) {
		if (authority[i] == '@') { user_info_pos = i; }
	}
	if (user_info_pos != std::string_view::npos) {
		part = authority.substr(user_info_pos + 1);
	}
	return parse_host_and_port(part, out_port);
}

inline bool parse_path_etc(std::string_view path_etc) {
	if (path_etc.empty()) { return true; }

	std::size_t i = 0;
	if (path_etc[i] != '?' && path_etc[i] != '#') {
		const std::size_t path_end = path_etc.find_first_of("?#", i);
		const std::string_view path =
			(path_end == std::string_view::npos) ? path_etc.substr(i) : path_etc.substr(i, path_end - i);
		if (!validate_percent_encoding(path)) { return false; }
		if (path_end == std::string_view::npos) { return true; }
		i = path_end;
	}

	if (i < path_etc.size() && path_etc[i] == '?') {
		++i;
		while (i < path_etc.size() && path_etc[i] != '#') { ++i; }
	}

	return true;
}

} // namespace detail

inline std::pair<bool, std::string> parse_uri_string(std::string_view uri) {
	if (uri.empty()) { return {true, {}}; }

	std::string scheme;
	unsigned short port = 0;

	std::size_t i = 0;
	const char first = uri[0];
	if (first != '/' && first != '.' && first != '?' && first != '#') {
		while (i < uri.size() && uri[i] != ':' && uri[i] != '?' && uri[i] != '#' && uri[i] != '/') {
			++i;
		}

		if (i < uri.size() && uri[i] == ':') {
			scheme = detail::to_lower_ascii_copy(uri.substr(0, i));
			++i;
			if (i == uri.size()) { return {false, {}}; }

			if (uri[i] == '/') {
				++i;
				if (i < uri.size() && uri[i] == '/') {
					++i;
					const std::size_t auth_end = uri.find_first_of("/?#", i);
					const std::string_view authority =
						(auth_end == std::string_view::npos) ? uri.substr(i) : uri.substr(i, auth_end - i);
					if (!detail::parse_authority(authority, port)) { return {false, {}}; }
					if (auth_end == std::string_view::npos) { return {true, std::move(scheme)}; }
					i = auth_end;
				} else {
					--i;
				}
			}

			if (!detail::parse_path_etc(uri.substr(i))) { return {false, {}}; }
			return {true, std::move(scheme)};
		}

		if (!detail::parse_path_etc(uri)) { return {false, {}}; }
		return {true, {}};
	}

	if (!detail::parse_path_etc(uri)) { return {false, {}}; }
	return {true, {}};
}

} // namespace incom::terminal_plot