"""
"""

import string
import random
import sys


NUM_LINES = 77280	# this should be the number of total strings % number of threads
MAX_LINE_LEN = 100
NUM_FILES = int(sys.argv[1])

string_sets = [set()] * NUM_FILES

print(string_sets)


for i in range(0, NUM_FILES):
	filename = "thread" + str(i+1) + ".txt"
	open(filename, "w").close()	# empty the file if it exists

	file = open(filename, "w")

	for _ in range(NUM_LINES):
		new_string = ""

		while True:
			found = False
			new_string = ''.join(random.choice(string.ascii_uppercase + string.digits + string.ascii_lowercase) for _ in range(random.randrange(1, MAX_LINE_LEN))) + "\n"
		
			for string_set in string_sets:
				if new_string in string_set:
					found = True
					break

			if not found:
				break

		file.write(new_string)
		string_sets[i].add(new_string)

	file.close()
