from PIL import Image
import os
import tqdm

ROOT = os.path.dirname(__file__)

def main():
    for i in tqdm.tqdm(range(10000)):
        convert(os.path.join(ROOT, '..', 'movie', 'coefficients', 'frame_%05i.png' % (i+1)),
                os.path.join(ROOT, '..', 'movie', 'structures', 'frame_%05i.png' % (i+1)),
                os.path.join(ROOT, '..', 'movie', 'out', 'frame_%05i.png' % (i+1)))

def convert(img1, img2, out):
    # Load images
    image1 = Image.open(img1)  # First image (1660x490)
    image2 = Image.open(img2)  # Second image (963x964)

    # Define output canvas size
    canvas_width, canvas_height = 1920, 1080
    canvas = Image.new("RGBA", (canvas_width, canvas_height), (255, 255, 255, 255))  # White background

    # Resize second image to fit below first image
    max_width = 1660  # Match width of the first image
    max_height = canvas_height - 490  # Remaining space below first image

    # Maintain aspect ratio while resizing
    image2.thumbnail((max_width, max_height), Image.LANCZOS)

    # Calculate positions to center images
    x1 = (canvas_width - image1.width) // 2  # Center first image horizontally
    y1 = 0  # Place first image at the top

    x2 = (canvas_width - image2.width) // 2  # Center second image horizontally
    y2 = image1.height  # Place second image right below the first

    # Paste images onto the canvas
    canvas.paste(image1, (x1, y1), image1 if image1.mode == 'RGBA' else None)
    canvas.paste(image2, (x2, y2), image2 if image2.mode == 'RGBA' else None)

    # Save the final image
    canvas.save(out)

if __name__ == '__main__':
    main()