#!/usr/bin/env python3
"""Fake 5E-HTE board for RF-calibration tests.
usage: fakerf.py <link-path> <ctrl-file>
ctrl-file (re-read every frame): "<tone_MHz|none> <level_dBFS> <gain_dB> <lo_MHz>"
Answers PING with ACK, sends RFST every 1 s and WAVD (124 pts) every 100 ms.
"""
import os, pty, sys, time, struct, math, random, tty as ttymod
sys.path.insert(0, "/home/claude/work/r3/fw/tools")
import vip_test_client as v

link, ctrl = sys.argv[1], sys.argv[2]
N, FS = 124, 60_000_000
m, s = pty.openpty(); ttymod.setraw(s)
try: os.unlink(link)
except FileNotFoundError: pass
os.symlink(os.ttyname(s), link)
os.set_blocking(m, False)

def ack(seq):
    h = struct.pack("<BBBHHBB", v.FRAME_ACK_STD, 9, 0, seq & 0xFFFF, 0, 0, 1)
    return struct.pack("<H", v.VIP_SYNC) + h + struct.pack("<H", v.crc16_ccitt_false(h))

seq = 0; buf = bytearray(); t_rfst = 0; rnd = random.Random(1)
while True:
    try:
        buf += os.read(m, 4096)
    except BlockingIOError:
        pass
    except OSError:
        time.sleep(0.05); continue
    for f in v.parse_vip_stream(buf):
        if f[0] == v.FRAME_PING_STD:
            os.write(m, ack(f[3]))
    try:
        tone, lvl, gain, lo = open(ctrl).read().split()
    except Exception:
        tone, lvl, gain, lo = "none", "-40", "0", "2450"
    lvl, gain, lo = float(lvl), int(gain), float(lo)
    now = time.time()
    if now - t_rfst > 1.0:
        t_rfst = now
        body = b"RFST" + struct.pack("<QIIiBBBB", int(lo * 1e6), FS, 45_000_000, gain, 0, N, 12, 0)
        os.write(m, v.vip_encode_data(0, 1, seq, v.kria_encode(seq & 0xFF, 39, body))); seq += 1
    db = [-90.0 + rnd.uniform(-3, 3) for _ in range(N)]
    if tone != "none":
        k = round((float(tone) - lo) * 1e6 / (FS / N)) % N
        db[k] = lvl
    body = b"WAVD" + bytes([N, 0]) + b"".join(struct.pack("<h", int(round(x * 100))) for x in db)
    os.write(m, v.vip_encode_data(0, 1, seq, v.kria_encode(seq & 0xFF, 39, body))); seq += 1
    time.sleep(0.1)
