from typing import Dict
from pathlib import Path
import re

from internal.checker import MatterSampleTestCase

"""
    DOCUMENTATION INDEX VERSION VALIDATION CHECK:
    
    Ensures correct Matter SDK version and table coherence in the index.rst
    documentation file. This check validates that Matter specification versions
    and SDK versions are properly documented and that the version table contains
    all required entries with consistent formatting.

    CONFIGURATION PARAMETERS:
    -------------------------
    • documentation.index.path: Path to the index.rst file relative to the nRF
                                repository root. This file contains the main
                                documentation with version information.

    • documentation.index.matter_pattern: Regular expression pattern to extract
                                         Matter specification and SDK versions from
                                         the version line. Should capture both versions
                                         as separate groups.

    • documentation.index.line_pattern: String pattern to identify the line containing
                                       the Matter version information in the document.
                                       Used to locate the target line for validation.

    VALIDATION STEPS:
    -----------------
    1. File Reading:
       - Locates the index.rst file using the configured path
       - Reads the entire content with UTF-8 encoding
       - Reports errors if the file cannot be accessed
       - Stores content for subsequent analysis

    2. NCS Version Line Validation:
       - Searches for the line matching the configured pattern
       - Extracts Matter specification and SDK versions using regex
       - Validates that both versions are present and parseable
       - Reports the line number and extracted version information
       - Issues errors if pattern matching fails
       - Stores extracted versions for comparison with table entries

    3. Version Table Parsing:
       - Locates the version table toggle section in the document
       - Parses the RST table format with pipe-separated columns
       - Handles special formatting like |release| substitutions
       - Processes spanning cells and row groups with shared values
       - Identifies nRF Connect SDK, Matter spec, and Matter SDK columns

    4. Table Entry Validation:
       - Verifies presence of |release| entry in the version table
       - Verifies presence of (latest) entry in the version table
       - Handles row grouping where Matter versions span multiple NCS versions
       - Extracts and validates version information for each entry
       - Reports complete information for special entries

    5. Issue Reporting:
       - Reports missing or malformed version patterns in the main text
       - Reports missing |release| or (latest) entries in the table
       - Provides detailed information about found entries (NCS, spec, SDK versions)
       - Reports version mismatches between line versions and table entries
       - Validates both Matter specification and SDK versions for |release| and (latest)
       - Confirms successful validation when all entries are correct

    NOTES:
    ------
    • The check handles RST table format with spanning cells across rows
    • Special substitutions like |release| are recognized and parsed correctly
    • Table parsing stops at the .. note:: section to avoid false positives
    • Row groups with shared Matter versions are handled by carrying forward values
    • Line numbers are 1-indexed for display purposes
"""

