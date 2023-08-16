#pragma once

#include <mach/mach.h>
#include <vector>
#include <string>

pid_t pid;
task_t task{};

kern_return_t WriteToProcessMemory(mach_vm_address_t address, void* value, mach_msg_type_number_t size) ;
uintptr_t FindDynamicAddr(mach_vm_address_t ptr, const std::vector<uintptr_t>& offsets) ;
static mach_vm_address_t GetMainModuleBaseAddress() ;
void patch(mach_vm_address_t address, void* buffer, mach_vm_size_t size) ;
void nop(mach_vm_address_t address, mach_vm_size_t size) ;
static pid_t FindPIDByProcessName(const std::string &processName);
