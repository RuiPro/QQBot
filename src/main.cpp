#include "main_process.h"
#include "Loger/loger.h"
#include "QQ/QQ.h"
#include <iostream>
#include <csignal>

bool terminate_flag = false;

void signalHandler(int signal) {
	if (!terminate_flag) {
		terminate_flag = true;
		std::cout << '\r';
		delete ProcessPtr;
		delete QQBotPtr;
		exit(signal);
	}
	else {
		exit(0);
	}
}

int main(int argc, char** argv) {
    signal(SIGINT, signalHandler);
	MainProcess::initMainProcessObj(argc, argv);
	ProcessPtr->exec();
	return 0;
}
