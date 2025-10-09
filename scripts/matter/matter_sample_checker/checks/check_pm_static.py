#
# Copyright (c) 2025 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause

import yaml
from pathlib import Path

from internal.checker import MatterSampleTestCase


"""
     PARTITION MANAGER STATIC VALIDATION CHECK:

    Validates that Partition Manager (PM) static configuration files contain all
    required memory partitions for different board types, MCU variants, and build
    configurations. Ensures proper memory layout for bootloader, application,
    factory data, and device-specific features.

    CONFIGURATION PARAMETERS:
    -------------------------
    • base_required_entries: Array of partition names that must exist in every PM
                            static file. These are fundamental partitions needed
                            for bootloader, application, and factory data.

    • wifi_required_entries: Additional partition names required for boards with
                            WiFi capability (nRF7002 DK, nRF7000 EK shield).
                            Used for WiFi firmware and credential storage.

    • nrf5340_required_entries: Additional partition names for nRF5340 dual-core
                               SoC. Includes network core (cpunet) partitions
                               for Bluetooth/Thread operation.

    • ns_required_entries: Additional partition names for TF-M (Trusted Firmware-M)
                          builds. Required when using _ns (non-secure) variants
                          with ARM TrustZone security.

    • file_patterns: Patterns to classify PM files by board type:
                    - wifi_patterns: Identifies WiFi-capable board configs
                    - nrf5340_patterns: Identifies nRF5340 board configs
                    - ns_patterns: Identifies TF-M non-secure configs
                    - internal_patterns: Identifies internal-only memory configs

    • exclusion_rules: Partitions that should NOT exist in certain variants:
                      - internal_exclude: Partitions forbidden in internal
                        flash-only builds (e.g., external_flash)

    VALIDATION STEPS:
    -----------------
    1. PM File Discovery:
       - Searches for all files matching pattern: pm_static_*.yml
       - Collects all PM files in sample directory
       - Each file corresponds to a specific board configuration
       - Reports if no PM files are found (error condition)

    2. YAML Parsing:
       - Opens each PM file and reads YAML content
       - Parses YAML structure to extract partition definitions
       - Validates YAML syntax is correct
       - Reports empty or malformed YAML files as errors

    3. Partition Name Extraction:
       - Extracts all top-level YAML keys as partition names
       - Each key represents a memory partition (e.g., "mcuboot", "app")
       - Creates set of partition names for comparison
       - Ignores partition properties (address, size, etc.)

    4. Base Requirements Validation:
       - Checks for presence of all base_required_entries
       - For internal variants: Excludes external_flash from requirements
       - Reports missing base partitions as errors
       - Examples:
         * Missing "mcuboot" → Error
         * Missing "factory_data" → Error
         * Missing "settings_storage" → Error

    5. WiFi-Specific Validation:
       - Detects WiFi files using wifi_patterns (e.g., "_nrf70ek", "nrf7002dk")
       - Validates presence of wifi_required_entries
       - Reports missing WiFi partitions for WiFi-enabled boards
       - Examples:
         * pm_static_nrf7002dk_nrf5340_cpuapp.yml must have "nrf70"
         * Files with "_nrf70ek" must have WiFi storage partitions

    6. nRF5340 Multi-Core Validation:
       - Detects nRF5340 files using nrf5340_patterns
       - Validates presence of network core partitions
       - Ensures both application and network core slots exist
       - Examples:
         * Must have "mcuboot_primary_1" for network core
         * Must have "mcuboot_secondary_1" for network core DFU

    7. TF-M Non-Secure Validation:
       - Detects TF-M builds using ns_patterns (filename contains "_ns")
       - Validates presence of TF-M secure firmware partitions
       - Ensures proper TrustZone memory layout
       - Examples:
         * pm_static_nrf5340dk_nrf5340_cpuapp_ns.yml must have "tfm"

    8. Internal Variant Validation:
       - Detects internal variants using internal_patterns
       - Validates that excluded partitions are NOT present
       - Ensures internal builds don't reference external flash
       - Examples:
         * pm_static_*_internal.yml must NOT have "external_flash"
         * Internal configs should have reduced partition set

    9. Pattern Matching:
       - Uses substring matching for pattern detection
       - Case-sensitive pattern matching for filenames
       - Multiple patterns can match same file (e.g., nRF5340 + WiFi)
       - Accumulates all applicable requirements

    10. Issue Reporting:
        - Missing base partitions: Error
        - Missing variant-specific partitions: Error
        - Forbidden partitions present: Error
        - Successfully validated files: Debug output
        - Shows which variant checks passed for each file

    NOTES:
    ------
    • PM files are board-specific memory layout definitions
    • Each board configuration requires its own PM static file
    • Partition names must match exactly (case-sensitive)
    • Addresses and sizes are not validated (only presence/absence)
    • Internal variants have smaller partition sets (no external flash)
    • Multi-core boards (nRF5340) require partitions for each core
    • WiFi boards need additional partitions for WiFi firmware
    • TF-M builds add secure firmware partitions
"""


