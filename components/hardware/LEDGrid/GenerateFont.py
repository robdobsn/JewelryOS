import argparse
import os

def convertChar(char, maxCharHeight):
    charWidth = 0
    charBytes = bytearray()
    for line in char:
        charWidth = max(charWidth, len(line))
    for line in char:
        binaryStr = line.replace('#', '1').replace(' ', '0')
        binaryStr = binaryStr + '0' * (charWidth - len(binaryStr))
        binaryStr = binaryStr + '0' * (8 - len(binaryStr))
        charBytes.append(int(binaryStr, 2))
    return charWidth, charBytes

def parseFile(filename):
    with open(filename, 'r') as f:
        lines = f.readlines()

    # Get the character data
    charCode = 0x00
    font = {}
    maxCharHeight = 0

    # Each character starts with a line $xx where xx is the character code
    # The following lines are the character data made using # characters for pixels on
    # and spaces for pixels off
    # A line starting with = sets the width of the character
    # The width of each character is variable and set by the widest line of the character
    for line in lines:
        if line.startswith('$'):
            charCode = int(line[1:], 16)
            font[charCode] = []
        elif line.startswith('='):
            font[charCode].append(" " * int(line[1:]))
        else:
            font[charCode].append(line.rstrip())
            maxCharHeight = max(maxCharHeight, len(font[charCode]))

    # Convert each character to a tuple containing a width and a list of bytes
    for charCode in font:
        font[charCode] = convertChar(font[charCode], maxCharHeight)

    # Find the start and end characters
    start = min(font.keys())
    end = max(font.keys())

    # Check if lower case letters are present
    if end < 0x61:
        # Lower case letters not present, add them by copying the upper case letters
        for charCode in range(0x61, 0x7b):
            font[charCode] = font[charCode - 0x20]
        end = 0x7a

    return maxCharHeight, start, end, font

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('inputfile', help='Input font file to convert')
    parser.add_argument('fontname', help='Font name')
    args = parser.parse_args()

    # Output file in same folder as input file
    outputfile = args.inputfile.split('.')[0] + '.h'
    outputfile = os.path.join(os.path.dirname(args.inputfile), outputfile)
    with open(outputfile, 'w') as f:

        # Parse the font file
        height, start, end, font = parseFile(args.inputfile)

        # Write the header
        f.write('#pragma once\n')
        f.write('#include <stdint.h>\n')
        f.write(f'namespace {args.fontname}\n')
        f.write('{\n')
        f.write(f'    constexpr uint8_t height = {height};\n')
        f.write(f'    constexpr uint8_t start = 0x{start:02x};\n')
        f.write(f'    constexpr uint8_t end = 0x{end:02x};\n')
        f.write(f'    // Characters (first value in array is width)\n')
        f.write(f'    const uint8_t font[][{height+1}] = \n')
        f.write('    {\n')

        # Write the character data
        for charCode in range(start, end+1):
            if charCode not in font:
                f.write(f'        {{ 0, ')
                for i in range(height):
                    f.write('0x00, ')
                f.write('},')
            else:
                f.write(f'        {{ {font[charCode][0]}, ')
                for i in range(height):
                    if i < len(font[charCode][1]):
                        byte = font[charCode][1][i]
                    else:
                        byte = 0
                    f.write(f'0x{byte:02x}, ')
                f.write('},')
            f.write(f' // 0x{charCode:02x}\n')

        # Write the footer
        f.write('    };\n')
        f.write('}\n')

if __name__ == '__main__':
    main()

