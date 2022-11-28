CC     := /home/arun/Desktop/CheckCBox_Compiler/llvm/cmake-build-debug/bin/clang
MACROS := 
CFLAGS := -I. -Wundef -Wall -Wextra -fw2c_sbx -ferror-limit=452935 -g $(MACROS)
LDFLAGS := -ldl -lstdc++ -lSBX_CON_LIB -lisc_lib_final
LIBS := -L/home/arun/Desktop/tiny-bignum-c/Checkcbox_LIBS

all:
	@$(CC) $(CFLAGS) bn.c ./tests/golden.c      -o ./build/test_golden $(LIBS) $(LDFLAGS)
	@$(CC) $(CFLAGS) bn.c ./tests/hand_picked.c -o ./build/test_hand_picked $(LIBS) $(LDFLAGS)
	@$(CC) $(CFLAGS) bn.c ./tests/load_cmp.c    -o ./build/test_load_cmp $(LIBS) $(LDFLAGS)
	@$(CC) $(CFLAGS) bn.c ./tests/factorial.c   -o ./build/test_factorial $(LIBS) $(LDFLAGS)
	@$(CC) $(CFLAGS) bn.c ./tests/randomized.c  -o ./build/test_random $(LIBS) $(LDFLAGS)
	@#$(CC) $(CFLAGS) bn.c ./tests/rsa.c         -o ./build/test_rsa $(LIBS) $(LDFLAGS)


test:
	@echo
	@echo ================================================================================
	@./build/test_golden
	@echo ================================================================================
	@./build/test_hand_picked
	@echo ================================================================================
	@./build/test_load_cmp
	@echo ================================================================================
	@python ./scripts/fact100.py
	@./build/test_factorial
	@echo ================================================================================
	@python ./scripts/test_old_errors.py
	@echo ================================================================================
	@#./build/test_rsa
	@#echo ================================================================================
	@python ./scripts/test_rand.py 1000
	@echo ================================================================================
	@echo

clean:
	@rm -f ./build/*


