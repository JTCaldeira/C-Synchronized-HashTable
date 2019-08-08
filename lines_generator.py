"""
"""

import string
import random
import sys
import getopt


MAX_LINE_LEN_FLAG = "l"
MAX_LINE_LEN = 100


def print_usage():

	message =	"USAGE:\tpython3 " + sys.argv[0] + " <NUM_CORES> <NUM_LINES>\n\n" \
					+ "Optional arguments:" \
					+ "\n\t-" + MAX_LINE_LEN_FLAG \
					+ ": The maximum length of each generated string." \
					+ "\n\t\tDefault value: 100"


	print(message)


def parse_args():

	num_lines = None

	if len(sys.argv) < 3:
		print_usage()
		sys.exit(2)
	else:
		global MULTIPLIER_FLAG
		global MAX_LINE_LEN_FLAG
		global NUM_LINES_FLAG

		try:
			flags = MAX_LINE_LEN_FLAG + ":"
			opts, args = getopt.gnu_getopt(sys.argv[1:], flags)
		except:
			print_usage()
			sys.exit(2)

		for opt, arg in opts:
			if opt == "-" + MAX_LINE_LEN_FLAG:
				MULTIPLIER = int(arg)
			elif opt == "-" + MAX_LINE_LEN_FLAG:
				MAX_LINE_LEN_FLAG = int(arg)
			elif opt == "-" + NUM_LINES_FLAG:
				num_lines = int(arg)
			else:
				print_usage()
				sys.exit(2)

	return int(sys.argv[1]), int(sys.argv[2])



def least_common_multiple(num_cores):
	"""
	Finds the least common multiple of all the integers
	from 1 through a a given number.
	@arg num_cores: the greatest number of the set of integers.
	@return: the least common multiple.
	"""

	numbers = list(range(1, num_cores + 1))

	multiple = 1

	while True:
		found = True

		for number in numbers:
			if multiple % number != 0:
				found = False
				break

		if found:
			return multiple

		multiple += 1


def main():

	global MULTIPLIER
	global MAX_LINE_LEN

	num_cores, num_lines = parse_args()
	lcm = least_common_multiple(num_cores)

	if not num_lines:
		num_lines = lcm * MULTIPLIER

	string_sets = [set()] * num_cores

	for i in range(0, num_cores):
		filename = "thread" + str(i+1) + ".txt"
		open(filename, "w").close()	# empty the file if it exists

		file = open(filename, "w")

		for _ in range(int(num_lines/num_cores)):
			new_string = ""

			while True:
				found = False
				new_string = ''.join(random.choice(string.ascii_uppercase + string.digits + string.ascii_lowercase) for _ in range(random.randrange(1, MAX_LINE_LEN))) + "\n"
			
				for string_set in string_sets:
					if new_string in string_set:
						found = True
						break

				if not found:
					break;

			file.write(new_string)
			string_sets[i].add(new_string)

		file.close()





if __name__ == "__main__":
	main()
