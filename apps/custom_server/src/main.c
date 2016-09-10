#include <sel4/sel4.h>
#include "pc99/serial.h"
#include "alloc/cslot_ao.h"
#include "alloc/untyped.h"
#include "alloc/object.h"
#include "errno.h"
#include "alloc/mem_page.h"
#include "alloc/mem_vspace.h"
#include "alloc/mem_fx.h"

void _assert_fail_static(const char *fail) {
    debug_println(fail);
    seL4_DebugHalt();
    while (1) {
        // loop forever
    }
}

static void print_range(const char *name, seL4_SlotRegion region) {
    debug_println(name);
    debug_printdec(region.start);
    debug_printdec(region.end);
}

size_t strlen(const char *ptr) {
    const char *cur = ptr;
    while (*cur) {
        cur++;
    }
    return cur - ptr;
}

void main(void) {
    debug_println("starting...");
    seL4_Error err = mem_fx_init();
    debug_println(err_to_string(err));
    const char *source = "Hello, serial world 2!\n";
    char *buf = mem_fx_alloc(64);
    char *dest = buf;
    do {
        *dest++ = *source;
    } while (*source++);
    serial_dev_t stdout;
    serial_init(SERIAL_COM1, seL4_CapIOPort, &stdout);
    serial_write(&stdout, buf, (size_t) strlen(buf));
}

extern char __executable_start;
// referenced from mem_page.c
seL4_CPtr current_vspace = seL4_CapInitThreadVSpace;

void premain(seL4_BootInfo *bi) {
    seL4_InitBootInfo(bi);
    mem_vspace_setup((bi->userImageFrames.end - bi->userImageFrames.start) * PAGE_SIZE, bi->ipcBuffer);
    print_range("userImageFrames", bi->userImageFrames);
    print_range("userImagePTs", bi->userImagePTs);
    print_range("userImagePDs", bi->userImagePDs);
    cslot_ao_setup(seL4_CapInitThreadCNode, bi->empty.start, bi->empty.end);
    assert(untyped_add_boot_memory(bi) == seL4_NoError);

    main();

    seL4_DebugHalt();
    while (1) {
        // do nothing
    }
}
