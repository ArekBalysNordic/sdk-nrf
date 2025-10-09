from dataclasses import dataclass


LEVELS = {
    "issue": "issue",
    "warning": "warning",
    "info": "info",
    "debug": "debug"
}

@dataclass
class MatterSampleCheckerResult:
    level: LEVELS = LEVELS["info"]
    message: str = ""
