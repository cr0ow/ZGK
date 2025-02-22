import cv2
import numpy as np
import sys
import os

def detect_eyes(_image):
    classifier = cv2.CascadeClassifier(cv2.data.haarcascades + "haarcascade_eye.xml")
    return classifier.detectMultiScale(_image, scaleFactor=1.3, minNeighbors=4, minSize=(100, 100))

def fill_holes(binary_mask):
    flood_filled_mask = binary_mask.copy()
    _height, _width = flood_filled_mask.shape[:2]
    temporary_mask = np.zeros((_height + 2, _width + 2), np.uint8)
    cv2.floodFill(flood_filled_mask, temporary_mask, (0, 0), 255)
    inv_flood_filled_mask = cv2.bitwise_not(flood_filled_mask)
    return inv_flood_filled_mask | binary_mask

def create_mask(_r, _bg):
    _mask = (_r > 150) & (_r > _bg)
    _mask = _mask.astype(np.uint8) * 255
    _mask = fill_holes(_mask)
    _mask = cv2.dilate(_mask, None, anchor=(-1, -1), iterations=3, borderType=1, borderValue=1)
    return _mask.astype(bool)[:, :, np.newaxis]

def new_eyes_color():
    _color = blue_green / 2
    return np.uint8(_color[:, :, np.newaxis])


if(len(sys.argv) < 2):
    print('Nie podano ścieżki do obrazu,\nużycie: python red-eyes-remover.py <ścieżka_do_obrazu>')
    sys.exit(-1)

input_path = sys.argv[1]
if not os.path.isfile(input_path):
    print(f"Plik '{input_path}' nie istnieje.")
    sys.exit(1)

image = cv2.imread(input_path, cv2.IMREAD_COLOR)
if image is None:
    print(f"Błąd podczas ładowania pliku: '{input_path}'")
    sys.exit(1)

imageOutput = image.copy()

eyes = detect_eyes(image)

for (x, y, width, height) in eyes:
    eye = image[y: y + height, x: x + width]
    eyeOutput = eye.copy()

    blue = eye[:, :, 0]
    green = eye[:, :, 1]
    red = eye[:, :, 2]

    blue_green = cv2.add(blue, green)
    mask = create_mask(red, blue_green)
    color = new_eyes_color()

    np.copyto(eyeOutput, color, where=mask)
    imageOutput[y: y + height, x: x + width, :] = eyeOutput


base_name, ext = os.path.splitext(os.path.basename(input_path))
output_path = os.path.join(os.path.dirname(input_path), f"{base_name}-edited{ext}")
cv2.imwrite(output_path, imageOutput)
print(f"Wynik zapisano do pliku: '{output_path}'")
