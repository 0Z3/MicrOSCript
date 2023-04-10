#ifndef MICROSCRIPT_H
#define MICROSCRIPT_H

#include "Arduino.h"
#include "ose_conf.h"
#include "libose/ose.h"

#define MICROSCRIPT_VMX_SIZE 8192

const int32_t microscript_self_offset
= (OSE_BUNDLE_HEADER_LEN + 4 + 8 + 4 + 4);

class MicrOSCript
{
protected:
    int32_t nbytes;
    char *bytes;
    ose_bundle bundle, osevm,
        vm_i, vm_s, vm_e, vm_c, vm_d, vm_x, vm_l;

public:
    MicrOSCript(void);
    void init(int32_t vm_x_size
              = MICROSCRIPT_VMX_SIZE);

    ose_bundle input(void) {return vm_i;};
    ose_bundle stack(void) {return vm_s;};
    ose_bundle env(void) {return vm_e;};
    ose_bundle control(void) {return vm_c;};
    ose_bundle dump(void) {return vm_d;};
    ose_bundle x(void) {return vm_x;};
    ose_bundle stdlib(void) {return vm_l;};

    void eval(void);
    void bindfn(ose_bundle bundle,
                const char * const addr,
                void (*fn)(ose_bundle));
    void bindfn(const char * const addr,
                void (*fn)(ose_bundle));

    void printstack(ose_bundle vm_s);
    
private:

};

#endif
