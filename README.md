# tsf-usb-ts

A Test Environment suite that exercises
[tsf-usb](https://github.com/interpretica-io/tsf-usb) (`tapi_usb`)
against the agent it runs on — enumerating its USB devices over
libusb-1.0 and reading a security posture over them.

| Test | What it checks |
|---|---|
| `list` | `tapi_usb_list()` returns a self-consistent snapshot; each device logs (VID/PID, classes, strings) and its full config/interface/endpoint tree dumps via `tapi_usb_device_dump()` |
| `audit` | `tapi_usb_audit()` produces a well-formed report (at least the inventory line) and the gate fails on any finding ≥ HIGH — the BadUSB input+storage composite being the one that would |

What is attached is the host's to decide, so **the suite asserts no
device count**: a host with nothing plugged in is a clean pass (the RPC
and enumeration path still ran), and the audit gates only on HIGH-or-
worse findings. Enumeration runs on the agent, in its RPC server, over
libusb-1.0 — read-only, no `lsusb`.

## Running it

Needs Docker and `test-environment` as a sibling directory:

```bash
./scripts/run.sh docker guess --cfg=localhost
```

On the `localhost` configuration the agent is the build container, so it
must carry **libusb-1.0 with its headers** (`libusb-1.0-0-dev`, already
in the suite's Dockerfile). `tapi_usb`'s posture reports through
tsf-cybersec, so the suite also builds the tsf-cybersec chain
(tsf-cybersec → tsf-kernel → tsf-devtool); their refs are in
`conf/external.yml`. Name a host in `conf/rcf.conf` and the same tests
run against it (where real USB devices make the suite more interesting).

`conf/builder.conf.lock` pins the commits actually built (empty until
the first build). The Builder clones the tsf-* repositories itself, so
the checkouts beside this suite are not what a run compiles.

## Status

**Not yet run.** This suite was written alongside tsf-usb but has not
been built or executed — there was no TE toolchain. tsf-usb's libusb-1.0
usage was syntax-checked against the real headers (1.0.29), but the C
was not compiled and no live enumeration was done. The first run should
expect the ordinary first-build fixes. A container typically has few or
no USB devices, so `list` exercises the empty/one-device paths and
`audit` the clean-report path; point the suite at a real host to see
findings.
