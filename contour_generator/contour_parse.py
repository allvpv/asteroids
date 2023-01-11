def get_pairs_from_content(contour_c):
    contour_p = contour_c.split('\n')[:-1]
    contours = [pair.split(';') for pair in contour_p]

    return contours

def pairs_shifted(pairs_a, width, height):
    shift_width = width / 2.
    shift_height = height / 2.

    return [(float(a) - shift_width, float(b) - shift_height) for [a, b] in pairs_a]

def process_file(filename, objectname, width, height, scaler):
    with open(f"{filename}_contour", "r") as contour_f:
        pairs_a = get_pairs_from_content(contour_f.read())
        pairs_b = pairs_shifted(pairs_a, width, height)

        print(f"    SpiritData {objectname} {{")
        print(f'        .filename = L"assets/{filename}.png",')
        print( '        .contour = {')
        print( "            .vertices = {")

        for (a, b) in pairs_b:
            print(f"                {{ {round(a*scaler, 2)}f, {round(b*scaler, 2)}f }},")
        print( "            },")

        print(f"            .half_of_sides = {{ {round(width * scaler / 2., 2)}f, {round(height * scaler / 2., 2)}f }},")
        print( "        },")
        print(f"        .scale = {scaler}f,")
        print( "    };")
        print( "")

print('/*')
print(' * This file is auto-generated by `contour_parse.py` (from this repo)')
print(' */')
print('')
print('#pragma once')
print('#include "math.hpp"')
print('')
print("""struct SpiritData {
    const wchar_t* filename;
    ObjectContour contour;
    float scale;
};

struct Spirits {""");

process_file("rocket", "controller", 128., 224., 0.5)
process_file("asteroid_small", "asteroid", 1000., 877., 0.1)
process_file("bullet", "bullet", 102., 571., 0.25)

print("};");