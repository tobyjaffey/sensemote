ifeq ($(APP),)
$(error No APP provided. Try make APP=xyz. To build everything, use build-all.sh)
endif

include config.mk   # user editable config

ifeq ($(CONFIG),)
$(error No CONFIG in config.mk)
endif

include apps/$(APP)/$(APP).mk

HEADERS=$(wildcard include/*.h)

#default to CC1110
ifeq ($(PLATFORM),)
PLATFORM=CC1110
endif

CCTLPROG = cctl/cctl-prog/cctl-prog

ifeq ($(PLATFORM),POSIX)
DRIVER_SUFFIX = _posix
CFLAGS += -DPOSIX_ENABLED
CFLAGS += -g -Wall -Werror #-pg
LDFLAGS += #-pg
OBJEXT=o
CC = gcc
LIBS += config/config.$(OBJEXT)
CFLAGS += -Iconfig -DUIP_CONF_BYTE_ORDER=UIP_LITTLE_ENDIAN
ifeq ($(NET_ENABLED),Y)
SOCK_NET_ENABLED=Y
UIP_NET_ENABLED=N
SOCK_NET_ASYNC_DNS_ENABLED=N    # -lcares required
endif    
NIBBLE_ENABLED=Y

else
ifeq ($(PLATFORM),CC1110)
DRIVER_SUFFIX = _cc1110
CC = sdcc
LDFLAGS += --code-loc 0x0400 --xram-loc 0xF000
LDFLAGS += --stack-auto --code-size $(CODE_SIZE) --xram-size 0x1000 #--nostdlib
TIPROG = /cygdrive/c/Program\ Files/Texas\ Instruments/SmartRF\ Tools/Flash\ Programmer/bin/SmartRFProgConsole.exe
CFLAGS += --opt-code-size
CFLAGS += --stack-auto
CFLAGS += --Werror
CFLAGS += -DCC1110_ENABLED
OBJEXT=rel
ifeq ($(NET_ENABLED),Y)
SOCK_NET_ENABLED=N
UIP_NET_ENABLED=Y
endif    
NIBBLE_ENABLED=Y

else
$(error no valid PLATFORM)
endif
endif

# config location
include flashvars.mk
CFLAGS += -DFLASHVARS_ADDR=$(FLASHVARS_ADDR)

# Core modules
ifeq ($(SKETCHWRAPPER_ENABLED),Y)
    # default config for sketches
    PB_ENABLED=Y
    CONS_ENABLED=Y
    CONS_RX_ENABLED=Y
    MAC_ENABLED=Y
    SLEEP_ENABLED=Y
    ITOA_ENABLED=Y
    CRYPTO_ENABLED=Y
    ADC_ENABLED=Y
    I2C_ENABLED=Y
    PWM_ENABLED=Y
    PORT0_ISR_ENABLED=Y
    TX_HIGHPOWER_ENABLED=Y
    SPI_ENABLED=Y

    CFLAGS += -DSKETCHFILE=apps/$(APP)/$(APP).ino
else
    LIBS += apps/$(APP)/$(APP).$(OBJEXT)
endif

LIBS += \
    os/clock$(DRIVER_SUFFIX).$(OBJEXT) \
    os/watchdog$(DRIVER_SUFFIX).$(OBJEXT) \
    os/timer$(DRIVER_SUFFIX).$(OBJEXT) \
    os/os.$(OBJEXT)      \
    os/config$(DRIVER_SUFFIX).$(OBJEXT) \
    os/radio$(DRIVER_SUFFIX).$(OBJEXT) \
    os/led.$(OBJEXT) \
    os/isr$(DRIVER_SUFFIX).$(OBJEXT) \
    os/random$(DRIVER_SUFFIX).$(OBJEXT)

# Optional modules

ifeq ($(SKETCHWRAPPER_ENABLED),Y)
LIBS += os/sketchwrapper.$(OBJEXT)
CFLAGS += -DSKETCHWRAPPER_ENABLED
endif

ifeq ($(NIBBLE_ENABLED),Y)
LIBS += os/nibble.$(OBJEXT)
CFLAGS += -DNIBBLE_ENABLED
endif

ifeq ($(UPTIME_ENABLED),Y)
CFLAGS += -DUPTIME_ENABLED
endif

ifeq ($(PB_ENABLED),Y)
LIBS += os/pb.$(OBJEXT)
CFLAGS += -DPB_ENABLED
endif

ifeq ($(ITOA_ENABLED),Y)
LIBS += os/itoa.$(OBJEXT)
CFLAGS += -DITOA_ENABLED
endif

ifeq ($(SHELL_ENABLED),Y)
LIBS += os/shell.$(OBJEXT)
CFLAGS += -DSHELL_ENABLED
endif

ifeq ($(SHELL_RAW_ENABLED),Y)
CFLAGS += -DSHELL_RAW_ENABLED
endif

ifeq ($(CRYPTO_ENABLED),Y)
LIBS += \
    os/crypt$(DRIVER_SUFFIX).$(OBJEXT) \
    os/pkt$(DRIVER_SUFFIX).$(OBJEXT)
CFLAGS += -DCRYPTO_ENABLED
endif

ifeq ($(CRYPTO_RANDOM_ENABLED),Y)
CFLAGS += -DCRYPTO_RANDOM_ENABLED
else
CFLAGS += -DSYS_RANDOM_ENABLED
endif

ifeq ($(UIP_NET_ENABLED),Y)
LIBS += \
    os/ether$(DRIVER_SUFFIX).$(OBJEXT) \
    os/net_uip.$(OBJEXT) \
    uip/uip.$(OBJEXT) \
    uip/clock-arch.$(OBJEXT) \
    uip/uip_arp.$(OBJEXT) \
    uip/resolv.$(OBJEXT) \
    uip/uiplib.$(OBJEXT) \
    uip/dhcpc.$(OBJEXT)

CFLAGS += -DNET_ENABLED -DUIP_NET_ENABLED
endif


ifeq ($(SOCK_NET_ENABLED),Y)
LIBS += os/net_sock.$(OBJEXT)
CFLAGS += -DNET_ENABLED -DSOCK_NET_ENABLED

ifeq ($(SOCK_NET_ASYNC_DNS_ENABLED),Y)
LDFLAGS += -lrt
CARESLIB = /usr/lib/libcares.a
endif
endif

ifeq ($(NET_TCP_ENABLED),Y)
ifeq ($(UIP_NET_ENABLED),Y)
LIBS += os/tcp_uip.$(OBJEXT)
endif
ifeq ($(SOCK_NET_ENABLED),Y)
LIBS += os/tcp_sock.$(OBJEXT)
endif
CFLAGS += -DNET_TCP_ENABLED
endif

ifeq ($(NET_WEBCLIENT_ENABLED),Y)
ifeq ($(UIP_NET_ENABLED),Y)
LIBS += uip/webclient.$(OBJEXT) \
    uip/webclient_strings.$(OBJEXT)
endif
CFLAGS += -DNET_WEBCLIENT_ENABLED
endif


SVNREV = $(shell svnversion)
CFLAGS += -DSVNREV='$(SVNREV)'
CFLAGS += -DBANNER_STR='$(APP)'

ifeq ($(WATCHDOG_ENABLED),Y)
CFLAGS += -DWATCHDOG_ENABLED
endif

ifeq ($(TX_HIGHPOWER_ENABLED),Y)
CFLAGS += -DTX_HIGHPOWER_ENABLED
endif

ifeq ($(MAC_ENABLED),Y)
CFLAGS += -DMAC_ENABLED
ifeq ($(COORDINATOR_ENABLED),Y)
LIBS += os/mac_coord.$(OBJEXT)
CFLAGS += -DCOORDINATOR_ENABLED
else
LIBS += os/mac_node.$(OBJEXT)
endif
endif

ifeq ($(SLEEP_ENABLED),Y)
LIBS += os/sleep$(DRIVER_SUFFIX).$(OBJEXT)
CFLAGS += -DSLEEP_ENABLED
endif

ifeq ($(LED_P0_6_ENABLED),Y)
CFLAGS += -DLED_P0_6_ENABLED
endif

ifeq ($(LED_P2_3_ENABLED),Y)
CFLAGS += -DLED_P2_3_ENABLED
endif

ifeq ($(LED_P1_1_ENABLED),Y)
CFLAGS += -DLED_P1_1_ENABLED
endif

ifeq ($(PLATFORM),POSIX)
CONS_ENABLED=Y
endif

ifeq ($(CONS_ENABLED),Y)
CFLAGS += -DCONS_TX_ENABLED -DCONS_ENABLED
LIBS += os/cons$(DRIVER_SUFFIX).$(OBJEXT)
endif

ifeq ($(CONS_TX_MINIMAL_ENABLED),Y)
CFLAGS += -DCONS_TX_MINIMAL_ENABLED
endif

ifeq ($(CONS_RX_MINIMAL_ENABLED),Y)
CFLAGS += -DCONS_RX_MINIMAL_ENABLED
endif

ifeq ($(CONS_RX_ENABLED),Y)
CFLAGS += -DCONS_RX_ENABLED
endif

ifeq ($(ADC_ENABLED),Y)
CFLAGS += -DADC_ENABLED
LIBS += os/adc$(DRIVER_SUFFIX).$(OBJEXT)
endif

ifeq ($(I2C_ENABLED),Y)
CFLAGS += -DI2C_ENABLED
LIBS += os/i2c$(DRIVER_SUFFIX).$(OBJEXT)
endif

ifeq ($(SPI_ENABLED),Y)
CFLAGS += -DSPI_ENABLED
LIBS += os/spi$(DRIVER_SUFFIX).$(OBJEXT)
endif

ifeq ($(PWM_ENABLED),Y)
CFLAGS += -DPWM_ENABLED
LIBS += os/pwm$(DRIVER_SUFFIX).$(OBJEXT)
endif

ifeq ($(CRYSTAL32KHZ_ENABLED),Y)
CFLAGS += -DCRYSTAL32KHZ_ENABLED
endif

ifeq ($(PORT1_ISR_ENABLED),Y)
CFLAGS += -DPORT1_ISR_ENABLED
endif

ifeq ($(PORT0_ISR_ENABLED),Y)
CFLAGS += -DPORT0_ISR_ENABLED
endif

OUTLIBS = $(addprefix obj/$(PLATFORM)/$(APP)/,$(LIBS))

CFLAGS += -DBUILD_VERSION=$(SVNREV)
CFLAGS += -Iinclude -I.


all: obj/$(PLATFORM)/$(APP).hex

# A hack to concatenate all C files then compile as one lump
# May help SDCC make optimisations, YMMV
combined:
	cat $(subst .rel,.c,$(LIBS)) > obj/$(PLATFORM)/$(APP)-combined.c
	$(CC) $(CFLAGS) -I$(APP) -Iuip -o obj/$(PLATFORM)/$(APP)-combined.rel -c obj/$(PLATFORM)/$(APP)-combined.c
	$(CC) $(LDFLAGS) -o obj/$(PLATFORM)/$(APP)-combined.ihx obj/$(PLATFORM)/$(APP)-combined.rel
	packihx < obj/$(PLATFORM)/$(APP)-combined.ihx > obj/$(PLATFORM)/$(APP)-combined.hex
	@echo Binary size `makebin -p < obj/$(PLATFORM)/$(APP)-combined.hex | wc -c` of $(shell printf "%d" $(FLASHVARS_ADDR))
	(config/patcher obj/$(PLATFORM)/$(APP)-combined.hex $(CONFIG) > obj/$(PLATFORM)/$(APP)-combined-patched.hex) || rm -f obj/$(PLATFORM)/$(APP)-combined-patched.hex



obj/$(PLATFORM)/$(APP)/%.$(OBJEXT) : %.c $(HEADERS)
	@mkdir -p obj/$(PLATFORM)/$(APP)/apps/$(APP)
	@mkdir -p obj/$(PLATFORM)/$(APP)/uip
	@mkdir -p obj/$(PLATFORM)/$(APP)/os
ifeq ($(PLATFORM),POSIX)
	@mkdir -p obj/$(PLATFORM)/$(APP)/config
endif
	$(CC) $(CFLAGS) -o $@ -c $<


obj/$(PLATFORM)/$(APP).hex: $(OUTLIBS)
ifeq ($(PLATFORM),CC1110)
	$(CC) $(LDFLAGS) -o obj/$(PLATFORM)/$(APP).ihx $(OUTLIBS)
	packihx < obj/$(PLATFORM)/$(APP).ihx > obj/$(PLATFORM)/$(APP).hex
	@echo Binary size `makebin -p < obj/$(PLATFORM)/$(APP).hex | wc -c` of $(shell printf "%d" $(FLASHVARS_ADDR))
endif
ifeq ($(PLATFORM),POSIX)
	$(CC) $(LDFLAGS) -o obj/$(PLATFORM)/$(APP).elf $(OUTLIBS) $(CARESLIB)
endif

config/patcher:
	make -C config

obj/$(PLATFORM)/$(APP)-patched.hex: obj/$(PLATFORM)/$(APP).hex config/patcher
	(config/patcher obj/$(PLATFORM)/$(APP).hex $(CONFIG) > obj/$(PLATFORM)/$(APP)-patched.hex) || rm -f obj/$(PLATFORM)/$(APP)-patched.hex

install-dongle: obj/$(PLATFORM)/$(APP)-patched.hex
	$(TIPROG) S EPV F="obj/$(PLATFORM)/$(APP)-patched.hex"

$(CCTLPROG):
	make -C cctl/cctl-prog

ifeq ($(PLATFORM),CC1110)
install: obj/$(PLATFORM)/$(APP)-patched.hex $(CCTLPROG)
	$(CCTLPROG) -t 30 -d $(CCTL_DEVICE) -c -f obj/$(PLATFORM)/$(APP)-patched.hex
endif
ifeq ($(PLATFORM),POSIX)
install: obj/$(PLATFORM)/$(APP).hex
	obj/$(PLATFORM)/$(APP).elf $(CONFIG)
endif


clean:
	rm -f obj/$(PLATFORM)/$(APP).*
	rm -rf obj/$(PLATFORM)/$(APP)

