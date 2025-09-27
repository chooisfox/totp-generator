#include "settings_manager.hpp"

#include "spdlog_wrapper.hpp"

#include <filesystem>
#include <memory>
#include <toml++/impl/table.hpp>

namespace
{
std::string getenv_safe(const char *name)
{
	const char *value = std::getenv(name);
	return value ? value : "";
}

const std::string DEFAULT_CONFIG_FILE_NAME = std::string(COMMON::d_project_name) + ".toml";

const std::vector<fs::path> DEFAULT_CONFIG_PATHS = [] {
	std::vector<fs::path> paths;

#if defined(_WIN32) || defined(WIN32)
	std::string appdata = getenv_safe("APPDATA");
	if (!appdata.empty())
	{
		paths.push_back(appdata);
	}
#elif defined(__APPLE__) && defined(__MACH__)
	std::string home = getenv_safe("HOME");
	if (!home.empty())
	{
		paths.push_back(fs::path(home) / "Library" / "Application Support");
	}
#elif defined(__unix__) || defined(__linux__)
	std::string xdg_config = getenv_safe("XDG_CONFIG_HOME");
	if (!xdg_config.empty())
	{
		paths.push_back(xdg_config);
	}

	std::string home = getenv_safe("HOME");
	if (!home.empty())
	{
		paths.push_back(fs::path(home) / ".config");
	}
#endif
	paths.push_back(fs::current_path());
	return paths;
}();

static constexpr std::string_view default_toml_format = R"(
    [application]
    name = "{project_name}"
    authors = ["{developer_name} <{developer_email}>"]
    [totp]
    secret = ""
    account_name = ""
    [notifications]
    enabled = false
    uri = ""
    username = ""
    password = ""
)";

std::string default_toml = fmt::format(default_toml_format,
									   fmt::arg("project_name", COMMON::d_project_name),
									   fmt::arg("developer_name", COMMON::d_developer_name),
									   fmt::arg("developer_email", COMMON::d_developer_email));
} // anonymous namespace

