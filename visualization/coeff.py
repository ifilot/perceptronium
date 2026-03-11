import os
import numpy as np
from PIL import Image, ImageDraw, ImageFont
from matplotlib import cm
from multiprocessing import Manager, cpu_count, Lock, Pool
from tqdm import tqdm

ROOT = os.path.dirname(__file__)

def main():
    # read data
    data = np.load("data/ch4_ht_coeff_normalized.npz")
    energies = data['energies']
    coefficients = data['coefficients'][:,:,1:]

    num_workers = min(12, len(coefficients))  # Use available CPU cores but not exceed number of frames
    num_rows = len(coefficients)

    minval = 0.0
    maxval = 1.0

    with Manager() as manager:
        progress = manager.list([0])  # Shared variable to track progress

        # Start the progress bar process
        with tqdm(total=len(coefficients), desc="Generating Images", unit="frame") as pbar:
            with Pool(num_workers) as pool:
                results = [pool.apply_async(visrow, args=(i, coefficients[i], progress)) for i in range(len(coefficients))]

                # Monitor progress
                while sum(progress) < len(coefficients):
                    pbar.n = sum(progress)
                    pbar.refresh()

                # Ensure all processes complete
                [r.get() for r in results]

def visrow(imgnr, data, progress):
    atoms = ['C', 'H', 'H', 'H', 'H']

    # Image parameters
    square_size = 20  # Size of each square
    spacing = 2       # Spacing between squares in a block
    block_spacing = 40  # Additional space between different blocks
    text_spacing = 35  # Space for labels above blocks
    row_spacing = 20
    start_x, start_y = 70, 10 + text_spacing  # Adjust start position to fit text

    # Labels for each block (with Greek letters ζ and λ)
    labels = [
        "G0",
        "G1",
        "G2",
        "G5(ζ=1, λ=1)",
        "G5(ζ=1.5, λ=1)",
        "G5(ζ=3, λ=1)",
        "G5(ζ=6, λ=1)",
        "G5(ζ=16, λ=1)",
        "G5(ζ=64, λ=1)",
        "G5(ζ=1, λ=-1)",
        "G5(ζ=1.5, λ=-1)",
        "G5(ζ=3, λ=-1)",
        "G5(ζ=6, λ=-1)",
        "G5(ζ=16, λ=-1)",
        "G5(ζ=64, λ=-1)"
    ]

    # Estimate image dimensions
    num_blocks = (len(data[0]) - 2) // 9  # Number of 3x3 blocks
    image_width = start_x + (num_blocks + 2) * (3 * square_size + 3 * spacing + block_spacing)
    image_height = start_y + (3 * square_size + 3 * spacing) * len(data) + row_spacing * (len(data) - 1) + text_spacing

    # Create a blank image
    image = Image.new("RGB", (image_width, image_height), "white")
    draw = ImageDraw.Draw(image)

    # Load a font (Default PIL font used, but can be replaced)
    try:
        font = ImageFont.truetype(os.path.join(ROOT,"tahoma.ttf"), 12)
    except IOError:
        font = ImageFont.load_default()

    def get_color(value):
        """Return a Viridis colormap color based on the normalized value."""
        color = cm.viridis(value)  # Get RGBA color from Viridis
        rgb_color = tuple(int(255 * c) for c in color[:3])  # Convert to 8-bit RGB
        return rgb_color

    for r,row in enumerate(data):
        # Drawing logic
        x, y = start_x, start_y + r * (3 * square_size + 3 * spacing + row_spacing)
        label_index = 0

        # print atom number
        draw.text((x - text_spacing, y), atoms[r], fill="black", font=font)

        # Draw text label for the first block
        if r == 0:
            draw.text((x, y - text_spacing), labels[label_index], fill="black", font=font)
            label_index += 1

        # First value (2x3 block)
        for i in range(2):
            for j in range(3):
                index = i*3+j
                value = row[index]
                draw.rectangle(
                    [x + j * (square_size + spacing), y + i * (square_size + spacing),
                    x + j * (square_size + spacing) + square_size,
                    y + i * (square_size + spacing) + square_size],
                    fill=get_color(value),
                    outline="black",
                )
        x += 3 * (square_size + spacing) + block_spacing  # Move right after 2x3 block

        # Draw text label for the second block
        if r == 0:
            draw.text((x, y - text_spacing), labels[label_index], fill="black", font=font)
            label_index += 1

        # Second value (single square)
        second_value = row[6]
        draw.rectangle(
            [x, y, x + square_size, y + square_size],
            fill=get_color(second_value),
            outline="black",
        )
        x += square_size + block_spacing  # Move right, add extra space

        # Remaining values (3x3 blocks)
        index = 7  # Start from the third value
        while index < len(row) and label_index < len(labels):
            if index + 9 <= len(row):  # Check if a full 3x3 block can be made
                # Add a label for the block
                if r == 0:
                    draw.text((x, y - text_spacing), labels[label_index], fill="black", font=font)
                    label_index += 1

                for i in range(3):
                    for j in range(3):
                        value = row[index]
                        draw.rectangle(
                            [x + j * (square_size + spacing), y + i * (square_size + spacing),
                            x + j * (square_size + spacing) + square_size,
                            y + i * (square_size + spacing) + square_size],
                            fill=get_color(value),
                            outline="black",
                        )
                        index += 1

                x += 3 * (square_size + spacing) + block_spacing  # Move right, add spacing between blocks
            else:
                break  # Stop if fewer than 9 elements remain

    # save the result
    outpath = 'movie/coefficients/frame_%05i.png' % (imgnr+1)
    image.save(outpath)
    image.close()

    # Update shared progress list safely
    progress[0] += 1

if __name__ == '__main__':
    main()