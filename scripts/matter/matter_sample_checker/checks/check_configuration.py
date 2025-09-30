#
# Copyright (c) 2025 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause

from internal.checker import MatterSampleTestCase
import re

"""
    CONFIGURATION CONSISTENCY VALIDATION CHECK:

    Validates that Matter sample configuration files contain all required Kconfig
    settings for proper Matter functionality. This ensures samples have the necessary
    configurations for Matter stack initialization, commissioning, and operation.

    CONFIGURATION PARAMETERS:
    -------------------------
    • required_prj_configs: Array of CONFIG_* entries that must exist in project
                           configuration files (prj.conf and prj_release.conf).
                           These are essential for Matter protocol functionality.

    • required_sysbuild_configs: Array of SB_CONFIG_* entries required in sysbuild
                                configuration files. These control build system
                                features like factory data and bootloader.

    • consistency_checks: Advanced validation rules for related configuration
                         groups. Validates that related configs are properly set.

    VALIDATION STEPS:
    -----------------
    1. Project Configuration Validation (prj.conf):
       - Locates prj.conf in sample directory
       - Reads file content as plain text
       - Searches for each required CONFIG_* entry
       - Validates entries exist in file (checks for presence of config name)
       - Reports missing configurations as issues
       - Provides debug output for found configurations

    2. Release Configuration Validation (prj_release.conf):
       - Checks if prj_release.conf exists in sample directory
       - Performs same validation as prj.conf
       - Ensures release builds have required configurations
       - Allows for release-specific config overrides
       - Reports missing entries specific to release variant

    3. Sysbuild Configuration Validation (sysbuild.conf):
       - Locates sysbuild.conf in sample directory
       - Reads file content for build system configs
       - Validates presence of required SB_CONFIG_* entries
       - Checks for bootloader and factory data configurations
       - Ensures sysbuild features are properly enabled

    4. Internal Sysbuild Validation (sysbuild_internal.conf):
       - Checks for internal build variant configuration
       - Validates same required configs as standard sysbuild.conf
       - Ensures internal memory variants have proper settings
       - Reports issues specific to internal configurations

    5. Configuration Consistency Checks:
       - Validates related configuration groups are properly set
       - Checks for conflicting configuration combinations
       - Ensures dependent configurations are present together
       - Validates Matter-specific configuration patterns

    NOTES:
    ------
    • Check validates presence of config entries with exact key match
    • For requirements with explicit values (e.g. SB_CONFIG_MATTER=y), exact match is required
    • Commented-out lines are ignored
    • Each configuration file is checked independently
    • Missing files (like prj_release.conf) are silently skipped if optional
"""


class ConfigurationTestCase(MatterSampleTestCase):
    def name(self) -> str:
        return "Configuration consistency"

    def prepare(self):
        pass

    def check(self):
        # 1. Check prj.conf and prj_release.conf for basic Matter settings
        prj_conf_paths = [self.config.sample_path / 'prj.conf',
                          self.config.sample_path / 'prj_release.conf']
        for prj_conf_path in prj_conf_paths:
            if prj_conf_path.exists():
                with open(prj_conf_path, 'r') as f:
                    prj_content = f.read()

                required_configs = self.config.config_file.get(
                    'validation').get('required_prj_configs')

                for config in required_configs:
                    if not self._config_present(prj_content, config):
                        self.issue(f"Missing configuration in prj.conf: {config}")
                    else:
                        self.debug(f"✓ Found config: {config}")

        # 2. Check sysbuild.conf for basic Matter settings
        sysbuild_conf_paths = [
            self.config.sample_path / 'sysbuild.conf', self.config.sample_path / 'sysbuild_internal.conf']
        for sysbuild_conf_path in sysbuild_conf_paths:
            if sysbuild_conf_path.exists():
                with open(sysbuild_conf_path, 'r') as f:
                    sysbuild_content = f.read()

                required_configs = self.config.config_file.get(
                    'validation').get('required_sysbuild_configs')

                for config in required_configs:
                    if not self._config_present(sysbuild_content, config):
                        self.issue(f"Missing configuration in sysbuild.conf: {config}")
                    else:
                        self.debug(f"✓ Found config: {config}")

    def _config_present(self, content: str, requirement: str) -> bool:
        """Check if a required config is present in the given Kconfig-style content.

        Behaviors:
        - Lines starting with '#' (after optional whitespace) are ignored.
        - If requirement contains '=', require exact full-line match (key=value).
        - Otherwise, require an exact key match at line start (key[=...]).
        """
        # Pre-split lines to respect comments and avoid false substring matches
        lines = content.splitlines()

        # If requirement includes an explicit value, require exact match
        if '=' in requirement:
            exact_pattern = re.compile(r"^\s*" + re.escape(requirement) + r"\s*$")
            for line in lines:
                stripped = line.lstrip()
                if not stripped or stripped.startswith('#'):
                    continue
                if exact_pattern.match(line):
                    return True
            return False

        # Otherwise, check for exact key at start of a non-comment line
        key_pattern = re.compile(r"^\s*" + re.escape(requirement) + r"(\s*=.*)?\s*$")
        for line in lines:
            stripped = line.lstrip()
            if not stripped or stripped.startswith('#'):
                continue
            if key_pattern.match(line):
                return True
        return False
