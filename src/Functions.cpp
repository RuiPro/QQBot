#include "functions.h"

void CorePlugin(const string& msg, MainProcess* m) {
	try {
		json QQevent = json::parse(msg, NULL, false);
		if (QQevent["post_type"] == "notice") {
			// 群变动时更新群列表，包括群内人数改变（包括Bot加群退群）
			if (QQevent["notice_type"] == "group_decrease" || QQevent["notice_type"] == "group_increase") {
				if (QQevent["user_id"] == QQBot.getThisBotID()) {	// 如果是Bot加群退群，更新bot群列表
					loger.info() << "QQBot group list changed.";
					QQBot.fetchThisBotGroupList();
					QQBot.printGroupList();
				}
				else {
					if (QQBot.fetchThisBotGroupMemberList(QQevent["group_id"]) == 0) {	// 如果是其他群，更新对应群即可
						loger.info() << "QQBot group " << QQevent["group_id"] << " updated.";
					}
				}
			}
			// 群成员信息变动
			if (QQevent["notice_type"] == "group_admin") {
				if (QQBot.fetchThisBotGroupMemberList(QQevent["group_id"]) == 0) {
					loger.info() << "QQBot group " << QQevent["group_id"] << " updated.";
				}
			}
			if (QQevent["notice_type"] == "notify" && QQevent["sub_type"] == "title") {
				if (QQBot.fetchThisBotGroupMemberInfo(QQevent["group_id"], QQevent["user_id"]) == 0) {
					loger.info() << "QQGroupMember " << QQevent["user_id"] << " in group " << QQevent["group_id"] << " updated.";
				}
			}
			if (QQevent["notice_type"] == "group_card") {
				if (QQBot.fetchThisBotGroupMemberInfo(QQevent["group_id"], QQevent["user_id"]) == 0) {
					loger.info() << "QQGroupMember " << QQevent["user_id"] << " in group " << QQevent["group_id"] << " updated.";
				}
			}
			// 好友变动时更新好友列表
			if (QQevent["notice_type"] == "friend_add") {
				loger.info() << "QQBot friend list changed.";
				QQBot.fetchThisBotFriendList();
				QQBot.fetchThisBotUFriendList();
				QQBot.printFriendList();
				QQBot.printUFriendList();
			}
			// 群内名片、头衔改变
			if (QQevent["notice_type"] == "group_card") {
				if (QQBot.fetchThisBotGroupMemberInfo(QQevent["group_id"], QQevent["user_id"]) == 0) {
					loger.info() << "Member " << QQevent["user_id"] << " in group " << QQevent["group_id"] << " information updated.";
				}
				else {
					loger.warn() << "Member " << QQevent["user_id"] << " in group " << QQevent["group_id"] << " information update failed!";
				}
			}
			if (QQevent["notice_type"] == "notify" && QQevent["sub_type"] == "title") {
				if (QQBot.fetchThisBotGroupMemberInfo(QQevent["group_id"], QQevent["user_id"]) == 0) {
					loger.info() << "Member " << QQevent["user_id"] << " in group " << QQevent["group_id"] << " information updated.";
				}
				else {
					loger.warn() << "Member " << QQevent["user_id"] << " in group " << QQevent["group_id"] << " information update failed!";
				}
			}
		}
		// 加群、加好友
		if (QQevent["post_type"] == "request") {
			if (QQevent["request_type"] == "friend") {
				loger.info() << QQevent["user_id"] << " requests to add bot as a friend: " << QQevent["comment"];
				if (m->m_auto_add_friend) {
					QQBot.applyAddFriendRequest(QQevent["flag"], true, "");
					loger.info() << "Automatically added friend " << QQevent["user_id"];
				}
			}
			if (QQevent["request_type"] == "group" && QQevent["sub_type"] == "invite") {
				loger.info() << "bot is invited to join group " << QQevent["group_id"];
				if (m->m_auto_join_group) {
					QQBot.applyAddGroupRequest(QQevent["flag"], QQevent["sub_type"], true, "");
					loger.info() << "Automatically join group " << QQevent["group_id"];
				}
			}
		}
	}
	catch (...) {
		loger.error() << "CorePlugin Exception.";
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
					loger.error() << "Failed to get data from input buffer!";
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
					if (data["self_id"] != QQBot.getThisBotID()) {
						loger.warn() << "Received messages from other QQ bots, ignored.";
						break;
					}
					if (data["self_id"] == "meta_event" && data["meta_event_type"] == "heartbeat") {
						break;
					}
					arg->msgQueueAdd(string(recv_data));
				} while (false);
			}
			else {
				loger.warn() << "The size of data received doesn't match client report.";
			}
			delete[] recv_data;
			chrono::duration<double> runTime = chrono::system_clock::now() - startTime;
			auto runTimeCount = runTime.count();
			if (runTimeCount > 4.75) {
				loger.warn() << "HTTPRequestCB run time: " << runTimeCount << " is longer than 5s.";
			}
		}
	}
	catch (...) {
		loger.error() << "Libevent's evhttp callback function HTTPRequestCB Exception.";
	}
}

void TickEventCB(evutil_socket_t fd, short event_t, void* cb_arg) {
	try {
		MainProcess* arg = (MainProcess*)cb_arg;
		for (auto& plugin : *arg->m_plugins_list) {
			arg->m_thread_pool->addTask((void(BasicPlugin::*)()) & BasicPlugin::pluginMain, plugin->getBasicPlugin());
		}
		string a_msg;
		if (arg->msgQueueGet(a_msg) != 0) return;
		CorePlugin(a_msg, arg);
		for (auto& plugin : *arg->m_plugins_list) {
			arg->m_thread_pool->addTask((void(BasicPlugin::*)(const string&)) & BasicPlugin::pluginMain, plugin->getBasicPlugin(), a_msg);
		}
	}
	catch (...) {
		loger.error() << "Libevent's timer callback function TickEventCB Exception.";
	}
}