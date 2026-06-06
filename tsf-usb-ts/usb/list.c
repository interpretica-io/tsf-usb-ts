/** @file
 * @brief USB Group
 *
 * Enumerate the agent's USB devices and read one back in full. The host
 * decides how many devices there are, so this does not assert a count -
 * it exercises the enumeration and descriptor-dump path and checks what
 * comes back is self-consistent. A host with no USB devices is a clean
 * pass (the RPC path still ran).
 *
 * Copyright (C) 2026 Interpretica Unipessoal Lda
 */

#define TE_TEST_NAME    "usb/list"

#include "te_config.h"
#include "tapi_test.h"
#include "te_string.h"
#include "te_vector.h"

#include "tapi_usb.h"
#include "tsapi_usb.h"

int
main(int argc, char **argv)
{
    tsapi_usb_session sess;
    te_vec devices = TE_VEC_INIT(tapi_usb_device);
    const tapi_usb_device *dev;
    unsigned int n;

    TEST_START;

    TEST_STEP("Open a session to the agent");
    CHECK_RC(tsapi_usb_session_init(&sess, "pco_usb_list"));

    TEST_STEP("Enumerate the USB devices");
    CHECK_RC(tapi_usb_list(sess.pco, &devices));
    n = te_vec_size(&devices);
    RING("the agent has %u USB device(s)", n);

    if (n == 0)
    {
        TEST_STEP("No devices attached - nothing more to check");
        TEST_SUCCESS;
    }

    TEST_STEP("Every device is self-consistent, and dumps in full");
    TE_VEC_FOREACH(&devices, dev)
    {
        te_string dump = TE_STRING_INIT;
        te_errno rc;

        tapi_usb_device_log(dev);

        /* VID/PID are 16-bit; the class spells out to something. */
        if (tapi_usb_class2str(dev->dev_class) == NULL)
            TEST_VERDICT("device %04x:%04x has no class string",
                         dev->vid, dev->pid);

        /* The detailed dump of the same bus/address must succeed. */
        rc = tapi_usb_device_dump(sess.pco, dev->bus, dev->addr, &dump);
        if (rc != 0)
        {
            te_string_free(&dump);
            TEST_VERDICT("dumping %04x:%04x (bus %d addr %d) failed",
                         dev->vid, dev->pid, dev->bus, dev->addr);
        }
        if (dump.len == 0 || strstr(te_string_value(&dump), "device\t") == NULL)
        {
            te_string_free(&dump);
            TEST_VERDICT("the dump of %04x:%04x has no device line",
                         dev->vid, dev->pid);
        }
        RING("descriptor dump of %04x:%04x:\n%s", dev->vid, dev->pid,
             te_string_value(&dump));
        te_string_free(&dump);
    }

    TEST_SUCCESS;

cleanup:
    tapi_usb_list_free(&devices);
    tsapi_usb_session_fini(&sess);
    TEST_END;
}
