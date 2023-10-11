"""Software starter.

Author: Stefano Magrini Alunno
Date: 2023 / 10 / 06

This script allows communication with the working system to initiate specific analyses.
Additionally, it removes previous files and reconstructs the necessary directories while
preparing the files required for the proper functioning of the software.

This script imports the following functions:
	* 'analyze_work',
	* 'analyze_author',
	* 'analyze'
from the 'analysis' file.

It requires the installation of:
	* 'os', 'shutil', 'subprocess', 'sys', 'typing'
"""

from analysis import work_analysis
from analysis import author_analysis
from analysis import analysis
from typing import List, Dict
import os
import shutil
import subprocess
import sys

""" All directory of the software """
training_directory = os.path.join('Set', 'Training')  # directory di Training
training_synthesis_directory = os.path.join('Set', 'Training_Synthesis')
training_analysis_directory = os.path.join('Set', 'Training_Analysis')
test_directory = os.path.join('Set', 'Test')  # directory di Test
test_synthesis_directory = os.path.join('Set', 'Test_Synthesis')
test_analysis_directory = os.path.join('Set', 'Test_Analysis')
temporary_directory = 'Temporary'
source_synthesis_directory = os.path.join('Source', 'C', 'synthesis')
source_comparison_directory = os.path.join('Source', 'C', 'comparison')


def read_training(directory: str) -> Dict[str, List[str]]:
	"""
	This function reads the training_set, extracting the authors and their works.

	Writes the number of Gigabytes read.

	Parameters
	----------
	directory: str
		Directory to read.

	Returns
	-------
	authors_info : Dict[str, List[str]]
		A dictionary that associates each author with their list of works.
		author_info[author_name] is a list.

	"""
	authors_info = {}
	authors_list = os.listdir(directory)

	for author in authors_list:
		author_path = os.path.join(directory, author)
		authors_info[author] = os.listdir(author_path)

	total_bytes = 0
	for author, works_list in authors_info.items():
		for work in works_list:
			total_bytes = total_bytes + \
				os.path.getsize(os.path.join(directory, author, work))
	mega_bytes = total_bytes / 1024 / 1024
	format_mb = "{:.0f}".format(mega_bytes)
	format_gb = "{:.2f}".format(mega_bytes/1024)
	print(f"Weight of train set: {format_mb}MB ({format_gb}GB)")

	return authors_info


def read_test(directory: str) -> List[str]:
	"""
	This function reads the test_set, extracting the authors and their works.

	Writes the number of Megabytes read.

	Parameters
	----------
	directory : str
		Directory to read.

	Returns
	-------
	works_info : List[str]
		List of works to attribute.

	"""
	works_info = os.listdir(directory)

	total_bytes = 0
	for work in works_info:
		total_bytes = total_bytes + os.path.getsize(os.path.join(directory, work))


	format_mb = "{:.2f}".format(total_bytes/1024/1024)
	print(f"Weight of test set: {format_mb}MB")
	return works_info


def train_synth(training: Dict[str, List[str]], test: List[str]):
	"""
	Perform a synth on all.

	Create the synthesis folder and prepare the input file.
	It will call the file that performs all the syntheses in parallel.

	Parameters
	----------
	training : Dict[str, List[str]]
		Is the dictionary of the training set
	test : List[str]
		Is the list of the test set

	Returns
	-------
	None.

	"""
	# refresh Training_Synthesis folder
	if os.path.exists(training_synthesis_directory):
		shutil.rmtree(training_synthesis_directory)

	for author in training:
		author_folder_path = os.path.join(
			training_synthesis_directory, author)
		os.makedirs(author_folder_path, exist_ok=True)

	# make directory of temporary file input.txt
	input_txt_path = os.path.join(temporary_directory, "input.txt")

	work_directory_list = []
	for author, works in training.items():
		for work in works:
			work_directory_list.append(os.path.join(author, f"{work}"))
	num_of_directory = len(work_directory_list)

	input_txt_contest = f"{training_directory}\n"
	input_txt_contest += f"{training_synthesis_directory}\n"
	input_txt_contest += f"{num_of_directory}\n"
	input_txt_contest += "\n".join(work_directory_list)

	with open(input_txt_path, "w") as file_input:
		file_input.write(input_txt_contest)

	# start synthesis
	executable_path = os.path.join(source_synthesis_directory, "synthesis")
	print("Starting synthesis program...")
	try:
		result = subprocess.run(
			[executable_path,
				os.path.join(temporary_directory, "input.txt")],
			stderr=subprocess.PIPE, text=True)
		if result.returncode != 0:
			print("Error...")
			print(result.stderr)
			sys.exit(1)
		else:
			print("Any Error!\n")
	except Exception as e:
		print(f"Error: {e}")
		sys.exit(1)

	return


