sudo dtc -@ -I dts -O dtb -o msm261.dtbo msm261-overlay.dts
sudo cp msm261.dtbo /boot/overlays
sudo chmod 755 /boot/overlays/msm261.dtbo
sudo rmmod msm261 || true
sudo cp msm261.ko /lib/modules/$(uname -r)/extra/
sudo depmod -a
sudo modprobe msm261 msm261_debug=1


