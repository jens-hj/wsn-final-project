import sys
import numpy as np
import os
from Crypto.Cipher import AES
from Crypto.Util.Padding import pad, unpad

BLOCK_SIZE = 16

def decrypt_block(hex_en):
    # input: AES encrypted string in hex
    # ex: 'd9 c9 20 63 38 d5 22 28 7c'
    # input MUST be of blocksize 16

    # hardcoded key from encryption
    key = [5, 0, 7, 6, 9, 9, 6, 2, 9, 1, 3, 8, 6, 8, 4, 0]
    key = bytes(key)

    # generate cipher in ECB-mode
    cipher = AES.new(key, AES.MODE_ECB)

    #print(hex_en)

    # remove empty spaces, 
    str_nospace = hex_en.replace(" ", "" )
    #print(str_nospace)

    str_fromhex = bytes.fromhex(str_nospace)
    #print(str_fromhex)

    str_decrypt = cipher.decrypt(str_fromhex)
    #print(str_decrypt)

    # List to store the decimal values
    decimal_values = []

    # Loop over the bytes in the sequence and convert each byte to decimal
    for byte in str_decrypt:
        decimal_values.append(byte)

    # Print the decimal values
    #print(decimal_values)

    #str_decode = str_decrypt.decode("utf-8")
    #print(str_decode)

    return decimal_values

def main():

    #print(decrypt_block('d9 c9 20 63 38 d5 22 28 7c 2c 12 ec 5a 64 8d d8'))
    #print(decrypt_block('6f 71 6e 6f 71 70 6c 6b 69 71 70 70 71 70 6b 71'))
    
    arg = sys.argv[1]
    extensions = ('.txt', '.csv')

    # if arg is filename then read file and apply decrypt to each line
    if arg.endswith('.txt'):
        with open(arg) as f:
            content = f.readlines()
        
        # strip '\n'
        lines = [line.strip() for line in content]

        #print(lines)

        # create output file
        arg_split = arg.rsplit('.', 1)
        new_file = arg_split[0] + "_out.txt"
        f = open(new_file, "w")

        # decrypt lines and append to output file
        for line in lines:
            out_list = decrypt_block(line)
            numbers_str = " ".join(str(n) for n in out_list)
            f.write(numbers_str + '\n')
    
    # if arg is not file, decrypt and print
    else:
        print(decrypt_block(arg))

if __name__ == "__main__":
    main()