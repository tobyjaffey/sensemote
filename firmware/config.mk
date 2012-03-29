# Serial port for CCTL
#
# For CYGWIN (note ttyS0 = COM1)
# CCTL_DEVICE=/dev/ttyS6    # COM7
#
# For OSX
# CCTL_DEVICE=/dev/tty.usbserial-FTE9OZ7V
#
# For Linux
# CCTL_DEVICE ?= /dev/ttyUSB0
$(error Edit config.mk for your serial port location, then comment out this line)

CONFIG = \
    --eui64=0000000000000002 \
    --mac=000102030405 \
    --server=beta.pachube.com \
    --port=8081 \
    --feedid=MYFEEDID \
    --apikey=MYPACHUBEAPIKEY\
    --keyenc=250B7847A2BB41856DD41B4017A9036A \
    --keymac=174AA251FFADDC276EA364EDA62EDFDF

