#include "ipc_server.h"
#include <corecrt_wstdio.h>

// Initialize the IPC server with a name prefix and a callback function
MBV_IPC_SERVER* MBV_IPCInitServer(const wchar_t* name_prefix, MBV_IPC_CALLBACK callback, void* callback_context) {

    if (!callback || !name_prefix)
        return nullptr;

	MBV_IPC_SERVER* server = (MBV_IPC_SERVER*)malloc(sizeof(MBV_IPC_SERVER)); // Allocate memory for the server structure
    RtlZeroMemory(server, sizeof(MBV_IPC_SERVER)); 
    server->callback = callback; 
    server->callback_context = callback_context;

    wcsncpy_s(server->name_prefix, name_prefix, 63);
    server->name_prefix[63] = 0;
    wchar_t sec_name[64];
    wchar_t req_evt[64];
    wchar_t res_evt[64];
    wchar_t mut_name[64];

    swprintf(sec_name, _countof(sec_name), L"\\BaseNamedObjects\\%s_o", name_prefix);
    swprintf(req_evt, _countof(req_evt), L"\\BaseNamedObjects\\%s_r", name_prefix);
    swprintf(res_evt, _countof(res_evt), L"\\BaseNamedObjects\\%s_s", name_prefix);
    swprintf(mut_name, _countof(mut_name), L"\\BaseNamedObjects\\%s_m", name_prefix);
    UNICODE_STRING sec_str;
    UNICODE_STRING req_str;
    UNICODE_STRING res_str;
    UNICODE_STRING mut_str;
    RtlInitUnicodeString(&sec_str, sec_name);
    RtlInitUnicodeString(&req_str, req_evt);
    RtlInitUnicodeString(&res_str, res_evt);
    RtlInitUnicodeString(&mut_str, mut_name);
    OBJECT_ATTRIBUTES oa;
    LARGE_INTEGER size;
    size.QuadPart = sizeof(ipc_shared_t);
    InitializeObjectAttributes(&oa, &sec_str, OBJ_CASE_INSENSITIVE, nullptr, nullptr);
    HANDLE hSection = nullptr;

	// Create a section for shared memory
    if (!NT_SUCCESS(NtCreateSection(&hSection, SECTION_ALL_ACCESS, &oa, &size, PAGE_READWRITE, SEC_COMMIT | SEC_NOCACHE, nullptr))) {
        delete server;
        return nullptr;
    }
    PVOID base = nullptr;
    SIZE_T view_size = sizeof(ipc_shared_t);

	// Map the section into the current process's address space
    if (!NT_SUCCESS(NtMapViewOfSection(hSection, NtCurrentProcess(), &base, 0, 0, nullptr, &view_size, ViewUnmap, 0, PAGE_READWRITE))) {
        NtClose(hSection);
        delete server;
        return nullptr;
    }
    InitializeObjectAttributes(&oa, &req_str, OBJ_CASE_INSENSITIVE, nullptr, nullptr);
    HANDLE hReq = nullptr;

	// Create an event for request signaling
    if (!NT_SUCCESS(NtCreateEvent(&hReq, EVENT_ALL_ACCESS, &oa, SynchronizationEvent, FALSE))) {
        NtUnmapViewOfSection(NtCurrentProcess(), base);
        NtClose(hSection);
        delete server;
        return nullptr;
    }
    InitializeObjectAttributes(&oa, &res_str, OBJ_CASE_INSENSITIVE, nullptr, nullptr);
    HANDLE hRes = nullptr;

	// Create an event for response signaling
    if (!NT_SUCCESS(NtCreateEvent(&hRes, EVENT_ALL_ACCESS, &oa, SynchronizationEvent, FALSE))) {
        NtClose(hReq);
        NtUnmapViewOfSection(NtCurrentProcess(), base);
        NtClose(hSection);
        delete server;
        return nullptr;
    }
    server->section = hSection;
    server->event_request = hReq;
    server->event_response = hRes;

    InitializeObjectAttributes(&oa, &mut_str, OBJ_CASE_INSENSITIVE, nullptr, nullptr);
    HANDLE hMut = nullptr;

	// Create a mutant for synchronization
    if (!NT_SUCCESS(NtCreateMutant(&hMut, MUTANT_ALL_ACCESS, &oa, FALSE))) {
        NtClose(hRes);
        NtClose(hReq);
        NtUnmapViewOfSection(NtCurrentProcess(), base);
        NtClose(hSection);
        delete server;
        return nullptr;
    }
    server->section = hSection;
    server->event_request = hReq;
    server->event_response = hRes;
    server->mutex = hMut;
    server->shared = reinterpret_cast<ipc_shared_t*>(base);
    server->shared->state = 0;
    return server;
}

// Start the IPC server loop to handle incoming requests
void MBV_IPCServerLoop(MBV_IPC_SERVER* server) {
    if (!server || !server->callback) return;
    while (true) {
        
		// Wait for a request to be made
        NtWaitForSingleObject(server->event_request, FALSE, nullptr);
        NtWaitForSingleObject(server->mutex, FALSE, nullptr);

        // Check if the server is still in a valid state
        if (server->shared->state != 1) {
            NtReleaseMutant(server->mutex, nullptr);
            continue;
        }

        ipc_request_t* req = &server->shared->request;
        ipc_response_t* res = &server->shared->response;
        res->status = 0;
        res->response_size = 0;

		// Call the registered callback function to handle the request
        server->callback(server->callback_context, req, res);
        server->shared->state = 2;

		// Signal the response event to notify the client
        NtReleaseMutant(server->mutex, nullptr);
        NtSetEvent(server->event_response, nullptr);
    }
}

// Shutdown the IPC server and clean up resources
void MBV_IPCShutdownServer(MBV_IPC_SERVER* server) {

    if (!server) return;
    if (server->shared) NtUnmapViewOfSection(NtCurrentProcess(), server->shared);
    if (server->event_request) NtClose(server->event_request);
    if (server->event_response) NtClose(server->event_response);
    if (server->mutex) NtClose(server->mutex);
    if (server->section) NtClose(server->section);
    free(server);
}


