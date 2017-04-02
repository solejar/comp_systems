all: part_a part_b part_c part_d

all_with_file: part_a part_b part_c part_d file_gen

part_a:
	gcc -std=c99 -o part_a part_a.c -lm

part_b:
	gcc -std=c99 -o part_b part_b.c -lm

part_c:
	gcc -std=c99 -o part_c part_c.c -lm

part_d:
	gcc -std=c99 -o part_d part_d.c -lm

file_gen:
	gcc -std=c99 -o file_gen file_gen.c -lm

clean:
	$(RM) part_a part_b part_c part_d file_gen	
