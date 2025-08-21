#include "./src/vm.hpp"


int main() {
	VirtualMachine vm;

	vm.load("./example/example.bin");
	vm.execute();

	return 0;
}
