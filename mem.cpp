#include "memproc.h"
#include <mach/mach.h>
#include <mach-o/dyld_images.h>
#include <mach-o/loader.h>
#include <sys/types.h>
#include <iostream>
#include <libproc.h>
#include <vector>
#include <string>


MemoryProcessor::MemoryProcessor(const std::string &processName) {

    pid = FindPIDByProcessName(processName);
    if (task_for_pid(mach_task_self(), pid, &task) != KERN_SUCCESS) {

        throw std::runtime_error("Error getting task for PID");

    }
}

kern_return_t MemoryProcessor::WriteToProcessMemory(mach_vm_address_t address, void* value, mach_msg_type_number_t size) {
    
    return mach_vm_write(task, address, (vm_offset_t)value, size);

}

uintptr_t MemoryProcessor::FindDynamicAddr(mach_vm_address_t ptr, std::vector<uintptr_t> offsets) {
    
    uintptr_t dynamicAddr = ptr;
    for (uintptr_t offset : offsets) {

        mach_vm_read(task, dynamicAddr, sizeof(dynamicAddr), &dynamicAddr, nullptr);
        dynamicAddr += offset;

    }
    return dynamicAddr;

}

mach_vm_address_t MemoryProcessor::GetMainModuleBaseAddress() {
    
    task_t task;
    if (task_for_pid(mach_task_self(), pid, &task) != KERN_SUCCESS) {

        return 0; // Error getting task for PID
    }

    mach_vm_address_t address;
    mach_msg_type_number_t count = TASK_DYLD_INFO_COUNT;
    struct task_dyld_info dyld_info;

    if (task_info(task, TASK_DYLD_INFO, (task_info_t)&dyld_info, &count) != KERN_SUCCESS) {

        return 0; // Error getting task info
    }

    address = dyld_info.all_image_info_addr;
    return address;
    
}

void MemoryProcessor::patch(mach_vm_address_t address, void* buffer, mach_vm_size_t size) {
    
    mach_vm_protect(task, address, size, false, VM_PROT_READ | VM_PROT_WRITE | VM_PROT_EXECUTE);
    mach_vm_write(task, address, (vm_offset_t)buffer, size);
    mach_vm_protect(task, address, size, false, VM_PROT_READ | VM_PROT_EXECUTE);

}

void MemoryProcessor::nop(mach_vm_address_t address, mach_vm_size_t size) {
    
    std::vector<uint8_t> nopArray(size, 0x90);
    patch(task, address, nopArray.data(), size);

}

pid_t MemoryProcessor::FindPIDByProcessName(const std::string &processName) {

    pid_t pids[1024];
    int numberOfProcesses = proc_listpids(PROC_ALL_PIDS, 0, nullptr, 0);
    proc_listpids(PROC_ALL_PIDS, 0, pids, sizeof(pids));

    for (int i = 0; i < numberOfProcesses; ++i) {

        if (pids[i] == 0) continue;

        char pathBuffer[PROC_PIDPATHINFO_MAXSIZE];
        proc_pidpath(pids[i], pathBuffer, sizeof(pathBuffer));
        if (pathBuffer[0] == 0) continue;

        std::string path(pathBuffer);
        std::string name = path.substr(path.rfind("/") + 1);
        
        if (name == processName) {

            return pids[i];
        }
    }

    return -1; // Return -1 if not found
}
