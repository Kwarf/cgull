import struct
import sys
from xml.dom.minidom import parse

if len(sys.argv) != 3:
    print("Usage: python rocket2bin.py <rocket file> <output file>")
    exit(1)

document = parse(sys.argv[1])
tracks = document.getElementsByTagName("track")

data = bytearray()

print(f"Writing {tracks.length} tracks")
data += struct.pack(f"<B", tracks.length)

for track in tracks:
    name = track.getAttribute("name").encode("utf-8") + b'\x00'
    data += struct.pack(f"<B{len(name)}sH", len(name), name, track.getElementsByTagName('key').length)

    print(f"Writing track '{name.decode()}' with {track.getElementsByTagName('key').length} keys")
    for key in track.getElementsByTagName("key"):
        data += struct.pack("<ifi", int(key.getAttribute("row")), float(key.getAttribute("value")), int(key.getAttribute("interpolation")))

with open(sys.argv[2], "wb") as f:
    f.write(data)