class CheckDocIndexTestCase(MatterSampleTestCase):
    def __init__(self):
        super().__init__()
        self.file_path = None
        self.version_pattern = None
        self.line_pattern = None
        self.content = ""
        self.line_spec_version = None
        self.line_sdk_version = None

    def name(self) -> str:
        return "Check Matter SDK version and table coherence in index.rst"
    
    def prepare(self):
        self.file_path = self.config.config_file.get("documentation").get("index").get("path")
        self.version_pattern = self.config.config_file.get("documentation").get("index").get("matter_pattern")
        self.line_pattern = self.config.config_file.get("documentation").get("index").get("line_pattern")
        try:
            with open(self.config.nrf_path / self.file_path, "r") as f:
                self.content = f.read()
        except Exception as e:
            self.issue(f"Error reading file: {e}")            

    def check(self):
        self._check_ncs_version_line(self.content)
        self._parse_version_table(self.content)

    def _check_ncs_version_line(self, content: str) -> Dict:
        
        # Search for the line containing the pattern
        target_line = None
        line_number = None
        
        for i, line in enumerate(content.splitlines()):
            if self.line_pattern in line:
                target_line = line
                line_number = i + 1  # 1-indexed for display
                break
        
        if target_line is None:
            self.issue(f"Could not find line containing '{self.line_pattern}' pattern")
            return
        
        self.debug(f"Found pattern at line {line_number}: {target_line}")
        
        # Extract versions
        match = re.search(self.version_pattern, target_line)
        
        if not match:
            self.issue(f"Could not parse Matter versions from line {line_number}")
            return
        
        spec_version, sdk_version = match.groups()
        self.line_spec_version = spec_version
        self.line_sdk_version = sdk_version
        self.info(f"Matter specification version: {spec_version}")
        self.info(f"Matter SDK version: {sdk_version}")
        
    def _parse_version_table(self, content: str) -> Dict:
        """Parse the Matter version table and extract key information"""
        
        self.debug(f"Parsing Matter version table...")
        
        # Find the table using the toggle section
        toggle_start = content.find(".. toggle:: nRF Connect SDK, Matter specification, and Matter SDK versions")
        if toggle_start == -1:
            self.issue(f"Version table not found")
            return
            
        table_section = content[toggle_start:]
        lines = table_section.splitlines()
        
        release_info = None
        latest_info = None
        
        # Variables to handle row groups and spanning cells
        current_matter_spec = ""
        current_matter_sdk = ""
        group_entries = []
        
        for i, line in enumerate(lines):
            line = line.strip()
            
            # Skip header lines and separators
            if (line.startswith('+==') or line.startswith('===') or 
                'nRF Connect SDK version' in line or not '|' in line):
                continue
                
            # Check for major row separators (indicates end of a row group)
            if line.startswith('+--') and line.count('+') >= 3 and '-----+-----' in line:
                # Apply current values to all entries in the group
                self._update_group_entries(group_entries, current_matter_spec, current_matter_sdk)
                # Reset for next group
                current_matter_spec = ""
                current_matter_sdk = ""
                group_entries = []
                continue
                
            # Process table rows
            if '|' in line:
                # Handle |release| specially since it has embedded pipes
                if '|release|' in line:
                    # Split around |release|
                    before_release = line[:line.find('|release|')]
                    after_release = line[line.find('|release|') + len('|release|'):]
                    ncs_version = '|release|'
                    # Parse the parts after |release|
                    after_parts = [p.strip() for p in after_release.split('|')]
                    matter_spec = after_parts[1] if len(after_parts) > 1 else ""
                    matter_sdk = after_parts[2] if len(after_parts) > 2 else ""
                else:
                    # Normal row parsing
                    parts = [p.strip() for p in line.split('|')]
                    if len(parts) < 4:  # Need at least empty, ncs, spec, sdk
                        continue
                    ncs_version = parts[1].strip()
                    matter_spec = parts[2].strip()
                    matter_sdk = parts[3].strip()
                
                # Update current values if we found non-empty ones
                if matter_spec and not current_matter_spec:
                    current_matter_spec = matter_spec
                if matter_sdk and not current_matter_sdk:
                    current_matter_sdk = matter_sdk
                
                # Create entry info
                entry_info = {
                    'ncs_version': ncs_version,
                    'matter_spec': matter_spec,
                    'matter_sdk': matter_sdk,
                    'line_number': i + 1,
                    'entry_type': None
                }
                
                # Check for special entries
                if '(latest)' in ncs_version:
                    entry_info['entry_type'] = 'latest'
                    latest_info = entry_info
                    group_entries.append(('latest', entry_info))
                elif '|release|' in ncs_version:
                    entry_info['entry_type'] = 'release'
                    release_info = entry_info  
                    group_entries.append(('release', entry_info))
                        
            # Stop at notes section
            if line.startswith('.. note::'):
                break
        
        # Handle any remaining group entries
        self._update_group_entries(group_entries, current_matter_spec, current_matter_sdk)
        
        # Display findings
        issues = []
        if release_info:
            self.info(f"Found |release| entry")
            self.info(f"    NCS Version: {release_info['ncs_version']}")
            self.info(f"    Matter Spec: {release_info['matter_spec']}")
            self.info(f"    Matter SDK: {release_info['matter_sdk']}")
            
            # Validate versions against the line versions
            if self.line_spec_version and self.line_sdk_version:
                # Extract just the version number from table entries (remove links/refs)
                table_spec = self._extract_version_from_text(release_info['matter_spec'])
                table_sdk = release_info['matter_sdk'].strip()
                
                if table_spec != self.line_spec_version:
                    self.issue(f"Matter specification version mismatch for |release|: line {self.line_spec_version} != table {table_spec}")
                    issues.append(f"Spec version mismatch for |release|")
                    
                if table_sdk != self.line_sdk_version:
                    self.issue(f"Matter SDK version mismatch for |release|: line {self.line_sdk_version} != table {table_sdk}")
                    issues.append(f"SDK version mismatch for |release|")
        else:
            self.issue(f"Could not find |release| entry in table")
            issues.append("|release| entry not found in version table")
            
        if latest_info:
            self.info(f"Found (latest) entry")
            self.info(f"    NCS Version: {latest_info['ncs_version']}")
            self.info(f"    Matter Spec: {latest_info['matter_spec']}")
            self.info(f"    Matter SDK: {latest_info['matter_sdk']}")
            
            # Validate versions against the line versions
            if self.line_spec_version and self.line_sdk_version:
                # Extract just the version number from table entries (remove links/refs)
                table_spec = self._extract_version_from_text(latest_info['matter_spec'])
                table_sdk = latest_info['matter_sdk'].strip()
                
                if table_spec != self.line_spec_version:
                    self.issue(f"Matter specification version mismatch for (latest): line {self.line_spec_version} != table {table_spec}")
                    issues.append(f"Spec version mismatch for (latest)")
                    
                if table_sdk and table_sdk != self.line_sdk_version:
                    self.issue(f"Matter SDK version mismatch for (latest): line {self.line_sdk_version} != table {table_sdk}")
                    issues.append(f"SDK version mismatch for (latest)")
        else:
            self.issue(f"Could not find (latest) entry in table")
            issues.append("(latest) entry not found in version table")
            
        if not issues:
            self.info(f"Version table entries look good")
            
    def _update_group_entries(self, group_entries, current_matter_spec, current_matter_sdk) -> None:
        """Update all entries in a group with shared values"""
        for entry_type, entry_info in group_entries:
            if not entry_info['matter_spec'] and current_matter_spec:
                entry_info['matter_spec'] = current_matter_spec
            if not entry_info['matter_sdk'] and current_matter_sdk:
                entry_info['matter_sdk'] = current_matter_sdk
    
    def _extract_version_from_text(self, text: str) -> str:
        """Extract version number from text that may contain RST references
        
        Example: ':ref:`1.5.2 <ug_matter_overview_dev_model_support>`' -> '1.5.2'
                 '1.4.2' -> '1.4.2'
        """
        # Try to match :ref:`VERSION <...>` pattern
        ref_match = re.search(r':ref:`([0-9.]+)\s*<[^>]+>`', text)
        if ref_match:
            return ref_match.group(1)
        
        # If no ref pattern, try to extract plain version number
        version_match = re.search(r'([0-9]+\.[0-9]+\.[0-9]+)', text)
        if version_match:
            return version_match.group(1)
        
        # Return stripped text if no pattern matches
        return text.strip()
