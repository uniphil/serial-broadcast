#!/usr/bin/env python
from serial import Serial
from serial.tools import list_ports
from PIL import Image
from time import sleep, time

C_PRINTERS = 3
B_PRINTERS = 4
PRINTER_WIDTH = 384
ROW_OFFSET = 72 * 8  # 72mm * 8px/mm

CMD_PATTERN = 0b11011000

CHUNK_SIZE = 64
def write_chunked(s, data):
    for i in range(0, len(data), CHUNK_SIZE):
        s.reset_input_buffer()
        s.write(bytearray([CMD_PATTERN | 2]))
        result = s.read(1)
        if len(result) == 0:
            print('oh noooo timeout 2')
            return
        elif ord(result) != 2:
            print('oh noooo chunk error: {:08b}'.format(ord(result)))
            return

        s.reset_input_buffer()
        s.write(bytearray(data[i:i + CHUNK_SIZE]))
        result = s.read(1)
        if len(result) == 0:
            print('oh noooo timeout 2 2')
        elif ord(result) != 3:
            print('oh noooo chunk end error: {:08b}'.format(ord(result)))


def print_im(im, s):
    width = (C_PRINTERS + B_PRINTERS) * PRINTER_WIDTH
    ratio = im.height / im.width
    im = im.rotate(180)
    im = im.resize((width, int(round(width * ratio))), resample=Image.LANCZOS)
    im = im.convert('1')

    print('resized to', im.size)

    for y in range(im.height - ROW_OFFSET):
        t0 = time()
        print('# row {}'.format(y))

        while True:
            s.reset_input_buffer()
            s.write([CMD_PATTERN | 1])
            result = s.read(1)
            if len(result) == 0:
                print('oh noooo row timeout')
                sleep(0.3)
                continue
            elif ord(result) != 1:
                print('oh noooo row error: {:08b}'.format(ord(result)))
                sleep(0.3)
                continue
            break

        out = bytearray([])
        for x in range(PRINTER_WIDTH):
            b = 0
            for p in range(C_PRINTERS):
                px = im.getpixel(((p * 2 + 1) * PRINTER_WIDTH + x, y + ROW_OFFSET))
                black = px < 128
                b |= black << (p + B_PRINTERS)
            for p in range(B_PRINTERS):
                px = im.getpixel((p * 2 * PRINTER_WIDTH + x, y))
                black = px < 128
                b |= black << p
            out.append(b)
        write_chunked(s, out)

        print('dt', time() - t0)
        sleep(0.02)


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
    s = Serial(port, 500000, timeout=1.3)
    sleep(1.3)
    ims = [Image.open(f) for f in filenames]
    while True:
        for im in ims:
            print_im(im, s)
        break
    sleep(0.3)
    s.close()