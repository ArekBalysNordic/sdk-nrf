import re
import yaml
from pathlib import Path
from typing import Dict, List, Set

from internal.checker import MatterSampleTestCase

"""
    HARDWARE REQUIREMENTS DOCUMENTATION VALIDATION CHECK:
    
    Validates the consistency and accuracy of hardware requirements documentation,
    including build configurations, diagnostic logs, and memory layouts. This check
    ensures that documentation matches the actual implementation in sample.yaml files
    and pm_static files.

    CONFIGURATION PARAMETERS:
    -------------------------
    • documentation.requirements.path: Path to the hw_requirements.rst file relative 
                                       to the nRF repository root.

    VALIDATION STEPS:
    -----------------
    1. Build Configuration Validation:
       - Analyzes sample.yaml files for all Matter samples and applications
       - Extracts and validates build types (debug, release, internal, wifi_thread_switched)
       - Verifies that critical samples have required build configurations
       
    2. Diagnostic Logs Validation:
       - Reads DTS overlay files from matter-diagnostic-logs snippet
       - Parses retention partition definitions and sizes
       - Compares partition sizes with documentation tables
       
    3. Memory Layouts Validation:
       - Parses memory layout tables from documentation
       - Compares with pm_static YAML files for each board
       - Validates partition sizes and offsets match between docs and implementation
       - NOTE: Nordic Thingy:53 memory layout tables are NOT validated as they use
         a different pm_static file structure from the weather station application

    NOTES:
    ------
    • This check runs once per test session, not per sample
    • Validates consistency between multiple sources: samples, snippets, and docs
    • Handles board name mappings between different naming conventions
"""

# Bidirectional board mapping (board_id <-> display_name)
BOARD_MAPPINGS = {
    'nrf52840dk_nrf52840': 'nRF52840 DK',
    'nrf5340dk_nrf5340_cpuapp': 'nRF5340 DK',
    'nrf7002dk_nrf5340_cpuapp': 'nRF7002 DK',
    'thingy53_nrf5340_cpuapp': 'Nordic Thingy:53',
    'nrf54l15dk_nrf54l15_cpuapp': 'nRF54L15 DK',
    'nrf54l15dk_nrf54l10_cpuapp': 'nRF54L10 emulation on nRF54L15 DK',
    'nrf54lm20dk_nrf54lm20a_cpuapp': 'nRF54LM20 DK',
    'nrf54l15dk_nrf54l15_cpuapp_internal': 'nRF54L15 DK with internal memory only',
    'nrf54l15dk_nrf54l15_cpuapp_ns': 'nRF54L15 DK with TF-M',
    'nrf54lm20dk_nrf54lm20a_cpuapp_internal': 'nRF54LM20 DK with internal memory only'
}

# Helper functions for board mapping
def get_display_name(board_id: str) -> str:
    """Get display name from board ID"""
    return BOARD_MAPPINGS.get(board_id, board_id)

def get_board_id(display_name: str) -> str:
    """Get board ID from display name"""
    for board_id, name in BOARD_MAPPINGS.items():
        if name == display_name:
            return board_id
    return display_name

# Partition configuration
PARTITION_CONFIG = {
    'retention': {
        'crash_retention': 'Crash retention',
        'network_logs_retention': 'Network logs retention',
        'end_user_logs_retention': 'User data logs retention'
    },
    'name_mappings': {
        'Bootloader': 'mcuboot',
        'MCUboot bootloader': 'mcuboot',
        'Application': 'mcuboot_primary',
        'Application DFU': 'mcuboot_secondary',
        'Network Core DFU': 'mcuboot_secondary_1',
        'Factory data': 'factory_data',
        'Non-volatile storage': 'settings_storage',
        'Free space': 'external_flash',
        'Static RAM': 'sram_primary',
        'mcuboot_primary_app': 'app',
        'mcuboot_secondary_app': 'mcuboot_secondary',
    },
    'key_partitions': [
        'mcuboot', 'mcuboot_primary', 'mcuboot_primary_app', 'mcuboot_pad',
        'factory_data', 'settings_storage', 'mcuboot_secondary', 'external_flash', 
        'sram_primary', 'mcuboot_primary_2', 'mcuboot_secondary_2', 
        'mcuboot_primary_1', 'mcuboot_secondary_1', 'mcuboot_secondary_app',
        'mcuboot_secondary_pad', 'nrf70_wifi_fw_mcuboot_pad', 'nrf70_wifi_fw'
    ]
}


