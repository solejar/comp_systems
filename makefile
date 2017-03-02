all: part_a part_b part_c part_d

all_with_file: part_a part_b part_c part_d file_gen

part_a:
	gcc -std=c99 -o part_a parta_old.c -lm

part_b:
	gcc -std=c99 -o part_b partb_old.c -lm

part_c:
	gcc -std=c99 -o part_c partc_old.c -lm

part_d:
	gcc -std=c99 -o part_d partd_old.c -lm

file_gen:
	gcc -std=c99 -o file_gen file_gen.c -lm

clean:
	$(RM) part_a part_b part_c part_d file_gen	
