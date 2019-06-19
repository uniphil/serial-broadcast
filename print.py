#!/usr/bin/env python
from serial import Serial
from serial.tools import list_ports
from PIL import Image
from time import sleep

PRINTERS = 2
PRINTER_WIDTH = 384

CMD_PATTERN = 0b11011000

CHUNK_SIZE = 8
def write_chunked(s, data):
    for i in range(0, len(data), CHUNK_SIZE):
        while True:
            if s.in_waiting > 0:
                print('chunked: clear buffer', s.in_waiting, s.read(s.in_waiting))
            # s.reset_input_buffer()
            s.write(bytearray([CMD_PATTERN | 2]))
            result = s.read(1)
            if len(result) == 0:
                print('oh noooo timeout 2')
                continue
            elif ord(result) != 2:
                print('oh noooo chunk error: {:08b}'.format(ord(result)))
                continue
            break

        s.reset_input_buffer()
        s.write(bytearray(data[i:i + CHUNK_SIZE]))
        result = s.read(1)
        if len(result) == 0:
            print('oh noooo timeout 2 2')
        elif ord(result) != 3:
            print('oh noooo chunk end error: {:08b}'.format(ord(result)))


def print_im(im, s):
    width = PRINTERS * PRINTER_WIDTH
    ratio = im.height / im.width
    im = im.resize((width, int(round(width * ratio))), resample=Image.LANCZOS)
    im = im.convert('1')

    print('resized to', im.size)

    for y in range(im.height):

        print('# row {}'.format(y))

        while True:
            print('clearing buffer...', s.in_waiting)
            if s.in_waiting > 0:
                print('row: clear buffer {}'.format(', '.join('{:08b}'.format(b)
                    for b in s.read(s.in_waiting))))
            # s.reset_input_buffer()
            print('cleared.')
            # sleep(0.1)
            print('sending command 1... {:8b}'.format(CMD_PATTERN | 1))
            s.write([CMD_PATTERN | 1])
            print('waiting for ack 1...')
            result = s.read(1)
            if len(result) == 0:
                print('oh noooo row timeout')
                sleep(0.3)
                continue
            elif ord(result) != 1:
                print('oh noooo row error: {:08b}'.format(ord(result)))
                sleep(0.3)
                continue
            print('got the right ack!')
            break

        out = bytearray([])
        for x in range(PRINTER_WIDTH):
            b = 0
            for p in range(PRINTERS):
                px = im.getpixel((p * PRINTER_WIDTH + x, y))
                black = px < 128
                b |= black << p
            out.append(b)
        write_chunked(s, out)
        # sleep(0.1)


            # out = bytearray([0] * 8)
            # x_start = xi * 8;
            # for pi in range(8):






        # for p in range(PRINTERS):
        #     out = bytearray([])
        #     for xi in range(p * ROW_LEN_BYTES * 8, (p + 1) * ROW_LEN_BYTES * 8, 8):
        #         b = 0
        #         for i in range(8):
        #             b <<= 1
        #             px = im.getpixel((xi + i, y)) if xi + i < im.width else 255
        #             black = px < 128
        #             b |= black
        #         out.append(b)
        #     s.write(out)
        #     print ''.join(['{:08b}'.format(x) for x in out])
        #     sleep(0.05)


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
    s = Serial(port, 115200, timeout=1.3)
    sleep(1.3)
    ims = [Image.open(f) for f in filenames]
    while True:
        for im in ims:
            print_im(im, s)
        break
    sleep(0.3)
    s.close()