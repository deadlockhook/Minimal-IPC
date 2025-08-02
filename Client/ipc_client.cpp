#include <windows.h>
#include <iostream>
#include "ipc_client.h"

// Initialize the IPC client with a name prefix
bool MBV_IPCConnect(const wchar_t* name_prefix, MBV_IPC_CONNECTION* out) {
   
    if (!name_prefix || !out) return false;
   
    wchar_t sec_name[64];
    wchar_t req_evt[64];
    wchar_t res_evt[64];
    wchar_t mut_name[64];
    swprintf(sec_name, 64, L"\\BaseNamedObjects\\%s_o", name_prefix);
    swprintf(req_evt, 64, L"\\BaseNamedObjects\\%s_r", name_prefix);
    swprintf(res_evt, 64, L"\\BaseNamedObjects\\%s_s", name_prefix);
    swprintf(mut_name, 64, L"\\BaseNamedObjects\\%s_m", name_prefix);
    UNICODE_STRING us_sec;
    UNICODE_STRING us_req;
    UNICODE_STRING us_res;
    UNICODE_STRING us_mut;
    RtlInitUnicodeString(&us_sec, sec_name);
    RtlInitUnicodeString(&us_req, req_evt);
    RtlInitUnicodeString(&us_res, res_evt);
    RtlInitUnicodeString(&us_mut, mut_name);

    OBJECT_ATTRIBUTES oa;
    InitializeObjectAttributes(&oa, &us_sec, OBJ_CASE_INSENSITIVE, nullptr, nullptr);
    HANDLE hSection = nullptr;

	// Open the section for shared memory
    if (!NT_SUCCESS(NtOpenSection(&hSection, SECTION_MAP_WRITE | SECTION_MAP_READ, &oa))) return false;
   
    PVOID base = nullptr;
    SIZE_T view_size = sizeof(ipc_shared_t);

	// Map the section into the current process's address space
    if (!NT_SUCCESS(NtMapViewOfSection(hSection, NtCurrentProcess(), &base, 0, 0, nullptr, &view_size, ViewUnmap, 0, PAGE_READWRITE))) {
        NtClose(hSection);
        return false;
    }
    InitializeObjectAttributes(&oa, &us_req, OBJ_CASE_INSENSITIVE, nullptr, nullptr);
    HANDLE hReq = nullptr;

	// Create an event for request signaling
    if (!NT_SUCCESS(NtOpenEvent(&hReq, EVENT_MODIFY_STATE, &oa))) {
        NtUnmapViewOfSection(NtCurrentProcess(), base);
        NtClose(hSection);
        return false;
    }
    InitializeObjectAttributes(&oa, &us_res, OBJ_CASE_INSENSITIVE, nullptr, nullptr);
    HANDLE hRes = nullptr;

	// Create an event for response signaling
    if (!NT_SUCCESS(NtOpenEvent(&hRes, SYNCHRONIZE, &oa))) {
        NtClose(hReq);
        NtUnmapViewOfSection(NtCurrentProcess(), base);
        NtClose(hSection);
        return false;
    }

    InitializeObjectAttributes(&oa, &us_mut, OBJ_CASE_INSENSITIVE, nullptr, nullptr);
    HANDLE hMut = nullptr;

	// Create a mutant for synchronization
    if (!NT_SUCCESS(NtOpenMutant(&hMut, MUTANT_ALL_ACCESS, &oa))) {
        NtClose(hRes);
        NtClose(hReq);
        NtUnmapViewOfSection(NtCurrentProcess(), base);
        NtClose(hSection);
        return false;
    }

    out->shared = reinterpret_cast<ipc_shared_t*>(base);
    out->section = hSection;
    out->event_request = hReq;
    out->event_response = hRes;
    out->mutex = hMut;
    return true;
}

bool MBV_IPCCall(MBV_IPC_CONNECTION* conn, uint32_t fn_id, const void* req_data, uint16_t req_size, void* out_data, uint16_t* out_size) {
  
    if (!conn || !conn->shared || !req_data || req_size > sizeof(conn->shared->request.request_data))
        return false;

	// Check if the connection is valid and the request size is within limits
    if (!NT_SUCCESS(NtWaitForSingleObject(conn->mutex, FALSE, nullptr)))
        return false;

	// Prepare the request data
    memcpy(conn->shared->request.request_data, req_data, req_size);
    conn->shared->request.function_index = fn_id;
    conn->shared->request.request_size = req_size;
    conn->shared->state = 1;

	// Signal the request event to notify the server
    NtReleaseMutant(conn->mutex, nullptr);
    NtSetEvent(conn->event_request, nullptr);

	// Wait for the server to process the request
    NtWaitForSingleObject(conn->event_response, FALSE, nullptr);

	// Check the response status
    if (conn->shared->response.status != 0) 
        return false;

	// Check if the output buffer is large enough to hold the response data
    if (out_data && out_size && *out_size >= conn->shared->response.response_size) {

		// Copy the response data to the output buffer
        memcpy(out_data, conn->shared->response.response_data, conn->shared->response.response_size);
        *out_size = conn->shared->response.response_size;
        return true;
    }
    return false;
}

// Disconnect from the IPC server and release resources
void MBV_IPCDisconnect(MBV_IPC_CONNECTION* conn) {
    if (!conn) return;
    if (conn->shared) NtUnmapViewOfSection(NtCurrentProcess(), conn->shared);
    if (conn->event_request) NtClose(conn->event_request);
    if (conn->event_response) NtClose(conn->event_response);
    if (conn->mutex) NtClose(conn->mutex);
    if (conn->section) NtClose(conn->section);
    *conn = {};
}

