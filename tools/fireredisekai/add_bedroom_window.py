#!/usr/bin/env python3
"""Add a 2x2-metatile window to the generic FRLG building tileset and place it on the
back wall of the player's bedroom (Red's room) at x=6..7, y=0..1.
Idempotent: if the window metatiles already exist it only (re)writes the map.
Run from the repo root: python3 tools/fireredisekai/add_bedroom_window.py"""
import os, struct
from PIL import Image

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
PRI = os.path.join(ROOT, 'data/tilesets/primary/building_frlg')
SEC = os.path.join(ROOT, 'data/tilesets/secondary/generic_building_1_frlg')
MAP = os.path.join(ROOT, 'data/layouts/PalletTown_PlayersHouse_2F_Frlg/map.bin')
MAP_W = 12
NUM_TILES_PRIMARY = 640
NUM_METATILES_PRIMARY = 640
PAL = 1  # wall palette: 0/6 beige, 1 outline, 3 grey-beige, 4 white, 7 sky, 8 blue, 15 deep blue

def tile_pixels(img, n):
    w = img.width // 8
    px = img.load()
    tx, ty = n % w, n // w
    return [[px[tx * 8 + x, ty * 8 + y] & 0xF for x in range(8)] for y in range(8)]

def main():
    pri = Image.open(os.path.join(PRI, 'tiles.png'))
    sec = Image.open(os.path.join(SEC, 'tiles.png'))
    metas = open(os.path.join(SEC, 'metatiles.bin'), 'rb').read()
    n_metas = len(metas) // 16
    marker = os.path.join(SEC, '.fireredisekai_window')
    if os.path.exists(marker):
        first_meta = int(open(marker).read())
    else:
        # 32x32 canvas: wall top (metatile 0x020 = tiles 1/17) over wall bottom (0x028 = tiles 2/18)
        canvas = [[0] * 32 for _ in range(32)]
        for row, (top, bot) in enumerate([(1, 17), (2, 18)]):
            for half, t in enumerate((top, bot)):
                p = tile_pixels(pri, t)
                for cx in range(4):
                    for y in range(8):
                        for x in range(8):
                            canvas[row * 16 + half * 8 + y][cx * 8 + x] = p[y][x]
        # draw the window: outline 1, frame 4 (white), glass 7 with a darker band 8, glints 4
        x0, x1, y0, y1 = 4, 27, 3, 24
        for y in range(y0, y1 + 1):
            for x in range(x0, x1 + 1):
                edge = x in (x0, x1) or y in (y0, y1)
                frame = x in (x0 + 1, x1 - 1, 15, 16) or y in (y0 + 1, y1 - 1, 13)
                if edge:
                    c = 1
                elif frame:
                    c = 4
                else:
                    c = 8 if y > 19 or (x + y) % 11 == 0 else 7
                canvas[y][x] = c
        for (gx, gy) in [(8, 6), (9, 6), (8, 7), (20, 6), (21, 6), (20, 7)]:
            canvas[gy][gx] = 4
        for x in range(x0 - 1, x1 + 2):   # sill
            canvas[y1 + 1][x] = 3
            canvas[y1 + 2][x] = 1
        # append 16 tiles (one more row of 16 tiles) to the secondary tileset image
        first_tile = (sec.width // 8) * (sec.height // 8)
        new = Image.new('P', (sec.width, sec.height + 8))
        new.putpalette(sec.getpalette())
        new.paste(sec, (0, 0))
        px = new.load()
        for t in range(16):
            mx, my, qx, qy = (t // 4) % 2, (t // 4) // 2, t % 2, (t % 4) // 2
            dx = (first_tile + t) % (new.width // 8)
            dy = (first_tile + t) // (new.width // 8)
            for y in range(8):
                for x in range(8):
                    px[dx * 8 + x, dy * 8 + y] = canvas[my * 16 + qy * 8 + y][mx * 16 + qx * 8 + x]
        new.save(os.path.join(SEC, 'tiles.png'))
        # four metatiles: TL, TR, BL, BR; bottom layer only
        out = bytearray(metas)
        for m in range(4):
            ids = [NUM_TILES_PRIMARY + first_tile + m * 4 + k for k in range(4)]
            entries = [i | (PAL << 12) for i in ids] + [0, 0, 0, 0]
            out += struct.pack('<8H', *entries)
        open(os.path.join(SEC, 'metatiles.bin'), 'wb').write(out)
        attrs = open(os.path.join(SEC, 'metatile_attributes.bin'), 'rb').read()
        attrs += struct.pack('<4I', 0, 0, 0, 0)
        open(os.path.join(SEC, 'metatile_attributes.bin'), 'wb').write(attrs)
        first_meta = NUM_METATILES_PRIMARY + n_metas
        open(marker, 'w').write(str(first_meta))
    # place on the map, keeping the collision/elevation bits
    m = bytearray(open(MAP, 'rb').read())
    for i, (x, y) in enumerate([(6, 0), (7, 0), (6, 1), (7, 1)]):
        off = (y * MAP_W + x) * 2
        old = struct.unpack_from('<H', m, off)[0]
        struct.pack_into('<H', m, off, (old & 0xFC00) | (first_meta + [0, 1, 2, 3][i]))
    open(MAP, 'wb').write(m)
    print('window metatiles from 0x%x' % first_meta)

if __name__ == '__main__':
    main()
