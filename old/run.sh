rsync -azP $HOME/* u1:~/
mpirun -np 3 -f machinefile ./genv1.out 
