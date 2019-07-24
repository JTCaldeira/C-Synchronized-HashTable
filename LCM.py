"""
Script to generate the least common multiple of all the integers
from 1 through a given number. Usage example:

python3 LCM.py 8

Output: 840
"""

import sys


numbers = list(range(1, int(sys.argv[1]) + 1))
print("Least common multiple from 1 through " + sys.argv[1] + ":")

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
