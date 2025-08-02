#pragma once
#include "nt.h"
#include <stddef.h> 
#include <cstdint>
#include <Windows.h>

struct ipc_request_t {
    uint32_t function_index;
    uint16_t request_size;
    uint8_t request_data[4096];
};

struct ipc_response_t {
    uint32_t status;
    uint16_t response_size;
    uint8_t response_data[4096];
};

struct ipc_shared_t {
    ipc_request_t request;
    ipc_response_t response;
    volatile LONG state;
};

using MBV_IPC_CALLBACK = void(*)(void* context, const ipc_request_t* req, ipc_response_t* out); // Callback type for handling IPC requests

struct MBV_IPC_SERVER {
    HANDLE section;
    HANDLE event_request;
    HANDLE event_response;
    HANDLE mutex;
    ipc_shared_t* shared;
    MBV_IPC_CALLBACK callback;
    void* callback_context;
    wchar_t name_prefix[64];
};

// Initialize the IPC server with a name prefix and a callback function
MBV_IPC_SERVER* MBV_IPCInitServer(const wchar_t* name_prefix, MBV_IPC_CALLBACK callback, void* callback_context);

// Start the IPC server loop to handle incoming requests
void MBV_IPCServerLoop(MBV_IPC_SERVER* server);

// Shutdown the IPC server and release resources
void MBV_IPCShutdownServer(MBV_IPC_SERVER* server);