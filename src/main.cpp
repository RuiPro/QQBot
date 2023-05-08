#include "MainProcess.h"
#include <csignal>

MainProcess* process = nullptr;

void signalHandler(int signal) {
	delete process;
    exit(signal);
}

int main(int argc, char** argv) {
    signal(SIGINT, signalHandler);
	process = new MainProcess(argc, argv);
	process->Exec();
	delete process;
	return 0;
}
