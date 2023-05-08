#include "Functions.h"

void CorePlugin(const string& msg, MainProcess* m) {
	QQBot* bot = m->bot_;
	try {
		json QQevent = json::parse(msg, NULL, false);
		if (QQevent["post_type"] == "notice") {
			// 群变动时更新群列表，包括群内人数改变（包括Bot加群退群）
			if (QQevent["notice_type"] == "group_decrease" || QQevent["notice_type"] == "group_increase") {
				if (QQevent["user_id"] == bot->GetBotID()) {	// 如果是Bot加群退群，更新bot群列表
					Qlog.Info() << "QQBot group list changed." << endl;
					bot->SetBotGroupList();
					bot->PrintGroupList();
				}
				else {
					if (bot->ResetBotGroup(QQevent["group_id"]) == 0) {	// 如果是其他群，更新对应群即可
						Qlog.Info() << "QQBot group " << QQevent["group_id"] << " updated." << endl;
					}
				}
			}
			// 群成员信息变动
			if (QQevent["notice_type"] == "group_admin") {
				if (bot->ResetBotGroup(QQevent["group_id"]) == 0) {
					Qlog.Info() << "QQBot group " << QQevent["group_id"] << " updated." << endl;
				}
			}
			if (QQevent["notice_type"] == "notify" && QQevent["sub_type"] == "title") {
				if (bot->SetGroupMemberInfo(QQevent["group_id"], QQevent["user_id"]) == 0) {
					Qlog.Info() << "QQGroupMember " << QQevent["user_id"] << " in group " << QQevent["group_id"] << " updated." << endl;
				}
			}
			if (QQevent["notice_type"] == "group_card") {
				if (bot->SetGroupMemberInfo(QQevent["group_id"], QQevent["user_id"]) == 0) {
					Qlog.Info() << "QQGroupMember " << QQevent["user_id"] << " in group " << QQevent["group_id"] << " updated." << endl;
				}
			}
			// 好友变动时更新好友列表
			if (QQevent["notice_type"] == "friend_add") {
				Qlog.Info() << "QQBot friend list changed." << endl;
				bot->SetBotFriendList();
				bot->SetBotUFriendList();
				bot->PrintFriendList();
				bot->PrintUFriendList();
			}
			// 群内名片、头衔改变
			if (QQevent["notice_type"] == "group_card") {
				if (bot->SetGroupMemberInfo(QQevent["group_id"], QQevent["user_id"]) == 0) {
					Qlog.Info() << "Member " << QQevent["user_id"] << " in group " << QQevent["group_id"] << " information updated." << endl;
				}
				else {
					Qlog.Warn() << "Member " << QQevent["user_id"] << " in group " << QQevent["group_id"] << " information update failed!" << endl;
				}
			}
			if (QQevent["notice_type"] == "notify" && QQevent["sub_type"] == "title") {
				if (bot->SetGroupMemberInfo(QQevent["group_id"], QQevent["user_id"]) == 0) {
					Qlog.Info() << "Member " << QQevent["user_id"] << " in group " << QQevent["group_id"] << " information updated." << endl;
				}
				else {
					Qlog.Warn() << "Member " << QQevent["user_id"] << " in group " << QQevent["group_id"] << " information update failed!" << endl;
				}
			}
		}
		// 加群、加好友
		if (QQevent["post_type"] == "request") {
			if (QQevent["request_type"] == "friend") {
				Qlog.Info() << QQevent["user_id"] << " requests to add bot as a friend: " << QQevent["comment"] << endl;
				if (m->auto_add_friend_) {
					bot->SetAddFriendRequest(QQevent["flag"], true, "");
					Qlog.Info() << "Automatically added friend " << QQevent["user_id"] << endl;
				}
			}
			if (QQevent["request_type"] == "group" && QQevent["sub_type"] == "invite") {
				Qlog.Info() << "bot is invited to join group " << QQevent["group_id"] << endl;
				if (m->auto_join_group_) {
					bot->SetAddFriendRequest(QQevent["flag"], true, "");
					Qlog.Info() << "Automatically join group " << QQevent["group_id"] << endl;
				}
			}
		}
	}
	catch (...) {
		Qlog.Error() << "CorePlugin Exception." << endl;
	}
}

void HTTPRequestCB(struct evhttp_request* req, void* cb_arg) {
	try {
		auto startTime = chrono::system_clock::now();
		MainProcess* arg = (MainProcess*)cb_arg;
		if (evhttp_request_get_command(req) == EVHTTP_REQ_GET) {
			evbuffer* send_buffer = evhttp_request_get_output_buffer(req);
			evbuffer_add_printf(send_buffer, "{}");
			evhttp_send_reply(req, HTTP_OK, "OK", send_buffer);
			evbuffer_free(send_buffer);
		}
		else {
			evkeyvalq* headers = evhttp_request_get_input_headers(req);
			ssize_t Content_Length = stoi(evhttp_find_header(headers, "Content-Length"));
			evbuffer* recv_buffer = evhttp_request_get_input_buffer(req);
			const ssize_t recv_size = evbuffer_get_length(recv_buffer);
			char* recv_data = new char[recv_size + 1] {};
			ssize_t counter = 0;
			while (recv_size > counter) {
				ssize_t once_size = evbuffer_remove(recv_buffer, recv_data, recv_size);
				if (once_size == -1) {
					Qlog.Error() << "Failed to get data from input buffer!" << endl;
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
					if (data["self_id"] != arg->bot_->GetBotID()) {
						Qlog.Warn() << "Received messages from other QQ bots, ignored." << endl;
						break;
					}
					if (data["self_id"] == "meta_event" && data["meta_event_type"] == "heartbeat") {
						break;
					}
					arg->MsgQueueAdd(string(recv_data));
				} while (false);
			}
			else {
				Qlog.Warn() << "The size of data received doesn't match client report." << endl;
			}
			delete[] recv_data;
			chrono::duration<double> runTime = chrono::system_clock::now() - startTime;
			auto runTimeCount = runTime.count();
			if (runTimeCount > 4.75) {
				Qlog.Warn() << "HTTPRequestCB run time: " << fixed << setprecision(2) << runTimeCount << " is longer than 5s." << endl;
			}
		}
	}
	catch (...) {
		Qlog.Error() << "Libevent's evhttp callback function HTTPRequestCB Exception." << endl;
	}
}

void TickEventCB(evutil_socket_t fd, short event_t, void* cb_arg) {
	try {
		MainProcess* arg = (MainProcess*)cb_arg;
		for (auto& plugin : *arg->plugins_list_) {
			arg->thread_pool_->AddTask((void(BasicPlugin::*)()) & BasicPlugin::PluginMain, plugin->GetBasicPlugin());
		}
		string a_msg;
		if (arg->MsgQueueGet(a_msg) != 0) return;
		CorePlugin(a_msg, arg);
		for (auto& plugin : *arg->plugins_list_) {
			arg->thread_pool_->AddTask((void(BasicPlugin::*)(const string&)) & BasicPlugin::PluginMain, plugin->GetBasicPlugin(), a_msg);
		}
	}
	catch (...) {
		Qlog.Error() << "Libevent's timer callback function TickEventCB Exception." << endl;
	}
}