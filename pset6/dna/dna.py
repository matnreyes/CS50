import csv
import sys
import re

STR_order = ["AGATC", "TTTTTTCT", "AATG", "TCTAG", "GATA", "TATC", "GAAA", "TCTG"]


def main():
	if len(sys.argv) != 3:
		print("Usage: python dna.py data.csv sequence.txt")
		quit()

	# Open dna sequence and asve onto memory
	with open(sys.argv[2], mode="r") as sequence:
		dna_sequence = sequence.read()

	# Get count for ind STRs
	STR_index = countSTR(dna_sequence)

	# Find killer
	killer = findMatch(STR_index, sys.argv[1])
	print(killer)


def findMatch(index, path):
	match = None
	with open(path, mode="r") as dna_database:
		database = csv.DictReader(dna_database)

		# Traverse rows of CSV database file til match is found
		for suspect in database:
			col = 0
			for column in STR_order:
				STRSEQ = float(suspect[column])

				# Skip person if item doens't match
				if (STRSEQ != index[col]):
					break

				if (column == 'TCTG'):
					match = suspect['name']
					return match
				col += 1


	return 'No Match'


def countSTR(dna_sequence):
	# Regex for each DNA Sequence
	reg_AGATC = r"(?:AGATC)+"
	reg_TTTTTTCT = r"(?:TTTTTTCT)+"
	reg_AATG = r"(?:AATG)+"
	reg_TCTAG = r"(?:TCTAG)+"
	reg_GATA = r"(?:GATA)+"
	reg_TATC = r"(?:TATC)+"
	reg_GAAA = r"(?:GAAA)+"
	reg_TCTG = r"(?:TCTG)+"

	# Find all matching instances for the DNA sequence in provided file
	AGATC_matches = re.finditer(reg_AGATC, dna_sequence)
	TTTTTTCT_matches = re.finditer(reg_TTTTTTCT, dna_sequence)
	AATG_matches = re.finditer(reg_AATG, dna_sequence)
	TCTAG_matches = re.finditer(reg_TCTAG, dna_sequence)
	GATA_matches = re.finditer(reg_GATA, dna_sequence)
	TATC_matches = re.finditer(reg_TATC, dna_sequence)
	GAAA_matches = re.finditer(reg_GAAA, dna_sequence)
	TCTG_matches = re.finditer(reg_TCTG, dna_sequence)

	# Initiliaze return list for STRs
	STRs = [0, 0, 0, 0, 0, 0, 0, 0]

	# Iterate through matches and find longest running match
	for match in AGATC_matches:
		start = match.start()
		end = match.end()
		seqlen = (end - start) / 5

		if (seqlen > STRs[0]):
			STRs[0] = seqlen

	for match in TTTTTTCT_matches:
		start = match.start()
		end = match.end()
		seqlen = (end - start) / 8

		if (seqlen > STRs[1]):
			STRs[1] = seqlen

	for match in AATG_matches:
		start = match.start()
		end = match.end()
		seqlen = (end - start) / 4

		if (seqlen > STRs[2]):
			STRs[2] = seqlen

	for match in TCTAG_matches:
		start = match.start()
		end = match.end()
		seqlen = (end - start) / 5

		if (seqlen > STRs[3]):
			STRs[3] = seqlen

	for match in GATA_matches:
		start = match.start()
		end = match.end()
		seqlen = (end - start) / 4

		if (seqlen > STRs[4]):
			STRs[4] = seqlen

	for match in TATC_matches:
		start = match.start()
		end = match.end()
		seqlen = (end - start) / 4

		if (seqlen > STRs[5]):
			STRs[5] = seqlen

	for match in GAAA_matches:
		start = match.start()
		end = match.end()
		seqlen = (end - start) / 4

		if (seqlen > STRs[6]):
			STRs[6] = seqlen

	for match in TCTG_matches:
		start = match.start()
		end = match.end()
		seqlen = (end - start) / 4

		if (seqlen > STRs[7]):
			STRs[7] = seqlen

	return STRs


if __name__ == "__main__":
	main()