#include "totp_manager.hpp"

#include "settings_manager.hpp"
#include "spdlog_wrapper.hpp"

#include <cctype>
#include <libcppotp/bytes.h>
#include <libcppotp/otp.h>
#include <string>

namespace
{
std::string normalizedBase32String(const std::string& unnorm)
{
	std::string ret;
	for (char c : unnorm)
	{
		if (c >= 'a' && c <= 'z')
		{
			ret.push_back(std::toupper(c));
		}
		else if ((c >= 'A' && c <= 'Z') || (c >= '2' && c <= '7'))
		{
			ret.push_back(c);
		}
	}
	return ret;
}
} // namespace

namespace UTILS
{

std::string_view TOTPManager::get_manager_name() const
{
	return "TOTP Manager";
}

void TOTPManager::initialize()
{
	m_settings_manager = UTILS::SettingsManager::instance();
	load_account();
}

TOTPManager::~TOTPManager()
{}

void TOTPManager::load_account()
{
	m_account_name	 = m_settings_manager->get_setting<std::string>("totp.account_name", "");
	m_account_secret = m_settings_manager->get_setting<std::string>("totp.secret", "");
}

void TOTPManager::save_account()
{
	std::lock_guard<std::mutex> lock(m_totp_mutex);

	if (!m_account_name.empty() && !m_account_secret.empty())
	{
		m_settings_manager->set_setting("totp.account_name", m_account_name);
		m_settings_manager->set_setting("totp.secret", m_account_secret);

		m_settings_manager->save_settings();
	}
}

std::string TOTPManager::generate_totp()
{
	std::string current_name;
	std::string current_secret;

	if (m_account_secret.empty())
	{
		SPD_WARN_CLASS(COMMON::d_settings_group_utils, "No TOTP secret configured.");
		return "";
	}
	current_name   = m_account_name;
	current_secret = m_account_secret;

	std::string normalizedKey = normalizedBase32String(current_secret);
	try
	{
		CppTotp::Bytes::ByteString secret_bytes = CppTotp::Bytes::fromUnpaddedBase32(normalizedKey);
		uint32_t				   code_val		= CppTotp::totp(secret_bytes, time(NULL), 0, 30, 6);

		std::string code_str = std::to_string(code_val);
		if (code_str.length() < 6)
		{
			code_str.insert(0, 6 - code_str.length(), '0');
		}
		return code_str;
	}
	catch (const std::exception& e)
	{
		SPD_ERROR_CLASS(COMMON::d_settings_group_utils, fmt::format("Failed to generate TOTP for account '{}': {}", current_name, e.what()));
		return "";
	}
}

bool TOTPManager::set_account(const std::string& account_name, const std::string& secret)
{
	if (account_name.empty() || secret.empty())
	{
		SPD_WARN_CLASS(COMMON::d_settings_group_utils, "Account name and secret cannot be empty.");
		return false;
	}

	m_account_name	 = account_name;
	m_account_secret = secret;

	save_account();
	SPD_INFO_CLASS(COMMON::d_settings_group_utils, fmt::format("TOTP account set to '{}'.", account_name));
	return true;
}

void TOTPManager::clear_account()
{
	{
		std::lock_guard<std::mutex> lock(m_totp_mutex);
		m_account_name.clear();
		m_account_secret.clear();
	}
	save_account();
}

std::string TOTPManager::get_account_name() const
{
	return m_account_name;
}

} // namespace UTILS
