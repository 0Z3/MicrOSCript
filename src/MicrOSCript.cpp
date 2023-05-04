#include "MicrOSCript.h"

#include "ose_conf.h"
#include "libose/ose_assert.h"
#include "libose/ose_util.h"
#include "libose/ose_context.h"
#include "libose/ose_stackops.h"
#include "libose/ose_vm.h"
#include "libose/ose_print.h"
#include "o.se.stdlib/ose_stdlib.h"
#include "o.se.oscript/ose_oscript.h"

MicrOSCript::MicrOSCript(void)
    : nbytes(0), bytes(NULL) {}

void MicrOSCript::init(int32_t vm_x_size)
{
    nbytes = heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT);
    bytes = (char *)malloc(nbytes);
    memset(bytes, 0, nbytes);
    bundle = ose_newBundleFromCBytes(nbytes, bytes);
    osevm = osevm_init(bundle);
    vm_i = OSEVM_INPUT(osevm);
    vm_s = OSEVM_STACK(osevm);
    vm_e = OSEVM_ENV(osevm);
    vm_c = OSEVM_CONTROL(osevm);
    vm_d = OSEVM_DUMP(osevm);
    ose_pushContextMessage(osevm, MICROSCRIPT_VMX_SIZE, "/_x");
    vm_x = ose_enter(osevm, "/_x");
    // /self should be first so we can cache its offset
    ose_pushMessage(vm_x, "/self", strlen("/self"),
                    1, OSETT_ALIGNEDPTR, this);
    ose_stdlib_init(osevm);
    ose_oscript_init(osevm);
    ose_appendBundle(vm_s, vm_x);
    vm_l = ose_enter(osevm, "/_l");
}

void uoscript_lookup(ose_bundle osevm)
{
    ose_bundle vm_s = OSEVM_STACK(osevm);
    ose_bundle vm_e = OSEVM_ENV(osevm);
    ose_bundle vm_x = ose_enter(osevm, "/_x");
    ose_bundle bndlenv = vm_e;
    int explicitbndl = 0;

    if(ose_peekType(vm_s) != OSETT_MESSAGE
       || !ose_isStringType(ose_peekMessageArgType(vm_s)))
    {
        /* this is probably an error */
        return;
    }
    const char * const address = ose_peekString(vm_s);
    if(address[3] == '/')
    {
        const char buf[4] = {
            address[0],
            address[1],
            address[2],
                        0
        };
        int32_t o = ose_getContextMessageOffset(osevm, buf);
        if(o >= 0)
        {
            bndlenv = ose_enterBundleAtOffset(osevm, o);
            explicitbndl = 1;
        }
    }

    if(explicitbndl)
    {
        int32_t mo = ose_getFirstOffsetForMatch(bndlenv, address + 3);
        if(mo >= OSE_BUNDLE_HEADER_LEN)
        {
            ose_drop(vm_s);
            ose_copyElemAtOffset(mo, bndlenv, vm_s);
            return;
        }
    }
    else
    {
        int32_t mo = ose_getFirstOffsetForMatch(vm_e, address);
        if(mo >= OSE_BUNDLE_HEADER_LEN)
        {
            ose_drop(vm_s);
            ose_copyElemAtOffset(mo, vm_e, vm_s);
            return;
        }
        /* if it wasn't present in env, lookup in _x */
        mo = ose_getFirstOffsetForMatch(vm_x, address);
        if(mo >= OSE_BUNDLE_HEADER_LEN)
        {
            ose_drop(vm_s);
            ose_copyElemAtOffset(mo, vm_x, vm_s);
            return;
        }
    }
}

void MicrOSCript::eval(void)
{
    osevm_run(osevm);
}

void MicrOSCript::bindfn(ose_bundle bundle,
                         const char * const addr,
                         void (*fn)(ose_bundle))
{
    ose_pushMessage(bundle, addr, strlen(addr),
                    1, OSETT_ALIGNEDPTR, fn);
}

void MicrOSCript::bindfn(const char * const addr,
                         void (*fn)(ose_bundle))
{
    bindfn(vm_x, addr, fn);
}

void MicrOSCript::printstack(ose_bundle vm_s)
{
    if(0)
    {
        const char * const b = ose_getBundlePtr(vm_s);
        for(int32_t i = 0;
            i < ose_readSize(vm_s);
            ++i)
        {
            Serial.printf("%d: %d %c 0x%x\n",
                          i, b[i], (unsigned char)(b[i]), b[i]);
        }
        Serial.println("done");
        // ose_drop(vm_s);
    }
    if(1)
    {
        int32_t n = ose_pprintBundle(vm_s, NULL, 0);
        int32_t pn = ose_pnbytes(n);
        ose_pushBlob(vm_s, pn, NULL);
        char *p = ose_peekBlob(vm_s) + 4;
        ose_pprintBundle(vm_s, p, pn);
        ose_pushInt32(vm_s, OSETT_STRING);
        ose_blobToType(vm_s);
        const char * const str = ose_peekString(vm_s);
        Serial.printf("%s\n", str);
        ose_drop(vm_s);
    }
}

void uoscript_evalType(ose_bundle osevm)
{
    ose_bundle vm_s = OSEVM_STACK(osevm);
    if(ose_peekType(vm_s) == OSETT_BUNDLE)
    {
        char buf[OSEVM_EVALTYPE_ADDR_LEN];
        int32_t len = snprintf(buf, OSEVM_EVALTYPE_ADDR_LEN,
                               OSEVM_EVALTYPE_ADDR, OSETT_BUNDLE);
        ose_pushString(vm_s, buf);
        OSEVM_LOOKUP(osevm);
        if(ose_peekMessageArgType(vm_s) == OSETT_BLOB)
        {
            osevm_apply(osevm);
        }
        else
        {
            ose_drop(vm_s);
        }
    }
    else
    {
        char tt = ose_peekMessageArgType(vm_s);
        char buf[OSEVM_EVALTYPE_ADDR_LEN];
        int32_t len = snprintf(buf, OSEVM_EVALTYPE_ADDR_LEN,
                               OSEVM_EVALTYPE_ADDR, tt);
        ose_pushString(vm_s, buf);
        OSEVM_LOOKUP(osevm);
        if(ose_peekMessageArgType(vm_s) == OSETT_BLOB)
        {
            osevm_apply(osevm);
        }
        else
        {
            ose_drop(vm_s);
        }
    }
}
