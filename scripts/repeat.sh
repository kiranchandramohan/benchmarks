#!/bin/bash

repeat_cmd()
{
	for (( i = 10; i >= 1; i-- ))
	do
		for (( j = 1 ; j <= 5; j++ ))
		do
			echo "sudo /usr/bin/time -f "%E real,%U user,%S sys" ./$1 0 1 $i"
			#sudo /usr/bin/time -f "%E real,%U user,%S sys" ./$1 0 1 $i >> res 2>&1
		done
	done
}

#repeat_cmd omx_matmul_m3
#repeat_cmd omx_matmul_a9 
#repeat_cmd omx_matmul_2t
repeat_cmd omx_matmul_nt
