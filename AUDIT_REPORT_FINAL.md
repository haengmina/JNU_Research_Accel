# ZYBO MEMACC PROJECT — FINAL AUDIT REPORT
**Project:** Zybo MemAcc — Interrupt-Based Memory Copy Accelerator  
**Board:** Zybo Z7-10 (Zynq-7000 SoC)  
**Toolchain:** Vivado / Vitis 2022.1  
**Audit Date:** 2026-02-24  
**Auditor:** AI Enterprise — Audit Team  
**Audit Waves Completed:** Wave 1 (Static), Wave 2 (Driver + Linker), Wave 3 (HW-SW Coherency)

---

## EXECUTIVE SUMMARY

The Zybo MemAcc project implements a hardware-accelerated memory copy function using Vivado HLS, integrated with the Zynq PS via an interrupt-driven mechanism. Three audit waves were conducted covering source code, linker configuration, hardware-software interface coherency, and deployment verification.

**Overall Verdict:** ⚠️ **CONDITIONAL PASS** — Two corrective actions must be resolved before production deployment.

---

## SECTION 1 — IMMEDIATE ACTIONS (BLOCKING)

> These findings represent defects that **must be corrected** before the project is considered production-ready.

---

### [IA-01] 64-bit Pointer Truncation Risk in `memcopy_accel.c`

| Field        | Detail |
|--------------|--------|
| **File**     | `MemAcc_app/memcopy_accel.c` |
| **Severity** | HIGH |
| **Wave**     | Wave 2 — Driver Sync |
| **Status**   | ⚠️ Requires Correction |

**Finding:**

The driver API accepts `uint32_t` for all address parameters:

```c
// memcopy_accel.h — Line 51
void memcopy_accel_start(uint32_t src_addr, uint32_t dst_addr, uint32_t len);
```

In `main.c`, heap-allocated pointers are cast directly to `uint32_t` before being passed to the accelerator:

```c
// main.c — Line 151
memcopy_accel_start((uint32_t)src_buf, (uint32_t)dst_buf, BYTE_LEN);
```

On the current Zynq-7000 (32-bit ARM Cortex-A9), this is safe because all physical addresses fit within 32 bits. However, the HLS-generated register map includes upper-32-bit address registers at offsets `+0x14` (SRC_HIGH) and `+0x20` (DST_HIGH) which are **never written**, leaving them at their reset value of `0x00000000`. This is only correct by coincidence on a 32-bit platform.

**Risk:** If this driver is ported to a 64-bit platform (e.g., Zynq UltraScale+ MPSoC with Cortex-A53), pointer truncation will cause the accelerator to read/write incorrect memory addresses, resulting in silent data corruption or a system hang.

**Recommended Correction:**

```c
// memcopy_accel.h — Updated prototype
void memcopy_accel_start(uintptr_t src_addr, uintptr_t dst_addr, uint32_t len);

// memcopy_accel.c — Updated implementation
void memcopy_accel_start(uintptr_t src_addr, uintptr_t dst_addr, uint32_t len)
{
    /* Write lower 32 bits */
    Xil_Out32(base_addr + MEMCOPY_ACCEL_SRC_OFFSET,       (uint32_t)(src_addr & 0xFFFFFFFFu));
    Xil_Out32(base_addr + MEMCOPY_ACCEL_SRC_OFFSET + 0x4, (uint32_t)(src_addr >> 32));

    Xil_Out32(base_addr + MEMCOPY_ACCEL_DST_OFFSET,       (uint32_t)(dst_addr & 0xFFFFFFFFu));
    Xil_Out32(base_addr + MEMCOPY_ACCEL_DST_OFFSET + 0x4, (uint32_t)(dst_addr >> 32));

    Xil_Out32(base_addr + MEMCOPY_ACCEL_LEN_OFFSET, len);
    Xil_Out32(base_addr + MEMCOPY_ACCEL_CTRL_OFFSET, MEMCOPY_AP_START_MASK);
}
```

> **Note:** The `base_addr` static variable in `memcopy_accel.c` (Line 5) is also `uint32_t` and should be promoted to `uintptr_t` for full consistency.

---

### [IA-02] Critical Heap Size Mismatch in `lscript.ld`

| Field        | Detail |
|--------------|--------|
| **File**     | `MemAcc_app/src/lscript.ld` |
| **Severity** | CRITICAL |
| **Wave**     | Wave 2 — Linker Script |
| **Status**   | ⚠️ Requires Correction |

**Finding:**

The linker script defines a heap of only **8 KB**:

