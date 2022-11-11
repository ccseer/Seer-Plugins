#!/usr/bin/env python
# -*- coding: utf-8 -*-
# -----------------------------------------------------------------------------
#
#  FreeType high-level python API - Copyright 2011-2015 Nicolas P. Rougier
#  Distributed under the terms of the new BSD license.
#
# -----------------------------------------------------------------------------
#
#   Project: http://1218.io
#   Corey Chen
#
# -----------------------------------------------------------------------------


from freetype import *
import numpy as np
from PIL import Image

margin_h = 100
margin_v = 80
sizes = (12, 18, 24, 36, 48, 60, 72)
# text = "A Quick Brown Fox Jumps Over The Lazy Dog 0123456789"
text = "It is the time you have wasted for your rose that makes your rose so important."


# TODO: paint font info to image

def get_image_size(face):
    face.set_char_size(sizes[sizes.__len__() - 1] * 64)
    width, height, baseline = 0, 0, 0

    previous = 0
    slot = face.glyph
    for i, c in enumerate(text):
        face.load_char(c)
        bitmap = slot.bitmap
        height = max(height,
                     bitmap.rows + max(0, -(slot.bitmap_top - bitmap.rows)))
        baseline = max(baseline, max(0, -(slot.bitmap_top - bitmap.rows)))
        kerning = face.get_kerning(previous, c)
        width += (slot.advance.x >> 6) + (kerning.x >> 6)
        previous = c

    height = 0
    for size in sizes:
        height += (size + 30)

    return (width, height - 30)


def render(filename, output):
    face = Face(filename)
    im_size = get_image_size(face)

    W, H = im_size[0] + margin_h, im_size[1] + margin_v
    Z = np.zeros((H, W), dtype=np.ubyte)

    margin_left = int(margin_h / 2) * 64
    pen = Vector(margin_left, int(H - margin_v / 2) * 64)

    for size in sizes:
        face.set_char_size(size * 64, 0, 72, 72)
        matrix = Matrix(int((1.0) * 0x10000), int((0.0) * 0x10000),
                        int((0.0) * 0x10000), int((1.0) * 0x10000))
        previous = 0
        pen.x = margin_left
        for current in text:
            face.set_transform(matrix, pen)
            face.load_char(current)
            pen.x += face.get_kerning(previous, current, FT_KERNING_UNSCALED).x
            glyph = face.glyph
            bitmap = glyph.bitmap
            x, y = glyph.bitmap_left, glyph.bitmap_top
            w, h, p = bitmap.width, bitmap.rows, bitmap.pitch
            buff = np.array(bitmap.buffer, dtype=np.ubyte).reshape((h, p))
            Z[H - y:H - y + h, x:x + w].flat |= buff[:, :w].flatten()
            pen.x += glyph.advance.x
            previous = current
        pen.y -= (size + 30) * 64

    # Gamma correction
    Z = (Z / 255.0) ** 1.5
    Z = ((1 - Z) * 255).astype(np.ubyte)
    I = Image.fromarray(Z, mode='L')
    I.save(output)


def usage(execname):
    print()
    print("Font previewer -- part of the Seer app: http://sourceforge.net/projects/ccseer")
    print("----------------------------------------------------------")
    print("Usage: " + execname + " [options] fontpath.ttf outputpath")
    print()
    print("  -t text      display 'text' with font ")
    print()
    sys.exit()


if __name__ == '__main__':
    import sys
    import getopt

    execname = sys.argv[0]

    if len(sys.argv) < 3:
        usage(execname)

    try:
        opts, args = getopt.getopt(sys.argv[1:], 't:', )
    except getopt.GetoptError:
        usage(execname)

    for o, a in opts:
        if o == "-t":
            text = a
        else:
            usage(execname)

    if args.__len__() != 2:
        usage(execname)

    path_input = args[0]
    path_output = args[1].lower()
    if path_output.endswith('.png') is False:
        path_output = args[1] + '.png'

    render(path_input, path_output)
