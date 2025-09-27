#include "application.hpp"

#include "spdlog_wrapper.hpp"

#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <thread>

#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
class RawTerminal
{
private:
	termios original_settings;

public:
	RawTerminal()
	{
		tcgetattr(STDIN_FILENO, &original_settings);
		termios new_settings = original_settings;
		new_settings.c_lflag &= ~(ICANON | ECHO);
		new_settings.c_cc[VMIN]	 = 0;
		new_settings.c_cc[VTIME] = 0;
		tcsetattr(STDIN_FILENO, TCSANOW, &new_settings);
	}

	~RawTerminal()
	{
		tcsetattr(STDIN_FILENO, TCSANOW, &original_settings);
	}
};
#endif

namespace APP
{
Application::Application(const int argc, const char** argv)
{
	this->initialize_managers(argc, argv);
	this->initialize_app();
	SPD_INFO_CLASS(COMMON::d_settings_group_application, "Application initialized");
}

Application::~Application()
{
	this->cleanup();
}

int Application::run()
{
	if (m_option_manager->has_option("h") || m_option_manager->has_option("d"))
	{
		return 0;
	}

	if (m_option_manager->has_option("s"))
	{
		handle_set_secret();
	}

	if (m_option_manager->has_option("w"))
	{
		run_watch_mode();
	}
	else
	{
		generate_and_print_once();
	}

	return 0;
}

bool Application::initialize_managers(const int argc, const char** argv)
{
	this->m_option_manager		 = UTILS::OptionManager::instance();
	this->m_settings_manager	 = UTILS::SettingsManager::instance();
	this->m_notification_manager = UTILS::NotificationManager::instance();
	this->m_totp_manager		 = UTILS::TOTPManager::instance();

	this->m_option_manager->add_option("h,help", "Prints this help menu.");
	this->m_option_manager->add_option("d,debug", "Prints debug info.");
	this->m_option_manager->add_option<std::string>("a,account", "Account name (used with -s).", "");
	this->m_option_manager->add_option<std::string>("s,secret", "Set a new TOTP secret for an account and prints the code.", "");
	this->m_option_manager->add_option("w,watch", "Watch and continuously update the TOTP code.");

	this->m_option_manager->parse_options(argc, argv);

	if (this->m_option_manager->has_option("h"))
	{
		this->m_option_manager->log_help();
	}

	if (this->m_option_manager->has_option("d"))
	{
		this->m_option_manager->debug_log();
	}

	auto			   timestamp = std::chrono::system_clock::now();
	std::time_t		   now_tt	 = std::chrono::system_clock::to_time_t(timestamp);
	std::tm			   tm		 = *std::localtime(&now_tt);
	std::ostringstream oss;
	oss << std::put_time(&tm, "%c %Z");
	this->m_settings_manager->set_setting("application.last-launch", oss.str());
	this->m_settings_manager->save_settings();

	return true;
}

bool Application::initialize_app()
{
	return true;
}

void Application::cleanup()
{
	this->m_notification_manager->shutdown();
	SPD_INFO_CLASS(COMMON::d_settings_group_application, "Application cleaned up");
}

void Application::handle_set_secret()
{
	std::string secret	= m_option_manager->get_option<std::string>("s");
	std::string account = m_option_manager->get_option<std::string>("a");

	if (secret.empty() || account.empty())
	{
		SPD_ERROR_CLASS(COMMON::d_settings_group_application, "Both --secret (-s) and --account (-a) must be provided.");
		return;
	}

	if (m_totp_manager->set_account(account, secret))
	{
		SPD_INFO_CLASS(COMMON::d_settings_group_application, fmt::format("Successfully set secret for account: {}", account));
		generate_and_print_once();
	}
	else
	{
		SPD_ERROR_CLASS(COMMON::d_settings_group_application, "Failed to set secret.");
	}
}

void Application::run_watch_mode()
{
	std::string account_name = m_totp_manager->get_account_name();
	if (account_name.empty())
	{
		SPD_ERROR_CLASS(COMMON::d_settings_group_application, "No account configured. Please set one using -s <secret> -a <name>");
		return;
	}

	SPD_INFO_CLASS(COMMON::d_settings_group_application, fmt::format("Starting watch mode for account: {}. Press 'q' to quit.", account_name));

#ifdef _WIN32
	while (true)
	{
		if (_kbhit())
		{
			char c = _getch();
			if (c == 'q' || c == 'Q')
			{
				break;
			}
		}

		std::string code		   = m_totp_manager->generate_totp();
		int			remaining_time = 30 - (time(NULL) % 30);
		std::cout << "Code: " << code << "  (updates in " << std::setw(2) << remaining_time << "s)  \r" << std::flush;
		std::this_thread::sleep_for(std::chrono::milliseconds(200));
	}
#else
	RawTerminal raw_term;
	while (true)
	{
		char c = 0;
		if (read(STDIN_FILENO, &c, 1) > 0)
		{
			if (c == 'q' || c == 'Q')
			{
				break;
			}
		}

		std::string code		   = m_totp_manager->generate_totp();
		int			remaining_time = 30 - (time(NULL) % 30);
		std::cout << "Code: " << code << "  (updates in " << std::setw(2) << remaining_time << "s)  \r" << std::flush;
		std::this_thread::sleep_for(std::chrono::milliseconds(200));
	}
#endif

	std::cout << std::endl;
	SPD_INFO_CLASS(COMMON::d_settings_group_application, "Watch mode stopped.");
}

void Application::generate_and_print_once()
{
	std::string account_name = m_totp_manager->get_account_name();
	if (account_name.empty())
	{
		SPD_ERROR_CLASS(COMMON::d_settings_group_application, "No account configured. Please set one using -s <secret> -a <name>");
		return;
	}

	std::string code = m_totp_manager->generate_totp();
	if (!code.empty())
	{
		std::cout << "Account: " << account_name << std::endl;
		std::cout << "TOTP Code: " << code << std::endl;
	}
	else
	{
		SPD_ERROR_CLASS(COMMON::d_settings_group_application, "Could not generate TOTP code.");
	}
}

} // namespace APP
