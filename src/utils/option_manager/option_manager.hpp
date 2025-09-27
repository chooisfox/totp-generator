#ifndef OPTION_MANAGER_HPP
#define OPTION_MANAGER_HPP

#include "manager_singleton.hpp"

#include <cxxopts.hpp>
#include <memory>
#include <string>
#include <type_traits>

namespace UTILS
{
class OptionManager : public UTILS::ManagerSingleton<OptionManager>
{
	friend class ManagerSingleton<OptionManager>;

private:
	OptionManager() = default;

	void initialize() override;

public:
	std::string_view get_manager_name() const override;

	~OptionManager();

	void   set_description(const std::string& app_name, const std::string& app_description);
	void   parse_options(const int argc, const char** argv);
	bool   has_option(const std::string& name) const;
	void   add_option(const std::string& name, const std::string& description);
	size_t get_option_count(const std::string& name) const;

	void log_help() const;
	void debug_log() const;

public:
	template<typename T>
	void add_option(const std::string& name, const std::string& description, const T& default_value);

	template<typename T>
	void add_option(const std::string& name, const std::string& description);

	template<typename T = std::string>
	T get_option(const std::string& name) const;

private:
	std::unique_ptr<cxxopts::Options>	  m_options;
	std::unique_ptr<cxxopts::ParseResult> m_parsed_options;

protected:
	mutable std::mutex m_options_mutex;
};

template<typename T>
void OptionManager::add_option(const std::string& name, const std::string& description, const T& default_value)
{
	static_assert(!std::is_same_v<T, void>, "Default value required for non-void options.");

	std::lock_guard<std::mutex> lock(this->m_options_mutex);

	if (!this->m_options)
	{
		SPD_ERROR_CLASS(COMMON::d_settings_group_options, "Unable to add option, option manager is not initialized");
		return;
	}

	this->m_options->add_options()(name, description, cxxopts::value<T>()->default_value(default_value));

	return;
}

template<typename T>
void OptionManager::add_option(const std::string& name, const std::string& description)
{
	std::lock_guard<std::mutex> lock(this->m_options_mutex);

	if (!this->m_options)
	{
		SPD_ERROR_CLASS(COMMON::d_settings_group_options, "Unable to add option, option manager is not initialized");
		return;
	}

	this->m_options->add_options()(name, description, cxxopts::value<T>());

	return;
}

template<typename T>
T OptionManager::get_option(const std::string& name) const
{
	std::lock_guard<std::mutex> lock(this->m_options_mutex);

	if (!this->m_parsed_options)
	{
		SPD_ERROR_CLASS(COMMON::d_settings_group_options, "Options have not been parsed yet. Call parse_options() first.");
		return T {};
	}

	if (!this->has_option(name))
	{
		SPD_WARN_CLASS(COMMON::d_settings_group_options, fmt::format("Option {} not found. Returning default value.", name));
		return T {};
	}

	try
	{
		return this->m_parsed_options->operator[](name).as<T>();
	}
	catch (const cxxopts::exceptions::exception& e)
	{
		SPD_ERROR_CLASS(COMMON::d_settings_group_options, fmt::format("Error getting option {}: {}", name, e.what()));
	}
	catch (const std::exception& e)
	{
		SPD_ERROR_CLASS(COMMON::d_settings_group_options, fmt::format("Standard exception while getting option '{}': {}", name, e.what()));
	}
	catch (...)
	{
		SPD_ERROR_CLASS(COMMON::d_settings_group_options, fmt::format("Unknown error occurred while getting option '{}'", name));
	}
	return T {};
}
} // namespace UTILS

#endif // OPTION_MANAGER_HPP
