#!/usr/bin/env python3
import binascii
import sys
import argparse

parser = argparse.ArgumentParser(description='Convert binary file to C-style array initializer.')
parser.add_argument("filename", nargs='+', help="the file to be converted")
parser.add_argument("-l", "--linebreak", type=int, help="add linebreak after every N element")
parser.add_argument("-L", "--linebreak-string", default="\n", help="use what to break link, defaults to \"\\n\"")
parser.add_argument("-S", "--separator-string", default=", ", help="use what to separate elements, defaults to \", \"")
parser.add_argument("-H", "--element-prefix", default="0x", help="string to be added to the head of element, defaults to \"0x\"")
parser.add_argument("-T", "--element-suffix", default="", help="string to be added to the tail of element, defaults to none")
parser.add_argument("-O", "--output", default="binaries.h", help="output file name")
parser.add_argument("-U", "--force-uppercase", action='store_true', help="force uppercase HEX representation")
args = parser.parse_args()

def make_sublist_group(lst: list, grp: int) -> list:
    """
    Group list elements into sublists.

    make_sublist_group([1, 2, 3, 4, 5, 6, 7], 3) = [[1, 2, 3], [4, 5, 6], 7]
    """
    return [lst[i:i+grp] for i in range(0, len(lst), grp)]

def do_convension(content: bytes, to_uppercase: bool=False) -> str:
    hexstr = binascii.hexlify(content).decode("UTF-8")
    if to_uppercase:
        hexstr = hexstr.upper()
    array = [args.element_prefix + hexstr[i:i + 2] + args.element_suffix for i in range(0, len(hexstr)-2, 2)]
    if args.linebreak:
        array = make_sublist_group(array, args.linebreak)
    else:
        array = [array,]
    result = args.linebreak_string.join([args.separator_string.join(e) + args.separator_string for e in array])
    result += args.element_prefix + hexstr[-2:] + args.element_suffix
    return result

def convert(fname, out_c_file, out_h_file):
    with open(fname, 'rb') as f:
        file_content = f.read()
    ret = do_convension(file_content, to_uppercase=args.force_uppercase)
    
    varName = fname.replace('.','_')
    varLen  = str(len(file_content))
    
    line = "const char "+varName+"["+varLen+"] PROGMEM ={"+ret+"};\n"
    out_c_file.write(line)
    
    line = "#define "+varName+"_len "+varLen+"\n"
    line += "extern const char "+varName+"["+varLen+"];\n"
    out_h_file.write(line)
    
   


if __name__ == "__main__":
    out_c_file=open(args.output + '.c', 'w')
    out_h_file=open(args.output + '.h', 'w')
    
    out_c_file.write("#include <avr/pgmspace.h>\n")
    out_h_file.write("#ifndef HTTP_RAW_BINARIES\n#define HTTP_RAW_BINARIES\nextern \"C\" {\n")
    
    for filename in args.filename:
        convert(filename,out_c_file,out_h_file)
    
    out_h_file.write("\n}\n#endif\n")
