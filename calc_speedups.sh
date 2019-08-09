num_cores=$1

if [ "$#" -ne 1 ]; then
    echo "Illegal number of parameters"
    exit 1
fi

lcm=$(python3 LCM.py "${num_cores}")

if ! gcc -Werror -Wall -Wextra -ggdb3 -std=c99 -D_GNU_SOURCE -pthread -o test big_string_test.c hashtable.c -lm; then
	echo "Compilation with errors or warnings."
	exit 1
fi

num_lines="$(("${lcm}"*100))"
num_lookups="$(("${num_lines}"*1000))"

for i in $(seq 1 $num_cores)
do
	echo "Generating $i files."
	python3 lines_generator.py $num_cores $num_lines

	echo "Running test with $i threads."
	if ! ./test $i $num_lines $num_lookups; then
		echo "Error running the test."
		exit 1
	fi
done
