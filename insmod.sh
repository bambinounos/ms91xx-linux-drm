#编译以及加载的示例脚本
make clean
make
sleep 5
sudo insmod ./drm/usbdisp_drm.ko
sudo insmod ./drm/usbdisp_usb.ko
lsmod | grep usbdisp
