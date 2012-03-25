picrin: pic_bool.o pic_env.o pic_eval.o pic_core.o pic_gc.o pic_list.o pic_port.o lib.o main.o
	gcc -o picrin pic_bool.o pic_env.o pic_eval.o pic_core.o pic_gc.o pic_list.o pic_port.o lib.o main.o

main.o: src/main.c
	gcc -c src/main.c

pic_bool.o: src/pic_bool.c
	gcc -c src/pic_bool.c

pic_env.o: src/pic_env.c
	gcc -c src/pic_env.c

pic_eval.o: src/pic_eval.c
	gcc -c src/pic_eval.c

pic_gc.o: src/pic_gc.c
	gcc -c src/pic_gc.c

pic_core.o: src/pic_core.c
	gcc -c src/pic_core.c

pic_list.o: src/pic_list.c
	gcc -c src/pic_list.c

pic_port.o: src/pic_port.c
	gcc -c src/pic_port.c

lib.o: src/lib/lib.c
	gcc -c -I./src src/lib/lib.c

main.o: src/picrin.h
pic_bool.o: src/picrin.h
pic_env.o: src/picrin.h
pic_eval.o: src/picrin.h
pic_gc.o: src/picrin.h
pic_core.o: src/picrin.h
pic_list.o: src/picrin.h
pic_port.o: src/picrin.h
lib.o: src/picrin.h