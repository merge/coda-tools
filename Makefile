all:
	gcc -Wall -Wextra -pedantic -Wstrict-overflow -fno-strict-aliasing -o imx_vpu_fw_cleaner imx_vpu_fw_cleaner.c
clean:
	rm -f *.o imx_vpu_fw_cleaner
