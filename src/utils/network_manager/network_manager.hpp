#ifndef NETWORK_MANAGER_HPP
#define NETWORK_MANAGER_HPP

#include "manager_singleton.hpp"

#include <curl/curl.h>
#include <mutex>
#include <string>
#include <vector>

namespace UTILS
{
enum class HttpMethod
{
	GET,
	POST,
	PUT,
	DELETE
};

struct NetworkRequest
{
	std::string										 url;
	HttpMethod										 method			 = HttpMethod::GET;
	std::vector<std::pair<std::string, std::string>> headers		 = {};
	std::string										 body			 = {};
	std::string										 user_agent		 = std::format("Mozilla/5.0 ({}; {}) {}/{}",
																				   COMMON::d_system_name,
																				   COMMON::d_system_version,
																				   COMMON::d_project_name,
																				   COMMON::d_project_version);
	std::string										 username		 = "";
	std::string										 password		 = "";
	long											 timeout_seconds = 30L;

	std::string download_file_path;
	std::string upload_file_path;
};

struct NetworkResponse
{
	long		http_code = 0;
	std::string body;
	std::string error;
};

struct CurlDeleter
{
	void operator()(CURL* curl)
	{
		if (curl)
		{
			curl_easy_cleanup(curl);
		}
	}
};

class NetworkManager : public UTILS::ManagerSingleton<NetworkManager>
{
	friend class ManagerSingleton<NetworkManager>;

private:
	NetworkManager() = default;

	void initialize() override;
	void cleanup();

public:
	std::string_view get_manager_name() const override;

	~NetworkManager();

	NetworkResponse make_request(const NetworkRequest& request);
	NetworkResponse make_request(HttpMethod												 method,
								 const std::string&										 url,
								 const std::vector<std::pair<std::string, std::string>>& headers	= {},
								 const std::string&										 post_data	= "",
								 const std::string&										 user_agent = "",
								 const std::string&										 username	= "",
								 const std::string&										 password	= "");
	bool			download_file(const std::string& url,
								  const std::string& output_path,
								  const std::string& user_agent = "",
								  const std::string& username	= "",
								  const std::string& password	= "");
	bool			upload_file(const std::string& url,
								const std::string& file_path,
								const std::string& user_agent = "",
								const std::string& username	  = "",
								const std::string& password	  = "");

private:
	static size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp);
	static size_t read_callback(void* ptr, size_t size, size_t nmemb, void* stream);

	void set_common_options(const NetworkRequest& request, char* error_buffer);

private:
	std::unique_ptr<CURL, CurlDeleter> m_curl;

protected:
	static std::mutex m_network_mutex;
};
} // namespace UTILS

#endif // NETWORK_MANAGER_HPP