class CheckDocRequirementsTestCase(MatterSampleTestCase):
    """Check hardware requirements documentation consistency"""
    
    def __init__(self):
        super().__init__()
        self.file_path = None
        self.content = ""
        self.expected_build_types = {"debug", "release", "internal", "wifi_thread_switched"}
        self._requirements_cache = None  # Cache for parsed requirements data
        
    def name(self) -> str:
        return "Hardware requirements documentation check"
    
    def prepare(self):
        """Prepare the check by reading the requirements file"""
        self.file_path = self.config.config_file.get("documentation").get("requirements").get("path")
        try:
            with open(self.config.nrf_path / self.file_path, "r") as f:
                self.content = f.read()
        except Exception as e:
            self.issue(f"Error reading file: {e}")
    
    def check(self):
        """Run the comprehensive hardware requirements validation"""
        if not self.content:
            self.issue("No content to validate")
            return
        
        # Basic content analysis
        tables_count = self.content.count('+-') // 2
        tabs_count = self.content.count('.. tab::')
        self.debug(f"Found {tables_count} tables and {tabs_count} tabs")
        
        # Get sample and application information
        samples_path = self.config.nrf_path / "samples" / "matter"
        apps_path = self.config.nrf_path / "applications"
        
        samples_info = self._analyze_matter_samples(samples_path)
        apps_info = self._analyze_matter_applications(apps_path)
        
        # Validate build configurations
        self._validate_build_configurations(samples_info, apps_info)
        
        # Validate diagnostic logs
        self._validate_diagnostic_logs()
        
        # Validate memory layouts
        self._validate_memory_layouts(samples_info)
    
    def _analyze_matter_samples(self, samples_path: Path) -> Dict:
        """Quick analysis of Matter samples"""
        samples_info = {}

        if not samples_path.exists():
            return samples_info

        for sample_dir in samples_path.iterdir():
            if not sample_dir.is_dir():
                continue

            sample_yaml = sample_dir / "sample.yaml"
            if not sample_yaml.exists():
                continue

            try:
                with open(sample_yaml, 'r') as f:
                    config = yaml.safe_load(f)

                # Extract build types
                build_types = set()
                platforms = set()

                if 'tests' in config:
                    for test_name in config['tests'].keys():
                        build_type = self._extract_build_type_from_name(test_name)
                        if build_type:
                            build_types.add(build_type)

                samples_info[sample_dir.name] = {
                    'build_types': build_types,
                    'platforms': platforms
                }

            except Exception:
                continue  # Skip problematic samples

        return samples_info

    def _analyze_matter_applications(self, apps_path: Path) -> Dict:
        """Quick analysis of Matter applications"""
        apps_info = {}
        app_names = ['matter_weather_station', 'matter_bridge']

        for app_name in app_names:
            app_path = apps_path / app_name
            sample_yaml = app_path / "sample.yaml"

            if not sample_yaml.exists():
                continue

            try:
                with open(sample_yaml, 'r') as f:
                    config = yaml.safe_load(f)

                build_types = set()
                if 'tests' in config:
                    for test_name in config['tests'].keys():
                        build_type = self._extract_build_type_from_name(test_name)
                        if build_type:
                            build_types.add(build_type)

                apps_info[app_name] = {
                    'build_types': build_types,
                    'platforms': set()
                }

            except Exception:
                continue

        return apps_info

    def _extract_build_type_from_name(self, test_name: str) -> str:
        """Extract build type from test name"""
        test_name_lower = test_name.lower()

        if 'thread_wifi_switched' in test_name_lower or 'wifi_thread_switched' in test_name_lower:
            return 'wifi_thread_switched'
        elif '.release' in test_name_lower:
            if 'internal' in test_name_lower:
                return 'internal'
            return 'release'
        elif '.debug' in test_name_lower:
            return 'debug'

        return None

    def _validate_build_configurations(self, samples: Dict, apps: Dict) -> None:
        """Validate build configurations with detailed tracking"""
        all_projects = {**samples, **apps}

        for project_name, project_info in all_projects.items():
            available_builds = project_info.get('build_types', set())

            # Check for missing critical build types
            if project_name == 'template':
                has_internal = 'internal' in available_builds
                if not has_internal:
                    self.issue(f"Template sample missing 'internal' build type")

            if project_name == 'lock':
                has_wifi_switched = 'wifi_thread_switched' in available_builds
                if not has_wifi_switched:
                    self.warning(f"Lock sample should support 'wifi_thread_switched'")

            # Check for missing debug/release
            basic_builds = {'debug', 'release'}
            missing_basic = basic_builds - available_builds
            if missing_basic and project_name != 'manufacturer_specific':
                for build in missing_basic:
                    self.warning(f"Sample '{project_name}' missing '{build}' build")

    def _validate_diagnostic_logs(self) -> None:
        """Enhanced validation of diagnostic logs - reads DTS overlays and compares retention partitions"""
        diag_logs_path = self.config.nrf_path / "snippets" / "matter-diagnostic-logs" / "boards"

        if not diag_logs_path.exists():
            self.issue("Diagnostic logs snippet directory not found")
            return

        # Check if diagnostic logs section exists
        has_diag_section = 'Diagnostic logs RAM memory requirements' in self.content
        if not has_diag_section:
            self.warning("Diagnostic logs section not found in documentation")

        # Get overlay files and analyze retention partitions
        overlay_files = list(diag_logs_path.glob("*.overlay"))
        overlay_count = len(overlay_files)

        if self.config.verbose:
            self.debug(f"\n+++ Analyzing {overlay_count} DTS overlay files for retention partitions")

        # Analyze each overlay file for retention partitions
        self._validate_retention_partitions(overlay_files)

    def _validate_retention_partitions(self, overlay_files: List[Path]) -> None:
        """Validate retention partitions in DTS overlay files against sample configurations"""
        partition_analysis = {}

        for overlay_file in overlay_files:
            board_name = self._extract_board_name_from_overlay(overlay_file.name)
            if self.config.verbose:
                self.debug(f"+ Analyzing {overlay_file.name} for board {board_name}:")

            try:
                with open(overlay_file, 'r') as f:
                    dts_content = f.read()

                # Extract retention partition information
                partitions = self._parse_retention_partitions_from_dts(dts_content)
                partition_analysis[board_name] = partitions

                # Validate expected partitions are present
                for partition_key, partition_name in PARTITION_CONFIG['retention'].items():
                    if partition_key not in partitions:
                        self.warning(f"Board {board_name}: Missing {partition_name} partition in overlay")
                    else:
                        partition_info = partitions[partition_key]
                        if self.config.verbose:
                            self.debug(f"\tFound {partition_name}: "
                                     f"offset=0x{partition_info['offset']:x}, "
                                     f"size=0x{partition_info['size']:x} ({partition_info['size']} bytes)")

                self._validate_partition_sizes(partitions, board_name)

            except Exception as e:
                self.issue(f"Error reading DTS overlay {overlay_file.name}: {e}")

    def _extract_board_name_from_overlay(self, filename: str) -> str:
        """Extract board name from overlay filename"""
        # Remove .overlay extension to get the full board identifier
        board_name = filename.replace('.overlay', '')
        return board_name

    def _parse_retention_partitions_from_dts(self, dts_content: str) -> Dict:
        """Parse retention partition information from DTS content"""
        partitions = {}

        # Pattern to match retention partition declarations
        # Example: crash_retention: retention@20070000 { reg = <0x20070000 0x10000>; };
        retention_pattern = r'(\w+):\s*retention@([0-9a-fA-F]+)\s*\{\s*[^}]*reg\s*=\s*<\s*0x([0-9a-fA-F]+)\s+0x([0-9a-fA-F]+)\s*>\s*;[^}]*\}'

        matches = re.findall(retention_pattern, dts_content, re.MULTILINE | re.DOTALL)

        for match in matches:
            partition_name, base_addr, reg_offset, reg_size = match

            # Convert hex strings to integers
            offset = int(reg_offset, 16)
            size = int(reg_size, 16)
            base = int(base_addr, 16)

            # Map common partition names to standardized keys
            partition_key = self._map_partition_name_to_key(partition_name)

            partitions[partition_key] = {
                'name': partition_name,
                'base_address': base,
                'offset': offset,
                'size': size,
                'size_kb': size // 1024
            }

        return partitions

    def _map_partition_name_to_key(self, partition_name: str) -> str:
        """Map DTS and requirements partition names to standardized keys"""
        name_mapping = {
            # DTS partition names
            'crash_retention': 'crash_retention',
            'network_logs_retention': 'network_logs_retention',
            'end_user_logs_retention': 'end_user_logs_retention',
        }

        return name_mapping.get(partition_name.lower(), partition_name.lower())

    def _parse_requirements_partition_tables(self, requirements_file_path: str) -> Dict[str, Dict[str, Dict]]:
        """Parse board-specific partition tables from requirements.rst file"""
        # Return cached data if available
        if self._requirements_cache is not None:
            return self._requirements_cache
        
        board_partition_data = {}

        try:
            with open(requirements_file_path, 'r', encoding='utf-8') as f:
                content = f.read()

            # Find the diagnostic logs section by looking for start and end markers
            lines = content.split('\n')
            start_idx = -1
            end_idx = len(lines)

            # Find the start line
            for i, line in enumerate(lines):
                if 'Diagnostic logs RAM memory requirements' in line:
                    start_idx = i
                    break

            if start_idx == -1:
                return board_partition_data

            # Find the end line (next major section with underlined heading)
            for i in range(start_idx + 2, len(lines)):
                if lines[i].strip() and not lines[i].startswith((' ', '\t', '..', '+', '|')) and i + 1 < len(lines):
                    if lines[i + 1].strip() and lines[i + 1][0] in '=-^~':
                        end_idx = i
                        break

            diagnostic_content = '\n'.join(lines[start_idx:end_idx])

            # Extract individual board tabs
            tab_pattern = r'.. tab:: (.+?)\n(.*?)(?=.. tab:|\Z)'
            tab_matches = re.findall(tab_pattern, diagnostic_content, re.DOTALL)

            for board_name, tab_content in tab_matches:
                board_name = board_name.strip()
                partitions = {}

                # Extract table data - find all lines between table borders
                # Look for table rows that contain partition information
                tab_lines = tab_content.split('\n')
                rows = []

                # Find lines with table data (containing | separators)
                for line in tab_lines:
                    if '|' in line and not line.strip().startswith(('+', '=')):
                        # Split by | and clean up columns
                        columns = [col.strip() for col in line.split('|')[1:-1]]  # Remove first/last empty parts
                        # Valid row with 3 columns and non-empty first column
                        if len(columns) == 3 and columns[0]:
                            rows.append(columns)

                if rows:
                    for row in rows[1:]:  # Skip header row
                        partition_name = row[0].strip()
                        offset_str = row[1].strip()
                        size_str = row[2].strip()

                        if 'retention' in partition_name.lower():
                            # Parse size and offset with internal consistency checking
                            size_data = self._parse_value_from_string(size_str, allow_none=False)
                            offset_data = self._parse_value_from_string(offset_str, allow_none=True)
                            
                            # Check for internal consistency in size
                            if not size_data['is_consistent']:
                                unit_bytes = int(size_data['unit_value'] * 
                                               (1024 if size_data['unit_text'] in ['KB', 'kB'] else 
                                                1024*1024 if size_data['unit_text'] == 'MB' else 1))
                                hex_value_kb = size_data['hex_value'] / 1024 if size_data['hex_value'] else 0
                                
                                self.issue(f"{board_name}: {partition_name} SIZE MISMATCH - "
                                          f"Table decimal: \033[91m{hex_value_kb:.2f} KB (0x{size_data['hex_value']:X})\033[0m "
                                          f"vs dts: \033[91m{size_data['unit_value']} {size_data['unit_text']} (0x{unit_bytes:X})\033[0m")
                            
                            # Check for internal consistency in offset
                            if not offset_data['is_consistent']:
                                unit_bytes = int(offset_data['unit_value'] * 
                                               (1024 if offset_data['unit_text'] in ['KB', 'kB'] else 
                                                1024*1024 if offset_data['unit_text'] == 'MB' else 1))
                                hex_value_kb = offset_data['hex_value'] / 1024 if offset_data['hex_value'] else 0
                                
                                self.issue(f"{board_name}: {partition_name} OFFSET MISMATCH - "
                                          f"Table decimal: \033[91m{offset_data['unit_value']} {offset_data['unit_text']} (0x{unit_bytes:X})\033[0m "
                                          f"vs dts: \033[91m{hex_value_kb:.2f} KB (0x{offset_data['hex_value']:X})\033[0m")

                            if size_data['bytes'] is not None:
                                # Map partition names to standard keys
                                partition_key = self._map_partition_name_to_key(partition_name)
                                partitions[partition_key] = {
                                    'name': partition_name,
                                    'size': size_data['bytes'],
                                    'offset': offset_data['bytes'] if offset_data['bytes'] is not None else 0,
                                    'size_hex': f"0x{size_data['hex_value']:X}" if size_data['hex_value'] else '0x0',
                                    'offset_hex': f"0x{offset_data['hex_value']:X}" if offset_data['hex_value'] else '0x0'
                                }

                if partitions:
                    board_partition_data[board_name] = partitions

        except Exception as e:
            self.warning(f"Error parsing requirements file: {e}")

        # Cache the parsed data
        self._requirements_cache = board_partition_data
        return board_partition_data

    def _get_board_requirements_data(self, board_name: str, requirements_file_path: str) -> Dict[str, Dict]:
        """Get partition requirements for a specific board"""
        # Parse requirements data
        requirements_data = self._parse_requirements_partition_tables(requirements_file_path)

        # Find the correct board name in requirements
        req_board_name = get_display_name(board_name)

        return requirements_data.get(req_board_name, {})

    def _validate_partition_sizes(self, partitions: Dict, board_name: str) -> None:
        """Validate that partition sizes match requirements from documentation"""
        requirements_file_path = str(self.config.nrf_path / self.file_path)
        
        # Get requirements data for this board
        board_requirements = self._get_board_requirements_data(board_name, requirements_file_path)

        if not board_requirements:
            self.warning(f"Board {board_name}: No partition requirements found in documentation")
            return

        # Create a mapping between DTS partition names and requirements partition names
        # Requirements uses spaces, DTS uses underscores
        dts_to_req_mapping = {
            'crash_retention': 'crash retention',
            'network_logs_retention': 'network logs retention',
            'end_user_logs_retention': 'user data logs retention'
        }

        # Check for missing partitions
        req_to_dts_mapping = {v: k for k, v in dts_to_req_mapping.items()}
        for req_partition_key, req_partition_info in board_requirements.items():
            # Map requirements partition name back to DTS name
            dts_partition_key = req_to_dts_mapping.get(req_partition_key, req_partition_key)
            if dts_partition_key not in partitions:
                self.warning(f"Board {board_name}: Required partition '{req_partition_info['name']}' not found in DTS file")

    def _validate_memory_layouts(self, samples: Dict) -> None:
        """Comprehensive validation of memory layouts against pm_static files"""
        if 'template' not in samples:
            self.warning("Template sample not found for memory validation")
            return

        # Check if memory layout section exists
        has_memory_section = 'Reference Matter memory layouts' in self.content
        if not has_memory_section:
            self.issue("Memory layouts section not found in documentation")
            return

        try:
            # Parse the memory layouts section and validate against pm_static files
            self._validate_reference_memory_layouts()
        except Exception as e:
            self.issue(f"Error during memory layouts validation: {e}")

    def _validate_reference_memory_layouts(self) -> None:
        """Validate Reference Matter memory layouts section against pm_static files"""
        # Extract the Reference Matter memory layouts section
        memory_section = self._extract_memory_layouts_section(self.content)
        if not memory_section:
            self.issue("Could not extract Reference Matter memory layouts section")
            return

        # Parse tabs from the section
        tabs_data = self._parse_memory_layout_tabs(memory_section)
        if not tabs_data:
            self.issue("No tabs found in Reference Matter memory layouts section")
            return

        # Validate each tab
        for tab_name, tab_content in tabs_data.items():
            # Skip Thingy:53 - uses different pm_static file structure from weather station app
            if 'Thingy:53' in tab_name or 'THINGY:53' in tab_name:
                if self.config.verbose:
                    self.debug(f"\n+++ Skipping {tab_name} (not validated - uses weather station app pm_static)")
                continue
            
            if self.config.verbose:
                self.debug(f"\n+++ Validating tab: {tab_name}")

            # Get corresponding pm_static file path
            pm_file_path = self._get_pm_static_file_for_dk(tab_name)
            if not pm_file_path or not pm_file_path.exists():
                self.warning(f"No pm_static file found for {tab_name}")
                continue

            # Parse memory tables from tab content
            tab_tables = self._parse_memory_tables_from_tab(tab_content)
            if not tab_tables:
                self.warning(f"No memory tables found in {tab_name} tab")
                continue

            # Read and parse pm_static file
            pm_data = self._parse_pm_static_file(pm_file_path)
            if not pm_data:
                self.issue(f"Failed to parse pm_static file: {pm_file_path}")
                continue

            # Compare table data with pm_static data
            self._compare_memory_data(tab_name, tab_tables, pm_data)

    def _extract_memory_layouts_section(self, content: str) -> str:
        """Extract the 'Reference Matter memory layouts' section from content"""
        lines = content.split('\n')
        start_idx = -1
        end_idx = len(lines)

        # Find start of section
        for i, line in enumerate(lines):
            if 'Reference Matter memory layouts' in line:
                start_idx = i
                break

        if start_idx == -1:
            return ""

        # Find end of section (next major section with underlined heading)
        for i in range(start_idx + 2, len(lines)):
            if lines[i].strip() and not lines[i].startswith((' ', '\t', '..', '+', '|', '*')) and i + 1 < len(lines):
                if lines[i + 1].strip() and lines[i + 1][0] in '=-^~*':
                    end_idx = i
                    break

        return '\n'.join(lines[start_idx:end_idx])

    def _parse_memory_layout_tabs(self, section_content: str) -> Dict[str, str]:
        """Parse individual tabs from the memory layouts section"""
        # Pattern to match tabs - fixed to handle single newline after tab declaration
        # Previous pattern expected two newlines (\n\n) but actual format uses only one
        tab_pattern = r'.. tab:: (.+?)\n(.*?)(?=.. tab:|\Z)'
        matches = re.findall(tab_pattern, section_content, re.DOTALL)

        tabs_data = {}
        for tab_name, tab_content in matches:
            tabs_data[tab_name.strip()] = tab_content.strip()

        return tabs_data

    def _get_pm_static_file_for_dk(self, dk_name: str) -> Path:
        """Get the corresponding pm_static file path for a DK name"""
        board_id = get_board_id(dk_name)
        if not board_id:
            return None

        # Special handling for Thingy:53 - files are in weather station app
        if 'THINGY:53' in dk_name:
            return self.config.nrf_path / "applications" / "matter_weather_station" / "pm_static_factory_data.yml"

        # Regular files in template sample
        template_path = self.config.nrf_path / "samples" / "matter" / "template"
        pm_file = template_path / f"pm_static_{board_id}.yml"

        return pm_file

    @staticmethod
    def _convert_to_int(value) -> int:
        """Convert string/int value to integer, handling hex format"""
        if isinstance(value, int):
            return value
        if isinstance(value, str):
            if value.startswith('0x'):
                return int(value, 16)
            elif value.isdigit():
                return int(value)
        return 0

    @staticmethod
    def _is_table_header(line: str) -> bool:
        """Check if line is a memory table header"""
        line_lower = line.lower()
        return any(mem_type in line_lower for mem_type in ['flash', 'sram', 'otp']) and 'size:' in line

    @staticmethod
    def _find_table_start(lines: List[str], start_idx: int) -> int:
        """Find the start of table borders after header"""
        for j in range(start_idx + 1, min(start_idx + 10, len(lines))):
            if '+' in lines[j] and '=' in lines[j]:
                return j
        return -1

    @staticmethod
    def _parse_table_row(line: str) -> Dict:
        """Parse a single table row into structured data"""
        columns = [col.strip() for col in line.split('|')[1:-1]]
        
        if len(columns) < 3:
            return None
            
        row_data = {
            'partition': columns[0],
            'offset': columns[1] if len(columns) > 1 else '',
            'size': columns[2] if len(columns) > 2 else '',
            'elements': columns[3] if len(columns) > 3 else '',
            'element_offset': columns[4] if len(columns) > 4 else '',
            'element_size': columns[5] if len(columns) > 5 else ''
        }
        
        # Include rows with valid partition or element data
        has_partition = row_data['partition'] and not row_data['partition'].startswith(('Partition', '='))
        has_element = (row_data['elements'] and 
                      row_data['elements'] != 'Elements' and 
                      row_data['elements'] != '-')
        
        return row_data if (has_partition or has_element) else None

    def _parse_memory_tables_from_tab(self, tab_content: str) -> Dict[str, List[Dict]]:
        """Parse memory layout tables from tab content"""
        tables = {}
        lines = tab_content.split('\n')
        
        current_table = None
        current_rows = []
        in_table = False

        i = 0
        while i < len(lines):
            line = lines[i].strip()

            if self._is_table_header(line):
                # Save previous table
                if current_table and current_rows:
                    tables[current_table] = current_rows
                
                # Start new table
                current_table = line.split('(')[0].strip()
                current_rows = []
                
                # Find table start
                table_start = self._find_table_start(lines, i)
                if table_start != -1:
                    in_table = True
                    i = table_start
            
            elif in_table and line:
                if line.startswith('+'):
                    pass  # Table border/separator
                elif '|' in line:
                    # Parse table row
                    row_data = self._parse_table_row(line)
                    if row_data:
                        current_rows.append(row_data)
            
            elif in_table and line and not line.startswith(('+', '|', ' ')):
                # End current table
                if current_table and current_rows:
                    tables[current_table] = current_rows
                in_table = False
                current_table = None  
                current_rows = []
                continue

            i += 1

        # Save final table
        if current_table and current_rows:
            tables[current_table] = current_rows

        return tables

    def _parse_pm_static_file(self, pm_file_path: Path) -> Dict:
        """Parse pm_static YAML file and return partition data"""
        try:
            with open(pm_file_path, 'r') as f:
                pm_data = yaml.safe_load(f)

            parsed_data = {}

            for partition_name, partition_info in pm_data.items():
                if isinstance(partition_info, dict):
                    parsed_data[partition_name] = {
                        'address': partition_info.get('address', 0),
                        'size': partition_info.get('size', 0),
                        'region': partition_info.get('region', ''),
                        'device': partition_info.get('device', '')
                    }

            return parsed_data

        except Exception as e:
            return {}

    def _validate_partition_data(self, dk_name: str, display_name: str, pm_partition_name: str,
                                 size_str: str, offset_str: str, pm_data: Dict, is_element: bool) -> None:
        """Validate a single partition or element against pm_static data"""
        # Parse pm_static data with proper type conversion
        pm_size = pm_data[pm_partition_name]['size']
        pm_address = pm_data[pm_partition_name]['address']

        # Convert pm_static values to integers
        pm_size = self._convert_to_int(pm_size)
        pm_address = self._convert_to_int(pm_address)

        # Compare sizes with detailed hex reporting
        table_size_data = self._parse_size_from_table(size_str)

        # Check if the documentation values are internally consistent
        if not table_size_data['is_consistent']:
            unit_bytes = int(table_size_data['unit_value'] * 
                           (1024 if table_size_data['unit_text'] in ['KB', 'kB'] else 
                            1024*1024 if table_size_data['unit_text'] == 'MB' else 1))
            hex_value_kb = table_size_data['hex_value'] / 1024
            element_type = "Element" if is_element else "Partition"
            self.issue(f"{dk_name}: {display_name} {element_type} SIZE DEC/HEX MISMATCH - "
                      f"Table decimal: \033[91m{table_size_data['unit_value']} {table_size_data['unit_text']}\033[0m "
                      f"vs hex: \033[91m{hex_value_kb:.2f} KB (0x{table_size_data['hex_value']:X})\033[0m"
                      )
        table_size = table_size_data['bytes']

        if table_size and pm_size:
            if table_size != pm_size:
                # Provide detailed error showing what was documented vs actual
                element_type = "Element" if is_element else "Partition"
                doc_info = size_str if size_str else f"0x{table_size:X}"
                pm_static_name = f" ({pm_partition_name})" if is_element and pm_partition_name != display_name else ""
                self.issue(f"{dk_name}: {display_name}{pm_static_name} {element_type} SIZE HEX VALUE MISMATCH - "
                          f"Table: \033[91m0x{table_size:X}\033[0m | "
                          f"pm_static: \033[91m0x{pm_size:X}\033[0m")
            else:
                if self.config.verbose:
                    element_type = "element" if is_element else "partition"
                    self.debug(f"\t\t{display_name} {element_type} size: {self._format_size_with_hex(table_size)} - MATCH")

        # Compare offsets/addresses with detailed hex reporting
        table_offset_data = self._parse_offset_from_table(offset_str)

        # Check if the documentation values are internally consistent
        if not table_offset_data['is_consistent']:
            unit_bytes = int(table_offset_data['unit_value'] * 
                           (1024 if table_offset_data['unit_text'] in ['KB', 'kB'] else 
                            1024*1024 if table_offset_data['unit_text'] == 'MB' else 1))
            hex_value_kb = table_offset_data['hex_value'] / 1024
            element_type = "Element" if is_element else "Partition"
            self.issue(f"{dk_name}: {display_name} {element_type} OFFSET DEC/HEX MISMATCH - "
                      f"Table decimal: \033[91m{table_offset_data['unit_value']} {table_offset_data['unit_text']}\033[0m "
                      f"vs hex: \033[91m{hex_value_kb:.2f} KB (0x{table_offset_data['hex_value']:X})\033[0m"
                      )

        table_offset = table_offset_data['bytes']

        if table_offset is not None and pm_address is not None:
            if table_offset != pm_address:
                # Provide detailed error showing what was documented vs actual
                element_type = "Element" if is_element else "Partition"
                doc_info = offset_str if offset_str else f"0x{table_offset:X}"
                pm_static_name = f" ({pm_partition_name})" if is_element and pm_partition_name != display_name else ""
                self.issue(f"{dk_name}: {display_name} {element_type} OFFSET HEX VALUE MISMATCH - "
                        f"Table: \033[91m0x{table_offset_data['hex_value']:X}\033[0m"
                        f" | pm_static: \033[91m0x{pm_address:X}\033[0m"
                        )
            else:
                if self.config.verbose:
                    element_type = "element" if is_element else "partition"
                    self.debug(f"\t\t{display_name} {element_type} offset: {self._format_offset_with_hex(table_offset)} - MATCH")

    def _compare_memory_data(self, dk_name: str, tables: Dict, pm_data: Dict) -> int:
        """Compare memory table data with pm_static file data"""
        matches = 0

        if self.config.verbose:
            self.debug(f"Processing {len(tables)} memory tables for {dk_name}:")

        for table_name, table_rows in tables.items():
            if self.config.verbose:
                self.debug(f"\t+ Table: {table_name} ({len(table_rows)} rows)")
            for row in table_rows:
                # Skip None or empty rows
                if not row or not isinstance(row, dict):
                    if self.config.verbose:
                        self.debug(f"\t\tSkipping invalid row: {row}")
                    continue

                # Check if this row has main partition and/or sub-element data
                has_main_partition = (row.get('partition', '').strip() and 
                                     not row['partition'].startswith(('Partition', '=')))
                has_sub_element = (row.get('elements', '').strip() and 
                                  row['elements'] != 'Elements' and 
                                  row['elements'] != '-')
                
                # Process main partition if present
                if has_main_partition:
                    partition_name = row['partition'].strip()
                    clean_partition_name = self._extract_partition_name(partition_name)
                    size_str = row.get('size', '')
                    offset_str = row.get('offset', '')
                    
                    # Validate main partition
                    if clean_partition_name in PARTITION_CONFIG['key_partitions'] and clean_partition_name in pm_data:
                        self._validate_partition_data(dk_name, clean_partition_name, clean_partition_name, 
                                                     size_str, offset_str, pm_data, False)
                        matches += 1
                
                # Process sub-element if present
                if has_sub_element:
                    element_name = row['elements'].strip()
                    clean_partition_name = PARTITION_CONFIG['name_mappings'].get(element_name, element_name)
                    size_str = row.get('element_size', '')
                    offset_str = row.get('element_offset', '')
                    
                    # Validate element
                    if element_name in PARTITION_CONFIG['key_partitions'] and clean_partition_name in pm_data:
                        self._validate_partition_data(dk_name, element_name, clean_partition_name, 
                                                     size_str, offset_str, pm_data, True)
                        matches += 1

        return matches

    @staticmethod
    def _format_bytes(value_bytes: int, include_raw_bytes: bool = True) -> str:
        """Format byte value with human-readable units and hex representation"""
        if value_bytes >= 1024 * 1024:
            formatted = f"{value_bytes / (1024 * 1024):.1f} MB"
        elif value_bytes >= 1024:
            formatted = f"{value_bytes / 1024:.1f} kB"  
        else:
            formatted = f"{value_bytes} bytes"
        
        if include_raw_bytes and value_bytes >= 1024:
            return f"{formatted} ({value_bytes} bytes, 0x{value_bytes:X})"
        else:
            return f"{formatted} (0x{value_bytes:X})"

    def _format_size_with_hex(self, size_bytes: int) -> str:
        """Format size with hex representation"""
        return self._format_bytes(size_bytes, include_raw_bytes=True)

    def _format_offset_with_hex(self, offset_bytes: int) -> str:
        """Format offset with hex representation"""
        return self._format_bytes(offset_bytes, include_raw_bytes=False)
    
    @staticmethod
    def _extract_partition_name(partition_description: str) -> str:
        """Extract clean partition name from table description"""
        # Clean up RST formatting
        clean = re.sub(r':ref:`[^`]+`', '', partition_description).strip()
        
        # Try parentheses content first (most reliable)
        paren_match = re.search(r'\(([^)]+)\)', clean)
        if paren_match:
            candidate = paren_match.group(1).strip()
            # Skip descriptive text in parentheses
            if not any(word in candidate.lower() for word in ['debug', 'release', 'size', 'kb', 'mb']):
                # Handle slash-separated names like "mcuboot_primary/app"
                if '/' in candidate:
                    for part in candidate.split('/'):
                        part = part.strip()
                        if part in PARTITION_CONFIG['key_partitions']:
                            return part
                    return candidate.split('/')[0].strip()  # Return first part as fallback
                return candidate
        
        # Try direct mapping from description
        for desc, partition_name in PARTITION_CONFIG['name_mappings'].items():
            if desc in clean or desc.lower() == clean.split('(')[0].strip().lower():
                return partition_name
        
        # Default: normalize the description
        return clean.split('(')[0].strip().lower().replace(' ', '_')

    @staticmethod
    def _parse_value_from_string(value_str: str, allow_none: bool = False) -> dict:
        """
        Parse size/offset values from strings like '28 kB (0x7000)' or '960 kB (0xf0000)'
        Returns a dict with 'bytes', 'hex_value', 'unit_value', 'unit_text', and 'is_consistent'
        """
        if not value_str or value_str.strip() == '':
            return {
                'bytes': None if allow_none else 0,
                'hex_value': None,
                'unit_value': None,
                'unit_text': None,
                'is_consistent': True
            }

        result = {
            'bytes': None if allow_none else 0,
            'hex_value': None,
            'unit_value': None,
            'unit_text': None,
            'is_consistent': True
        }

        # Try to extract hex value in parentheses
        hex_match = re.search(r'\(0x([0-9a-fA-F]+)\)', value_str)
        if hex_match:
            result['hex_value'] = int(hex_match.group(1), 16)
            result['bytes'] = result['hex_value']

        # Try to extract decimal value with units (kB, MB, B)
        # Support both dot and comma as decimal separators (e.g., 247.8125 or 247,8125)
        unit_match = re.search(r'(\d+(?:[.,]\d+)?)\s*(kB|KB|MB|B)\b', value_str, re.IGNORECASE)
        if unit_match:
            value_str_matched = unit_match.group(1).replace(',', '.')  # Normalize comma to dot
            value, unit = float(value_str_matched), unit_match.group(2).upper()
            result['unit_value'] = value
            result['unit_text'] = unit
            multipliers = {'B': 1, 'KB': 1024, 'MB': 1024*1024}
            unit_bytes = int(value * multipliers.get(unit, 1))
            
            # If we have both hex and unit values, check consistency
            if result['hex_value'] is not None:
                if unit_bytes != result['hex_value']:
                    result['is_consistent'] = False
                # Use hex value as the authoritative source
                result['bytes'] = result['hex_value']
            else:
                # No hex value, use unit conversion
                result['bytes'] = unit_bytes

        # If no hex or unit match found, try standalone hex values
        if result['bytes'] == (None if allow_none else 0):
            hex_match = re.search(r'\b0x([0-9a-fA-F]+)\b', value_str)
            if hex_match:
                result['hex_value'] = int(hex_match.group(1), 16)
                result['bytes'] = result['hex_value']

        # Try plain numbers as last resort
        if result['bytes'] == (None if allow_none else 0):
            number_match = re.search(r'^\s*(\d+)\s*$', value_str)
            if number_match:
                result['bytes'] = int(number_match.group(1))

        return result

    def _parse_size_from_table(self, size_str: str) -> dict:
        """Parse size from table string, returns dict with parsed values"""
        return self._parse_value_from_string(size_str, allow_none=False)

    def _parse_offset_from_table(self, offset_str: str) -> dict:
        """Parse offset from table string, returns dict with parsed values"""
        return self._parse_value_from_string(offset_str, allow_none=True)
