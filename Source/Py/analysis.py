"""Set analyzer.

Author: Stefano Magrini Alunno
Date: 2023 / 10 / 06

This file defines functions that can be used to analyze the results.

It requires the installation of:
    * 'struct', 'os', 'PIL'
"""

import struct
import os
from PIL import Image


def work_analysis(src_file: str, dest_dir: str) -> tuple:
    """
    This function analyzes individual works.

    It creates the following files in 'dest_dir':
        * map.pgm: an image mapping the relevant grams.
        * bw.pbm: an image depicting how it was analyzed.

    Parameters
    ----------
    src_file : str
        Directory of the work's synthesis to be analyzed.
    dest_dir : str
        Directory of the destination folder.

    Returns
    -------
    data : List[Tuple[bytes, int]]
        Position of the synthesis.

    """
    """
    # extract data from source file
    with open(src_file, 'rb') as file:
        size = struct.unpack('i', file.read(4))[0]
        num_of_data = struct.unpack('i', file.read(4))[0]

        grams = []
        recurrences = []

        num_of_px = size ** 2
        for _ in range(num_of_data):
            grams.append(file.read(num_of_px))

        for _ in range(num_of_data):
            recurrences.append(struct.unpack('i', file.read(4))[0])

        width = struct.unpack('i', file.read(4))[0]
        height = struct.unpack('i', file.read(4))[0]
        recurrence_map = struct.unpack(
            'f'*width*height,
            file.read(width*height*4))
        bw_imag = file.read(width*height*4)

    # make map
    normalized_values = [int(value * 255) for value in recurrence_map]
    image = Image.new("L", (width, height))
    image.putdata(normalized_values)
    map_path = os.path.join(dest_dir, 'map.pgm')
    image.save(map_path)

    # make bw image
    image = Image.new("1", (width, height))
    image.putdata(bw_imag)
    bw_path = os.path.join(dest_dir, 'bw.pbm')
    image.save(bw_path)

    # list of grams with recurrences
    data = list(zip(grams, recurrences))

    return (data,)
    """
    return None


def author_analysis(directory, works_list):
    """
    def merge(list1, list2):
        merged = []
        i = 0
        j = 0
        while i < len(list1) and j < len(list2):
            data_i = list1[i]
            data_j = list2[j]

            # comparison
            for b1, b2 in zip(data_i[0], data_j[0]):
                if b1 < b2:
                    ret = -1
                    break
                elif b1 > b2:
                    ret = 1
                    break
            else:
                ret = 0

            # append in merged list
            if ret < 0:
                merged.append(data_i)
                i += 1
            elif ret > 0:
                merged.append(data_j)
                j += 1
            else:
                merged.append((data_i[0], data_i[1] + data_j[1]))
                i += 1
                j += 1
        merged += list1[i:]
        merged += list2[j:]
        return merged

    def multiple_merge(lists):
        while len(lists) > 1:
            lists = sorted(lists, key=lambda x: len(x[0]), reverse=True)
            # perform a merge of the last two elements of lists
            lists.append((merge(lists.pop()[0], lists.pop()[0]),))
        return lists[0]

    author_grams_list = multiple_merge(works_list)[0]

    sorted_data = sorted(author_grams_list, key=lambda x: x[1])
    size = len(sorted_data[0])
    order_grams_file = os.path.join(directory, 'order_grams.bin')
    with open(order_grams_file, 'wb') as file:
        file.write(struct.pack('i', size))
        for data in sorted_data:
            file.write(data[0])
            file.write(struct.pack('i', data[1]))
    return
    """
    return

def analysis(directory, authors_list):
	return
