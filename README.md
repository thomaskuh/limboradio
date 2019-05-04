# LimboRadio

DIY webradio turning on and off depending on light and movement conditions. Custom made case, software with WebUI and PCB. Build around a RaspberryPi Zero W.

This readme describes the complete setup from zero to hero. It's based on arch linux, because it's my favorite distribution but can be easily customized.

## Why?
- Simply because there's no such thing to buy out there in the whole f***in world.
- Because I can.

## Hardware
TODO: Documentation

## Software

Setup a SD card using the [official arch guide](https://archlinuxarm.org/platforms/armv6/raspberry-pi#installation) for RaspberryPi Zero W. But it in your Pi and follow the guide:

```
# Ssh to your raspberry (alarm/alarm , root/root)
ssh alarm@PI_IP
su

# Finish arch setup
pacman-key --init
pacman-key --populate archlinuxarm

# Install required packages
pacman -Syu --noconfirm vim alsa-utils wiringpi i2c-tools mpd ncmpc git base-devel libwebsockets jansson wpa_supplicant cronie wget

# Set timezone
timedatectl set-timezone Europe/Berlin

# Configure custom stuff. I prefer to allow root ssh login and delete user "alarm" because I hate su/sudo overhead:
vim /etc/ssh/sshd_config
PermitRootLogin yes
userdel -f alarm

# Get LimboRadio
cd /opt
git clone https://github.com/thomaskuh/limboradio
cd limboradio

# Configure Hardware/IO/Pi/System/OS
make setup

# Build LimboRadio
make

# Install LimboRadio (Install binaries, systemd autostart)
make install

# Done. Now reboot to kick everything off.
reboot
```

The above should be quite common for those of you having some basic linux/arch skills. The only step that requires some explanation is ```make setup```. This one-liner is sets up hardware interfaces and OS services so LimboRadio could work smoothly. If you're interested in details, just take a look at the Makefile. Basically it does the following:
- Setup I2S and I2C busses (via /boot/config.txt and kernel modules)
- Setup linux sound system (alsa, i2s amp)
- Setup MPD autostart
- Setup Wifi with profile based auto-re-connect using netctl.


### WiFi configuration
To allow changes in wifi configuration without cable-connect, LimboRadio comes with the following setup:
- With "make setup" you already configured netctl for profile based wifi auto-re-connect.
- There's a fallback profile trying to connect to a wifi hotspot with SSID "yeah" and password "yeahyeah".
- There's another profile that could be configured via LimboRadio WebUI by opening http://[IP shown in display] in your browser.

So if you're still cable-connected: Simply enter the URL. Otherwise create an access point with the credentials mentioned above an you'll see LimboRadio connecting in a minute.


### Useful tools for development and troubleshooting

Alsa
```
# List audio playback devices
aplay -l

# Test playback
speaker-test -c2
speaker-test -c2 --test=wav -w /usr/share/sounds/alsa/Front_Center.wav
```

MPD
```
# ncurses based client for MPD control and testing
ncmpc
```

GPIO / I2C
```
# Read all GPIO pin states
gpio readall

# Display should be on bus 0 address 0x4a
i2cdetect -y 0

# Lux sensor should be on bus 1 address 0x3c
i2cdetect -y 1

```


### PCB design flaws / ToDos for future versions
* 3 resistors at the bottom are not required. Internal Pi pull-ups are doing just fine.
* We could remove some mm at the bottom cause it collides with ethernet port when working on a full sized Pi (instead of the zero).
* Display mount holes are a bit to small.
* Prototyping with a full sized Pi worked pretty well but after switching to the Pi zero voltage seems to break down on high volume/baselines, maybe due to no/lower capacitory on the pi zero. As a workaround I put an additional capacitor into my power screw block but there really should be dedicated place on the PCB.
