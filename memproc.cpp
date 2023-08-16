#include "memproc.h"
#include <mach/mach.h>
#include <mach/mach_vm.h>
#include <mach-o/loader.h>
#include <libproc.h>
#include <vector>
#include <string>


kern_return_t WriteToProcessMemory(mach_vm_address_t address, void* value, mach_msg_type_number_t size) const {
    
    return mach_vm_write(task, address, (vm_offset_t)value, size);

}

uintptr_t FindDynamicAddr(mach_vm_address_t ptr, const std::vector<uintptr_t> &offsets) const {
    
    uintptr_t dynamicAddr = ptr;
    for (uintptr_t offset : offsets) {

        mach_vm_read(task, dynamicAddr, sizeof(dynamicAddr), &dynamicAddr, nullptr);
        dynamicAddr += offset;

    }
    return dynamicAddr;

}

mach_vm_address_t GetMainModuleBaseAddress() {

    /*
    task_t tsk;
    if (task_for_pid(mach_task_self(), pid, &tsk) != KERN_SUCCESS) {

        return 0; // Error getting task for PID
    }

    mach_vm_address_t address;
    mach_msg_type_number_t count = TASK_DYLD_INFO_COUNT;
    struct task_dyld_info dyld_info{};

    if (task_info(tsk, TASK_DYLD_INFO, (task_info_t)&dyld_info, &count) != KERN_SUCCESS) {

        return 0; // Error getting task info
    }

    address = dyld_info.all_image_info_addr;

    */

    pthread_t self = pthread_self();
    void* addr = pthread_get_stackaddr_np(self);
    return reinterpret_cast<mach_vm_address_t>(addr);

}

void patch(mach_vm_address_t address, void* buffer, mach_vm_size_t size) const {
    
    vm_protect(task, address, size, false, VM_PROT_READ | VM_PROT_WRITE | VM_PROT_EXECUTE);
    mach_vm_write(task, address, (vm_offset_t)buffer, size);
    mach_vm_protect(task, address, size, false, VM_PROT_READ | VM_PROT_EXECUTE);

}

void nop(mach_vm_address_t address, mach_vm_size_t size) const {
    
    std::vector<uint8_t> nopArray(size, 0x90);
    patch(address, nopArray.data(), size);

}

pid_t FindPIDByProcessName(const std::string &processName) {

    pid_t pids[1024];
    int numberOfProcesses = proc_listpids(PROC_ALL_PIDS, 0, nullptr, 0);
    proc_listpids(PROC_ALL_PIDS, 0, pids, sizeof(pids));

    for (int i = 0; i < numberOfProcesses; ++i) {

        if (pids[i] == 0) continue;

        char pathBuffer[PROC_PIDPATHINFO_MAXSIZE];
        proc_pidpath(pids[i], pathBuffer, sizeof(pathBuffer));
        if (pathBuffer[0] == 0) continue;

        std::string path(pathBuffer);
        std::string name = path.substr(path.rfind('/') + 1);
        
        if (name == processName) {

            return pids[i];
        }
    }

    return -1; // Return -1 if not found
}