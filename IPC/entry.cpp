#include "ipc_server.h"
#include <iostream>

enum : uint32_t {
	FUNC_ADD_NUMBERS = 1
};

struct add_args_t {
	uint64_t a;
	uint64_t b;
};

void example_callback(void* context, const ipc_request_t* req, ipc_response_t* out) {
	std::cout << "Received IPC request: function_index = " << req->function_index << ", request_size = " << req->request_size << "\n";
	
	if (req->function_index == 1 && req->request_size == sizeof(uint64_t) * 2) {
		// Process the request to add two numbers
		const uint64_t* args = reinterpret_cast<const uint64_t*>(req->request_data);
		uint64_t result = args[0] + args[1];

		std::cout << "Adding numbers: " << args[0] << " + " << args[1] << " = " << result << "\n";

		memcpy(out->response_data, &result, sizeof(result));
		out->response_size = sizeof(result);
		out->status = 0;
	}
	else {
		// Invalid request, set an error status
		out->status = 1;
		out->response_size = 0;
	}
}

int main()
{

	// Initialize the IPC server with a name prefix as mbv_test and a callback function
	auto* server = MBV_IPCInitServer(L"mbv_test", example_callback, nullptr);

	if (!server) {
		std::cerr << "Failed to start IPC server.\n";
		system("pause");
		return 0;
	}

	std::cout << "IPC server running...\n";

	// Start the IPC server loop to handle incoming requests
	MBV_IPCServerLoop(server); 

	system("pause");
	return 0;
}