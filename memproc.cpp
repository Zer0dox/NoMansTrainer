#include "memproc.h"
#include <mach/mach.h>
#include <mach/mach_vm.h>
#include <mach-o/loader.h>
#include <mach-o/dyld.h>
#include <mach-o/dyld_images.h>
#include <dlfcn.h>
#include <libproc.h>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <string>


kern_return_t WriteToProcessMemory(mach_vm_address_t address, void* value, mach_msg_type_number_t size)  {
    
    return mach_vm_write(task, address, (vm_offset_t)value, size);

}

uintptr_t FindDynamicAddr(uintptr_t ptr, const std::vector<unsigned int>& offsets)
{
    uintptr_t dynamicAddr = ptr;
    mach_msg_type_number_t dataCount;
    kern_return_t kr;

    for(unsigned int offset : offsets)
    {
        dataCount = sizeof(dynamicAddr);
        kr = vm_read_overwrite(task, dynamicAddr, dataCount, (vm_address_t)&dynamicAddr,
                               reinterpret_cast<vm_size_t *>(&dataCount));

        if (kr != KERN_SUCCESS)
        {
            return 0; // Error reading memory
        }

        dynamicAddr += offset;
    }

    return dynamicAddr;
}

uintptr_t GetModuleBaseAddress(pid_t procId)
{
    task_for_pid(mach_task_self(), procId, &task);

    struct task_dyld_info dyld_info{};
    mach_msg_type_number_t count = TASK_DYLD_INFO_COUNT;
    if(task_info(task, TASK_DYLD_INFO, (task_info_t)&dyld_info, &count) != KERN_SUCCESS)
    {
        return 0; // Failure
    }

    auto* all_image_infos = (struct dyld_all_image_infos*)dyld_info.all_image_info_addr;
    auto* image_infos = const_cast<dyld_image_info *>(all_image_infos->infoArray);

    for(int i = 0; i < all_image_infos->infoArrayCount; i++)
    {
        const char* image_path = image_infos[i].imageFilePath;
        if(image_path != nullptr)
        {
            return (uintptr_t)image_infos[i].imageLoadAddress;
        }
    }

    return 0; // Module not found
}

void patch(mach_vm_address_t address, void* buffer, mach_vm_size_t size)  {
    
    vm_protect(task, address, size, false, VM_PROT_READ | VM_PROT_WRITE | VM_PROT_EXECUTE);
    mach_vm_write(task, address, (vm_offset_t)buffer, size);
    mach_vm_protect(task, address, size, false, VM_PROT_READ | VM_PROT_EXECUTE);

}

void nop(mach_vm_address_t address, mach_vm_size_t size)  {
    
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
