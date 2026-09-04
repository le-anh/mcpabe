import csv
import subprocess
from pathlib import Path


def measure_file_sizes(
	first_attribute=1,
	last_attribute=50,
	results_file="benchmark-mcpabe-filesize-results.csv",
):
	project_dir = Path(__file__).resolve().parent
	plaintext_dir = project_dir.parent / "plaintext"
	setup_executable = project_dir / "setup"
	keygen_executable = project_dir / "keygen"
	enc_executable = project_dir / "enc"
	public_key = project_dir / "pub_key"
	master_key = project_dir / "master_key"
	private_key = project_dir / "lhanh_priv_key"
	plaintext_files = [
		plaintext_dir / f"plaintext_{number}.pdf" for number in range(1, 11)
	]

	for required_file in (
		setup_executable,
		keygen_executable,
		enc_executable,
		*plaintext_files,
	):
		if not required_file.is_file():
			raise FileNotFoundError(
				f"{required_file} was not found. Build the tools and provide all plaintext files first."
			)

	results_path = project_dir / results_file
	with results_path.open("w", newline="", encoding="utf-8-sig") as csv_file:
		writer = csv.writer(csv_file)
		writer.writerow(
			[
				"Plaintext file",
				"Number of attributes",
				"Private key size (bytes)",
				"Plaintext size (bytes)",
				"Ciphertext size (bytes)",
			]
		)

	for plaintext_file in plaintext_files:
		for attribute_count in range(first_attribute, last_attribute + 1):
			attributes = [f"att{number}" for number in range(1, attribute_count + 1)]
			policy = " and ".join(attributes)

			subprocess.run(
				[str(setup_executable)],
				cwd=project_dir,
				stdout=subprocess.DEVNULL,
				stderr=subprocess.DEVNULL,
				check=True,
			)
			subprocess.run(
				[
					str(keygen_executable),
					"-o",
					str(private_key),
					str(public_key),
					str(master_key),
					*attributes,
				],
				cwd=project_dir,
				stdout=subprocess.DEVNULL,
				stderr=subprocess.DEVNULL,
				check=True,
			)

			private_key_size = private_key.stat().st_size
			with results_path.open("a", newline="", encoding="utf-8") as csv_file:
				writer = csv.writer(csv_file)
				ciphertext_file = Path(f"{plaintext_file}.cpabe")
				subprocess.run(
					[
						str(enc_executable),
						"-k",
						"-o",
						str(ciphertext_file),
						str(public_key),
						str(plaintext_file),
						policy,
					],
					cwd=project_dir,
					stdout=subprocess.DEVNULL,
					stderr=subprocess.DEVNULL,
					check=True,
				)
				writer.writerow(
					[
						plaintext_file.name,
						attribute_count,
						private_key_size,
						plaintext_file.stat().st_size,
						ciphertext_file.stat().st_size,
					]
				)

			print(f"{plaintext_file.name}: {attribute_count:>2} attribute(s) measured")

	print(f"\nResults exported to {results_path}")


if __name__ == "__main__":
	measure_file_sizes()