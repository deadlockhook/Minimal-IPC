#pragma once
#include "nt.h"
#include <cstdint>

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

struct MBV_IPC_CONNECTION {
    ipc_shared_t* shared;
    HANDLE section;
    HANDLE event_request;
    HANDLE event_response;
    HANDLE mutex;
};

// Initialize the IPC client with a name prefix
bool MBV_IPCConnect(const wchar_t* name_prefix, MBV_IPC_CONNECTION* out);

// Call a function on the IPC server
bool MBV_IPCCall(MBV_IPC_CONNECTION* conn, uint32_t fn_id, const void* req_data, uint16_t req_size, void* out_data, uint16_t* out_size);

// Disconnect from the IPC server
void MBV_IPCDisconnect(MBV_IPC_CONNECTION* conn);