def analysis_training(training: Dict[str, List[str]], test: List[str]):
	"""
	Perform an analysis on the synthesis.

	Make directory for analysis, call module 'train'.

	Parameters
	----------
	training : Dict[str, List[str]]
		Is the dictionary of the training set
	test : List[str]
		Il the list of the test set

	Returns
	-------
	None.

	"""
	# refresh folder Training_Analysis_Works
	if os.path.exists(training_analysis_directory):
		shutil.rmtree(training_analysis_directory)

	for author in training:
		author_folder_path = os.path.join(
			training_analysis_directory, author)
		os.makedirs(author_folder_path, exist_ok=True)
		for work in training[author]:
			works_path = os.path.join(
				training_analysis_directory, author, work.replace('.ppm', ''))
			os.makedirs(works_path, exist_ok=True)

	# analysis of the synthesis, di author e del dataset
	print("Starting analysis of synthesis...")

	author_works_analysis = []
	for author in training:
		works_list_analysis = []
		print(f"\t * analyzing {author}")
		for work in training[author]:
			src_file = os.path.join(
				training_synthesis_directory, author, work.replace('.ppm', '.bin'))
			dest_dir = os.path.join(
				training_analysis_directory, author, work.replace('.ppm', ''))
			works_list_analysis.append(
				work_analysis(src_file, dest_dir))

		author_works_analysis.append(author_analysis(os.path.join(
			training_analysis_directory, author), works_list_analysis))
	analysis(training_analysis_directory, author_works_analysis)

	print("Any Error!\n")
	return


def test_synth(training: Dict[str, List[str]], test: List[str]):
	"""
	Performs synthesis for each individual work.

	Creates the synthesis folder and prepares the input file.
	It will call the file that performs all the syntheses in parallel.

	Parameters
	----------
	training : Dict[str, List[str]]
		It's the training set dictionary.
	test : List[str]
		It's the test set list.

	Returns
	-------
	None.

	"""
	# refresh Test_Synthesis folder
	if os.path.exists(test_synthesis_directory):
		shutil.rmtree(test_synthesis_directory)

	if len(test) != 0:
		os.makedirs(test_synthesis_directory, exist_ok=True)

	# make dir for input.txt file
	input_txt_path = os.path.join(temporary_directory, "input.txt")

	work_directory_list = test
	num_of_directory = len(work_directory_list)

	input_txt_contest = f"{test_directory}\n"
	input_txt_contest += f"{test_synthesis_directory}\n"
	input_txt_contest += f"{num_of_directory}\n"
	input_txt_contest += "\n".join(work_directory_list)

	with open(input_txt_path, "w") as file_input:
		file_input.write(input_txt_contest)

	# start synth program
	executable_path = os.path.join(source_synthesis_directory, "synthesis")
	print("Starting synthesis program...")
	try:
		result = subprocess.run(
			[executable_path,
				os.path.join(temporary_directory, "input.txt")],
			stderr=subprocess.PIPE, text=True)
		if result.returncode != 0:
			print("Error...")
			print(result.stderr)
			sys.exit(1)
		else:
			print("Any Error!\n")
	except Exception as e:
		print(f"Error: {e}")
		sys.exit(1)

	return


def analysis_test(training, test):
	"""
	Performs the analysis of each synthesis.

	Creates the analysis folder using the 'train' functions.

	Parameters
	----------
	training : Dict[str, List[str]]
		It's the training set dictionary.
	test : List[str]
		It's the test set list.

	Returns
	-------
	None.

	"""
	# refresh Test_Analysis_Works folder
	if os.path.exists(test_analysis_directory):
		shutil.rmtree(test_analysis_directory)

	for work in test:
		works_path = os.path.join(
			test_analysis_directory, work.replace('.ppm', ''))
		os.makedirs(works_path, exist_ok=True)

	# Analysis of directories
	print("Starting analysis of syntheses...")

	works_list_analysis = []
	for work in test:
		print(f"\t * analyzing work {work}")
		src_file = os.path.join(
			test_synthesis_directory, work.replace('.ppm', '.bin'))
		dest_dir = os.path.join(
			test_analysis_directory, work.replace('.ppm', ''))
		works_list_analysis.append(
			work_analysis(src_file, dest_dir))

	analysis(test_analysis_directory, works_list_analysis)

	print("Any Error!\n")
	return


def comparison(training, test):
	return


def analysis_comparison(training, test):
	return


def attribution(training, test):
	return


if __name__ == "__main__":
	print("\n\n\n\n ----- ATTRIBUTION OF WORKS ----- \n")
	print("Reading file...")
	authors_info = read_training(training_directory)
	works_info = read_test(test_directory)
	print("End reading!\n")

	print("Set the operation that you would performs:")
	print("\t0) synth of training set")
	print("\t1) analysis of training set")
	print("\t2) synth of test set")
	print("\t3) analysis of test set")
	print("\t4) comparison")
	print("\t5) analysis of comparison")
	print("\t6) attribution")
	chosen = int(input("Set the operation that you would performs, specifying the number: "))
	instruction = (train_synth,
					analysis_training,
					test_synth,
					analysis_test,
					comparison,
					analysis_comparison,
					attribution)
	print(f"Start program da {chosen}\n")

	for i in range(7):
		if i >= chosen:
			instruction[i](authors_info, works_info)
