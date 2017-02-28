rsync -azP $HOME/* u1:~/
mpirun -np 2 -f machinefile ./genv2.out 
