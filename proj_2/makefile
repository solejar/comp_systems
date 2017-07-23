ll: part_1a part_1b part_1c mtsort

part_1a:
	gcc -std=c99 -o part_1a part1a.c -lm -D_POSIX_C_SOURCE=199309L

part_1b:
	gcc -std=c99 -o part_1b part1b.c -lm -D_POSIX_C_SOURCE=199309L

part_1c:
	gcc -std=c99 -o part_1c part1c.c -lm -D_POSIX_C_SOURCE=199309L

mtsort:
	gcc -o mtsort mtsort.c mypthread.c

clean:
	$(RM) part_1a part_1b part_1c mtsort	
