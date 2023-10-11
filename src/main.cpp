#include "main_process.h"
#include "Loger/loger.h"
#include "QQ/QQ.h"
#include <iostream>
#include <csignal>

MainProcess* process = nullptr;

void signalHandler(int signal) {
	std::cout << '\r';
	delete ProcessPtr;
	delete QQBotPtr;
    exit(signal);
}

int main(int argc, char** argv) {
    signal(SIGINT, signalHandler);
	MainProcess::initMainProcessObj(argc, argv);
	loger.debug() << "ProcessPtr: " << to_string((long)ProcessPtr);
	ProcessPtr->exec();
	return 0;
}
