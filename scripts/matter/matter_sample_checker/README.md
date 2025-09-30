# NCS Matter Sample Checker

A comprehensive consistency checker for Matter samples in the nRF Connect SDK. This tool automatically validates file structure, configuration consistency, license years, PM static files, ZAP files, and detects common copy-paste mistakes in Matter samples.

## Overview

The NCS Matter Sample Checker is designed to help developers maintain consistency and quality across Matter samples in the nRF Connect SDK. It performs automated validation against a configurable set of rules and best practices, ensuring that samples follow the established patterns and avoid common mistakes.

## Features

### 🔌 Dynamic Check Discovery
- **Automatically discovers and runs all checks** from the `checks` directory
- No manual registration or imports needed when adding new checks
- Simply create a new check file inheriting from `MatterSampleTestCase` and it will be automatically included
- Smart parameter detection and instantiation for different check types

### 📁 File Structure Validation
- Verifies presence of required files (`CMakeLists.txt`, `prj.conf`, `sample.yaml`, etc.)
- Checks for required directories (`src`, `sysbuild`, `sysbuild/mcuboot`)
- Validates optional files and directories

### ⚡ ZAP Files Validation
- Ensures ZAP configuration files (`.zap`, `.matter`) are present
- Verifies ZAP-generated files exist and are up-to-date
- Checks for proper ZAP directory structure

### 📝 License Year Checking
- Validates copyright years in source files
- Supports multiple valid years configuration
- Checks C/C++/H files and configuration files
- Skipped by default; enabled with `--year` flag

### ⚙️ Configuration Consistency
- Validates required Matter configurations in `prj.conf`
- Checks `sample.yaml` format and required tags
- Ensures proper sysbuild configuration
- Verifies Matter-specific settings

### 💾 PM Static Files Validation
- Validates partition manager static configuration files
- Checks board-specific requirements (WiFi, nRF5340, non-secure)
- Ensures proper partition definitions
- Validates internal vs external flash variants

### 🔍 Copy-Paste Error Detection
- Dynamically discovers all Matter samples
- Detects references to other sample names in current sample
- Intelligent filtering to avoid false positives
- Checks documentation, configuration, and source files
- Supports allowlists for legitimate cross-sample references

### 📊 Template Comparison
- Compares configuration files with the Matter template sample
- Highlights deviations from the standard template
- Excludes copyright differences from comparison
- Provides detailed diff information in verbose mode

## Installation

No installation required. The tool is a standalone Python script that requires Python 3.6+ and the following packages:
- `pyyaml` (for YAML configuration parsing)

```bash
pip install pyyaml
```

## Usage

### Basic Usage

```bash
# Check current directory
python matter_sample_checker.py

# Check specific sample
python matter_sample_checker.py /path/to/ncs/nrf/samples/matter/light_bulb

# Check with verbose output
python matter_sample_checker.py -v /path/to/sample

# Enable license year checking for current year
python matter_sample_checker.py --year /path/to/sample
```

### Advanced Usage

```bash
# Check for specific copyright year(s)
python matter_sample_checker.py --year 2024 /path/to/sample
python matter_sample_checker.py -y 2023 2024 2025 /path/to/sample

# Use custom configuration file
python matter_sample_checker.py --config custom_config.yaml /path/to/sample

# Save report to file
python matter_sample_checker.py -o report.txt /path/to/sample

# Allow specific names in copy-paste checking (useful for legitimate cross-references)
python matter_sample_checker.py --allow-names "light bulb" template /path/to/sample
python matter_sample_checker.py -a "light bulb" "door lock" /path/to/sample

# Combined options
python matter_sample_checker.py -v -y 2024 -a "light_bulb" -o report.txt /path/to/sample
```

### Command Line Options

| Option | Description |
|--------|-------------|
| `sample_path` | Path to Matter sample directory (default: current directory) |
| `--verbose, -v` | Show verbose output during checks |
| `--year, -y` | Expected copyright year(s) (enables year checking; omit to skip year checking) |
| `--config, -c` | Path to custom configuration file |
| `--output, -o` | Output report to file instead of stdout |
| `--allow-names, -a` | List of names/terms to allow during copy-paste error checking (case-insensitive) |

