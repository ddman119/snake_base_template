import os
import errno
import os
import os.path

FIFO = '/tmp/snakegame_fifo'
if os.path.exists(FIFO) == False:
    os.mkfifo(FIFO)

data_string = ""
while True:
    print("Opening FIFO...")
    with open(FIFO) as fifo:
        print("FIFO opened")
        while True:
            # Non-blocking read.
            for nextfetch in fifo:
                print(nextfetch)
            break
