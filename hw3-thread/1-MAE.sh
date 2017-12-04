g++ -std=c++11 -pthread $1
./a.out
echo "\n--- check Blur1.bmp ---"
./MAE.out ANS/Blur1.bmp Blur1.bmp
echo "--- check Blur2.bmp ---"
./MAE.out ANS/Blur2.bmp Blur2.bmp
echo "--- check Blur3.bmp ---"
./MAE.out ANS/Blur3.bmp Blur3.bmp
echo "--- check Blur4.bmp ---"
./MAE.out ANS/Blur4.bmp Blur4.bmp
echo "--- check Blur5.bmp ---"
./MAE.out ANS/Blur5.bmp Blur5.bmp
./1-clean.sh
