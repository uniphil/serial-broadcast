#!/usr/bin/env python
from serial import Serial
from serial.tools import list_ports
from PIL import Image
from time import sleep

SPECIAL_BYTE = 0b10110100
ROW_LEN_BYTES = 48
PRINTERS = 6



def print_im(im, s):
    width = ROW_LEN_BYTES * 8 * PRINTERS
    ratio = im.height / im.width
    im.resize((width, int(round(width * ratio))), resample=Image.LANCZOS)

    im = im.convert('1')

    for y in range(im.height):
        print '# row {}'.format(y)
        # s.write(SPECIAL_BYTE)
        # s.write(bytearray([0b00001111] * ROW_LEN_BYTES * PRINTERS))
        for p in range(PRINTERS):
            out = bytearray([])
            for xi in range(p * ROW_LEN_BYTES * 8, (p + 1) * ROW_LEN_BYTES * 8, 8):
                b = 0
                for i in range(8):
                    b <<= 1
                    px = im.getpixel((xi + i, y)) if xi + i < im.width else 255
                    black = px < 128
                    b |= black
                out.append(b)
            s.write(out)
            print ''.join(['{:08b}'.format(x) for x in out])
            sleep(0.05)


if __name__ == '__main__':
    import sys
    port = sys.argv[1]
    filenames = sys.argv[2:]
    if not port.startswith('/dev'):
        filenames = [port] + filenames
        maybes = list(list_ports.grep('usb'))
        if len(maybes) == 0:
            sys.stderr.write('missing serial port (probably /dev/tty.usbserial-something)\n')
            sys.exit(1)
        if len(maybes) > 1:
            sys.stderr.write('not sure which serial port to use. likely candidates:\n{}\n'.format(
                '\n'.join(map(lambda m: '{}\t{}\t{}'.format(m.device, m.description, m.manufacturer), maybes))))
            sys.exit(1)
        port = maybes[0].device
    s = Serial(port, 115200)
    sleep(0.3)
    ims = [Image.open(f) for f in filenames]
    while True:
        for im in ims:
            print_im(im, s)
        break
    sleep(0.3)
    s.close()