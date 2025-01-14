echo 'options msm261 msm261_debug=1' | sudo tee /etc/modprobe.d/msm261.conf
echo -n "soc:msm261" | sudo tee /sys/bus/platform/drivers/msm261/unbind || true
make clean
make
sudo rmmod msm261 || true
sudo cp msm261.ko /lib/modules/$(uname -r)/extra/
echo -n "sudo depmod -a" | sudo depmod -a
echo -n "sudo modprobe msm261 msm261_debug=1" | sudo modprobe msm261 msm261_debug=1
