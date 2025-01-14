#!/bin/bash

echo "=========================="
echo "cat /boot/config.txt"
echo "=========================="
cat /boot/config.txt
echo

echo "=========================="
echo "cat /boot/firmware/config.txt | grep dtoverlay"
echo "=========================="
cat /boot/firmware/config.txt | grep dtoverlay
echo

echo "=========================="
echo "uname -a"
echo "=========================="
uname -a
echo

echo "=========================="
echo "grep -i "dtoverlay" /boot/config.txt"
echo "=========================="
grep -i "dtoverlay" /boot/config.txt
echo

echo "=========================="
echo "ls -la /boot/firmware/overlays | grep msm261"
echo "=========================="
ls -la /boot/firmware/overlays | grep msm261
echo

echo "=========================="
echo "sudo vcdbg log msg |& grep -i overlay"
echo "=========================="
sudo vcdbg log msg |& grep -i overlay
echo

echo "=========================="
echo "ls /dev/snd/"
echo "=========================="
sudo ls /dev/snd/
echo

echo "=========================="
echo "sudo cat /proc/device-tree/soc/sound/status"
echo "=========================="
sudo cat /proc/device-tree/soc/sound/status
echo

echo "=========================="
echo 'sudo dmesg | grep -i "audio\|i2s\|soc\|msm261"'
echo "=========================="
sudo dmesg | grep -i "audio\|i2s\|soc\|msm261"
echo

echo "=========================="
echo "lsmod | grep -E 'msm261|i2s|snd_soc'"
echo "=========================="
lsmod | grep -E 'msm261|i2s|snd_soc'
echo

echo "=========================="
echo " Device Tree Nodes"
echo "=========================="
find /proc/device-tree/ | grep -i msm261
echo

echo "=========================="
echo "sudo cat /sys/kernel/debug/gpio"
echo "=========================="
sudo cat /sys/kernel/debug/gpio
echo

echo "=========================="
echo " Sound Cards"
echo "=========================="
arecord -l
aplay -l
echo

echo "=========================="
echo "dmesg | grep -Ei 'msm261|i2s|snd_soc|error'"
echo "=========================="
dmesg | grep -Ei 'msm261|i2s|snd_soc|error'
echo

echo "=========================="
echo "cat /etc/asound.conf"
echo "=========================="
cat /etc/asound.conf
cat ~/.asoundrc 2>/dev/null || echo "~/.asoundrc not found"
echo

echo "=========================="
echo " End of Debug Information"
echo "=========================="
