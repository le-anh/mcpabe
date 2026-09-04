import csv
import resource
import subprocess
import time
from pathlib import Path


def measure_cpu(
	command,
	loops,
	project_dir,
):
	children_before = resource.getrusage(resource.RUSAGE_CHILDREN)
	start = time.perf_counter()
	for _ in range(loops):
		subprocess.run(
			command,
			cwd=project_dir,
			stdout=subprocess.DEVNULL,
			stderr=subprocess.DEVNULL,
			check=True,
		)
	elapsed = time.perf_counter() - start
	children_after = resource.getrusage(resource.RUSAGE_CHILDREN)
	cpu_time = (
		children_after.ru_utime
		+ children_after.ru_stime
		- children_before.ru_utime
		- children_before.ru_stime
	)
	return cpu_time, elapsed


def run_pbcpabe_benchmark(
	first_attribute=1,
	last_attribute=50,
	loops=10000,
	results_file="benchmark-pbcpabe-cpu-results.csv",
):
	project_dir = Path(__file__).resolve().parent
	setup_executable = project_dir / "setup"
	keygen_executable = project_dir / "keygen"
	enc_executable = project_dir / "enc"
	dec_executable = project_dir / "dec"
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
		f"Measuring setup, keygen, encryption, and decryption CPU for {first_attribute} to "
		f"{last_attribute} attributes, {loops:,} loops each..."
	)

	results_path = project_dir / results_file
	with results_path.open("a", newline="", encoding="utf-8-sig") as csv_file:
		writer = csv.writer(csv_file)
		writer.writerow(
			[
				"Loop number",
				"Number of attributes",
				"Setup CPU total (ms)",
				"Setup CPU average (ms)",
				"Setup CPU utilization (%)",
				"Keygen CPU total (ms)",
				"Keygen CPU average (ms)",
				"Keygen CPU utilization (%)",
				"Encryption CPU total (ms)",
				"Encryption CPU average (ms)",
				"Encryption CPU utilization (%)",
				"Decryption CPU total (ms)",
				"Decryption CPU average (ms)",
				"Decryption CPU utilization (%)",
			]
		)

	setup_command = [str(setup_executable)]
	for attribute_count in range(first_attribute, last_attribute + 1):
		attributes = [f"att{number}" for number in range(1, attribute_count + 1)]
		policy = " and ".join(attributes)

		setup_cpu, setup_elapsed = measure_cpu(setup_command, loops, project_dir)

		keygen_command = [
			str(keygen_executable),
			"-o",
			str(private_key),
			str(public_key),
			str(master_key),
			*attributes,
		]
		keygen_cpu, keygen_elapsed = measure_cpu(keygen_command, loops, project_dir)

		encrypt_command = [
			str(enc_executable),
			"-k",
			"-o",
			str(output_file),
			str(public_key),
			str(input_file),
			policy,
		]

		enc_cpu, enc_elapsed = measure_cpu(encrypt_command, loops, project_dir)

		decrypt_command = [
			str(dec_executable),
			str(public_key),
			str(private_key),
			str(output_file),
		]
		dec_cpu, dec_elapsed = measure_cpu(decrypt_command, loops, project_dir)

		print(
			f"{attribute_count:>2} attribute(s): "
			f"setup CPU {setup_cpu * 1000:.6f}ms "
			f"(average {setup_cpu / loops * 1000:.6f}ms, "
			f"utilization {setup_cpu / setup_elapsed * 100:.2f}%), "
			f"keygen CPU {keygen_cpu * 1000:.6f}ms "
			f"(average {keygen_cpu / loops * 1000:.6f}ms, "
			f"utilization {keygen_cpu / keygen_elapsed * 100:.2f}%), "
			f"encryption CPU {enc_cpu * 1000:.6f}ms "
			f"(average {enc_cpu / loops * 1000:.6f}ms, "
			f"utilization {enc_cpu / enc_elapsed * 100:.2f}%), "
			f"decryption CPU {dec_cpu * 1000:.6f}ms "
			f"(average {dec_cpu / loops * 1000:.6f}ms, "
			f"utilization {dec_cpu / dec_elapsed * 100:.2f}%)"
		)

		with results_path.open("a", newline="", encoding="utf-8-sig") as csv_file:
			writer = csv.writer(csv_file)
			writer.writerow([
				loops,
				attribute_count,
				setup_cpu * 1000,
				setup_cpu / loops * 1000,
				setup_cpu / setup_elapsed * 100,
				keygen_cpu * 1000,
				keygen_cpu / loops * 1000,
				keygen_cpu / keygen_elapsed * 100,
				enc_cpu * 1000,
				enc_cpu / loops * 1000,
				enc_cpu / enc_elapsed * 100,
				dec_cpu * 1000,
				dec_cpu / loops * 1000,
				dec_cpu / dec_elapsed * 100,
			])

	print(f"\nResults exported to {results_path}")
	print("Setup, keygen, encryption, and decryption CPU measurement complete.")

if __name__ == "__main__":
	run_pbcpabe_benchmark()
