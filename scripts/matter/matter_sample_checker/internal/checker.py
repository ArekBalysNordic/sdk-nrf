#
# Copyright (c) 2025 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause

from pathlib import Path
from typing import List
from datetime import datetime
from typing import Type, List
from internal.utils.defines import LEVELS, MatterSampleCheckerResult
from internal.check import MatterSampleTestCase, MatterCheckerConfig
from internal.check_discovery import CheckDiscovery


class MatterSampleChecker:
    def __init__(self, config_file: dict, nrf_path: Path, sample_path: Path = None, verbose: bool = False, allowed_names: List[str] = None, expected_years: List[int] = None, check_classes: List[Type[MatterSampleTestCase]] = None):
        self.result: List[MatterSampleCheckerResult] = []
        self.config = MatterCheckerConfig(
            config_file=config_file,
            sample_path=sample_path,
            nrf_path=nrf_path,
            verbose=verbose,
            skip=False,
            expected_years=expected_years,
            allowed_names=allowed_names
        )

        self.check_classes = check_classes

    def run_test_case(self, test_case: MatterSampleTestCase):
        self.result.extend(test_case.run(self.config))

    def generate_report(self) -> str:
        """Generate the final report."""
        report = []

        if self.config.sample_path:

            report.append("=" * 60)
            report.append(f"NCS Matter Sample Consistency Check Report")
            report.append(f"Sample: {self.config.sample_path.name}")
            report.append(f"Path: {self.config.sample_path}")
            report.append(
                f"Check time: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
            report.append("=" * 60)
        else:
            report.append("=" * 60)
            report.append(f"NCS Matter Documentation Check Report")
            report.append(
                f"Check time: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
            report.append("=" * 60)

        report.append(f"\nℹ️  INFORMATION:")
        for info_item in self.result:
            if info_item.level == LEVELS["info"]:
                report.append(f"  {info_item.message}")
            if info_item.level == LEVELS["debug"] and self.config.verbose:
                report.append(f"  {info_item.message}")

        warnings_count = len(
            [result for result in self.result if result.level == LEVELS["warning"]])
        if warnings_count > 0:
            report.append(f"\n⚠️  WARNINGS ({warnings_count}):")
            for i, warning in enumerate([result for result in self.result if result.level == LEVELS["warning"]], 1):
                report.append(f"  {i}. {warning.message}")

        issues_count = len(
            [result for result in self.result if result.level == LEVELS["issue"]])
        if issues_count > 0:
            report.append(f"\n❌ ISSUES FOUND ({issues_count}):")
            for i, issue in enumerate([result for result in self.result if result.level == LEVELS["issue"]], 1):
                report.append(f"  {i}. {issue.message}")

        report.append("\n" + "=" * 60)
        if issues_count == 0:
            report.append("✅ RESULT: Sample appears to be consistent!")
        else:
            report.append("❌ RESULT: Issues found that need to be addressed.")
        report.append("=" * 60)

        return "\n".join(report)

    def run_checks(self):

        # Run all checks
        for check_class in self.check_classes:
            try:
                # Prepare check-specific parameters if needed
                check_parameters = {'allowed_names': self.config.allowed_names}

                # Only pass year-related parameters to LicenseYearTestCase
                if check_class.__name__ == 'LicenseYearTestCase':
                    check_parameters['expected_years'] = self.config.expected_years

                check_instance = CheckDiscovery(
                    self.config.nrf_path).instantiate_check(
                        check_class)
                self.run_test_case(check_instance)
            except Exception as e:
                print(f"Error running check {check_class.__name__}: {e}")
                if self.config.verbose:
                    import traceback
                    traceback.print_exc()

        # Generate report
        report = self.generate_report()

        # Count issues
        issue_count = len(
            [result for result in self.result if result.level == LEVELS["issue"]])

        return report, issue_count
