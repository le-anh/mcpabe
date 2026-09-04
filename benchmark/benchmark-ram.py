import csv
import subprocess
from pathlib import Path


def get_process_ram_kb(process_id):
	with open(f"/proc/{process_id}/status", encoding="utf-8") as status_file:
		for line in status_file:
			if line.startswith("VmRSS:"):
				return int(line.split()[1])
	return 0


def measure_command(command, loops, project_dir):
	peak_ram_kb = 0
	for _ in range(loops):
		process = subprocess.Popen(
			command,
			cwd=project_dir,
			stdout=subprocess.DEVNULL,
			stderr=subprocess.DEVNULL,
		)
		try:
			peak_ram_kb = max(peak_ram_kb, get_process_ram_kb(process.pid))
		except FileNotFoundError:
			pass
		while process.poll() is None:
			try:
				peak_ram_kb = max(peak_ram_kb, get_process_ram_kb(process.pid))
			except FileNotFoundError:
				break
		process.wait()
		if process.returncode != 0:
			raise subprocess.CalledProcessError(process.returncode, command)
	return peak_ram_kb


def run_mcpabe_benchmark(
	first_attribute=1,
	last_attribute=50,
	loops=10000,
	results_file="benchmark-mcpabe-ram-results.csv",
):
	project_dir = Path(__file__).resolve().parent
	setup_executable = project_dir / "setup.out"
	keygen_executable = project_dir / "keygen.out"
	enc_executable = project_dir / "enc.out"
	dec_executable = project_dir / "dec.out"
	public_key = project_dir / "pub_key"
	master_key = project_dir / "master_key"
	private_key = project_dir / "lhanh_priv_key"
	input_file = project_dir / "plaintext.txt"
	output_file = project_dir / "plaintext.txt.cpabe"

	for required_file in (
		setup_executable,
		keygen_executable,
		enc_executable,
		dec_executable,
		# public_key,
		# master_key,
		input_file,
	):
		if not required_file.is_file():
			raise FileNotFoundError(
				f"{required_file} was not found. Build setup, keygen, encryption, and decryption first."
			)

	print(
		f"Measuring setup, keygen, encryption, and decryption RAM for {first_attribute} to "
		f"{last_attribute} attributes, {loops:,} loops each..."
	)

	results_path = project_dir / results_file
	with results_path.open("a", newline="", encoding="utf-8-sig") as csv_file:
		writer = csv.writer(csv_file)
		writer.writerow(
			[
				"Loop number",
				"Number of attributes",
				"Setup peak RAM (KB)",
				"Keygen peak RAM (KB)",
				"Encryption peak RAM (KB)",
				"Decryption peak RAM (KB)",
			]
		)

	setup_command = [str(setup_executable)]
	for attribute_count in range(first_attribute, last_attribute + 1):
		attributes = [f"att{number}" for number in range(1, attribute_count + 1)]
		policy = " and ".join(attributes)

		setup_ram = measure_command(setup_command, loops, project_dir)

		keygen_command = [
			str(keygen_executable),
			"-o",
			str(private_key),
			str(public_key),
			str(master_key),
			*attributes,
		]
		keygen_ram = measure_command(keygen_command, loops, project_dir)

		encrypt_command = [
			str(enc_executable),
			"-k",
			"-o",
			str(output_file),
			str(public_key),
			str(input_file),
			policy,
		]

		enc_ram = measure_command(encrypt_command, loops, project_dir)

		decrypt_command = [
			str(dec_executable),
			str(public_key),
			str(private_key),
			str(output_file),
		]
		dec_ram = measure_command(decrypt_command, loops, project_dir)

		print(
			f"{attribute_count:>2} attribute(s): "
			f"setup peak RAM {setup_ram} KB, "
			f"keygen peak RAM {keygen_ram} KB, "
			f"encryption peak RAM {enc_ram} KB, "
			f"decryption peak RAM {dec_ram} KB"
		)

		with results_path.open("a", newline="", encoding="utf-8-sig") as csv_file:
			writer = csv.writer(csv_file)
			writer.writerow(
				[
					loops,
					attribute_count,
					setup_ram,
					keygen_ram,
					enc_ram,
					dec_ram,
				]
			)

	print(f"\nResults exported to {results_path}")
	print("Setup, keygen, encryption, and decryption RAM measurement complete.")

if __name__ == "__main__":
	run_mcpabe_benchmark()