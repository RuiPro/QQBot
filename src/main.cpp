#include "main_process.h"
#include "QQ/QQ.h"
#include <csignal>

MainProcess* process = nullptr;

void signalHandler(int signal) {
	delete ProcessPtr;
	delete QQBotPtr;
    exit(signal);
}

int main(int argc, char** argv) {
    signal(SIGINT, signalHandler);
	MainProcess::initMainProcessObj(argc, argv);
	ProcessPtr->exec();
	return 0;
}
