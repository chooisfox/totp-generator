#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include "notification_manager.hpp"
#include "option_manager.hpp"
#include "settings_manager.hpp"
#include "totp_manager.hpp"

#include <memory>

namespace APP
{
class Application
{
public:
	Application(const int argc, const char** argv);
	~Application();

	Application(const Application&)			   = delete;
	Application& operator=(const Application&) = delete;

	int run();

private:
	bool initialize_managers(const int argc, const char** argv);
	bool initialize_app();
	void cleanup();

	void handle_set_secret();
	void run_watch_mode();
	void generate_and_print_once();

private:
	std::shared_ptr<UTILS::NotificationManager> m_notification_manager;
	std::shared_ptr<UTILS::SettingsManager>		m_settings_manager;
	std::shared_ptr<UTILS::OptionManager>		m_option_manager;
	std::shared_ptr<UTILS::TOTPManager>			m_totp_manager;
};
} // namespace APP

#endif // APPLICATION_HPP
