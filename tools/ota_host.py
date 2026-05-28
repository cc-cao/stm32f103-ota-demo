#!/usr/bin/env python3
"""
STM32 OTA 上位机
================
和板子 (apps/app/ota.c) 对接，通过 USART1 把固件 .bin 推到 C 槽。

帧格式:
  [SOF=0xAA][LEN:2 LE][CMD:1][SEQ:1][PAYLOAD:N][CRC16:2 LE]
  LEN = CMD+SEQ+PAYLOAD+CRC 的总字节数
  CRC 覆盖 LEN..PAYLOAD

用法:
  pip install pyserial
  python ota_host.py /dev/tty.usbserial-XXXX firmware.bin
"""

import argparse
import struct
import sys
import time
import zlib

import serial

SOF = 0xAA
CMD_BEGIN = 0x02
CMD_DATA = 0x03
CMD_END = 0x04
CMD_ACK = 0x80
CMD_NAK = 0xEE

CHUNK = 252  # 设备端 OTA_MAX_PAYLOAD=256，DATA 帧 payload = 4(offset) + CHUNK
ACK_TIMEOUT = 2.0
MAX_RETRY = 5


def crc16_ccitt(data: bytes) -> int:
    crc = 0xFFFF
    for b in data:
        crc ^= b << 8
        for _ in range(8):
            crc = ((crc << 1) ^ 0x1021) & 0xFFFF if (crc & 0x8000) else (crc << 1) & 0xFFFF
    return crc


def build_frame(cmd: int, seq: int, payload: bytes) -> bytes:
    len_field = 1 + 1 + len(payload) + 2  # cmd + seq + payload + crc
    body = struct.pack("<HBB", len_field, cmd, seq) + payload
    crc = crc16_ccitt(body)
    return bytes([SOF]) + body + struct.pack("<H", crc)


def parse_frame(buf: bytearray):
    """从 buf 头部解一帧。返回 (cmd, seq, payload, consumed) 或 None (数据不够)。"""
    while buf and buf[0] != SOF:
        del buf[0]
    if len(buf) < 5:
        return None
    len_field = buf[1] | (buf[2] << 8)
    if len_field < 4 or len_field > 1024:
        del buf[0]
        return None
    total = 3 + len_field
    if len(buf) < total:
        return None

    body = bytes(buf[1 : 1 + 2 + len_field - 2])  # LEN+CMD+SEQ+PAYLOAD
    got_crc = buf[1 + 2 + len_field - 2] | (buf[1 + 2 + len_field - 2 + 1] << 8)
    if crc16_ccitt(body) != got_crc:
        del buf[0]
        return None

    cmd = buf[3]
    seq = buf[4]
    payload = bytes(buf[5 : 3 + len_field - 2])
    del buf[:total]
    return cmd, seq, payload


class Host:
    def __init__(self, port: str, baud: int = 115200):
        self.ser = serial.Serial(port, baud, timeout=0.1)
        self.rx = bytearray()
        self.seq = 0

    def _next_seq(self) -> int:
        s = self.seq
        self.seq = (self.seq + 1) & 0xFF
        return s

    def _read_until(self, deadline: float):
        while time.monotonic() < deadline:
            chunk = self.ser.read(256)
            if chunk:
                self.rx.extend(chunk)
                frame = parse_frame(self.rx)
                if frame:
                    return frame
            else:
                time.sleep(0.005)
        return None

    def _expect_ack(self, expect_cmd: int, expect_seq: int):
        deadline = time.monotonic() + ACK_TIMEOUT
        while True:
            frame = self._read_until(deadline)
            if frame is None:
                return None
            cmd, seq, payload = frame
            if cmd == CMD_NAK:
                return ("NAK", payload[0] if payload else 0xFF)
            if cmd == (CMD_ACK | expect_cmd) and seq == expect_seq:
                return ("ACK", payload)

    def transact(self, cmd: int, payload: bytes):
        for attempt in range(MAX_RETRY):
            seq = self._next_seq()
            self.ser.write(build_frame(cmd, seq, payload))
            result = self._expect_ack(cmd, seq)
            if result is None:
                print(f"  timeout, retry {attempt+1}", file=sys.stderr)
                continue
            kind, info = result
            if kind == "ACK":
                return info
            print(f"  NAK 0x{info:02x}, retry {attempt+1}", file=sys.stderr)
        raise RuntimeError(f"transact cmd 0x{cmd:02x} failed after {MAX_RETRY} retries")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("port")
    ap.add_argument("firmware")
    ap.add_argument("--baud", type=int, default=115200)
    args = ap.parse_args()

    with open(args.firmware, "rb") as f:
        image = f.read()
    total = len(image)
    crc = zlib.crc32(image) & 0xFFFFFFFF
    print(f"firmware: {total} bytes, crc32 = 0x{crc:08x}")

    host = Host(args.port, args.baud)

    print("BEGIN...")
    host.transact(CMD_BEGIN, struct.pack("<III", total, crc, 0))
    print("  ok, flash erased on device")

    print("DATA...")
    offset = 0
    t0 = time.monotonic()
    while offset < total:
        n = min(CHUNK, total - offset)
        block = image[offset : offset + n]
        host.transact(CMD_DATA, struct.pack("<I", offset) + block)
        offset += n
        pct = offset * 100 // total
        bar = "#" * (pct // 2) + "." * (50 - pct // 2)
        print(f"\r  [{bar}] {offset}/{total}", end="", flush=True)
    print()
    elapsed = time.monotonic() - t0
    print(f"  {total/elapsed:.0f} B/s")

    print("END...")
    host.transact(CMD_END, b"")
    print("  device will reset to bootloader and apply update")


if __name__ == "__main__":
    main()
