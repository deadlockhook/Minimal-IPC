#pragma once
#include <Windows.h>
#include <winternl.h>

#pragma comment(lib, "ntdll.lib")

#define NtCurrentProcess() ((HANDLE)(LONG_PTR)-1)

typedef enum _SECTION_INHERIT
{
    ViewShare = 1,
    ViewUnmap = 2
} SECTION_INHERIT;

typedef enum _EVENT_TYPE {
    NotificationEvent,
    SynchronizationEvent
} EVENT_TYPE;

extern "C" {
    NTSYSCALLAPI NTSTATUS NTAPI NtCreateSection(
        PHANDLE SectionHandle,
        ACCESS_MASK DesiredAccess,
        POBJECT_ATTRIBUTES ObjectAttributes,
        PLARGE_INTEGER MaximumSize,
        ULONG SectionPageProtection,
        ULONG AllocationAttributes,
        HANDLE FileHandle
    );

    NTSYSCALLAPI NTSTATUS NTAPI NtOpenSection(
        PHANDLE SectionHandle,
        ACCESS_MASK DesiredAccess,
        POBJECT_ATTRIBUTES ObjectAttributes
    );

    NTSYSCALLAPI NTSTATUS NTAPI NtMapViewOfSection(
        HANDLE SectionHandle,
        HANDLE ProcessHandle,
        PVOID* BaseAddress,
        ULONG_PTR ZeroBits,
        SIZE_T CommitSize,
        PLARGE_INTEGER SectionOffset,
        PSIZE_T ViewSize,
        DWORD InheritDisposition,
        ULONG AllocationType,
        ULONG Win32Protect
    );

    NTSYSCALLAPI NTSTATUS NTAPI NtUnmapViewOfSection(
        HANDLE ProcessHandle,
        PVOID BaseAddress
    );

    NTSYSCALLAPI NTSTATUS NTAPI NtCreateMutant(
        PHANDLE MutantHandle,
        ACCESS_MASK DesiredAccess,
        POBJECT_ATTRIBUTES ObjectAttributes,
        BOOLEAN InitialOwner
    );

    NTSYSCALLAPI NTSTATUS NTAPI NtOpenMutant(
        PHANDLE MutantHandle,
        ACCESS_MASK DesiredAccess,
        POBJECT_ATTRIBUTES ObjectAttributes
    );

    NTSYSCALLAPI NTSTATUS NTAPI NtReleaseMutant(
        HANDLE MutantHandle,
        PLONG PreviousCount
    );

    NTSYSCALLAPI NTSTATUS NTAPI NtCreateEvent(
        PHANDLE EventHandle,
        ACCESS_MASK DesiredAccess,
        POBJECT_ATTRIBUTES ObjectAttributes,
        EVENT_TYPE EventType,
        BOOLEAN InitialState
    );
    NTSYSCALLAPI NTSTATUS NTAPI NtOpenEvent(
        PHANDLE EventHandle,
        ACCESS_MASK DesiredAccess,
        POBJECT_ATTRIBUTES ObjectAttributes
	);

    NTSYSCALLAPI NTSTATUS NTAPI NtSetEvent(
        HANDLE EventHandle,
        PLONG PreviousState OPTIONAL
    );

    NTSYSCALLAPI NTSTATUS NTAPI NtResetEvent(
        HANDLE EventHandle,
        PLONG PreviousState OPTIONAL
	);

}
