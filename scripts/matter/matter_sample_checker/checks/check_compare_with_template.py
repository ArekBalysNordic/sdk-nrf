#
# Copyright (c) 2025 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause

from pathlib import Path

from internal.checker import MatterSampleTestCase
from internal.utils.utils import find_template_directory, has_internal_build_configs, find_config_files, create_diff_report
from internal.utils.processing import extract_boards_from_sample_yaml, is_board_specific_file
from internal.utils.comparisons import compare_config_files


"""
    TEMPLATE COMPARISON VALIDATION CHECK:

    Validates that Matter samples maintain consistency with the template sample by
    comparing file structure, configuration content, and board-specific overlays.
    This ensures all samples follow the same architectural patterns and include
    required configurations defined in the template.

    CONFIGURATION PARAMETERS:
    -------------------------
    • compare_file_extensions: Array of file extensions to validate against template.
                              Files with these extensions are checked for content
                              consistency with their template counterparts.

    • exclude_files: List of file paths that are allowed to differ from template.
                    Supports glob patterns:
                    - Exact matches: "sample.yaml"
                    - Wildcards: "*.md"
                    - Nested patterns: "src/**" matches all files under src/

    VALIDATION STEPS:
    -----------------
    1. Template Discovery:
       - Locates the template sample directory (typically samples/matter/template)
       - Reads template's sample.yaml to extract supported boards
       - Identifies internal build configurations in template
       - Creates baseline for comparison

    2. Sample Analysis:
       - Reads current sample's sample.yaml for board support
       - Identifies boards supported in template but not in current sample
       - Extracts internal build configuration flags
       - Compares build variant support (debug, release, internal)

    3. File Structure Mapping:
       - Scans both template and current sample directories
       - Creates relative path mappings for all config files
       - Filters files based on compare_file_extensions
       - Applies exclude_files patterns to skip expected differences

    4. Configuration Content Comparison:
       - Iterates through config files (.conf, .overlay) present in both locations
       - Extracts configuration entries from each file:
         * Kconfig options (CONFIG_* entries) from .conf files
         * Device tree nodes and properties from .overlay files
       - Compares template content against sample content
       - Validates that all template configurations are present in sample
       - Additional sample configs are allowed (not flagged as issues)

    5. Board-Specific Validation:
       - Identifies board-specific files (e.g., nrf52840dk_nrf52840.overlay)
       - Skips comparison for boards not supported in current sample
       - Validates only boards declared in sample.yaml
       - Handles special cases:
         * Internal build variants (_internal suffix)
         * Non-secure variants (_ns suffix)
         * Multi-core board configurations

    6. Missing File Detection:
       - Reports files present in template but missing in sample
       - Excludes board-specific files for unsupported boards
       - Identifies missing internal build configurations
       - Detects optional add-ons (shields, nrf21540) and treats as warnings

    7. Additional File Reporting:
       - Lists files in sample not present in template (informational)
       - Distinguishes between legitimate additions and potential issues
       - Provides debug output for sample-specific enhancements

    8. Diff Generation:
       - Creates detailed diff reports for mismatched configurations
       - Shows missing template entries in sample files
       - Formats output for easy identification of issues
       - Includes line-by-line comparison where applicable

    NOTES:
    ------
    • Only template content is required in samples; extra configs are allowed
    • Comparison is content-based, not line-by-line text matching
    • Configuration parsing extracts semantic meaning, not exact formatting
    • Warnings vs Errors: Missing required files are errors, board-specific
      missing files for unsupported boards are debug messages
"""


