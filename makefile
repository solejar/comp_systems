all: part_a part_b part_c part_d

part_a:
	gcc -std=c99 -o part_a parta.c -lm

part_b:
	gcc -std=c99 -o part_b partb.c -lm

part_c:
	gcc -std=c99 -o part_c partc.c -lm

part_d:
	gcc -std=c99 -o part_d partd.c -lm

clean:
	$(RM) part_a part_b part_c part_d	
