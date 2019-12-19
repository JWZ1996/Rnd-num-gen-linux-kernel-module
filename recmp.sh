

echo "===== REMOVING MODULE ======"
echo ""
rmmod rnd_gen.ko
echo ""
echo "========== CLEAN ==========="
echo ""
make clean
echo ""
echo "=========== MAKE ==========="
echo ""
make
echo ""
echo "====== INSMOD MODULE ======="
echo ""

insmod rnd_gen.ko

echo ""
echo "======== RECOMPILE ========="
echo ""
g++ -o ioctl ./src/app_ioctl.cpp
g++ -o read ./src/app.cpp