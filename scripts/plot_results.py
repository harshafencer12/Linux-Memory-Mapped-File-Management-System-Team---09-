import csv
import matplotlib.pyplot as plt

CSV_FILE = "results/benchmark.csv"

sizes = []
size_labels = []
read_throughput = []
mmap_throughput = []
read_faults = []
mmap_faults = []

with open(CSV_FILE, newline="") as f:
    reader = csv.DictReader(f)

    rows = list(reader)

for row in rows:
    size = int(row["size_bytes"])

    if size not in sizes:
        sizes.append(size)
        size_labels.append(f"{size / (1024 * 1024):g} MB")

    if row["method"] == "read":
        read_throughput.append(float(row["throughput_mb_s"]))
        read_faults.append(int(row["minor_faults"]))

    elif row["method"] == "mmap":
        mmap_throughput.append(float(row["throughput_mb_s"]))
        mmap_faults.append(int(row["minor_faults"]))

x = list(range(len(size_labels)))
width = 0.35

# ==========================================
# Throughput comparison
# ==========================================

plt.figure(figsize=(9, 5))

x_read = [i - width / 2 for i in x]
x_mmap = [i + width / 2 for i in x]

plt.bar(x_read, read_throughput, width, label="read()")
plt.bar(x_mmap, mmap_throughput, width, label="mmap()")

plt.xticks(x, size_labels)
plt.xlabel("File Size")
plt.ylabel("Throughput (MB/s)")
plt.title("Traditional read() vs Memory-Mapped I/O")
plt.legend()
plt.grid(axis="y", alpha=0.3)
plt.tight_layout()

plt.savefig(
    "results/throughput_comparison.png",
    dpi=300,
    bbox_inches="tight"
)

plt.close()

# ==========================================
# Minor page faults comparison
# ==========================================

plt.figure(figsize=(9, 5))

plt.bar(x_read, read_faults, width, label="read()")
plt.bar(x_mmap, mmap_faults, width, label="mmap()")

plt.xticks(x, size_labels)
plt.xlabel("File Size")
plt.ylabel("Minor Page Faults")
plt.title("Minor Page Faults: read() vs mmap()")

# Log scale makes the 4-vs-800 difference readable.
plt.yscale("log")

plt.legend()
plt.grid(axis="y", alpha=0.3, which="both")
plt.tight_layout()

plt.savefig(
    "results/page_faults_comparison.png",
    dpi=300,
    bbox_inches="tight"
)

plt.close()

print("Graphs generated successfully.")
print("  results/throughput_comparison.png")
print("  results/page_faults_comparison.png")
