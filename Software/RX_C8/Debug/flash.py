#!/usr/bin/env python3
"""
USB CDC bootloader flash tool for STM32F103C8
Usage: python flash.py <COMx> <firmware.bin>
"""
import serial
import sys
import struct
import time

BAUD      = 115200
PAGE_SIZE = 1024   # must match FLASH_PAGE_SIZE on board
USB_PKT   = 64     # USB full-speed bulk max packet size
T_READY   = 5.0    # seconds to wait for READY
T_PAGE    = 5.0    # seconds to wait for K (per page ack)


def flash(port: str, firmware_path: str) -> bool:
    with open(firmware_path, "rb") as f:
        firmware = f.read()

    fw_size = len(firmware)
    pages   = (fw_size + PAGE_SIZE - 1) // PAGE_SIZE
    print(f"Firmware : {firmware_path}")
    print(f"Size     : {fw_size} bytes  ({pages} pages)")

    with serial.Serial(port, BAUD, timeout=T_PAGE) as ser:
        time.sleep(1.0)
        ser.reset_input_buffer()

        print(f"\nConnected to {port}. Sending trigger...")
        ser.write(b"U")

        resp = ser.read_until(b"\n")
        if b"READY" not in resp:
            print(f"ERROR: expected READY, got {resp!r}")
            return False
        print("Bootloader ready.\n")

        ser.write(struct.pack("<I", fw_size))

        for page in range(pages):
            offset     = page * PAGE_SIZE
            page_data  = firmware[offset : offset + PAGE_SIZE]

            # send page in USB-packet-sized pieces
            for i in range(0, len(page_data), USB_PKT):
                ser.write(page_data[i : i + USB_PKT])

            sent = offset + len(page_data)
            pct  = sent * 100 // fw_size
            bar  = "#" * (pct // 4)
            print(f"\r  [{bar:<25}] {pct:3}%  {sent}/{fw_size} B", end="", flush=True)

            resp = ser.read_until(b"\n")

            if b"DONE" in resp:
                print(f"\r  [{'#'*25}] 100%  {fw_size}/{fw_size} B")
                print("\nDone! Board is jumping to app.")
                return True
            elif b"ERROR" in resp:
                print(f"\nERROR: board reported a flash error on page {page}.")
                return False
            elif b"K" not in resp:
                print(f"\nERROR: unexpected response on page {page}: {resp!r}")
                return False

        # all chunks sent — wait for final DONE (shouldn't normally reach here)
        ser.timeout = T_PAGE
        resp = ser.read_until(b"\n")
        if b"DONE" in resp:
            print("\nDone! Board is jumping to app.")
            return True

        print(f"ERROR: no DONE after all pages: {resp!r}")
        return False


if __name__ == "__main__":
    if len(sys.argv) != 3:
        print(f"Usage: python {sys.argv[0]} <COMx> <firmware.bin>")
        sys.exit(1)

    ok = flash(sys.argv[1], sys.argv[2])
    sys.exit(0 if ok else 1)
