#
# Copyright (c) 2025 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause

from pathlib import Path
import re
from typing import List

from internal.checker import MatterSampleTestCase
from internal.utils.utils import discover_matter_samples

"""
    COPY-PASTE ERROR DETECTION CHECK:

    Detects accidental copy-paste errors where a sample incorrectly references other
    Matter samples in documentation or configuration files. This prevents publishing
    samples with incorrect names, descriptions, or references left over from copying
    another sample as a starting point.

    VALIDATION STEPS:
    -----------------
    1. Sample Discovery:
       - Scans parent directory to discover all Matter samples
       - Extracts sample names from directory structure
       - Builds list of all samples in the Matter samples collection
       - Excludes current sample from forbidden words list

    2. Forbidden Word Collection:
       - Collects names of all other samples as "forbidden words"
       - Each other sample's name should not appear in current sample
       - Filters out current sample name (self-references are allowed)
       - Creates comprehensive list of sample names to detect

    3. Name Variant Generation:
       - Creates underscore variants: "window_covering" → "window covering"
       - Handles both directory names and user-friendly versions
       - Accounts for different naming conventions in documentation
       - Expands forbidden words list to catch all variations

    4. Allowed Name Filtering:
       - Applies allowed_names filter from configuration
       - Some words may be sample names but also generic terms
       - Case-insensitive matching for allowed names
       - Removes explicitly allowed names from forbidden list

    5. Minimum Length Filtering:
       - Removes sample names shorter than min_word_length
       - Prevents false positives from common short words
       - Configurable threshold (typically 4-5 characters)
       - Balances detection accuracy with false positive rate

    6. File Content Scanning:
       - Checks each file in files_to_check list
       - Opens files with UTF-8 encoding
       - Splits content into individual lines for analysis
       - Tracks line numbers for precise error reporting

    7. Pattern Matching:
       - Uses word-boundary regex to match whole words only
       - Case-insensitive matching to catch all variants
       - Prevents false positives from partial matches
       - Example: "lock" matches "lock" but not "block"

    8. Skip Pattern Application:
       - Checks each matched line against skip_patterns
       - Allows legitimate references in documentation
       - Examples of skipped patterns:
         * "See also: :ref:`matter_lock_sample`" (RST reference)
         * "Based on the window_covering sample" (acknowledgment)
         * "Similar to light_bulb configuration" (comparison)

    9. Severity Classification:
       - README.rst violations are reported as warnings
         (may legitimately reference other samples in "See also" sections)
       - Other file violations are reported as errors
         (likely actual copy-paste mistakes)

    10. Issue Reporting:
        - Reports filename, line number, and content preview
        - Shows found forbidden word in error message
        - Provides context (up to 100 characters of line content)
        - Differentiates between warnings and errors based on file type

    COMMON COPY-PASTE MISTAKES:
    --------------------------
    • Sample name in README.rst header
    • Sample description in sample.yaml
    • CMake project name from another sample
    • File headers with incorrect sample references
    • Documentation examples from another sample

    NOTES:
    ------
    • Only scans specific documentation and config files, not source code
    • Case-insensitive matching catches all naming variations
    • Word boundaries prevent false positives from partial matches
    • README.rst is treated more leniently due to legitimate cross-references
    • Template sample is included in forbidden words (catch copy-paste from template)
    • Allowed names can be configured for terms that are both sample names and
      generic terms (e.g., "common", "utils")
"""


class CopyPasteTestCase(MatterSampleTestCase):

    def __init__(self):
        super().__init__()
        self.allowed_names = []
        self.forbidden_words = []
        self.all_samples = []
        self.files_to_check = []
        self.skip_patterns = []
        self.minimum_word_length = 0

    def name(self) -> str:
        return "Copy Paste Errors"

    def prepare(self):
        # Names that can be excluded from the check
        self.allowed_names = self.config.allowed_names
        self.all_samples = discover_matter_samples(self.config.sample_path)
        self.minimum_word_length = self.config.config_file.get(
            'copy_paste_detection').get('min_word_length')
        self.files_to_check = self.config.config_file.get(
            'copy_paste_detection').get('files_to_check')
        self.skip_patterns = self.config.config_file.get(
            'copy_paste_detection').get(
            'skip_patterns')

        self.discover_forbidden_words()

    def check(self):
        self.debug(
            f"Checking for references to other samples: {', '.join(self.forbidden_words)}")

        for file_path in self.files_to_check:
            full_path = self.config.sample_path / file_path
            if full_path.exists():
                try:
                    with open(full_path, 'r', encoding='utf-8') as f:
                        content = f.read()

                    for word in self.forbidden_words:
                        # Use word boundary regex to match only whole words
                        word_pattern = r'\b' + re.escape(word) + r'\b'
                        if re.search(word_pattern, content, re.IGNORECASE):
                            lines = content.split('\n')
                            for i, line in enumerate(lines, 1):
                                if re.search(word_pattern, line, re.IGNORECASE):
                                    # Skip various legitimate documentation patterns
                                    line_lower = line.lower().strip()

                                    # Skip defined patterns
                                    if any(pattern in line_lower for pattern in self.skip_patterns):
                                        continue

                                    # Now we have a likely copy-paste error
                                    # Treat README.rst copy-paste errors as warnings since they may legitimately reference other samples
                                    if file_path == 'README.rst':
                                        self.warning(
                                            f"Possible copy-paste error in {file_path}:{i} - found '{word}' in: {line.strip()[:100]}")
                                    else:
                                        self.issue(
                                            f"Possible copy-paste error in {file_path}:{i} - found '{word}' in: {line.strip()[:100]}")
                except Exception as e:
                    self.warning(
                        f"Could not check {file_path} for copy-paste errors: {e}")

    def discover_forbidden_words(self):
        """Discover all forbidden words from all samples."""
        for sample in self.all_samples:
            if sample != self.sample_name:
                self.forbidden_words.append(sample)

        # Do not split names into parts; only use the full sample name as forbidden
        # Remove duplicates and very short words
        forbidden_words = list(
            set([word for word in self.forbidden_words if len(word) >= self.minimum_word_length]))

        # Also add forbidden words with underscores replaced by spaces (for copy-paste errors in documentation)
        forbidden_words_with_spaces = []
        for word in forbidden_words:
            if '_' in word:
                forbidden_words_with_spaces.append(word.replace('_', ' '))
        forbidden_words.extend(forbidden_words_with_spaces)

        # Filter out allowed names (case-insensitive matching)
        if self.allowed_names:
            allowed_names_lower = {name.lower() for name in self.allowed_names}
            original_count = len(forbidden_words)
            forbidden_words = [
                word for word in forbidden_words if word.lower() not in allowed_names_lower]
            filtered_count = original_count - len(forbidden_words)
            if filtered_count > 0:
                self.debug(
                    f"Filtered out {filtered_count} allowed name(s): {', '.join(sorted(self.allowed_names))}")
