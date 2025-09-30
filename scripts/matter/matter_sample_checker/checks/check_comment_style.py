#
# Copyright (c) 2025 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause

from pathlib import Path

from internal.utils.utils import find_line_comment_issues

from internal.checker import MatterSampleTestCase

"""
    COMMENT STYLE VALIDATION CHECK:
    
    Ensures consistent comment style across all source files in the Matter sample.
    This check enforces the use of block comments (/* */) instead of line comments (//)
    to maintain code style consistency throughout the codebase.

    CONFIGURATION PARAMETERS:
    -------------------------
    • file_extensions: Array of file extensions that will be scanned for comment style
                      issues. Only files with these extensions are validated.

    • exclude_dirs: List of directory patterns to exclude from checking. Supports:
                    - Wildcards: "build*" matches build, build_debug, etc.
                    - Nested patterns: "**/zap-generated/**" matches any nested path
                    - Exact matches: "third_party" matches the directory name

    • required_style: The enforced comment style. Set to "block_comment" to require
                     /* */ style comments instead of // style comments.

    VALIDATION STEPS:
    -----------------
    1. File Discovery:
       - Recursively scans the sample directory for files matching specified extensions
       - Applies exclusion rules to skip build artifacts and generated code
       - Builds a list of source files to validate

    2. Directory Exclusion:
       - Processes each exclude_dirs pattern:
         * Wildcard patterns (e.g., "build*") match directory prefixes
         * Nested patterns (e.g., "**/generated/**") match anywhere in path
         * Exact patterns match specific directory names
       - Skips files in excluded directories silently

    3. Comment Style Analysis:
       - Reads each source file with UTF-8 encoding
       - Parses content to identify comment patterns
       - Uses regex patterns to detect line comments (//)
       - Excludes comments inside strings and block comments from analysis
       - Tracks line numbers where violations occur

    4. Issue Reporting:
       - Reports files containing line comments (//) when block comments are required
       - Shows file path, line number, and preview of problematic content
       - Provides debug output for files with correct comment style
       - Summarizes total number of files checked

    NOTES:
    ------
    • This check is automatically skipped if no configuration is found
    • Files are checked only if they match the specified extensions
    • Generated code and third-party files should be excluded
    • The check preserves file encoding (UTF-8) during analysis
"""


class CommentStyleTestCase(MatterSampleTestCase):
    def __init__(self):
        super().__init__()
        self.file_extensions = []
        self.exclude_dirs = []
        self.required_style = 'block_comment'
        self.source_files = []
        self.files_with_line_comments = []

    def name(self) -> str:
        return "Comment style"

    def prepare(self):
        comment_config = self.config.config_file.get('comment_style', {})
        if not comment_config:
            self.info(
                "Comment style checking skipped (no configuration found)")
            self.config.skip = True
            return

        self.file_extensions = comment_config.get('file_extensions')
        self.exclude_dirs = set(comment_config.get('exclude_dirs'))
        self.required_style = comment_config.get(
            'required_style', 'block_comment')

        if not self.file_extensions:
            self.info(
                "Comment style checking skipped (no file extensions configured)")
            self.config.skip = True
            return

    def check(self):
        for ext in self.file_extensions:
            for file_path in self.config.sample_path.rglob(f'*{ext}'):
                # Skip if file is in excluded directory
                should_skip = False
                for exclude_dir in self.exclude_dirs:
                    if exclude_dir.endswith('/**'):
                        # Handle nested directory exclusions
                        exclude_pattern = exclude_dir[:-3]  # Remove /**
                        if exclude_pattern.startswith('**/'):
                            exclude_pattern = exclude_pattern[3:]  # Remove **/
                        if exclude_pattern in str(file_path):
                            should_skip = True
                            break
                    elif exclude_dir.endswith('*'):
                        # Handle wildcard patterns like build*
                        exclude_pattern = exclude_dir[:-1]  # Remove *
                        if any(part.startswith(exclude_pattern) for part in file_path.parts):
                            should_skip = True
                            break
                    else:
                        # Handle exact directory matches
                        if exclude_dir in file_path.parts:
                            should_skip = True
                            break

                if not should_skip:
                    self.source_files.append(file_path)

        if not self.source_files:
            self.info("No files found to check for comment style")
            return

        files_checked = 0

        for file_path in self.source_files:
            try:
                with open(file_path, 'r', encoding='utf-8') as f:
                    content = f.read()

                files_checked += 1
                # Check for line comments (//) that are not inside block comments or strings
                line_comment_issues = find_line_comment_issues(
                    content)

                if line_comment_issues:
                    self.files_with_line_comments.append(
                        (file_path.relative_to(self.config.sample_path), line_comment_issues))
                else:
                    self.debug(
                        f"✓ Good comment style: {file_path.relative_to(self.config.sample_path)}")

            except Exception as e:
                self.warning(
                    f"Could not check comment style in {file_path.relative_to(self.config.sample_path)}: {e}")

        # Report results
        if self.required_style == 'block_comment':
            for file_path, issues in self.files_with_line_comments:
                for line_num, line_content in issues:
                    self.issue(
                        f"Line comment found in {file_path}:{line_num} - use /* */ instead: {line_content.strip()[:80]}")

        self.debug(f"Checked {files_checked} files for comment style")