class PmStaticTestCase(MatterSampleTestCase):
    def __init__(self):
        super().__init__()
        self.pm_files = []

    def name(self) -> str:
        return "PM Static"

    def prepare(self):
        self.pm_files = list(self.config.sample_path.glob('pm_static_*.yml'))

    def check(self):
        # If no PM static files are found, return an issue
        if not self.pm_files:
            self.issue("No pm_static_*.yml files found")
            return

        # Validate all possible PM static files
        for pm_file in self.pm_files:
            self.debug(f"✓ Found PM static config: {pm_file.name}")
            filename = pm_file.name

            # Check if file is readable and parse YAML
            try:
                with open(pm_file, 'r', encoding='utf-8') as f:
                    pm_content = f.read()
                    if not pm_content.strip():
                        self.issue(
                            f"PM static file is empty: {filename}")
                        continue

                # Parse YAML to get all partition entries
                try:
                    pm_data = yaml.safe_load(pm_content)
                    if not pm_data:
                        self.issue(
                            f"PM static file has no valid YAML content: {filename}")
                        continue

                    # Get all top-level keys (partition names)
                    partition_names = set(pm_data.keys())

                    # 1. Check base required entries
                    self._check_base_required_entries(
                        filename, partition_names)

                    # 2. Check WiFi required entries
                    self._check_wifi_required_entries(
                        filename, partition_names)

                    # 3. Check nRF5340 required entries
                    self._check_nrf5340_required_entries(
                        filename, partition_names)

                    # 4. Check non-secure (TFM) required entries
                    self._check_ns_required_entries(filename, partition_names)

                    # 5. Check that internal files don't contain excluded entries
                    self._check_internal_exclude(filename, partition_names)

                except yaml.YAMLError as e:
                    self.issue(
                        f"PM static file {filename} has invalid YAML format: {e}")

            except Exception as e:
                self.warning(
                    f"Could not read PM static file {filename}: {e}")

    def _check_base_required_entries(self, filename: str, partition_names: set) -> None:
        base_required_entries = set(self.config.config_file.get(
            'pm_static').get('base_required_entries'))

        # Adjust base required entries for internal files (exclude external_flash)
        if any(pattern in filename for pattern in self.config.config_file.get('pm_static').get('file_patterns').get('internal_patterns')):
            base_required_entries -= set(self.config.config_file.get('pm_static').get(
                'exclusion_rules').get('internal_exclude'))

        missing_base = base_required_entries - partition_names
        if missing_base:
            for missing_entry in sorted(missing_base):
                self.issue(
                    f"PM static file {filename} missing required entry: {missing_entry}")

    def _check_wifi_required_entries(self, filename: str, partition_names: set) -> None:
        wifi_required_entries = set(self.config.config_file.get(
            'pm_static').get('wifi_required_entries'))
        wifi_patterns = self.config.config_file.get('pm_static').get(
            'file_patterns').get('wifi_patterns')

        if any(pattern in filename for pattern in wifi_patterns):
            missing_wifi = wifi_required_entries - partition_names
            if missing_wifi:
                for missing_entry in sorted(missing_wifi):
                    self.issue(
                        f"PM static file {filename} missing WiFi-specific entry: {missing_entry}")
            else:
                self.debug(
                    f"  ✓ WiFi entries present in {filename}")

    def _check_nrf5340_required_entries(self, filename: str, partition_names: set) -> None:
        nrf5340_required_entries = set(self.config.config_file.get(
            'pm_static').get('nrf5340_required_entries'))
        nrf5340_patterns = self.config.config_file.get('pm_static').get(
            'file_patterns').get('nrf5340_patterns')

        if any(pattern in filename for pattern in nrf5340_patterns):
            missing_nrf5340 = nrf5340_required_entries - partition_names
            if missing_nrf5340:
                for missing_entry in sorted(missing_nrf5340):
                    self.issue(
                        f"PM static file {filename} missing nRF5340-specific entry: {missing_entry}")
            else:
                self.debug(
                    f"  ✓ nRF5340 entries present in {filename}")

    def _check_ns_required_entries(self, filename: str, partition_names: set) -> None:
        ns_required_entries = set(self.config.config_file.get(
            'pm_static').get('ns_required_entries'))
        ns_patterns = self.config.config_file.get('pm_static').get(
            'file_patterns').get('ns_patterns')

        if any(pattern in filename for pattern in ns_patterns):
            missing_ns = ns_required_entries - partition_names
            if missing_ns:
                for missing_entry in sorted(missing_ns):
                    self.issue(
                        f"PM static file {filename} missing non-secure (_ns) entry: {missing_entry}")

    def _check_internal_exclude(self, filename: str, partition_names: set) -> None:
        internal_exclude = set(self.config.config_file.get('pm_static').get(
            'exclusion_rules').get('internal_exclude'))
        internal_patterns = self.config.config_file.get('pm_static').get(
            'file_patterns').get('internal_patterns')

        if any(pattern in filename for pattern in internal_patterns):
            excluded_entries = internal_exclude & partition_names
            if excluded_entries:
                for excluded_entry in sorted(excluded_entries):
                    self.issue(
                        f"PM static file {filename} should not contain '{excluded_entry}' entry (internal flash variant)")
            else:
                self.debug(
                    f"  ✓ Internal variant correctly excludes {', '.join(sorted(self.config.config_file.get('pm_static').get('exclusion_rules').get('internal_exclude')))} in {filename}")
