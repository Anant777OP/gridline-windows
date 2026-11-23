"""Synthesizes the original "Gridline" UI sound set (48 kHz, 16-bit WAV).

Digital synth tones: soft square-wave blips, pitch sweeps and a short echo,
all generated here from scratch.
usage: python make_sounds.py <out_dir>
"""
import math
import os
import struct
import sys
import wave

RATE = 48000


def osc(phase, kind):
    """Band-limited-ish oscillators built from a few harmonics (keeps the tones soft)."""
    if kind == "sine":
        return math.sin(phase)
    if kind == "square":  # odd harmonics 1, 3, 5, 7
        return (math.sin(phase) + math.sin(3 * phase) / 3 + math.sin(5 * phase) / 5 + math.sin(7 * phase) / 7) * 0.9
    if kind == "saw":  # harmonics 1..6
        return sum(math.sin(k * phase) / k for k in range(1, 7)) * 0.62
    raise ValueError(kind)


def tone(f0, f1=None, dur=0.08, vol=0.5, kind="square", attack=0.004, decay=30.0, vibrato=0.0):
    """A note that glides from f0 to f1 (exponential sweep) with an attack/decay envelope."""
    f1 = f1 or f0
    n = int(RATE * dur)
    out, phase = [], 0.0
    for i in range(n):
        t = i / RATE
        u = i / max(1, n - 1)
        f = f0 * (f1 / f0) ** u * (1 + vibrato * math.sin(2 * math.pi * 6 * t))
        phase += 2 * math.pi * f / RATE
        env = min(1.0, t / attack) * math.exp(-decay * t)
        out.append(vol * env * osc(phase, kind))
    return out


def silence(dur):
    return [0.0] * int(RATE * dur)


def seq(*parts):
    out = []
    for p in parts:
        out.extend(p)
    return out


def layer(*parts):
    n = max(len(p) for p in parts)
    out = [0.0] * n
    for p in parts:
        for i, v in enumerate(p):
            out[i] += v
    return out


def echo(s, delay=0.09, fb=0.32, taps=3):
    d = int(RATE * delay)
    out = s + [0.0] * d * taps
    for k in range(1, taps + 1):
        g = fb ** k
        for i, v in enumerate(s):
            out[i + d * k] += v * g
    return out


def write(path, samples):
    peak = max(1e-9, max(abs(s) for s in samples))
    gain = min(1.0, 0.85 / peak)
    samples = samples + [0.0] * int(RATE * 0.01)
    with wave.open(path, "wb") as w:
        w.setnchannels(1)
        w.setsampwidth(2)
        w.setframerate(RATE)
        w.writeframes(b"".join(struct.pack("<h", int(max(-1, min(1, s * gain)) * 32767)) for s in samples))


SOUNDS = {
    # two rising digital blips
    "notify.wav": echo(seq(tone(880, dur=0.06), silence(0.025), tone(1318.5, dur=0.1, decay=22))),
    # quick double chirp
    "message.wav": echo(seq(tone(1600, 2100, 0.045, decay=50), silence(0.03), tone(1600, 2400, 0.07, decay=34)), 0.07),
    # three-step arpeggio
    "reminder.wav": echo(seq(tone(659.25, dur=0.07), tone(987.77, dur=0.07), tone(1318.5, dur=0.16, decay=16))),
    # tiny click-blip
    "tick.wav": tone(2400, 1800, 0.03, 0.4, "square", 0.001, 90),
    # soft rising sine ping
    "info.wav": echo(tone(1046.5, 1396.9, 0.14, 0.5, "sine", 0.004, 20), 0.08, 0.25, 2),
    # two flat saw pulses
    "warning.wav": seq(tone(740, dur=0.09, kind="saw", decay=22), silence(0.05), tone(740, dur=0.13, kind="saw", decay=18)),
    # falling saw sweep
    "error.wav": layer(tone(520, 260, 0.32, 0.55, "saw", 0.003, 8), tone(522, 262, 0.32, 0.3, "square", 0.003, 8)),
    # upward sweep
    "connect.wav": echo(tone(500, 1500, 0.16, 0.5, "square", 0.003, 10), 0.08, 0.28, 2),
    # downward sweep
    "disconnect.wav": echo(tone(1500, 500, 0.16, 0.5, "square", 0.003, 10), 0.08, 0.28, 2),
    # three low pulses
    "battery-low.wav": seq(*[seq(tone(392, dur=0.09, kind="saw", decay=20), silence(0.07)) for _ in range(3)]),
    # power-up: rising sweep into a bright chord
    "unlock.wav": seq(tone(220, 880, 0.22, 0.45, "saw", 0.01, 3),
                      echo(layer(tone(880, dur=0.5, vol=0.3, kind="square", decay=6),
                                 tone(1108.7, dur=0.5, vol=0.25, kind="square", decay=6),
                                 tone(1318.5, dur=0.5, vol=0.25, kind="sine", decay=6, vibrato=0.004)), 0.1, 0.3, 2)),
}

# Windows sound event -> file
EVENTS = {
    ".Default": "tick.wav", "SystemNotification": "notify.wav", "Notification.Default": "notify.wav",
    "Notification.IM": "message.wav", "Notification.Mail": "message.wav", "MailBeep": "message.wav",
    "Notification.Reminder": "reminder.wav", "SystemAsterisk": "info.wav", "SystemExclamation": "warning.wav",
    "SystemHand": "error.wav", "DeviceConnect": "connect.wav", "DeviceDisconnect": "disconnect.wav",
    "DeviceFail": "error.wav", "LowBatteryAlarm": "battery-low.wav", "CriticalBatteryAlarm": "battery-low.wav",
    "WindowsUnlock": "unlock.wav", "WindowsLogon": "unlock.wav",
}

if __name__ == "__main__":
    out = sys.argv[1]
    os.makedirs(out, exist_ok=True)
    for name, s in SOUNDS.items():
        write(os.path.join(out, name), s)
    with open(os.path.join(out, "events.txt"), "w") as f:
        for ev, fn in EVENTS.items():
            f.write(f"{ev}={fn}\n")
    print("wrote", len(SOUNDS), "sounds")
