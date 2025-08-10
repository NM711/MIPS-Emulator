#include "./src/vm.hpp"


int main() {
	VirtualMachine vm;

	vm.load("./example.bin");
	vm.execute();

	return 0;
}