class CompareWithTemplateTestCase(MatterSampleTestCase):
    def __init__(self):
        super().__init__()
        self.template_dir = None
        self.current_boards = None
        self.template_boards = None
        self.current_sample_yaml = None
        self.template_sample_yaml = None
        self.current_file_map = {}
        self.template_file_map = {}
        self.current_has_internal = None
        self.unsupported_boards = None
        # Track which unsupported boards have been reported
        self.reported_boards = set()
        # Track which platforms have internal files but no internal build support
        self.internal_platforms_reported = set()

    def name(self) -> str:
        return "Compare with template sample"

    def prepare(self):

        # Find template directory
        self.template_dir = find_template_directory(self.config.sample_path)

        self.current_sample_yaml = self.config.sample_path / 'sample.yaml'
        self.template_sample_yaml = self.template_dir / 'sample.yaml'

        self.current_boards = extract_boards_from_sample_yaml(
            self.current_sample_yaml, self.warning)
        self.template_boards = extract_boards_from_sample_yaml(
            self.template_sample_yaml, self.warning)

        self.current_has_internal = has_internal_build_configs(
            self.current_sample_yaml)

        # Identify boards that are supported in template but not in current sample
        self.unsupported_boards = self.template_boards - self.current_boards

        self.create_file_map()

    def check(self):

        if not self.template_dir:
            self.issue(
                "Could not find matter template directory for comparison")
            return

        self.debug(f"Comparing with template at: {self.template_dir}")

        # Skip template comparison if we are checking the template itself
        if self.sample_name == 'template':
            self.info(
                "Skipping template comparison for template sample itself")
            return

        self.compare_files()
        self.report_files_that_are_in_current_but_not_in_template()
        self.report_files_that_are_in_template_but_not_in_current()
        self.report_platrofms_that_are_not_supported()
        self.debug("\n")

    def create_file_map(self):
        # Find all config files in current sample
        current_files = find_config_files(
            self.config.sample_path, self.config.config_file)
        template_files = find_config_files(
            self.template_dir, self.config.config_file)

        # Create mapping of relative paths to files for easier comparison
        self.current_file_map
        for file_path in current_files:
            rel_path = file_path.relative_to(self.config.sample_path)
            self.current_file_map[rel_path] = file_path

        self.template_file_map
        for file_path in template_files:
            rel_path = file_path.relative_to(self.template_dir)
            self.template_file_map[rel_path] = file_path

    def report_files_that_are_in_current_but_not_in_template(self):
        self.debug(
            "\nAdditional files in current sample that are not in template sample:")
        # Report files that exist only in current sample (could be legitimate additions)
        only_in_current = set(self.current_file_map.keys()) - \
            set(self.template_file_map.keys())
        for rel_path in sorted(only_in_current):
            # Skip board-specific files for unsupported boards
            if not is_board_specific_file(self.config.sample_path / rel_path, self.unsupported_boards):
                self.debug(f"--- {rel_path}")

    def compare_files(self):
        # Compare files that exist in both template and current sample
        for rel_path, current_file in self.current_file_map.items():
            if rel_path in self.template_file_map:
                template_file = self.template_file_map[rel_path]

                # Check if this file is board-specific for an unsupported board
                # So the board that exists in the template but not in the current sample.
                if is_board_specific_file(current_file, self.unsupported_boards):
                    continue

                # Compare the files
                diff_result = compare_config_files(
                    current_file, template_file, self.config.config_file)
                if diff_result:
                    # Add a warning for each file that differs from template
                    self.warning(
                        f"Configuration file differs from template: {rel_path} \n{create_diff_report(diff_result)}")
                else:
                    self.debug(f"✓ {rel_path} matches template")

    def report_files_that_are_in_template_but_not_in_current(self):
        self.debug(
            "\nAdditional files in template sample that are not in current sample:")
        # Report files that exist only in template but skip board-specific files for unsupported boards
        only_in_template = set(self.template_file_map.keys()) - \
            set(self.current_file_map.keys())
        for rel_path in sorted(only_in_template):
            if is_board_specific_file(self.template_dir / rel_path, self.unsupported_boards):
                # Find which board this file is for and report once per board
                for board in self.unsupported_boards:
                    if board in str(rel_path) and board not in self.reported_boards:
                        self.debug(
                            f"--- {board}")
                        self.reported_boards.add(board)
                        break
            else:
                if not self._is_internal_file(rel_path):
                    # Check for additional shield/add-on patterns that might be optional
                    is_optional_addon = False
                    # Common shield patterns
                    addon_patterns = ['nrf21540dk', 'nrf21540ek', 'shield']

                    for pattern in addon_patterns:
                        if pattern in str(rel_path).lower():
                            self.debug(
                                f"Optional add-on/shield '{pattern}' is not used in this sample - skipping file: {rel_path}")
                            is_optional_addon = True
                            break

                    if not is_optional_addon:
                        self.warning(
                            f"File missing compared to template: {rel_path}")

    def report_platrofms_that_are_not_supported(self):
        # Report internal platforms that are not supported
        if self.internal_platforms_reported:
            for platform in sorted(self.internal_platforms_reported):
                self.debug(
                    f"Internal build variant is not supported for platform '{platform}' - skipping internal files")

    def _is_internal_file(self, rel_path: Path) -> bool:
        """Check if a file is an internal file."""
        if '_internal' in str(rel_path) and not self.current_has_internal:
            # Extract platform name from the internal file path
            file_name = rel_path.name
            if '_internal' in file_name:
                # Extract platform part (e.g., from 'nrf54l15dk_nrf54l15_cpuapp_internal.conf' get 'nrf54l15dk_nrf54l15_cpuapp')
                platform_part = file_name.split('_internal')[0]

                # Remove common prefixes to get the actual platform name
                if platform_part.startswith('pm_static_'):
                    # Remove 'pm_static_' prefix
                    platform_part = platform_part[10:]

                self.internal_platforms_reported.add(platform_part)
            return True
        return False
