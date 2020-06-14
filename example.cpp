#include "threadpool.h"

#include <iostream>

int main() {
	{
		threadpool tp(3);

		for (int i = 1; i <= 100; ++i) {
			tp.add([i] {
				std::cout << i << "\n";
			});
		}
	}

	return 0;
}
