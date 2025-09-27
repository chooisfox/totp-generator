#ifndef TOTP_MANAGER_HPP
#define TOTP_MANAGER_HPP

#include "manager_singleton.hpp"
#include "settings_manager.hpp"

#include <mutex>
#include <string>

namespace UTILS
{

class TOTPManager : public UTILS::ManagerSingleton<TOTPManager>
{
	friend class ManagerSingleton<TOTPManager>;

private:
	TOTPManager() = default;

	void initialize() override;

public:
	std::string_view get_manager_name() const override;
	~TOTPManager();

	std::string generate_totp();

	bool set_account(const std::string& account_name, const std::string& secret);
	void clear_account();

	std::string get_account_name() const;

private:
	void load_account();
	void save_account();

private:
	std::string m_account_name;
	std::string m_account_secret;

	std::shared_ptr<UTILS::SettingsManager> m_settings_manager;

protected:
	mutable std::mutex m_totp_mutex;
};

} // namespace UTILS

#endif // TOTP_MANAGER_HPP
