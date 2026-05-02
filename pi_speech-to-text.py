import json
import queue
import time
import numpy as np
import serial
import sounddevice as sd
from vosk import KaldiRecognizer, Model
from openwakeword.model import Model as WakeModel


# constants
SERIAL_PORT = "/dev/ttyUSB0"
BAUD_RATE = 9600
VOSK_MODEL = "vosk-model-small-en-us-0.15"
WAKE_WORD = "Hey_Hawk"
SAMPLE_RATE = 16000
CHUNK_FRAMES = 1280
LISTEN_WINDOW = 6 # how long it waits for command


# Commands dictionary for all the available commands that we can use. Expandable
# 'Phrase' : 'Command to send to the arduino'
COMMANDS = {
    # ── Red LED ──
    "red on":        b"R1\n",
    "turn on red":   b"R1\n",
    "red off":       b"R0\n",
    "turn off red":  b"R0\n",
    # ── Green LED ──
    "green on":      b"G1\n",
    "turn on green": b"G1\n",
    "green off":     b"G0\n",
    "turn off green":b"G0\n",
    # ── Blue LED ──
    "blue on":       b"B1\n",
    "turn on blue":  b"B1\n",
    "blue off":      b"B0\n",
    "turn off blue": b"B0\n",
    # ── All LEDs ──
    "all on":        b"A1\n",
    "lights on":     b"A1\n",
    "all off":       b"A0\n",
    "lights off":    b"A0\n",
    "turn off all":  b"A0\n",
    "turn on all":   b"A1\n",
    # ── Blink ──
    "blink red":     b"BR\n",
    "blink green":   b"BG\n",
    "blink blue":    b"BB\n",
    "blink all":     b"BA\n",
}


# communicate with the arduino
def open_serial():
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
    time.sleep(2)  # Wait for Arduino to reset after serial open
    print(f"[serial] Connected to Arduino on {SERIAL_PORT}")
    return ser


def send_command(ser, cmd_bytes):
    if ser and ser.is_open:
        ser.write(cmd_bytes)
        print(f"[serial] Sent: {cmd_bytes.decode().strip()}")
    else:
        print("[serial] No serial connection — command dropped.")


# use speech to text to parse user's voice to a command

def parse_command(transcript: str):
   # If a spoken command matches one in our dictionary, return the value of the corresponding command
    text = transcript.lower().strip()
    if text in COMMANDS:
        return COMMANDS[text]
    return None


#Capture the audio form microphone in chunks
audio_q = queue.Queue()

def audio_callback(indata, frames, time_info, status):

    if status:
        print(f"[audio] {status}")
    audio_q.put(bytes(indata))


def load_vosk():
    print("[vosk] Loading speech model...")
    model = Model(VOSK_MODEL)
    rec   = KaldiRecognizer(model, SAMPLE_RATE)
    print("[vosk] Speech model ready.")
    return rec

def listen_for_command(rec, timeout=LISTEN_WINDOW) -> str:
    #FUNCTION TO DETECT ANY WORD SAID

    print(f"[vosk] Listening for command ({timeout}s)...")
    rec.Reset()
    deadline = time.time() + timeout

    while time.time() < deadline:
        try:
            chunk = audio_q.get(timeout=0.1)
        except queue.Empty:
            continue
        if rec.AcceptWaveform(chunk):
            result = json.loads(rec.Result())
            text = result.get("text", "")
            if text:
                print(f"[vosk] Heard: '{text}'")
                return text

    # Grab any partial result and return it
    partial = json.loads(rec.FinalResult()).get("text", "")
    if partial:
        print(f"[vosk] Heard (partial): '{partial}'")
    return partial



def main():
    ser = open_serial()
    _, rec = load_models()

    print("\n[main] System ready. Say 'Park' to begin.\n")

    with sd.RawInputStream(
        samplerate=SAMPLE_RATE,
        blocksize=CHUNK_FRAMES,
        dtype="int16",
        channels=1,
        callback=audio_callback,
    ):
        rec.Reset()
        while True:
            try:
                chunk = audio_q.get(timeout=1)
            except queue.Empty:
                continue

            if not rec.AcceptWaveform(chunk):
                continue

            result = json.loads(rec.Result())
            text = result.get("text", "").strip().lower()

            if not text:
                continue

            # Stage 1: listening for wake word
            if "park" not in text:
                continue

            # Wake word heard
            print("\n[wake] Park detected! Say your command...")
            send_command(ser, b"W\n")
            rec.Reset()

            # Drain queue
            while not audio_q.empty():
                audio_q.get()

            # Stage 2: listen for command
            text = listen_for_command(rec, timeout=LISTEN_WINDOW)

            if not text:
                print("[main] Nothing heard — back to sleep.")
                continue

            # Stage 3: execute command
            cmd = parse_command(text)
            if cmd:
                send_command(ser, cmd)
                print("[main] Command executed. Waiting for wake word...")
            else:
                print(f"[main] Unknown command: '{text}'")

            rec.Reset()


if __name__ == "__main__":
    main()
