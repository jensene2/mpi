all: main.out # genv2.out genv1.out scatter.out

# genv1.out: genv1.c
# 	mpic++ --std=c++11 genv1.c -o genv1.out
# genv2.out: genv2.c
# 	mpic++ --std=c++11 genv2.c -o genv2.out
# scatter.out: scatter.c
# 	mpic++ --std=c++11 scatter.c -o scatter.out
main.out: main.c
	mpic++ --std=c++11 main.c -o main.out

clean:
	rm *.out
