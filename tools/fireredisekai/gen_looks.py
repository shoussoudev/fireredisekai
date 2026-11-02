#!/usr/bin/env python3
"""Generate the MC's look palettes (recolours of the FRLG player sprite).

Writes JASC .pal files to graphics/fireredisekai/looks/:
  look<N>_ow.pal     16 colours - overworld sprites AND battle back sprite (they share a palette)
  look<N>_front.pal  16 colours - trainer front pic (trainer card, Hall of Fame)
  look<N>_intro.pal  32 colours - the large picture in Oak's intro
Run from the repo root:  python3 tools/fireredisekai/gen_looks.py
"""
import os

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
OUT = os.path.join(ROOT, 'graphics', 'fireredisekai', 'looks')

SOURCES = {
    'ow': ('graphics/object_events/palettes/player_frlg.pal', {
        'cap': [8, 11, 12], 'navy': [5, 6], 'jeans': [7], 'pack': [1, 13, 14],
        'skin': [2, 3], 'hair_outline': [4]}),
    'front': ('graphics/trainers/front_pics/red.png', {
        'cap': [12, 13], 'jeans': [5, 6, 7, 8], 'pack': [10, 11], 'hair': [4], 'skin': [1, 2, 3]}),
    'intro': ('graphics/oak_speech/red/pal.pal', {
        'cap': [27, 28, 29, 30, 31], 'jeans': [17, 18, 19, 20, 21], 'pack': [23, 24, 25, 26],
        'hair': [8, 9], 'skin': [1, 2, 3, 4, 5, 7]}),
}

# The five looks (same as the approved recolour preview). Never Red's red or Blue's blue.
LOOKS = [
    ('GREY CAP',   dict(cap=(96, 96, 104),  navy=(92, 108, 64), pack=(150, 110, 70), hair=(40, 36, 36),   jeans=(70, 80, 110))),
    ('NAVY CAP',   dict(cap=(52, 62, 110),  navy=(70, 70, 78),   pack=(120, 120, 128), hair=(60, 42, 30),  jeans=(150, 130, 100), skin=(176, 120, 86))),
    ('BEIGE CAP',  dict(cap=(200, 180, 140), navy=(120, 40, 52), pack=(60, 72, 120),  hair=(150, 110, 70), jeans=(80, 90, 120))),
    ('BLACK CAP',  dict(cap=(48, 48, 54),   navy=(40, 120, 128), pack=(220, 120, 50), hair=(30, 30, 34),   jeans=(60, 60, 70),    skin=(120, 80, 56))),
    ('GREEN CAP',  dict(cap=(60, 120, 72),  navy=(190, 150, 50), pack=(110, 80, 60),  hair=(200, 170, 110), jeans=(90, 110, 150), skin=(236, 190, 150))),
]

def read_jasc(path):
    lines = open(path).read().split()
    n = int(lines[2])
    vals = list(map(int, lines[3:3 + 3 * n]))
    return [tuple(vals[i * 3:i * 3 + 3]) for i in range(n)]

def read_png_pal(path):
    from PIL import Image
    p = Image.open(path).getpalette()
    return [tuple(p[i * 3:i * 3 + 3]) for i in range(16)]

def lum(c):
    return 0.299 * c[0] + 0.587 * c[1] + 0.114 * c[2]

def to_gba(c):
    # round to the 5-bit colour the GBA will actually show
    return tuple((v >> 3) << 3 for v in c)

def recolour(pal, classes, spec):
    out = list(pal)
    for cls, idxs in classes.items():
        key = 'hair' if cls == 'hair_outline' else cls
        if key not in spec:
            continue
        target = spec[key]
        if cls == 'hair_outline':
            # face/hair outline: keep it dark whatever the hair colour
            target = tuple(int(v * 0.55) for v in target)
        mean = sum(lum(pal[i]) for i in idxs) / len(idxs)
        for i in idxs:
            k = lum(pal[i]) / mean if len(idxs) > 1 else 1.0
            out[i] = tuple(max(0, min(255, int(t * k))) for t in target)
    return [to_gba(c) for c in out]

def write_jasc(path, pal):
    with open(path, 'w', newline='\r\n') as f:
        f.write('JASC-PAL\n0100\n%d\n' % len(pal))
        for c in pal:
            f.write('%d %d %d\n' % c)

def main():
    os.makedirs(OUT, exist_ok=True)
    for kind, (src, classes) in SOURCES.items():
        path = os.path.join(ROOT, src)
        pal = read_png_pal(path) if src.endswith('.png') else read_jasc(path)
        for n, (name, spec) in enumerate(LOOKS):
            write_jasc(os.path.join(OUT, 'look%d_%s.pal' % (n, kind)), recolour(pal, classes, spec))
    print('wrote', len(LOOKS) * len(SOURCES), 'palettes to', OUT)

if __name__ == '__main__':
    main()
