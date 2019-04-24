CC := gcc
SRCDIR := src
BUILDDIR := build
BINDIR := bin
TARGET := bin/limboradio

SRCEXT := c
SOURCES := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.o))
CFLAGS := -g # -Wall
LIB := -pthread -lmpdclient -lwiringPi -lwebsockets -ljansson -L lib -lm
INC := -I include

$(TARGET): $(OBJECTS)
	@echo " Linking..."
	@mkdir -p $(BINDIR)
	@echo " $(CC) $^ -o $(TARGET) $(LIB)"; $(CC) $^ -o $(TARGET) $(LIB)

$(BUILDDIR)/%.o: $(SRCDIR)/%.$(SRCEXT)
	@echo " Building..."
	@mkdir -p $(BUILDDIR)
	@echo " $(CC) $(CFLAGS) $(INC) -c -o $@ $<"; $(CC) $(CFLAGS) $(INC) -c -o $@ $<

setup:
	install -m 0644 etc/asound.conf /etc
	install -m 0755 etc/config.txt /boot
	install -m 0644 etc/modload-limboradio.conf /etc/modules-load.d
	install -m 0644 etc/modprobe-limboradio.conf /etc/modprobe.d
	install -m 0644 etc/fallback /etc/netctl
	systemctl enable mpd
	systemctl enable netctl-auto@wlan0

install:
	install -m 0755 $(TARGET) /usr/bin
	install -m 0755 etc/limboradio.service /usr/lib/systemd/system
	install -m 0755 etc/limbonoise.service /usr/lib/systemd/system
	systemctl enable limboradio
	systemctl enable limbonoise
	rm -rf /var/limboradio
	cp -rf web/dist /var/limboradio

uninstall:
	systemctl disable limbonoise
	systemctl disable limboradio
	rm -f /usr/lib/systemd/system/limbonoise.service
	rm -f /usr/lib/systemd/system/limboradio.service
	rm -f /usr/bin/limboradio
	rm -f /etc/limboradio.json
	rm -rf /var/limboradio

clean:
	@echo " Cleaning...";
	@echo " $(RM) -r $(BUILDDIR) $(TARGET)"; $(RM) -r $(BUILDDIR) $(TARGET)

all:	$(TARGET)
