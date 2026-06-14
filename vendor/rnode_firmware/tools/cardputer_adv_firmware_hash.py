#!/usr/bin/env python3

import argparse
import hashlib
import sys
import time

try:
    import serial
except ImportError:
    serial = None


FEND = 0xC0
FESC = 0xDB
TFEND = 0xDC
TFESC = 0xDD

CMD_FW_HASH = 0x58
CMD_HASHES = 0x60
CMD_RESET = 0x55
CMD_RESET_BYTE = 0xF8


def firmware_hash_from_file(path):
    with open(path, "rb") as handle:
        data = handle.read()

    if len(data) > 32:
        calculated = hashlib.sha256(data[:-32]).digest()
        embedded = data[-32:]
        if calculated == embedded:
            return embedded

    return hashlib.sha256(data).digest()


def parse_hash(value):
    try:
        digest = bytes.fromhex(value)
    except ValueError as exc:
        raise SystemExit(f"invalid hash hex: {exc}")

    if len(digest) != 32:
        raise SystemExit("firmware hash must be exactly 32 bytes / 64 hex characters")

    return digest


def escape_kiss(payload):
    escaped = bytearray()
    for byte in payload:
        if byte == FEND:
            escaped.extend((FESC, TFEND))
        elif byte == FESC:
            escaped.extend((FESC, TFESC))
        else:
            escaped.append(byte)
    return bytes(escaped)


def frame(command, payload=b""):
    return bytes((FEND, command)) + escape_kiss(payload) + bytes((FEND,))


def read_frames(port, timeout):
    deadline = time.monotonic() + timeout
    in_frame = False
    escaped = False
    command = None
    payload = bytearray()

    while time.monotonic() < deadline:
        chunk = port.read(1)
        if not chunk:
            continue

        byte = chunk[0]
        if byte == FEND:
            if in_frame and command is not None:
                yield command, bytes(payload)
            in_frame = True
            escaped = False
            command = None
            payload = bytearray()
            continue

        if not in_frame:
            continue

        if command is None:
            command = byte
            continue

        if byte == FESC:
            escaped = True
            continue

        if escaped:
            if byte == TFEND:
                byte = FEND
            elif byte == TFESC:
                byte = FESC
            escaped = False

        payload.append(byte)


def require_serial():
    if serial is None:
        raise SystemExit("pyserial is required for serial hash writes")


def read_running_firmware_hash(port, timeout):
    port.reset_input_buffer()
    port.write(frame(CMD_HASHES, bytes((0x02,))))
    port.flush()

    for command, payload in read_frames(port, timeout):
        if command == CMD_HASHES and len(payload) == 33 and payload[0] == 0x02:
            return payload[1:]

    raise SystemExit("timed out waiting for the device's running firmware hash")


def write_firmware_hash(port, digest, reset):
    port.write(frame(CMD_FW_HASH, digest))
    port.flush()
    time.sleep(0.02)

    if reset:
        port.write(frame(CMD_RESET, bytes((CMD_RESET_BYTE,))))
        port.flush()


def main():
    parser = argparse.ArgumentParser(
        description="Set the RNode firmware hash for the Cardputer Adv build."
    )
    source = parser.add_mutually_exclusive_group()
    source.add_argument(
        "--firmware",
        default="build/esp32.esp32.esp32s3/RNode_Firmware.ino.bin",
        help="ESP32 app binary to hash",
    )
    source.add_argument("--hash", help="64-character firmware hash hex string")
    source.add_argument(
        "--from-device",
        action="store_true",
        help="read the running firmware hash from the device and write it back as target",
    )
    parser.add_argument("--port", help="serial port for the Cardputer Adv")
    parser.add_argument("--baud", type=int, default=115200)
    parser.add_argument("--timeout", type=float, default=3.0)
    parser.add_argument("--settle", type=float, default=1.5, help="seconds to wait after opening serial")
    parser.add_argument("--reset", action="store_true", help="reset after writing the hash")
    parser.add_argument("--print-only", action="store_true", help="print the hash and do not write")
    args = parser.parse_args()

    if args.hash:
        digest = parse_hash(args.hash)
    elif args.from_device:
        if args.print_only:
            raise SystemExit("--from-device requires a serial write session")
        require_serial()
        if not args.port:
            raise SystemExit("--port is required with --from-device")
        with serial.Serial(args.port, args.baud, timeout=0.1, write_timeout=args.timeout) as port:
            time.sleep(args.settle)
            digest = read_running_firmware_hash(port, args.timeout)
            write_firmware_hash(port, digest, args.reset)
        print(digest.hex())
        return 0
    else:
        digest = firmware_hash_from_file(args.firmware)

    if args.print_only:
        print(digest.hex())
        return 0

    require_serial()
    if not args.port:
        raise SystemExit("--port is required unless --print-only is used")

    with serial.Serial(args.port, args.baud, timeout=0.1, write_timeout=args.timeout) as port:
        time.sleep(args.settle)
        write_firmware_hash(port, digest, args.reset)

    print(digest.hex())
    return 0


if __name__ == "__main__":
    sys.exit(main())
