import os
import errno

FIFO = '/tmp/snakegame_fifo'

#try:
#    os.mkfifo(FIFO)
#except OSError as oe:
#    if oe.errno != errno.EEXIST:
#        raise

data_string = ""

while True:
    print("Opening FIFO...")
    with open(FIFO) as fifo:
        print("FIFO opened")
        while True:
            # Non-blocking read.
            data = fifo.read(1)
            if data != '\n' and data != '':
                data_string += str(data)
            else:
                if data_string != "":
                    print('Read: "{0}"'.format(data_string))
                    data_string = ""
                break
