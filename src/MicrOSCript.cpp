#include "MicrOSCript.h"
#include "ose_conf.h"
#include "libose/ose_assert.h"
#include "libose/ose_util.h"
#include "libose/ose_context.h"
#include "libose/ose_stackops.h"
#include "libose/ose_vm.h"
#include "libose/ose_print.h"
#include "o.se.stdlib/ose_stdlib.h"

void MicrOSCript::init(int32_t nbytes,
                       char *bytes,
                       int32_t vm_x_size)
{
    Serial.printf("%s: nbytes: %d, bytes: %d\n", __func__, nbytes, bytes);
    memset(bytes, 0, nbytes);
    Serial.printf("%s: memset done\n", __func__);
    bundle = ose_newBundleFromCBytes(nbytes, bytes);
    Serial.printf("%s: bundle: %p\n", __func__, ose_getBundlePtr(bundle));
    osevm = osevm_init(bundle);
    Serial.printf("%s: osevm_init done\n", __func__);
    vm_i = OSEVM_INPUT(osevm);
    vm_s = OSEVM_STACK(osevm);
    vm_e = OSEVM_ENV(osevm);
    vm_c = OSEVM_CONTROL(osevm);
    vm_d = OSEVM_DUMP(osevm);
    ose_pushContextMessage(osevm, vm_x_size, "/_x");
    vm_x = ose_enter(osevm, "/_x");
    ose_stdlib_init(osevm);
    vm_l = ose_enter(osevm, "/_l");
}

MicrOSCript::MicrOSCript(int32_t nbytes, char *bytes)
{
    init(nbytes, bytes, MICROSCRIPT_VMX_SIZE);
}

MicrOSCript::MicrOSCript(int32_t nbytes,
                         char *bytes,
                         int32_t vm_x_size)
{
    init(nbytes, bytes, vm_x_size);
}

void MicrOSCript::test(void)
{
    ose_bundle vm_i = OSEVM_INPUT(osevm);
    ose_bundle vm_s = OSEVM_STACK(osevm);
    ose_pushString(vm_i, "/!/_l/ADD");
    ose_pushString(vm_i, "/,/i/10");
    ose_pushString(vm_i, "/,/i/20");
    osevm_run(osevm);
    int32_t n = ose_pprintBundle(vm_s, NULL, 0);
    int32_t pn = ose_pnbytes(n);
    ose_pushBlob(vm_s, pn, NULL);
    char *p = ose_peekBlob(vm_s) + 4;
    ose_pprintBundle(vm_s, p, pn);
    ose_pushInt32(vm_s, OSETT_STRING);
    ose_blobToType(vm_s);
    const char * const str = ose_peekString(vm_s);
    Serial.printf("%s\n", str);
    ose_clear(vm_s);
}
