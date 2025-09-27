#include "network_manager.hpp"

#include <fstream>
#include <memory>

namespace UTILS
{
std::mutex NetworkManager::m_network_mutex;

NetworkManager::~NetworkManager()
{
	cleanup();
}

void NetworkManager::initialize()
{
	curl_global_init(CURL_GLOBAL_ALL);

	this->m_curl.reset(curl_easy_init());

	if (!m_curl)
	{
		SPD_ERROR_CLASS(COMMON::d_settings_group_utils, "Failed to initialize CURL handle.");
	}
}

void NetworkManager::cleanup()
{
	curl_global_cleanup();
}

std::string_view NetworkManager::get_manager_name() const
{
	return "Network Manager";
}

NetworkResponse NetworkManager::make_request(const NetworkRequest& request)
{
	std::lock_guard<std::mutex> lock(m_network_mutex);
	NetworkResponse				response;

	if (!this->m_curl)
	{
		response.error = "CURL handle is not initialized.";
		SPD_ERROR_CLASS(COMMON::d_settings_group_utils, response.error);
		return response;
	}

	curl_easy_reset(this->m_curl.get());

	char error_buffer[CURL_ERROR_SIZE] = {0};
	set_common_options(request, error_buffer);

	switch (request.method)
	{
		case HttpMethod::POST: {
			curl_easy_setopt(this->m_curl.get(), CURLOPT_POST, 1L);
			curl_easy_setopt(this->m_curl.get(), CURLOPT_POSTFIELDS, request.body.c_str());
			break;
		}
		case HttpMethod::PUT: {
			curl_easy_setopt(this->m_curl.get(), CURLOPT_CUSTOMREQUEST, "PUT");

			if (!request.upload_file_path.empty())
			{
				auto input_file = std::make_unique<std::ifstream>(request.upload_file_path, std::ios::binary);

				if (!input_file->is_open())
				{
					response.error = fmt::format("Failed to open file for upload: {}", request.upload_file_path);
					SPD_ERROR_CLASS(COMMON::d_settings_group_utils, response.error);
					return response;
				}

				input_file->seekg(0, std::ios::end);
				long long file_size = input_file->tellg();
				input_file->seekg(0, std::ios::beg);

				curl_easy_setopt(this->m_curl.get(), CURLOPT_UPLOAD, 1L);
				curl_easy_setopt(this->m_curl.get(), CURLOPT_READFUNCTION, read_callback);
				curl_easy_setopt(this->m_curl.get(), CURLOPT_READDATA, input_file.get());
				curl_easy_setopt(this->m_curl.get(), CURLOPT_INFILESIZE_LARGE, file_size);
			}
			else
			{
				curl_easy_setopt(this->m_curl.get(), CURLOPT_POSTFIELDS, request.body.c_str());
			}
			break;
		}
		case HttpMethod::DELETE: {
			curl_easy_setopt(this->m_curl.get(), CURLOPT_CUSTOMREQUEST, "DELETE");
			break;
		}
		case HttpMethod::GET:
		default:
			break;
	}

	struct curl_slist* header_list = nullptr;

	for (const auto& header : request.headers)
	{
		header_list = curl_slist_append(header_list, fmt::format("{}: {}", header.first, header.second).c_str());
	}

	if (header_list)
	{
		curl_easy_setopt(this->m_curl.get(), CURLOPT_HTTPHEADER, header_list);
	}

	std::unique_ptr<std::ofstream> output_file;

	if (!request.download_file_path.empty())
	{
		output_file = std::make_unique<std::ofstream>(request.download_file_path, std::ios::binary);

		if (!output_file->is_open())
		{
			response.error = fmt::format("Failed to open file for writing: {}", request.download_file_path);

			SPD_ERROR_CLASS(COMMON::d_settings_group_utils, response.error);

			curl_slist_free_all(header_list);

			return response;
		}
		curl_easy_setopt(this->m_curl.get(), CURLOPT_WRITEDATA, output_file.get());
	}
	else
	{
		curl_easy_setopt(this->m_curl.get(), CURLOPT_WRITEDATA, &response.body);
	}

	CURLcode res = curl_easy_perform(this->m_curl.get());

	if (res != CURLE_OK)
	{
		response.error = fmt::format("curl_easy_perform() failed: {}", std::string(error_buffer));
		SPD_ERROR_CLASS(COMMON::d_settings_group_utils, response.error);
	}

	curl_easy_getinfo(this->m_curl.get(), CURLINFO_RESPONSE_CODE, &response.http_code);

	if (header_list)
	{
		curl_slist_free_all(header_list);
	}

	SPD_DEBUG_CLASS(COMMON::d_settings_group_utils, fmt::format("Request to {} completed with HTTP code {}", request.url, response.http_code));

	return response;
}

NetworkResponse NetworkManager::make_request(HttpMethod												 method,
											 const std::string&										 url,
											 const std::vector<std::pair<std::string, std::string>>& headers,
											 const std::string&										 post_data,
											 const std::string&										 user_agent,
											 const std::string&										 username,
											 const std::string&										 password)
{
	NetworkRequest request;
	request.method	   = method;
	request.url		   = url;
	request.headers	   = headers;
	request.body	   = post_data;
	request.user_agent = user_agent;
	request.username   = username;
	request.password   = password;
	return make_request(request);
}

bool NetworkManager::download_file(const std::string& url,
								   const std::string& output_path,
								   const std::string& user_agent,
								   const std::string& username,
								   const std::string& password)
{
	NetworkRequest request;
	request.url				   = url;
	request.download_file_path = output_path;
	request.user_agent		   = user_agent;
	request.username		   = username;
	request.password		   = password;

	NetworkResponse response = make_request(request);
	return response.error.empty() && response.http_code >= 200 && response.http_code < 300;
}

bool NetworkManager::upload_file(const std::string& url,
								 const std::string& file_path,
								 const std::string& user_agent,
								 const std::string& username,
								 const std::string& password)
{
	NetworkRequest request;
	request.method			 = HttpMethod::PUT;
	request.url				 = url;
	request.upload_file_path = file_path;
	request.user_agent		 = user_agent;
	request.username		 = username;
	request.password		 = password;

	NetworkResponse response = make_request(request);
	return response.error.empty() && response.http_code >= 200 && response.http_code < 300;
}

void NetworkManager::set_common_options(const NetworkRequest& request, char* error_buffer)
{
	curl_easy_setopt(this->m_curl.get(), CURLOPT_URL, request.url.c_str());
	curl_easy_setopt(this->m_curl.get(), CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(this->m_curl.get(), CURLOPT_TIMEOUT, request.timeout_seconds);
	curl_easy_setopt(this->m_curl.get(), CURLOPT_WRITEFUNCTION, write_callback);
	curl_easy_setopt(this->m_curl.get(), CURLOPT_ERRORBUFFER, error_buffer);

	if (!request.user_agent.empty())
	{
		curl_easy_setopt(this->m_curl.get(), CURLOPT_USERAGENT, request.user_agent.c_str());
	}
	if (!request.username.empty() || !request.password.empty())
	{
		std::string userpwd = fmt::format("{}:{}", request.username, request.password);
		curl_easy_setopt(this->m_curl.get(), CURLOPT_USERPWD, userpwd.c_str());
	}
}

size_t NetworkManager::write_callback(void* contents, size_t size, size_t nmemb, void* userp)
{
	size_t real_size = size * nmemb;
	if (auto* str = static_cast<std::string*>(userp))
	{
		try
		{
			str->append(static_cast<char*>(contents), real_size);
		}
		catch (const std::bad_alloc& e)
		{
			return 0;
		}
	}
	else if (auto* stream = static_cast<std::ostream*>(userp))
	{
		stream->write(static_cast<char*>(contents), real_size);
	}
	return real_size;
}

size_t NetworkManager::read_callback(void* ptr, size_t size, size_t nmemb, void* stream)
{
	auto* read_stream = static_cast<std::istream*>(stream);
	read_stream->read(static_cast<char*>(ptr), size * nmemb);
	return read_stream->gcount();
}

} // namespace UTILS
