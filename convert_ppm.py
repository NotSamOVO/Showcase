from PIL import Image
import sys
import os

def convert_ppm_to_png(input_path, output_path):
    try:
        with Image.open(input_path) as img:
            img.save(output_path)
        print(f"Successfully converted {input_path} to {output_path}")
    except Exception as e:
        print(f"Error converting file: {e}")

if __name__ == "__main__":
    # Default paths
    input_file = "build/piece.ppm"
    output_file = "piece.png"
    
    if len(sys.argv) > 1:
        input_file = sys.argv[1]
    if len(sys.argv) > 2:
        output_file = sys.argv[2]

    if not os.path.exists(input_file):
        print(f"Error: Input file '{input_file}' not found.")
    else:
        convert_ppm_to_png(input_file, output_file)
