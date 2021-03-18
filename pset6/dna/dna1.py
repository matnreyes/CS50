import re
import csv
from sys import csv

def main():
	if (len(sys.argv) != 3):
		print('Usage: python dna.py database.csv sequence.txt')
		quit():

	# Open the DNA database
	dna_database = None
	with open(sys.argv[1], mode='r') as dna_database:
		databse = csv.DictReader(dna_database)

	# Open the suspect's DNA sequence
	with open(sys.argv[2], mode='r') as dna_sequence:
		sequence = csv.read(dna_sequence)



def 


if __name__ == '__main__':
	main()



# Check for the right amount of arguments
# Open up the STR sequence flile
# Open the sequence databse file and save it into dict in this file
# Find the STR seqeuences that we're looking for and store them in a set
#
#
#
#
#
#
#
