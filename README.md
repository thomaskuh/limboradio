# LimboRadio

DIY webradio turning on and off depending on light and movement conditions. Custom made case, software with WebUI and PCB. Build around a RaspberryPi Zero W.

This readme describes the complete setup from zero to hero. It's based on arch linux, because it's my favorite distribution but can be easily customized.

## Why?
- Simply because there's no such thing to buy out there in the whole f***in world.
- Because I can.

## Hardware
TODO: Documentation

## Software

### Setup RaspberryPi

First of all: Setup SD card using the [official arch guide](https://archlinuxarm.org/platforms/armv6/raspberry-pi#installation) for RaspberryPi Zero W.

Now connect ethernet, power things up and finish setup:
```
# Ssh to your raspberry (alarm/alarm , root/root)
ssh alarm@PI_IP
su

# Finish arch setup
pacman-key --init
pacman-key --populate archlinuxarm

# Install required packages
pacman -Syu vim alsa-utils wiringpi i2c-tools mpd ncmpc git base-devel libwebsockets jansson wpa_actiond dialog cronie

# Set timezone
timedatectl set-timezone Europe/Berlin

# Configure your stuff. I prefer to delete user "alarm" and allow root ssh login because I hate su/sudo overhead:
userdel -f alarm
vim /etc/ssh/sshd_config
PermitRootLogin yes

# Reboot
reboot
```


### Get LimboRadio
```
cd /opt
git clone https://github.com/thomaskuh/limboradio
cd limboradio
```

### Setup your RaspberryPi
There's a bunch of things to before you'll be able to run a full featured LimboRadio:
- Setup I2S and I2C busses
- Setup linux sound system (alsa, i2s amp)
- Setup MPD autostart
- Setup Wifi with profile based auto-re-connect using netctl.

I prepared a one-liner to get things up and running but if you're interested in details: Take a look at the Makefile.
```
# Setup hardware interfaces and system stuff
make setup
```

### Build and Setup LimboRadio itself
```
# Build LimboRadio
make
# Install LimboRadio (Install binaries, systemd autostart)
make install

# Reboot
reboot
```

### WiFi configuration
To allow changes in wifi configuration without cable-connect, LimboRadio comes with the following setup:
- With "make setup" you already configured netctl for profile based wifi auto-re-connect.
- There's a fallback profile trying to connect to a wifi hotspot with SSID "yeah" and password "yeahyeah".
- There's another profile that could be configured via LimboRadio WebUI by opening http://[IP shown in display] in your browser.

So if you're still cable-connected: Simply enter the URL. Otherwise create an access point with the credentials mentioned above an you'll see LimboRadio connecting in a minute.

## DONE! -> ENJOY!


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


### Open improvements for future versions
#### PCB design
* 3 resistors at the bottom are not required. RasPi internal pull-ups doing things just fine.
* Remove some mm at the bottom because it collides with the ethernet port on a full raspberry.
* Display mount holes are a bit to small.
