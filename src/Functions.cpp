#include "Functions.h"

void CorePlugin(const string& msg, QQBot* bot) {
	json QQevent = json::parse(msg, NULL, false);
	if (QQevent["post_type"] != "notice") return;
	// 群变动时更新群列表，包括群内人数改变（包括Bot加群）
	if (QQevent["notice_type"] == "group_decrease" || QQevent["notice_type"] == "group_increase") {
		Info() << "QQBot group list changed." << std::endl;
		bot->GetBotGroupList();
		bot->PrintGroupList();
	}
	// 好友变动时更新好友列表
	if (QQevent["notice_type"] == "friend_add") {
		Info() << "QQBot friend list changed." << std::endl;
		bot->GetBotFriendList();
		bot->PrintFriendList();
	}
	// 群内名片、头衔改变
	if (QQevent["notice_type"] == "group_card") {
		if (bot->UpdateGroupInfo(QQevent["group_id"]) == 0) {
			Info() << "QQBot group " << QQevent["group_id"] << " information updated." << std::endl;
		}
		else {
			Warn() << "QQBot group " << QQevent["group_id"] << " information update failed!" << std::endl;
		}
	}
	if (QQevent["notice_type"] == "notify" && QQevent["sub_type"] == "title") {
		if (bot->UpdateGroupInfo(QQevent["group_id"]) == 0) {
			Info() << "QQBot group " << QQevent["group_id"] << " information updated." << std::endl;
		}
		else {
			Warn() << "QQBot group " << QQevent["group_id"] << " information update failed!" << std::endl;
		}
	}
}

void HTTPRequestCB(struct evhttp_request* req, void* cb_arg) {
	auto startTime = std::chrono::system_clock::now();
	MainProcess* arg = (MainProcess*)cb_arg;
	if (evhttp_request_get_command(req) == EVHTTP_REQ_GET) {
		evbuffer* send_buffer = evhttp_request_get_output_buffer(req);
		evbuffer_add_printf(send_buffer, "{}");
		evhttp_send_reply(req, HTTP_OK, "OK", send_buffer);
		evbuffer_free(send_buffer);
	}
	else {
		evkeyvalq* headers = evhttp_request_get_input_headers(req);
		ssize_t Content_Length = std::stoi(evhttp_find_header(headers, "Content-Length"));
		evbuffer* recv_buffer = evhttp_request_get_input_buffer(req);
		const ssize_t recv_size = evbuffer_get_length(recv_buffer);
		char* recv_data = new char[recv_size + 1] {};
		ssize_t counter = 0;
		while (recv_size > counter) {
			ssize_t once_size = evbuffer_remove(recv_buffer, recv_data, recv_size);
			if (once_size == -1) {
				Error() << "Failed to get data from input buffer!" << std::endl;
				return;
			}
			else {
				counter += once_size;
			}
		}
		evbuffer* send_buffer = evhttp_request_get_output_buffer(req);
		evbuffer_add_printf(send_buffer, "{}");
		evhttp_add_header(req->output_headers, "Content-Type", "application/json");
		evhttp_send_reply(req, HTTP_OK, "OK", send_buffer);
		// evbuffer_free(send_buffer);	无需手动释放，evhttp_send_reply已经自动释放了

		if (Content_Length == recv_size) {
			// 处理数据入口
			json data = json::parse(recv_data);
			do {
				if (data["self_id"] != arg->bot_->GetQQbotID()) {
					Warn() << "Received messages from other QQ bots, ignored." << std::endl;
					break;
				}
				if (data["self_id"] == "meta_event" && data["meta_event_type"] == "heartbeat") {
					break;
				}
				arg->MsgQueueAdd(string(recv_data));
			} while (false);
		}
		else {
			Warn() << "The size of data received doesn't match client report." << std::endl;
		}
		delete[] recv_data;
		std::chrono::duration<double> runTime = std::chrono::system_clock::now() - startTime;
		auto runTimeCount = runTime.count();
		if (runTimeCount > 4.75) {
			Warn() << "HTTPRequestCB run time: " << fixed << setprecision(2) << runTimeCount << " is longer than 5s." << std::endl;
		}
	}
}

void TickEventCB(evutil_socket_t fd, short event_t, void* cb_arg) {
	MainProcess* arg = (MainProcess*)cb_arg;
	for (auto& plugin : *arg->plugins_list_) {
		arg->thread_pool_->AddTask((void(BasicPlugin::*)()) & BasicPlugin::PluginMain, plugin->GetBasicPlugin());
	}
	string a_msg;
	if (arg->MsgQueueGet(a_msg) != 0) return;
	CorePlugin(a_msg, arg->bot_);
	for (auto& plugin : *arg->plugins_list_) {
		arg->thread_pool_->AddTask((void(BasicPlugin::*)(const string&))&BasicPlugin::PluginMain, plugin->GetBasicPlugin(), a_msg);
	}
}