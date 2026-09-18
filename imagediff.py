#!/usr/bin/env python3

import glob
import sys


def read_word(f):
    c = f.read(1)
    while c in b" \t\r\n":
        c = f.read(1)

    word = b""
    while c not in b" \t\r\n":
        word += c
        c = f.read(1)
    return word


def read_ppm(filename):
    f = open(filename, "rb")
    read_word(f)
    width = int(read_word(f))
    height = int(read_word(f))
    read_word(f)
    pixels = f.read()
    f.close()
    return width, height, pixels


left = sorted(glob.glob(sys.argv[1]))
right = sorted(glob.glob(sys.argv[2]))

different = 0
total = 0
largest = 0

for left_name, right_name in zip(left, right):
    left_width, left_height, left_pixels = read_ppm(left_name)
    right_width, right_height, right_pixels = read_ppm(right_name)

    for i in range(0, len(left_pixels), 3):
        red = abs(left_pixels[i] - right_pixels[i])
        green = abs(left_pixels[i + 1] - right_pixels[i + 1])
        blue = abs(left_pixels[i + 2] - right_pixels[i + 2])
        difference = max(red, green, blue)
        if difference:
            different += 1
        largest = max(largest, difference)

    total += left_width * left_height

print("%d/%d pixels differ" % (different, total))
print("maximum absolute difference: %d/255" % largest)