```ld
/* lscript.ld — Line 7 */
_HEAP_SIZE = DEFINED(_HEAP_SIZE) ? _HEAP_SIZE : 0x2000;   /* 0x2000 = 8,192 bytes = 8 KB */
```

`main.c` allocates **three** 32-byte-aligned buffers of 32 KB each via `memalign()`:

```c
// main.c — Lines 123–125
const uint32_t BYTE_LEN = NUM_WORDS * 4;   /* 8192 words × 4 = 32,768 bytes = 32 KB */
uint32_t *src_buf    = (uint32_t *)memalign(32, BYTE_LEN);   /* 32 KB */
uint32_t *dst_buf    = (uint32_t *)memalign(32, BYTE_LEN);   /* 32 KB */
uint32_t *dst_buf_cpu = (uint32_t *)memalign(32, BYTE_LEN);  /* 32 KB */
```

**Total heap demand: 96 KB. Available heap: 8 KB. Deficit: 88 KB.**

The application correctly checks for allocation failure (`if (!src_buf || !dst_buf || !dst_buf_cpu)`), so it will print an error and return `-1` rather than crash silently. However, the root cause — the undersized heap — must be fixed.

**Recommended Correction:**

```ld
/* lscript.ld — Line 7 — Corrected */
_HEAP_SIZE = DEFINED(_HEAP_SIZE) ? _HEAP_SIZE : 0x20000;   /* 0x20000 = 131,072 bytes = 128 KB */
```

128 KB provides the required 96 KB plus a 32 KB safety margin for runtime library overhead and future buffer growth.

| Metric | Current | Required | Recommended |
|--------|---------|----------|-------------|
| Heap Size | 8 KB (0x2000) | 96 KB | 128 KB (0x20000) |
| DDR Available | 511 MB (0x1FF00000) | — | No constraint |

> **Note:** The Zynq DDR region in `lscript.ld` (Line 17) spans `0x100000` to `0x1FFFFFFF` (511 MB). Increasing the heap to 128 KB has zero impact on memory pressure.

---

## SECTION 2 — BEST PRACTICES (NON-BLOCKING)

> These findings are observations and recommendations that improve robustness, maintainability, and portability. They do not block the current release but should be addressed in the next development cycle.

---

### [BP-01] Duplicate `#include` in `main.c`

| Field    | Detail |
|----------|--------|
| **File** | `MemAcc_app/main.c` — Lines 9–10 |
| **Wave** | Wave 1 — Static Analysis |

`#include "xiltimer.h"` appears twice consecutively. While harmless due to include guards, it indicates a copy-paste artifact and should be cleaned up for code hygiene.

---

### [BP-02] Interrupt ID Fallback Relies on a Magic Number

| Field    | Detail |
|----------|--------|
| **File** | `MemAcc_app/main.c` — Line 20 |
| **Wave** | Wave 3 — HW-SW Coherency |

```c
#define MEMCOPY_INTR_ID 29  /* Based on xparameters.h discovery */
```

The fallback value `29` was validated against `xparameters.h` during Wave 3 and is **confirmed correct** for this hardware configuration. However, hardcoding an interrupt ID as a magic number is fragile — a Vivado re-export or IP re-address will silently break this without a compile-time error.

**Recommendation:** Ensure `xparameters.h` is always included in the BSP export and that `XPAR_FABRIC_MEMCOPY_ACCEL_0_INTR` or `XPAR_FABRIC_MEMCOPY_ACCEL_0_INTERRUPT_INTR` resolves correctly. The existing `#ifndef` guard chain (Lines 16–24) is the correct pattern; the magic number fallback should be treated as a last resort with a `#warning` directive:

```c
#else
  #warning "MEMCOPY_INTR_ID not found in xparameters.h — using hardcoded fallback 29"
  #define MEMCOPY_INTR_ID 29
#endif
```

---

### [BP-03] Base Address Fallback Also Uses a Magic Number

| Field    | Detail |
|----------|--------|
| **File** | `MemAcc_app/main.c` — Line 27 / `memcopy_accel.h` — Line 11 |
| **Wave** | Wave 3 — HW-SW Coherency |

```c
#define MEMCOPY_BASE 0x40000000u   /* main.c */
#define MEMCOPY_ACCEL_BASEADDR 0x40000000u  /* memcopy_accel.h */
```

Both values were validated against `xparameters.h` (Wave 3) and are **confirmed correct**. Same recommendation as BP-02: add a `#warning` to the fallback path to make silent mismatches visible at compile time.

---

### [BP-04] `memcopy_accel_wait_done()` Uses Busy-Wait Polling

