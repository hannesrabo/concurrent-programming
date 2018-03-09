echo "" > circular.dat
mpirun -np 2 ./circular >> circular.dat
mpirun -np 4 ./circular >> circular.dat
mpirun -np 8 ./circular >> circular.dat
mpirun -np 16 ./circular >> circular.dat
echo "Run finished!"
