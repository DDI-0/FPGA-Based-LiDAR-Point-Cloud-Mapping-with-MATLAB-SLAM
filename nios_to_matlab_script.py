import csv
import sys
from pathlib import Path

# input file from command line
if len(sys.argv) != 2:
    print("Usage: python nios_to_matlab_script.py <input_file>")
    sys.exit(1)

input_path = Path(sys.argv[1])
if not input_path.exists():
    print(f"Error: File not found.")
    sys.exit(1)

# Output CSV path
output_path = input_path.with_suffix('.csv')

# Read and process
with open(input_path, 'r') as f:
    lines = [line.strip() for line in f if line.strip() and len(line.split()) == 3]

print("#RPLIDAR SCAN DATA")
print(f"#COUNT={len(lines)}")
print(" #Angule Distance Quality")

rows = []
for line in lines:
    parts = line.split()
    angle = parts[0]
    dist = float(parts[1])
    qual = int(parts[2])
    formatted_angle = angle.rjust(7) if float(angle) < 100 else angle.ljust(7)
    print(f"{formatted_angle} {dist:.1f} {qual}")
    rows.append([angle.strip(), dist, qual])

# Save CSV
with open(output_path, 'w', newline='') as f:
    writer = csv.writer(f)
    writer.writerow(['Angle', 'Distance', 'Quality'])
    writer.writerows(rows)

print(f"\n Data conversion complete'")