## Configuration

The tool uses a YAML configuration file (`matter_sample_checker_config.yaml`) that defines:

- **File Structure**: Required and optional files/directories
- **ZAP Configuration**: Expected ZAP files and generated content
- **License Settings**: Source file extensions and copyright patterns
- **PM Static Rules**: Partition manager validation rules
- **Copy-Paste Detection**: Files to check and patterns to skip
- **Validation Rules**: Required configurations and consistency checks
- **Exclusions**: Directories and files to exclude from checks

### Sample Configuration Structure

```yaml
file_structure:
  required_files:
    - "CMakeLists.txt"
    - "prj.conf"
    - "sample.yaml"
    # ... more files

pm_static:
  base_required_entries:
    - "mcuboot"
    - "app"
    # ... more entries

validation:
  required_prj_configs:
    - "CONFIG_CHIP_PROJECT_CONFIG"
    - "CONFIG_CHIP_FACTORY_DATA"
    # ... more configs
```

## Output Format

The tool generates a comprehensive report with three types of findings:

### ❌ Issues (Return Code > 0)
Critical problems that must be fixed:
- Missing required files
- Configuration errors
- Copy-paste mistakes
- Invalid PM static configurations

### ⚠️ Warnings
Potential issues that should be reviewed:
- Missing optional files
- Template deviations
- Suspicious patterns

### ℹ️ Information
Successful validations and detailed check results:
- Found required files
- Successful configuration checks
- Template comparison results

## Integration with CI/CD

The tool returns appropriate exit codes for CI/CD integration:
- `0`: No issues found
- `>0`: Number of issues found (fails CI/CD pipeline)

Example CI usage:
```bash
python matter_sample_checker.py /path/to/sample || exit 1
```

## Examples

### Checking Light Bulb Sample
```bash
python matter_sample_checker.py /home/arbl/ncs/nrf/samples/matter/light_bulb
```

### Checking with Multiple Years and Verbose Output
```bash
python matter_sample_checker.py -v -y 2023 2024 2025 ../template
```

### Custom Configuration for Specific Project
```bash
python matter_sample_checker.py -c project_specific_config.yaml -o validation_report.txt .
```

## Copy-Paste Error Checking

The copy-paste error detection feature automatically scans for references to other Matter sample names within the current sample. This helps catch common mistakes where developers copy content from one sample to another and forget to update sample-specific references.

### Understanding Copy-Paste Detection

By default, the tool:
- Discovers all Matter samples in the SDK
- Creates a list of "forbidden words" from other sample names
- Scans files for these references and reports them as potential copy-paste errors

### When to Use `--allow-names`

Use the `--allow-names` option when you have legitimate cross-sample references that should not be flagged as errors:

- **Documentation references**: When your README mentions other samples for comparison
- **Technical discussions**: When configuration comments reference other sample patterns
- **Legitimate cross-references**: When samples intentionally reference related samples

### Examples of Legitimate Use Cases

```bash
# Light switch sample legitimately references "light bulb" in documentation
python matter_sample_checker.py --allow-names "light bulb" /path/to/light_switch

# Template sample references multiple other samples in documentation
python matter_sample_checker.py -a template "door lock" "window covering" /path/to/sample

# Case-insensitive matching works for variations
python matter_sample_checker.py -a "Light_Bulb" "light bulb" /path/to/sample
```

## Troubleshooting

### Common Issues

1. **"Configuration file not found"**
   - Ensure `matter_sample_checker_config.yaml` is in the same directory as the script
   - Use `-c` option to specify custom configuration path

2. **"Could not find Matter samples directory"**
   - Run from within the Matter samples directory structure
   - The tool automatically discovers the samples directory by walking up the path

3. **"No pm_static_*.yml files found"**
   - Ensure PM static files follow the naming convention `pm_static_*.yml`
   - Check that the sample has proper board configurations

