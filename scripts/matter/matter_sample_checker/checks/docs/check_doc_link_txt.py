from pathlib import Path
from typing import Dict
import re

from internal.checker import MatterSampleTestCase
from internal.utils.utils import get_latest_matter_sha, get_matter_sha_link

"""
    DOCUMENTATION LINKS VERSION VALIDATION CHECK:
    
    Ensures that all Matter SDK GitHub links in the documentation links.txt file
    reference the correct and current Matter SDK commit SHA. This check prevents
    outdated documentation links that point to old versions of the Matter SDK
    repository, maintaining consistency between the integrated SDK version and
    documentation references.

    CONFIGURATION PARAMETERS:
    -------------------------
    • documentation.link_txt.path: Path to the links.txt file relative to the nRF
                                   repository root. This file contains reusable link
                                   definitions used throughout the documentation.

    VALIDATION STEPS:
    -----------------
    1. File Reading:
       - Locates the links.txt file using the configured path
       - Reads the entire content with UTF-8 encoding
       - Stores content for subsequent pattern matching and analysis

    2. Latest Matter SHA Detection:
       - Retrieves the current Matter SDK commit SHA from the repository
       - Uses the latest merge SHA as the synchronization reference point
       - Generates a GitHub link for the detected SHA for verification
       - Reports the detected SHA with commit details for debugging

    3. GitHub Link Extraction:
       - Scans the file content for GitHub URLs to sdk-connectedhomeip
       - Matches both 'tree' and 'blob' URL patterns with SHA references
       - Uses regex pattern to extract commit SHAs from URLs
       - Creates a list of all unique SHAs found in the documentation
       - Counts usage frequency for each SHA found

    4. Version Comparison:
       - Compares each found SHA against the current repository SHA
       - Identifies matches that align with the latest merge commit
       - Reports correct SHAs with usage count for verification
       - Collects outdated SHAs that don't match the current version
       - Deduplicates SHAs to avoid redundant reporting

    5. Issue Reporting:
       - Reports each line containing outdated SHA references
       - Displays line number, outdated SHA, and expected SHA
       - Shows the full line content for context (trimmed for readability)
       - Reports successful validation when all links are current
       - Handles errors gracefully with detailed error messages

    NOTES:
    ------
    • The check validates links to both tree (directory) and blob (file) views
    • Only links to nrfconnect/sdk-connectedhomeip repository are checked
    • Multiple references to the same SHA are counted and reported
    • The latest SHA is determined from the Matter submodule state
    • Line numbers are 1-indexed for easy location in the file
"""


class CheckDocLinkTxtTestCase(MatterSampleTestCase):
    def __init__(self):
        super().__init__()
        self.file_path = None
        self.content = ""

    def name(self) -> str:
        return "Documentation links.txt check"

    def prepare(self):
        self.file_path = self.config.config_file.get(
            "documentation").get("link_txt").get("path")
        with open(self.config.nrf_path / self.file_path, "r") as f:
            self.content = f.read()

    def check(self):
        try:
            current_sha = get_latest_matter_sha(self.config.nrf_path)
            self.debug(
                f"Latest Matter merge SHA (sync reference): {current_sha}")

            # Get SHA link for display and return data
            sha_link = get_matter_sha_link(self.config.nrf_path, current_sha)
            if sha_link and sha_link != "Unknown":
                self.debug(f"{sha_link}")

            # Find SHA links
            sha_pattern = r'https://github\.com/nrfconnect/sdk-connectedhomeip/(?:tree|blob)/([a-f0-9]+)/'
            matches = re.findall(sha_pattern, self.content)

            outdated_shas = []
            for sha in set(matches):
                count = matches.count(sha)
                if sha == current_sha:
                    self.debug(
                        f"Matches latest merge SHA: {sha} (used {count} times)")
                else:
                    outdated_shas.append(sha)

            # Show problematic lines
            if outdated_shas:
                for i, line in enumerate(self.content.splitlines(), 1):
                    if 'sdk-connectedhomeip' in line and any(sha in line for sha in outdated_shas):
                        self.issue(f"Line {i} with SHA {sha} that don't match latest ({current_sha}): {line.strip()}")

        except Exception as e:
            self.error(f"Error checking links: {e}")
