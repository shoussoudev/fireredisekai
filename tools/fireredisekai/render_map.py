#!/usr/bin/env python3
"""Render a map layout (or a tileset's metatile sheet) to PNG, for checking map edits.
usage: render_map.py layout LAYOUT_ID out.png
       render_map.py sheet PRIMARY_DIR SECONDARY_DIR out.png
FRLG layout: primary has 640 tiles / 640 metatiles / palettes 0-6, secondary the rest."""
import json, struct, sys, os
from PIL import Image, ImageDraw

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
NUM_TILES_PRIMARY = 640
NUM_METATILES_PRIMARY = 640
NUM_PALS_PRIMARY = 7

def read_pal(path):
    v = open(path).read().split()
    return [tuple(map(int, v[3 + i * 3:6 + i * 3])) for i in range(16)]

def load_tileset(d):
    tiles = Image.open(os.path.join(d, 'tiles.png'))
    idx = tiles.load()
    w = tiles.width // 8
    out = []
    for t in range((tiles.width // 8) * (tiles.height // 8)):
        tx, ty = t % w, t // w
        out.append([[idx[tx * 8 + x, ty * 8 + y] & 0xF for x in range(8)] for y in range(8)])
    mt = open(os.path.join(d, 'metatiles.bin'), 'rb').read()
    metas = [struct.unpack('<8H', mt[i:i + 16]) for i in range(0, len(mt), 16)]
    pals = [read_pal(os.path.join(d, 'palettes', '%02d.pal' % i)) for i in range(16)]
    return out, metas, pals

def tileset_dir(sym):
    # map gTileset_X symbol to its directory by reading headers.h + graphics.h
    hdr = open(os.path.join(ROOT, 'src/data/tilesets/headers.h')).read()
    i = hdr.index('const struct Tileset %s =' % sym)
    tiles_sym = hdr[i:].split('.tiles = ')[1].split(',')[0].strip()
    gfx = open(os.path.join(ROOT, 'src/data/tilesets/graphics.h')).read()
    j = gfx.index(tiles_sym + '[]')
    path = gfx[j:].split('"')[1]
    return os.path.join(ROOT, os.path.dirname(path))

class Renderer:
    def __init__(self, pdir, sdir):
        self.pt, self.pm, self.pp = load_tileset(pdir)
        self.st, self.sm, self.sp = load_tileset(sdir)
        self.pals = self.pp[:NUM_PALS_PRIMARY] + self.sp[NUM_PALS_PRIMARY:]

    def tile(self, n):
        return self.pt[n] if n < NUM_TILES_PRIMARY else self.st[n - NUM_TILES_PRIMARY]

    def metatile(self, mid):
        m = self.pm[mid] if mid < NUM_METATILES_PRIMARY else self.sm[mid - NUM_METATILES_PRIMARY]
        img = Image.new('RGB', (16, 16), (255, 0, 255))
        px = img.load()
        for layer in range(2):
            for k in range(4):
                e = m[layer * 4 + k]
                t = self.tile(e & 0x3FF)
                hf, vf, pal = (e >> 10) & 1, (e >> 11) & 1, e >> 12
                ox, oy = (k % 2) * 8, (k // 2) * 8
                for y in range(8):
                    for x in range(8):
                        c = t[7 - y if vf else y][7 - x if hf else x]
                        if c == 0 and layer == 1:
                            continue
                        px[ox + x, oy + y] = self.pals[pal][c]
        return img

def render_layout(layout_id, out):
    L = json.load(open(os.path.join(ROOT, 'data/layouts/layouts.json')))['layouts']
    l = [x for x in L if x.get('id') == layout_id][0]
    r = Renderer(tileset_dir(l['primary_tileset']), tileset_dir(l['secondary_tileset']))
    w, h = l['width'], l['height']
    b = open(os.path.join(ROOT, l['blockdata_filepath']), 'rb').read()
    v = struct.unpack('<%dH' % (w * h), b)
    img = Image.new('RGB', (w * 16, h * 16))
    for y in range(h):
        for x in range(w):
            img.paste(r.metatile(v[y * w + x] & 0x3FF), (x * 16, y * 16))
    img = img.resize((img.width * 3, img.height * 3), Image.NEAREST)
    d = ImageDraw.Draw(img)
    for y in range(h):
        for x in range(w):
            d.text((x * 48 + 2, y * 48 + 2), '%d,%d' % (x, y), fill=(255, 0, 0))
    img.save(out)

def render_sheet(pdir, sdir, out, start, count):
    r = Renderer(pdir, sdir)
    cols = 16
    img = Image.new('RGB', (cols * 48, ((count + cols - 1) // cols) * 60), (255, 255, 255))
    d = ImageDraw.Draw(img)
    for i in range(count):
        mid = start + i
        try:
            t = r.metatile(mid).resize((48, 48), Image.NEAREST)
        except IndexError:
            break
        x, y = (i % cols) * 48, (i // cols) * 60
        img.paste(t, (x, y + 12))
        d.text((x + 2, y), '%03x' % mid, fill=(0, 0, 0))
    img.save(out)

if __name__ == '__main__':
    if sys.argv[1] == 'layout':
        render_layout(sys.argv[2], sys.argv[3])
    else:
        render_sheet(sys.argv[2], sys.argv[3], sys.argv[4], int(sys.argv[5], 0), int(sys.argv[6]))
