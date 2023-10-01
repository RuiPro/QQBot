#include "main_process.h"
#include <csignal>

MainProcess* process = nullptr;

void signalHandler(int signal) {
	delete process;
    exit(signal);
}

int main(int argc, char** argv) {
    signal(SIGINT, signalHandler);
	process = new MainProcess(argc, argv);
	process->exec();
	delete process;
	return 0;
}
