.PHONY: build rebuild flash rtt rtt-tail

BOARD ?= nrf54l15dk/nrf54l15/cpuapp
WEST ?= $(HOME)/zephyrproject/.venv/bin/west
JLINK_DIR ?= /opt/SEGGER/JLink
JLINK_RTT_LOGGER ?= $(JLINK_DIR)/JLinkRTTLoggerExe
RTT_LOG ?= build/rtt.log

build:
	$(WEST) build -b $(BOARD)

rebuild:
	$(WEST) build -p always -b $(BOARD)

flash:
	$(WEST) flash

rtt:
	$(JLINK_RTT_LOGGER) -Device NRF54L15_M33 -If SWD -Speed 4000 -RTTChannel 0 $(RTT_LOG)

rtt-tail:
	tail -f $(RTT_LOG)
