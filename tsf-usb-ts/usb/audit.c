/** @file
 * @brief USB Group
 *
 * Read the agent's USB posture with tapi_usb_audit() and gate on it.
 * What the findings are depends on what is attached, so this does not
 * assert a particular finding; it checks the audit runs, produces a
 * well-formed report, and fails only when something at least HIGH is
 * present (a BadUSB input+storage composite is the one that would) -
 * the shape a real gate takes.
 *
 * Copyright (C) 2026 Interpretica Unipessoal Lda
 */

#define TE_TEST_NAME    "usb/audit"

#include "te_config.h"
#include "tapi_test.h"
#include "te_string.h"

#include "tapi_cybersec.h"
#include "tapi_usb_audit.h"
#include "tsapi_usb.h"

int
main(int argc, char **argv)
{
    tsapi_usb_session sess;
    tapi_cybersec_report report;
    te_string verdict = TE_STRING_INIT;
    bool report_ready = false;

    TEST_START;

    TEST_STEP("Open a session to the agent");
    CHECK_RC(tsapi_usb_session_init(&sess, "pco_usb_audit"));

    TEST_STEP("Read the USB posture into a report");
    tapi_cybersec_report_init(&report);
    report_ready = true;
    CHECK_RC(tapi_usb_audit(sess.pco, NULL, &report));
    tapi_cybersec_report_log(&report);

    TEST_STEP("The report is well-formed: at least the inventory line");
    /*
     * Whatever is attached, the audit adds either one usb.device-present
     * per device or a single usb.none - so an empty report means the
     * audit did not run as it should.
     */
    if (tapi_cybersec_report_count(&report, TAPI_CYBERSEC_SEV_INFO) == 0)
        TEST_VERDICT("the USB audit produced no findings at all");

    TEST_STEP("Gate: fail on anything at least HIGH (e.g. a BadUSB stick)");
    if (tapi_cybersec_report_verdict(&report, TAPI_CYBERSEC_SEV_HIGH,
                                     &verdict))
    {
        TEST_VERDICT("%s", verdict.ptr);
    }

    TEST_SUCCESS;

cleanup:
    te_string_free(&verdict);
    if (report_ready)
        tapi_cybersec_report_free(&report);
    tsapi_usb_session_fini(&sess);
    TEST_END;
}
