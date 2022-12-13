import re
import sys
import numpy as np
import os

def is_hex(s):
    # Use a regular expression to check if a string is a hexadecimal value
    # surrounded by quotation marks and separated by spaces
    return bool(re.match(r'^\s*".*"\s*$'))

def remove_quotes_and_last_space(s):
    return s[1:-1]

def main():

    arg = sys.argv[1]
    extensions = ('.txt', '.csv')

    # if arg is filename then read file and apply decrypt to each line
    if arg.endswith('.txt'):
        with open(arg) as f:
            file_content = f.read()
        
            # strip '\n'
            # lines = [line.strip() for line in content]

            regex = re.compile(r'".*"')
            # regex2 = re.compile(r'"[0-9a-fA-F]+"')

            # Filter the input list for strings containing hexadecimal numbers
            filtered_list = regex.findall(file_content, re.DOTALL)

            # Strip the quotation marks
            for i in range(len(filtered_list)):
                filtered_list[i] = filtered_list[i].strip()

            # Filter citation marks
            output_list = [remove_quotes_and_last_space(s) for s in filtered_list]

            # create output file
            arg_split = arg.rsplit('.', 1)
            new_file = arg_split[0] + "_filtered.txt"
            f = open(new_file, "w")

            # decrypt lines and append to output file
            for line in output_list:
                f.write(line + '\n')

if __name__ == "__main__":
    main()