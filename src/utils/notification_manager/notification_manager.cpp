#include "notification_manager.hpp"

#include "network_manager.hpp"
#include "settings_manager.hpp"

namespace UTILS
{
std::mutex NotificationManager::m_notification_mutex;

std::string_view NotificationManager::get_manager_name() const
{
	return "Notification Manager";
}

void NotificationManager::initialize()
{}

NotificationManager::~NotificationManager()
{
	this->shutdown();
}

void NotificationManager::shutdown()
{
	for (auto& fut : this->m_futures)
	{
		fut.wait();
	}
	m_futures.clear();
}

void NotificationManager::send_notification(std::string_view				topic,
											std::string_view				message,
											std::string_view				title,
											NotificationPriority			priority,
											const std::vector<std::string>& tags,
											bool							enable_markdown,
											std::string_view				schedule)
{
	NotificationMessage notification;
	notification.topic			 = topic;
	notification.message		 = message;
	notification.title			 = title;
	notification.priority		 = priority;
	notification.tags			 = tags;
	notification.enable_markdown = enable_markdown;
	notification.schedule		 = schedule;
	this->send_notification(notification);
	return;
}

void NotificationManager::send_notification(const NotificationMessage& notification)
{
	std::lock_guard<std::mutex> lock(this->m_notification_mutex);

	m_futures.push_back(std::async(std::launch::async, [notification] {
		auto settings_manager = UTILS::SettingsManager::instance();
		if (settings_manager->get_setting<std::string>("notifications.enabled", "false").compare("false"))
		{
			SPD_DEBUG_CLASS(COMMON::d_settings_group_utils, "Unable to send notification, notifications are disabled.");
			return;
		}

		auto notifications_uri = settings_manager->get_setting<std::string>("notifications.uri", "");
		if (notifications_uri.empty())
		{
			SPD_DEBUG_CLASS(COMMON::d_settings_group_utils, "Unable to send notification, notifications server is empty.");
			return;
		}

		auto network_manager = UTILS::NetworkManager::instance();

		std::string tags;
		for (const auto& tag : notification.tags)
		{
			tags += tag + ",";
		}
		if (!tags.empty())
		{
			tags.pop_back();
		}

		std::string actions;
		for (const auto& action : notification.actions)
		{
			actions += action + ";";
		}
		if (!actions.empty())
		{
			actions.pop_back();
		}

		std::vector<std::pair<std::string, std::string>> headers;
		headers.push_back({"Title", std::string(notification.title)});
		headers.push_back({"Priority", std::to_string(static_cast<int>(notification.priority))});
		headers.push_back({"Tags", tags});
		headers.push_back({"Markdown", (notification.enable_markdown ? "true" : "false")});
		headers.push_back({"Delay", std::string(notification.schedule)});
		headers.push_back({"Click", std::string(notification.click_action)});
		headers.push_back({"Attach", std::string(notification.attachment_url)});
		headers.push_back({"Email", std::string(notification.email_recipient)});
		headers.push_back({"Actions", actions});

		NetworkRequest request;
		request.method	= HttpMethod::POST;
		request.url		= notifications_uri;
		request.headers = headers;
		request.body	= notification.message;

		request.username = settings_manager->get_setting<std::string>("notifications.username", "");
		request.password = settings_manager->get_setting<std::string>("notifications.password", "");

		auto response = network_manager->make_request(request);
		if (!response.error.empty())
		{
			SPD_DEBUG_CLASS(COMMON::d_settings_group_utils,
							fmt::format("Unable to send notification. Response code {}: {}", response.http_code, response.error));
		}
	}));
}

} // namespace UTILS