4. **Excessive copy-paste warnings for legitimate references**
   - Use `--allow-names` to exclude legitimate cross-sample references
   - Common legitimate cases: documentation comparisons, technical discussions
   - Example: `python matter_sample_checker.py -a "light bulb" "template" /path/to/sample`

### Debug Tips

- Use `--verbose` flag to see detailed check progress
- Check the configuration file for patterns and rules
- Review exclusion rules if checks are too strict
- Omit `--year` flag if copyright validation is problematic (year checking is skipped by default)

## Contributing

To extend the checker functionality:

### Adding New Checks

The checker now uses an **automatic check discovery system**. Adding a new check is simple:

1. **Create a new check file** in the `checks` directory (e.g., `check_my_feature.py`)
2. **Inherit from `MatterSampleTestCase`** base class
3. **Implement required methods**:
   - `name()`: Return the display name of your check
   - `prepare()`: Set up any data or variables needed for the check
   - `check()`: Implement the actual check logic

4. **That's it!** The checker will automatically discover and run your new check

#### Example Check Implementation

```python
from internal.checker import MatterSampleTestCase
from internal.utils.defines import MatterSampleCheckerResult
from typing import List

class MyFeatureTestCase(MatterSampleTestCase):
    def name(self) -> str:
        return "My Feature Check"
    
    def prepare(self):
        # Initialize variables needed for the check
        self.files_to_check = []
    
    def check(self) -> List[MatterSampleCheckerResult]:
        # Implement your check logic here
        if some_condition:
            self.issue("Found an issue")
        else:
            self.debug("Check passed")
```

#### Registering results in your check (info, debug, warning, issue)

Every check inherits helper methods from `MatterSampleTestCase` to register findings:

- **info(message: str)**: General information and successful validation notes
- **debug(message: str)**: Detailed progress messages (shown only with `--verbose`)
- **warning(message: str)**: Non-fatal problems that should be reviewed
- **issue(message: str)**: Definite errors; these increase the exit code and fail CI

Behind the scenes, these methods append a `MatterSampleCheckerResult` with a level from `LEVELS` to the check’s internal result list. The framework aggregates them and builds the final report:

- **Information & Debug**: Shown under the INFORMATION section (debug only if `--verbose`)
- **Warnings**: Counted and listed under WARNINGS
- **Issues**: Counted and listed under ISSUES and affect the final result/exit code

Minimal usage example in `check()`:

```python
def check(self) -> List[MatterSampleCheckerResult]:
    self.debug("Starting configuration validation")

    if not self._has_required_file:
        self.issue("Missing required file: prj.conf")
        return

    if self._has_suspicious_pattern:
        self.warning("Suspicious pattern detected in README.rst")

    self.info("All required files are present")
```

Notes and best practices:

- Prefer clear, actionable messages (include which file/config/line is affected)
- Use `debug()` for verbose tracing (counts, parsed items, paths) to aid debugging
- Throwing an exception inside `check()` is caught by the framework and recorded as an `issue()` with the exception message
- You do not need to print anything yourself; just use the helper methods

#### Custom Constructor Parameters

If your check needs additional parameters, you can define a custom `__init__`:

```python
def __init__(self, config: dict, sample_path: Path, skip: bool = False, custom_param: str = None):
    super().__init__(config, sample_path, skip)
    self.custom_param = custom_param
```

The check discovery system will automatically detect the parameters and pass them if available in the global `check_parameters` dictionary.

### Modifying Configuration

Update the `matter_sample_checker_config.yaml` file if your new check requires configuration settings.

### How Dynamic Discovery Works

1. The `CheckDiscovery` class scans the `checks/` directory for all `.py` files
2. It dynamically imports each module and finds classes inheriting from `MatterSampleTestCase`
3. Each check is instantiated with appropriate parameters and run automatically
4. No manual registration or imports needed in the main script! 

## Related Tools

- **ZAP (ZCL Application Toolkit)**: For generating Matter cluster code
- **Partition Manager**: For nRF Connect SDK memory layout management
- **West**: Build system and package manager for Zephyr/nRF Connect SDK

## License

This tool is part of the nRF Connect SDK utilities and follows the same licensing terms.