namespace UTILS
{

std::string_view SettingsManager::get_manager_name() const
{
	return "Settings Manager";
}

void SettingsManager::initialize()
{
	try
	{
		m_config_default = std::make_unique<toml::table>(toml::parse(default_toml));
	}
	catch (const toml::parse_error &err)
	{
		SPD_ERROR_CLASS(COMMON::d_settings_group_utils, fmt::format("Failed to parse default TOML string: {}", err.what()));
		return;
	}

	if (!this->load_settings())
	{
		this->create_default_settings();
	}
}

void SettingsManager::create_default_settings()
{
	if (!this->restore_defaults())
	{
		SPD_ERROR_CLASS(COMMON::d_settings_group_utils, "Unable to load default settings.");
		return;
	}

	if (!this->save_settings())
	{
		SPD_ERROR_CLASS(COMMON::d_settings_group_utils, "Unable to create default settings file.");
		return;
	}
}

std::vector<std::string_view> SettingsManager::split_path(std::string_view path)
{
	std::vector<std::string_view> keys;
	size_t						  start = 0;
	size_t						  end	= path.find('.');

	while (end != std::string_view::npos)
	{
		keys.push_back(path.substr(start, end - start));
		start = end + 1;
		end	  = path.find('.', start);
	}

	keys.push_back(path.substr(start));

	return keys;
}

bool SettingsManager::load_settings()
{
	SPD_DEBUG_CLASS(COMMON::d_settings_group_utils, "Loading settings.");

	for (auto &path : DEFAULT_CONFIG_PATHS)
	{
		SPD_DEBUG_CLASS(COMMON::d_settings_group_utils, fmt::format("Trying folder: {}.", path.string()));

		if (!fs::exists(path) || !fs::is_directory(path) || path.empty())
		{
			continue;
		}

		auto config_file = path / DEFAULT_CONFIG_FILE_NAME;

		if (this->load_settings(config_file))
		{
			SPD_INFO_CLASS(COMMON::d_settings_group_utils, fmt::format("Settings successfully loaded from: {}", this->m_config_path.string()));
			return true;
		}

		config_file = path / COMMON::d_project_name / DEFAULT_CONFIG_FILE_NAME;

		if (this->load_settings(config_file))
		{
			SPD_INFO_CLASS(COMMON::d_settings_group_utils, fmt::format("Settings successfully loaded from: {}", this->m_config_path.string()));
			return true;
		}
	}

	SPD_WARN_CLASS(COMMON::d_settings_group_utils, "Unable to load settings file");

	return false;
}

bool SettingsManager::load_settings(fs::path file_path)
{
	std::lock_guard<std::mutex> lock(m_settings_mutex);

	if (!fs::exists(file_path) || fs::is_directory(file_path) || file_path.empty())
	{
		return false;
	}

	try
	{
		this->m_config = std::make_unique<toml::table>(toml::parse_file(file_path.string()));
	}
	catch (const std::exception &error)
	{
		SPD_ERROR_CLASS(COMMON::d_settings_group_utils, fmt::format("Failed to parse settings file at {}: {}", file_path.string(), error.what()));
		return false;
	}

	this->m_config_path = file_path;

	return true;
}

bool SettingsManager::save_settings()
{
	SPD_DEBUG_CLASS(COMMON::d_settings_group_utils, "Saving settings.");

	if (!this->m_config)
	{
		this->create_default_settings();
	}

	if (!this->m_config_path.empty() && !fs::is_directory(this->m_config_path))
	{
		if (this->save_settings(this->m_config_path))
		{
			SPD_INFO_CLASS(COMMON::d_settings_group_utils, fmt::format("Settings successfully saved: {}", this->m_config_path.string()));
			return true;
		}
	}

	for (auto &path : DEFAULT_CONFIG_PATHS)
	{
		SPD_DEBUG_CLASS(COMMON::d_settings_group_utils, fmt::format("Trying folder: {}.", path.string()));
		if (!fs::exists(path) || !fs::is_directory(path) || path.empty())
		{
			continue;
		}

		auto config_file = path / COMMON::d_project_name / DEFAULT_CONFIG_FILE_NAME;

		if (this->save_settings(config_file))
		{
			this->m_config_path = path / COMMON::d_project_name / DEFAULT_CONFIG_FILE_NAME;
			SPD_INFO_CLASS(COMMON::d_settings_group_utils, fmt::format("Settings successfully saved: {}", this->m_config_path.string()));
			return true;
		}
	}

	SPD_ERROR_CLASS(COMMON::d_settings_group_utils, "Unable to save settings file");

	return false;
}

bool SettingsManager::save_settings(fs::path file_path)
{
	std::lock_guard<std::mutex> lock(m_settings_mutex);

	if (fs::is_directory(file_path) || file_path.empty())
	{
		return false;
	}

	if (!this->m_config)
	{
		if (!restore_defaults())
		{
			SPD_ERROR_CLASS(COMMON::d_settings_group_utils, "Cannot save settings without a default configuration.");
			return false;
		}
	}

	try
	{
		if (!fs::exists(file_path.parent_path()))
		{
			if (!fs::create_directories(file_path.parent_path()))
			{
				SPD_ERROR_CLASS(COMMON::d_settings_group_utils,
								fmt::format("Failed to create config directory: {}", file_path.parent_path().string()));
				return false;
			}
		}

		std::ofstream file(file_path, std::ios::out | std::ios::trunc);

		if (!file.is_open())
		{
			SPD_ERROR_CLASS(COMMON::d_settings_group_utils, fmt::format("Failed to create config file: {}", file_path.string()));
			return false;
		}

		file << *this->m_config.get();

		file.close();
	}
	catch (const fs::filesystem_error &e)
	{
		SPD_ERROR_CLASS(COMMON::d_settings_group_utils,
						fmt::format("Filesystem error while saving settings to {}: {}", file_path.string(), e.what()));
		return false;
	}

	return true;
}

bool SettingsManager::restore_defaults()
{
	std::lock_guard<std::mutex> lock(m_settings_mutex);

	if (!this->m_config_default)
	{
		return false;
	}

	this->m_config = std::make_unique<toml::table>(*this->m_config_default.get());

	return true;
}

std::string SettingsManager::dump() const
{
	if (!m_config)
	{
		return "# No configuration is currently loaded.\n";
	}

	std::ostringstream ss;
	ss << (*m_config);
	return ss.str();
}

void SettingsManager::dump(std::ostream &output) const
{
	if (!m_config)
	{
		output << "# No configuration is currently loaded.\n";
		return;
	}

	output << (*m_config);
}

SettingsManager::~SettingsManager()
{
	return;
}
} // namespace UTILS
