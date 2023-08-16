#pragma once

#include <mach/mach.h>
#include <vector>
#include <string>

pid_t pid;
task_t task{};

kern_return_t WriteToProcessMemory(mach_vm_address_t address, void* value, mach_msg_type_number_t size) ;
uintptr_t FindDynamicAddr(uintptr_t ptr, const std::vector<unsigned int>& offsets) ;
uintptr_t GetMainModuleBaseAddress(pid_t procId) ;
void patch(mach_vm_address_t address, void* buffer, mach_vm_size_t size) ;
void nop(mach_vm_address_t address, mach_vm_size_t size) ;
pid_t FindPIDByProcessName(const std::string &processName);
