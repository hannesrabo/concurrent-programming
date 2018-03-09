echo "" > centralized.dat
mpirun -np 2 ./centralized >> centralized.dat
mpirun -np 4 ./centralized >> centralized.dat
mpirun -np 8 ./centralized >> centralized.dat
mpirun -np 16 ./centralized >> centralized.dat
echo "Run finished!"
