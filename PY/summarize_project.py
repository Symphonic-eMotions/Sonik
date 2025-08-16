#!/usr/bin/env python3

import os
import re

# Pad naar root van het project
root_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
output_file = os.path.join(os.path.dirname(__file__), 'project_summary.txt')

# Alleen .swift bestanden meenemen
valid_extensions = {'.swift'}

# Regex om comments te verwijderen
single_line_comment = re.compile(r'^\s*//.*$', re.MULTILINE)
multi_line_comment = re.compile(r'/\*.*?\*/', re.DOTALL)

def strip_comments(code: str) -> str:
    code = multi_line_comment.sub('', code)
    code = single_line_comment.sub('', code)
    return code.strip()

with open(output_file, 'w', encoding='utf-8') as summary_file:
    for dirpath, dirnames, filenames in os.walk(root_dir):
        # Sla de PY folder zelf over
        if 'PY' in dirnames:
            dirnames.remove('PY')

        for filename in filenames:
            file_path = os.path.join(dirpath, filename)
            _, ext = os.path.splitext(filename)

            if ext.lower() not in valid_extensions:
                continue

            try:
                with open(file_path, 'r', encoding='utf-8') as file:
                    code = file.read()
                    cleaned_code = strip_comments(code)

                    if cleaned_code.strip():
                        relative_path = os.path.relpath(file_path, root_dir)
                        summary_file.write(f"\n// === {relative_path} ===\n")
                        summary_file.write(cleaned_code)
                        summary_file.write("\n")
            except Exception as e:
                print(f"❌ Error reading {file_path}: {e}")