| Field    | Detail |
|----------|--------|
| **File** | `MemAcc_app/memcopy_accel.c` — Lines 28–35 |
| **Wave** | Wave 2 — Driver Sync |

The polling path (`memcopy_accel_copy_polling`) spins on `ap_done` with no timeout:

```c
while (!memcopy_accel_is_done()) { /* busy wait */ }
```

The interrupt-driven path in `main.c` correctly implements a 2-second WFI timeout (Lines 154–165). The polling helper function has no equivalent protection and will hang indefinitely if the accelerator stalls. Add a timeout parameter or a cycle-count guard to `memcopy_accel_wait_done()` for defensive use.

---

### [BP-05] ELF Load Address Confirmed — Document in README

| Field    | Detail |
|----------|--------|
| **File** | `MemAcc_app/src/lscript.ld` — Line 17 |
| **Wave** | Wave 4 — Deployment Logs |

Deployment logs confirmed successful PS initialization and ELF load at `0x100000`, which matches the linker script DDR origin (`ORIGIN = 0x100000`). This is correct. The project `README.md` does not document the load address or the memory map. Adding a brief memory map table to the README will aid future developers and auditors.

---

## SECTION 3 — HW-SW COHERENCY VALIDATION (CONFIRMED PASS)

| Check | Expected | Actual | Result |
|-------|----------|--------|--------|
| Interrupt ID | 29 | 29 (xparameters.h) | ✅ PASS |
| Accelerator Base Address | 0x40000000 | 0x40000000 (xparameters.h) | ✅ PASS |
| DDR Origin (ELF Load) | 0x100000 | 0x100000 (deployment log) | ✅ PASS |
| GIE Register Offset | 0x04 | 0x04 (HLS-generated) | ✅ PASS |
| IER Register Offset | 0x08 | 0x08 (HLS-generated) | ✅ PASS |
| ISR Register Offset | 0x0C | 0x0C (HLS-generated) | ✅ PASS |
| ISR Read-Back Barrier | Required (Zynq AXI) | Present (main.c Line 65) | ✅ PASS |
| Cache Flush Before DMA | Required | Present (main.c Lines 145–146) | ✅ PASS |
| Cache Invalidate After DMA | Required | Present (main.c Line 173) | ✅ PASS |

---

## SECTION 4 — BUILD ARTIFACT VERIFICATION

| Metric | Value |
|--------|-------|
| ELF File | `MemAcc_app/build/MemAcc_app.elf` |
| `.text` (code) | 38,514 bytes |
| `.data` (initialized) | 2,976 bytes |
| `.bss` (zero-init) | 22,996 bytes |
| **Total (dec)** | **64,486 bytes (~63 KB)** |
| DDR Region | 511 MB — no size constraint |

Build artifact is present and within expected size bounds for a bare-metal Zynq application.

---

## SECTION 5 — AUDIT VERDICT

```
╔══════════════════════════════════════════════════════════════════╗
║                                                                  ║
║   AUDIT RESULT:  ⚠️  CONDITIONAL PASS                           ║
║                                                                  ║
║   The Zybo MemAcc project demonstrates correct hardware-         ║
║   software integration, valid interrupt handling, proper         ║
║   cache coherency management, and a confirmed deployment.        ║
║                                                                  ║
║   TWO BLOCKING ITEMS must be resolved before production:         ║
║                                                                  ║
║   [IA-01]  Promote address types to uintptr_t in driver          ║
║            (memcopy_accel.c / memcopy_accel.h)                   ║
║                                                                  ║
║   [IA-02]  Increase _HEAP_SIZE from 0x2000 to 0x20000           ║
║            in lscript.ld (8 KB → 128 KB)                        ║
║                                                                  ║
║   Upon resolution of IA-01 and IA-02, this project is           ║
║   cleared for production deployment.                             ║
║                                                                  ║
╚══════════════════════════════════════════════════════════════════╝
```

### Condition Checklist for Full Pass

- [ ] **IA-01** — `uintptr_t` promotion applied in `memcopy_accel.h` and `memcopy_accel.c`
- [ ] **IA-02** — `_HEAP_SIZE = 0x20000` applied in `lscript.ld`
- [ ] Re-build ELF and confirm `memalign()` allocations succeed at runtime
- [ ] Re-run functional test: Accelerator test PASSED + CPU test PASSED printed to UART

---

*Report generated by AI Enterprise Audit Team — Zybo MemAcc Project*  
*Audit Waves: 1 (Static) · 2 (Driver + Linker) · 3 (HW-SW Coherency) · 4 (Deployment)*
