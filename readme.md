# Minimal Native IPC Framework (Section + Mutex, NTAPI)

A minimal and efficient inter-process communication (IPC) framework using native Windows NT APIs. This project implements a client-server model backed by shared memory (`NtCreateSection`), kernel event signaling (`NtCreateEvent`), and mutex-based synchronization (`NtCreateMutant`), without relying on Win32 APIs or CRT dependencies.

---

## 🔧 Project Description

This framework provides **structured, low-overhead, and thread-safe IPC** for local Windows processes. It leverages the following NT primitives:

- **Section objects** for shared memory  
- **Event objects** for request/response signaling  
- **Mutant (mutex) objects** for synchronization  

The server registers a callback and enters a loop to process incoming requests. Clients construct and send typed requests, then wait for the response.

---

## 📦 Components

### `ipc_server.{h,cpp}`
Implements the server side of the IPC framework:
- Initializes section, events, and mutex with a name prefix
- Registers a user-provided callback for request processing
- Enters a blocking loop to handle incoming client messages

### `ipc_client.{h,cpp}`
Implements the client side of the IPC framework:
- Connects to the server’s shared memory and synchronization objects
- Provides a function to send a request and receive a response
- Handles cleanup and unmapping automatically

### Shared Structures
All communication is performed using the following fixed-size types:
- `ipc_request_t`: contains function index and request data
- `ipc_response_t`: contains return status and response data
- `ipc_shared_t`: combines request, response, and a `state` field to coordinate progress

---

## 🧪 Example Usage

### Server Setup
```cpp
auto* server = MBV_IPCInitServer(L"MyService", [](void* ctx, const ipc_request_t* req, ipc_response_t* res) {
    res->status = 0;
    res->response_size = req->request_size;
    memcpy(res->response_data, req->request_data, req->request_size);
}, nullptr);

MBV_IPCServerLoop(server);
```

### Client Call
```cpp
MBV_IPC_CONNECTION conn = {};
if (MBV_IPCConnect(L"MyService", &conn)) {
    const char msg[] = "hello";
    char response[4096] = {};
    uint16_t size = sizeof(response);

    if (MBV_IPCCall(&conn, 1, msg, sizeof(msg), response, &size)) {
        printf("Reply: %.*s\n", size, response);
    }

    MBV_IPCDisconnect(&conn);
}
```

---

## 🔐 Security Considerations

This IPC framework is intended for **general-purpose local inter-process communication** between trusted applications running on the same machine.

### Recommendations:
- The default behavior uses global named objects under `\BaseNamedObjects\`. Limit exposure using custom `SECURITY_DESCRIPTOR`s if needed.
- Validate all request sizes (`request_size`) before accessing buffers.
- Assign unique `name_prefix` values to avoid collisions across unrelated IPC instances.
- Optionally implement request filtering or authentication in the callback handler.

---

## ⚙️ Build Notes

- Designed for Visual Studio with full support for MSVC or Clang toolchains.
- Requires `nt.h` to define prototypes for native NT functions:
  - `NtCreateSection`, `NtMapViewOfSection`
  - `NtCreateEvent`, `NtSetEvent`, `NtWaitForSingleObject`
  - `NtCreateMutant`, `NtReleaseMutant`, etc.

No CRT is required if compiled with:
```sh
/GS- /GR- /EHsc /NODEFAULTLIB
```

---

## 📁 File Structure

```
.
├── ipc_server.h
├── ipc_server.cpp
├── ipc_client.h
├── ipc_client.cpp
├── nt.h              // Required NT function declarations
├── main.cpp          // Example (not included)
```

---

## ✅ Features

- Native-only implementation (no Win32 dependencies)
- Simple, consistent memory layout for both client and server
- Blocking server loop with callback invocation
- Clean shutdown support and resource management
- Optional zero-CRT usage for minimal stub projects

---

## 🧼 Cleanup

Both client and server ensure all handles are closed and memory is unmapped properly using native cleanup routines (`NtClose`, `NtUnmapViewOfSection`, etc.).

---

## 📜 License

This project is open source. Use it freely in your own applications or projects.
