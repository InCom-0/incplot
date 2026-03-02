#pragma once

#include <filesystem>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>


namespace incom::terminal_plot {

class URISyntaxException: public std::invalid_argument {
public:
	explicit URISyntaxException(std::string_view message);
	URISyntaxException(std::string_view message, std::string_view uri);
};

// TODO: Move this class into incstd after some experience with it
class URI {
public:
	using QueryParameters = std::vector<std::pair<std::string, std::string>>;

	URI();
	explicit URI(std::string_view uri, bool heuristics = false);
	explicit URI(const char* uri, bool heuristics = false);
	URI(std::string_view scheme, std::string_view pathEtc);
	URI(std::string_view scheme, std::string_view authority, std::string_view pathEtc);
	URI(std::string_view scheme, std::string_view authority, std::string_view path, std::string_view query);
	URI(std::string_view scheme, std::string_view authority, std::string_view path, std::string_view query,
		std::string_view fragment);
	URI(const URI& uri);
	URI(URI&& uri) noexcept;
	URI(const URI& baseURI, std::string_view relativeURI);
	explicit URI(const std::filesystem::path& path);
	~URI();

	URI& operator=(const URI& uri);
	URI& operator=(URI&& uri) noexcept;
	URI& operator=(std::string_view uri);
	URI& operator=(const char* uri);

	void swap(URI& uri) noexcept;
	void clear();

	std::string toString() const;

	const std::string& getScheme() const;
	void setScheme(std::string_view scheme);

	const std::string& getUserInfo() const;
	void setUserInfo(std::string_view userInfo);

	const std::string& getHost() const;
	void setHost(std::string_view host);

	unsigned short getPort() const;
	void setPort(unsigned short port);
	unsigned short getSpecifiedPort() const;

	std::string getAuthority() const;
	void setAuthority(std::string_view authority);

	const std::string& getPath() const;
	void setPath(std::string_view path);

	std::string getQuery() const;
	void setQuery(std::string_view query);
	void addQueryParameter(std::string_view param, std::string_view val = "");
	const std::string& getRawQuery() const;
	void setRawQuery(std::string_view query);
	QueryParameters getQueryParameters(bool plusIsSpace = true) const;
	void setQueryParameters(const QueryParameters& params);

	std::string getFragment() const;
	void setFragment(std::string_view fragment);
	std::string getRawFragment() const;
	void setRawFragment(std::string_view fragment);

	void setPathEtc(std::string_view pathEtc);
	std::string getPathEtc() const;
	std::string getPathAndQuery() const;

	void resolve(std::string_view relativeURI);
	void resolve(const URI& relativeURI);

	bool isRelative() const;
	bool empty() const;

	bool operator==(const URI& uri) const;
	bool operator==(std::string_view uri) const;
	bool operator!=(const URI& uri) const;
	bool operator!=(std::string_view uri) const;

	void normalize();
	void getPathSegments(std::vector<std::string>& segments) const;

	static void encode(std::string_view str, std::string_view reserved, std::string& encodedStr);
	static void decode(std::string_view str, std::string& decodedStr, bool plusAsSpace = false);

protected:
	bool equals(const URI& uri) const;
	bool isWellKnownPort() const;
	unsigned short getWellKnownPort() const;

	void parse(std::string_view uri);
	void parse(std::string_view uri, bool heuristics);
	void parseAuthority(std::string_view authority);
	void parseHostAndPort(std::string_view hostAndPort);
	void parsePath(std::string_view path);
	void parsePathEtc(std::string_view pathEtc);
	void parseQuery(std::string_view query);
	void parseFragment(std::string_view fragment);
	void mergePath(std::string_view path);
	void removeDotSegments(bool removeLeading = true);
	static void getPathSegments(std::string_view path, std::vector<std::string>& segments);
	void buildPath(const std::vector<std::string>& segments, bool leadingSlash, bool trailingSlash);

	static const std::string RESERVED_PATH;
	static const std::string RESERVED_QUERY;
	static const std::string RESERVED_QUERY_PARAM;
	static const std::string RESERVED_FRAGMENT;
	static const std::string ILLEGAL;

private:
	std::string _scheme;
	std::string _userInfo;
	std::string _host;
	unsigned short _port;
	std::string _path;
	std::string _query;
	std::string _fragment;
};

inline const std::string& URI::getScheme() const {
	return _scheme;
}

inline const std::string& URI::getUserInfo() const {
	return _userInfo;
}

inline const std::string& URI::getHost() const {
	return _host;
}

inline const std::string& URI::getPath() const {
	return _path;
}

inline const std::string& URI::getRawQuery() const {
	return _query;
}

inline std::string URI::getRawFragment() const {
	return _fragment;
}

inline unsigned short URI::getSpecifiedPort() const {
	return _port;
}

inline void swap(URI& u1, URI& u2) noexcept {
	u1.swap(u2);
}

std::pair<bool, std::string> parse_uri_string(std::string_view uri);

} // namespace incom::terminal_plot