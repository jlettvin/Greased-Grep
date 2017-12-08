// mine4 demo of pthreads
// g++ -DMAIN --std=c++17 -o mine5 mine5.cpp -pthread -lfmt

#include <thread>

#include "thread_queue.h"


#ifdef MAIN
//______________________________________________________________________________
int main() {
	auto cpus = std::thread::hardware_concurrency();
	std::vector<std::string> tokens{{
		"a","b","c","d","e","f","g","h","i","j","k","l","m",
		"n","o","p","q","r","s","t","u","v","w","x","y","z"
	}};

	Lettvin::Distribute distribute (cpus);

	distribute (tokens);

	return 0;
} // main
#endif
