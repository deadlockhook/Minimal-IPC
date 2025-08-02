#include "ipc_client.h"
#include <iostream>


int main()
{
    MBV_IPC_CONNECTION conn{};

	// Initialize the IPC client with a name prefix
    if (!MBV_IPCConnect(L"mbv_test", &conn)) {
        std::cout << "Failed to connect.\n";
        system("pause");
        return 0;
    }

    struct { uint64_t a; uint64_t b; } args{ 1337, 420 };
    uint64_t result = 0;
    uint16_t result_size = sizeof(result);
    std::cout << "Calling function with args: a = " << args.a << ", b = " << args.b << "\n";

	// Make an IPC call to the server with function index 1 and the provided arguments
    if (MBV_IPCCall(&conn, 1, &args, sizeof(args), &result, &result_size)) {
        std::cout << "Result: " << result << "\n";
        system("pause");
    }
    else {
        std::cout << "Call failed." << "\n";
        system("pause");
    }

	// Disconnect from the IPC server and release resources
    MBV_IPCDisconnect(&conn);

	std::cout << "Disconnected from IPC server.\n";
	system("pause");
	return 0;
}