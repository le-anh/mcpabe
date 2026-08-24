import csv
import subprocess
import time
from pathlib import Path


def measure_enc_time(
	first_attribute=1,
	last_attribute=50,
	loops=10000,
	results_file="benchmark-results.csv",
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
		f"Measuring setup, keygen, encryption, and decryption time for {first_attribute} to "
		f"{last_attribute} attributes, {loops:,} loops each..."
	)

	setup_command = [str(setup_executable)]
	results = []
	for attribute_count in range(first_attribute, last_attribute + 1):
		attributes = [f"att{number}" for number in range(1, attribute_count + 1)]
		policy = " and ".join(attributes)

		start = time.perf_counter()
		for _ in range(loops):
			subprocess.run(
				setup_command,
				cwd=project_dir,
				stdout=subprocess.DEVNULL,
				stderr=subprocess.DEVNULL,
				check=True,
			)
		setup_elapsed = time.perf_counter() - start

		keygen_command = [
			str(keygen_executable),
			"-o",
			str(private_key),
			str(public_key),
			str(master_key),
			*attributes,
		]
		start = time.perf_counter()
		for _ in range(loops):
			subprocess.run(
				keygen_command,
				cwd=project_dir,
				stdout=subprocess.DEVNULL,
				stderr=subprocess.DEVNULL,
				check=True,
			)
		keygen_elapsed = time.perf_counter() - start

		encrypt_command = [
			str(enc_executable),
			"-k",
			"-o",
			str(output_file),
			str(public_key),
			str(input_file),
			policy,
		]

		start = time.perf_counter()
		for _ in range(loops):
			subprocess.run(
				encrypt_command,
				cwd=project_dir,
				stdout=subprocess.DEVNULL,
				stderr=subprocess.DEVNULL,
				check=True,
			)
		enc_elapsed = time.perf_counter() - start

		decrypt_command = [
			str(dec_executable),
			str(public_key),
			str(private_key),
			str(output_file),
		]
		start = time.perf_counter()
		for _ in range(loops):
			subprocess.run(
				decrypt_command,
				cwd=project_dir,
				stdout=subprocess.DEVNULL,
				stderr=subprocess.DEVNULL,
				check=True,
			)
		dec_elapsed = time.perf_counter() - start

		results.append(
			(
				loops,
				attribute_count,
				setup_elapsed,
				setup_elapsed / loops,
				keygen_elapsed,
				keygen_elapsed / loops,
				enc_elapsed,
				enc_elapsed / loops,
				dec_elapsed,
				dec_elapsed / loops,
			)
		)
		print(
			f"{attribute_count:>2} attribute(s): "
			f"setup total {setup_elapsed:.6f}s "
			f"(average {setup_elapsed / loops:.6f}s), "
			f"keygen total {keygen_elapsed:.6f}s "
			f"(average {keygen_elapsed / loops:.6f}s), "
			f"encryption total {enc_elapsed:.6f}s "
			f"(average {enc_elapsed / loops:.6f}s), "
			f"decryption total {dec_elapsed:.6f}s "
			f"(average {dec_elapsed / loops:.6f}s)"
		)

	results_path = project_dir / results_file
	with results_path.open("w", newline="", encoding="utf-8-sig") as csv_file:
		writer = csv.writer(csv_file)
		writer.writerow(
			[
				"Loop number",
				"Number of attributes",
				"Setup total",
				"Setup average",
				"Keygen total",
				"Keygen average",
				"Encryption total",
				"Encryption average",
				"Decryption total",
				"Decryption average",
			]
		)
		writer.writerows(results)

	print(f"\nResults exported to {results_path}")
	print("Setup, keygen, encryption, and decryption time measurement complete.")
	return results


if __name__ == "__main__":
	measure_enc_time(1, 10, 10)