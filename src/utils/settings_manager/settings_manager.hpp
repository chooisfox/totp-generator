#ifndef SETTINGS_MANAGER_HPP
#define SETTINGS_MANAGER_HPP

#include "manager_singleton.hpp"

#include <cstdlib>
#include <filesystem>
#include <memory>
#include <ostream>
#include <string>
#include <toml++/toml.hpp>

namespace fs = std::filesystem;

namespace UTILS
{
class SettingsManager : public UTILS::ManagerSingleton<SettingsManager>
{
	friend class ManagerSingleton<SettingsManager>;

private:
	SettingsManager() = default;

	void initialize() override;

	void create_default_settings();

	static std::vector<std::string_view> split_path(std::string_view path);

public:
	std::string_view get_manager_name() const override;

	~SettingsManager();

	bool load_settings();
	bool load_settings(fs::path file_path);
	bool save_settings();
	bool save_settings(fs::path file_path);
	bool restore_defaults();

	template<typename T>
	T get_setting(std::string_view path, T default_value) const;

	template<typename T>
	bool set_setting(std::string_view path, T value);

	std::string dump() const;
	void		dump(std::ostream& output) const;

private:
	fs::path					 m_config_path;
	std::unique_ptr<toml::table> m_config;
	std::unique_ptr<toml::table> m_config_default;

protected:
	mutable std::mutex m_settings_mutex;
};

template<typename T>
T SettingsManager::get_setting(std::string_view path, T default_value) const
{
	std::lock_guard<std::mutex> lock(m_settings_mutex);

	if (!this->m_config)
	{
		return default_value;
	}

	const toml::node* current_node = this->m_config.get();
	for (const auto& key : this->split_path(path))
	{
		if (!current_node || !current_node->is_table())
		{
			return default_value;
		}
		current_node = current_node->as_table()->get(key);
	}

	if (!current_node)
	{
		return default_value;
	}

	return current_node->value_or(default_value);
}

template<typename T>
bool SettingsManager::set_setting(std::string_view path, T value)
{
	std::lock_guard<std::mutex> lock(m_settings_mutex);

	if (!this->m_config)
	{
		SPD_ERROR_CLASS(COMMON::d_settings_group_utils, "Settings are not loaded, cannot perform set_settings.");
		return false;
	}

	std::vector<std::string_view> keys = this->split_path(path);

	if (keys.empty())
	{
		SPD_WARN_CLASS(COMMON::d_settings_group_utils, "Invalid key path provided.");
		return false;
	}

	toml::table* current_table = this->m_config.get();

	for (size_t i = 0; i < keys.size() - 1; ++i)
	{
		const std::string_view key		 = keys[i];
		toml::node*			   next_node = current_table->get(key);

		if (next_node)
		{
			if (toml::table* next_table = next_node->as_table())
			{
				current_table = next_table;
			}
			else
			{
				SPD_ERROR_CLASS(COMMON::d_settings_group_utils,
								fmt::format("Cannot create setting. Path conflict at key '{}' which is not a table.", key));
				return false;
			}
		}
		else
		{
			current_table = current_table->emplace<toml::table>(key).first->second.as_table();
		}
	}

	current_table->insert_or_assign(keys.back(), value);

	return true;
}

} // namespace UTILS

#endif // SETTINGS_MANAGER_HPP
