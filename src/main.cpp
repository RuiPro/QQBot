#include "main_process.h"
#include "loger.hpp"
#include "QQ/QQ.h"
#include <iostream>
#include <csignal>

bool terminate_flag = false;

void signalHandler(int signal) {
	if (!terminate_flag) {
		terminate_flag = true;
		std::cout << '\r';
		delete MainProcPtr;
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
	MainProcPtr->exec();
	return 0;
}
