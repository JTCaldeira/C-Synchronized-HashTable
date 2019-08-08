"""
Script to generate the least common multiple of all the integers
from 1 through a given number. Usage example:

python3 LCM.py 8

Output: 840
"""

import sys


num_threads = int(sys.argv[1])
numbers = list(range(1, int(num_threads) + 1))


multiple = 1

while True:
	found = True

	for number in numbers:
		if multiple % number != 0:
			found = False
			break

	if found:
		print(multiple)
		break

	multiple += 1
