echo "" > symmetric.dat
mpirun -np 2 ./symmetric >> symmetric.dat
mpirun -np 4 ./symmetric >> symmetric.dat
mpirun -np 8 ./symmetric >> symmetric.dat
mpirun -np 16 ./symmetric >> symmetric.dat
echo "Run finished!"
