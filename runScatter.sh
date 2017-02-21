rsync -azP $HOME/* u1:~/
mpirun -np 20 -f machinefile ./scatter.out 
