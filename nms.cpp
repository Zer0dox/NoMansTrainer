//
// Created by Zyy on 8/15/23.
//

#include <iostream>
#include <string>
#include <unistd.h>
#include "nms.h"

nms::nms()  {

    procName = reinterpret_cast<const std::string *>(L"No Man's Sky");
    init();

}

void nms::init() {

    std::cout << "Init No Man's Sky trainer..." << std::endl;
    procID = FindPIDByProcessName(*procName);
    while(!procID) {

        std::cout << "Please ensure No Man's Sky is running before starting the trainer." << std::endl;
        sleep(5);
        std::cout << "\033[H\033[2J"; // Clear the screen
        procID = FindPIDByProcessName(*procName);

    }
    std::cout << "\033[H\033[2J";

    // Initialize handle for process
    handle = task_for_pid(mach_task_self(), procID, &task);
    baseAddr = GetMainModuleBaseAddress(procID);

}

mach_vm_address_t nms::getUnitAddr() const {

    std::vector<unsigned int> offsets{ 0x029766E0, 0x310, 0x250, 0x38, 0xEE8 };
    return FindDynamicAddr(baseAddr, offsets);

}