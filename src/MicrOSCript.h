#ifndef MICROSCRIPT_H
#define MICROSCRIPT_H

#include "Arduino.h"
#include "ose_conf.h"
#include "libose/ose.h"

#define MICROSCRIPT_VMX_SIZE 8192

class MicrOSCript
{
public:
    MicrOSCript(int32_t nbytes, char *bytes);
    MicrOSCript(int32_t nbytes, char *bytes, int32_t vm_x_size);
    ose_bundle bundle, osevm,
        vm_i, vm_s, vm_e, vm_c, vm_d, vm_x, vm_l;

    void test(void);
private:
    void init(int32_t nbytes,
              char *bytes,
              int32_t vm_x_size);
};

#endif
