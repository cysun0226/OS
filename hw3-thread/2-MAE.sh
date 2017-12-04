g++ -std=c++11 -pthread $1 -o sobel.out
./sobel.out
echo "\n--- check Sobel1.bmp ---"
./MAE.out ANS/Sobel1.bmp Sobel1.bmp
echo "--- check Sobel2.bmp ---"
./MAE.out ANS/Sobel2.bmp Sobel2.bmp
echo "--- check Sobel3.bmp ---"
./MAE.out ANS/Sobel3.bmp Sobel3.bmp
echo "--- check Sobel4.bmp ---"
./MAE.out ANS/Sobel4.bmp Sobel4.bmp
echo "--- check Sobel5.bmp ---"
./MAE.out ANS/Sobel5.bmp Sobel5.bmp

./2-clean.sh
