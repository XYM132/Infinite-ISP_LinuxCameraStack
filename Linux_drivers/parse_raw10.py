import numpy as np
import cv2
import os
import matplotlib.pyplot as plt

def read_xy10_raw_image(file_path, width, height):
    """
    Read RAW10 image in XY10 format (4 bytes store one 10-bit pixel)
    Parameters:
        file_path: file path
        width: image width
        height: image height
    Returns:
        Unpacked 10-bit RAW image data (numpy array)
    """
    expected_size = width * height * 4
    if os.path.getsize(file_path) != expected_size:
        raise ValueError(f"File size doesn't match expected size, should be {expected_size} bytes")

    with open(file_path, 'rb') as f:
        raw_data = np.fromfile(f, dtype=np.uint8)

    packed_data = raw_data.reshape(height, width, 4)
    unpacked_data = np.zeros((height, width), dtype=np.uint16)
    
    for y in range(height):
        for x in range(width):
            byte0 = packed_data[y, x, 0]  # High 8 bits
            byte1 = packed_data[y, x, 1]  # Low 2 bits
            pixel = (byte0 << 2) | (byte1 >> 6)
            unpacked_data[y, x] = pixel

    return unpacked_data

# Using NumPy vectorization (10x+ speed improvement)
def read_xy10_packed_fast(file_path, width, height):
    with open(file_path, 'rb') as f:
        data = np.fromfile(f, dtype=np.uint32)  # Read directly as 32-bit
    
    zero_bits = (data >> 30) & 0x3
    if np.any(zero_bits != 0):
        print(f"Warning: Found {np.sum(zero_bits != 0)} 32-bit words with non-zero bits 31-30!")
    # Unpack all pixels (single operation)
    pixels = np.zeros((len(data), 3), dtype=np.uint16)
    pixels[:, 0] = (data & 0x3FF)  # P0
    pixels[:, 1] = ((data >> 10) & 0x3FF)  # P1
    pixels[:, 2] = ((data >> 20) & 0x3FF)  # P2

    zero = ((data >> 30) & 0x3)

    
    unpacked = pixels.flatten()[:width * height]
    return unpacked.reshape(height, width)


def process_raw_image(raw10_data, black_level=None, white_level=1023, enhance_contrast=True):
    """
    Optimized 10-bit RAW to 8-bit conversion function with auto black level correction and contrast enhancement
    
    Parameters:
        raw10_data: Input 10-bit RAW data (0-1023)
        black_level: Manually specified black level (if None, calculated automatically)
        white_level: White level (default 1023)
        enhance_contrast: Whether to enable contrast enhancement (default True)
    """
    # Auto calculate black level (if not manually specified)
    if black_level is None:
        black_level = np.percentile(raw10_data, 0.5)  # Use 0.5% percentile as black level

    # Basic processing: subtract black level and normalize to [0, 255]
    processed = raw10_data.astype(np.float32) - black_level
    processed = np.clip(processed, 0, white_level - black_level)
    processed = (processed / (white_level - black_level) * 255).astype(np.uint8)

    # Contrast enhancement (optional)
    if enhance_contrast:
        # Method 1: Histogram equalization (global)
        processed = cv2.equalizeHist(processed)
        
        # Method 2: CLAHE (local contrast enhancement, better for high dynamic range scenes)
        # clahe = cv2.createCLAHE(clipLimit=2.0, tileGridSize=(8, 8))
        # processed = clahe.apply(processed)

    return processed

def demosaic_raw_image(raw8_data, bayer_pattern=cv2.COLOR_BayerBG2BGR):
    """Demosaic processing"""
    return cv2.cvtColor(raw8_data, bayer_pattern)

def display_with_plt(raw_file, width, height):
    """Display image using matplotlib"""
    try:
        # 1. Read and process image
        raw10_data = read_xy10_packed_fast(raw_file, width, height)
        raw8_data = process_raw_image(raw10_data)
        rgb_image = demosaic_raw_image(raw8_data, cv2.COLOR_BayerRG2BGR)
        
        cv2.imwrite("raw8.png", raw8_data)
        cv2.imwrite("rgb.png", rgb_image)

        # 2. Create canvas
        plt.figure(figsize=(15, 6))
        
        # 3. Display RAW image
        plt.subplot(1, 2, 1)
        plt.imshow(raw8_data, cmap='gray')
        plt.title('RAW Image (8bit)')
        plt.axis('off')
        
        # 4. Display RGB image
        plt.subplot(1, 2, 2)
        plt.imshow(cv2.cvtColor(rgb_image, cv2.COLOR_BGR2RGB))  # OpenCV uses BGR format which needs conversion
        plt.title('Demosaiced RGB Image')
        plt.axis('off')
        
        # 5. Adjust layout and display
        plt.tight_layout()
        plt.show()
        
        
    except Exception as e:
        print(f"Error processing image: {e}")

# Usage example
if __name__ == "__main__":
    # Modify these parameters according to actual situation
    width = 1920    # Image width
    height = 1080   # Image height
    raw_file = "camera.raw10"  # XY10 format RAW file
    
    display_with_plt(raw_file, width, height)
