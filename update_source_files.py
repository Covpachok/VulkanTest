import os
import re

def find_source_files(directory, extensions):
    source_files = []
    for root, _, files in os.walk(directory):
        for file in files:
            if any(file.endswith(ext) for ext in extensions):
                rel_path = directory + "/" + os.path.relpath(os.path.join(root, file), directory).replace('\\', '/')
                source_files.append(rel_path)
    return sorted(source_files)

def update_cmake_file(cmake_file, variable_name, file_list):
    with open(cmake_file, 'w') as f:
        f.write(f"set({variable_name}\n")
        for file in file_list:
            f.write(f"    {file}\n")
        f.write(")\n")

if __name__ == "__main__":
    source_dir = "src"
    cmake_file = "source_files.cmake"
    variable_name = "PROJECT_SOURCE_FILES"
    extensions = (".cpp")

    source_files = find_source_files(source_dir, extensions)

    update_cmake_file(cmake_file, variable_name, source_files)

    print(f"Updated {cmake_file} with {len(source_files)} source files.")
