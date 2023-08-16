//
// Created by Corey Richardson on 8/15/23.
//

#ifndef NOMANSTRAINER_NMS_H
#define NOMANSTRAINER_NMS_H
#include "memproc.h"
#include <string>

class nms
{

private:

    pid_t procID{};
    uintptr_t baseAddr;
    void init();

public:

    const std::string* procName{};
    kern_return_t handle;
    mach_vm_address_t getUnitAddr() const;
    nms();

};
#endif //NOMANSTRAINER_NMS_H
