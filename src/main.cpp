#include "MainProcess.h"

int main(int argc, char** argv) {
	MainProcess* process = new MainProcess(argc, argv);
	process->Exec();
	delete process;
	return 0;
}
