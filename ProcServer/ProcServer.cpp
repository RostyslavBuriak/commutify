#include "server.h"

int main() {
	ThreadPool pool(8); //threads are used for best perfomance
	server s(&pool);
	s.Start();

	while (true) {}
	s.Stop();

	return 0;
}