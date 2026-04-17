from __future__ import annotations

from dataclasses import dataclass
from typing import Iterable

import serial
from serial.tools import list_ports


BAUD_RATE = 115200
SERIAL_TIMEOUT_SECONDS = 2.0


@dataclass(frozen=True)
class PortChoice:
    device: str
    description: str
    manufacturer: str
    hwid: str

    @property
    def display_name(self) -> str:
        parts = [self.device]
        if self.description:
            parts.append(self.description)
        if self.manufacturer:
            parts.append(self.manufacturer)
        return " - ".join(parts)


@dataclass(frozen=True)
class DeviceStatus:
    state: str
    phase: str | None = None
    outcome: str | None = None
    run_sequence: int | None = None
    row_count: int | None = None


@dataclass(frozen=True)
class ExportRow:
    index: int
    pass_fail: str
    test_name: str
    expected_result: str
    actual_result: str


@dataclass(frozen=True)
class ExportRun:
    run_sequence: int
    outcome: str
    rows: list[ExportRow]


class ExportClientError(Exception):
    pass


class NoPortsAvailableError(ExportClientError):
    pass


class PortSelectionRequiredError(ExportClientError):
    def __init__(self, ports: list[PortChoice], reason: str) -> None:
        super().__init__(reason)
        self.ports = ports
        self.reason = reason


class DeviceEmptyError(ExportClientError):
    pass


class DeviceRunningError(ExportClientError):
    pass


class DeviceAlreadyExportedError(ExportClientError):
    pass


class DeviceProtocolError(ExportClientError):
    pass


def list_available_ports() -> list[PortChoice]:
    ports: list[PortChoice] = []
    for port in list_ports.comports():
        ports.append(
            PortChoice(
                device=port.device,
                description=port.description or "",
                manufacturer=port.manufacturer or "",
                hwid=port.hwid or "",
            )
        )
    return ports


def detect_preferred_port() -> PortChoice:
    ports = list_available_ports()
    if not ports:
        raise NoPortsAvailableError("No serial ports were found on this PC.")

    stlink_ports = [port for port in ports if _looks_like_stlink(port)]
    if len(stlink_ports) == 1:
        return stlink_ports[0]

    if len(stlink_ports) > 1:
        raise PortSelectionRequiredError(
            stlink_ports,
            "Multiple ST-LINK virtual COM ports were detected. Choose the correct one.",
        )

    raise PortSelectionRequiredError(
        ports,
        "No ST-LINK virtual COM port was detected automatically. Choose the NUCLEO port manually.",
    )


class AutoTestAlreadyRunningError(ExportClientError):
    pass


class AutoTestStartError(ExportClientError):
    pass


class ExportProtocolClient:
    def get_status(self, port_device: str) -> DeviceStatus:
        with self._open_port(port_device) as port:
            return self._query_status(port)

    def start_auto_test(self, port_device: str) -> str:
        """Send START_AUTO and return the reply string."""
        with self._open_port(port_device) as port:
            self._send_line(port, "START_AUTO")
            line = self._read_line(port)
            if line == "AUTO_STARTED":
                return line
            if line.startswith("AUTO_ERROR|"):
                error_code = line.removeprefix("AUTO_ERROR|")
                if error_code == "ALREADY_RUNNING":
                    raise AutoTestAlreadyRunningError("An automatic test is already running on the fixture.")
                raise AutoTestStartError(f"Fixture rejected START_AUTO: {error_code}")
            raise DeviceProtocolError(f"Unexpected START_AUTO reply: {line}")

    def export_latest_run(self, port_device: str) -> ExportRun:
        with self._open_port(port_device) as port:
            status = self._query_status(port)

            if status.state == "EMPTY":
                raise DeviceEmptyError("The NUCLEO does not have a completed test run to export.")
            if status.state in {"RUNNING", "WAITING_VISUAL"}:
                raise DeviceRunningError("A test is still running on the NUCLEO. Export is only available after completion.")
            if status.state not in ("READY", "EXPORTED") or status.run_sequence is None:
                raise DeviceProtocolError(f"Unexpected export status: {status.state}")

            self._send_line(port, f"EXPORT_RUN|{status.run_sequence}")
            return self._read_export_run(port, status.run_sequence)

    def _open_port(self, port_device: str) -> serial.Serial:
        port = serial.Serial(
            port=port_device,
            baudrate=BAUD_RATE,
            bytesize=serial.EIGHTBITS,
            parity=serial.PARITY_NONE,
            stopbits=serial.STOPBITS_ONE,
            timeout=SERIAL_TIMEOUT_SECONDS,
            write_timeout=SERIAL_TIMEOUT_SECONDS,
        )
        port.reset_input_buffer()
        port.reset_output_buffer()
        return port

    def _query_status(self, port: serial.Serial) -> DeviceStatus:
        self._send_line(port, "EXPORT_STATUS")
        line = self._read_line(port)
        parts = line.split("|")

        if parts == ["STATUS", "EMPTY"]:
            return DeviceStatus(state="EMPTY")
        if parts == ["STATUS", "RUNNING"]:
            return DeviceStatus(state="RUNNING")
        if len(parts) == 3 and parts[0] == "STATUS" and parts[1] in {"RUNNING", "WAITING_VISUAL"}:
            return DeviceStatus(state=parts[1], phase=parts[2])
        if parts == ["STATUS", "WAITING_VISUAL"]:
            return DeviceStatus(state="WAITING_VISUAL")
        if len(parts) == 5 and parts[0] == "STATUS" and parts[1] in {"READY", "EXPORTED"} and parts[2] in {"PASS", "FAIL"}:
            return DeviceStatus(
                state=parts[1],
                outcome=parts[2],
                run_sequence=self._parse_int(parts[3], "run sequence"),
                row_count=self._parse_int(parts[4], "row count"),
            )

        raise DeviceProtocolError(f"Unexpected status reply: {line}")

    def _read_export_run(self, port: serial.Serial, expected_run_sequence: int) -> ExportRun:
        begin_line = self._read_line(port)
        if begin_line.startswith("EXPORT_ERROR|"):
            self._raise_export_error(begin_line)

        begin_parts = begin_line.split("|")
        if len(begin_parts) != 4 or begin_parts[0] != "EXPORT_BEGIN" or begin_parts[2] not in {"PASS", "FAIL"}:
            raise DeviceProtocolError(f"Unexpected export begin reply: {begin_line}")

        run_sequence = self._parse_int(begin_parts[1], "run sequence")
        row_count = self._parse_int(begin_parts[3], "row count")
        if run_sequence != expected_run_sequence:
            raise DeviceProtocolError(
                f"Expected export sequence {expected_run_sequence}, but device replied with {run_sequence}."
            )

        rows: list[ExportRow] = []
        for expected_index in range(row_count):
            row_line = self._read_line(port)
            if row_line.startswith("EXPORT_ERROR|"):
                self._raise_export_error(row_line)
            row = self._parse_row(row_line, expected_index)
            rows.append(row)

        end_line = self._read_line(port)
        end_parts = end_line.split("|")
        if len(end_parts) != 2 or end_parts[0] != "EXPORT_END":
            raise DeviceProtocolError(f"Unexpected export end reply: {end_line}")

        end_sequence = self._parse_int(end_parts[1], "run sequence")
        if end_sequence != expected_run_sequence:
            raise DeviceProtocolError(
                f"Expected export end sequence {expected_run_sequence}, but device replied with {end_sequence}."
            )

        return ExportRun(run_sequence=run_sequence, outcome=begin_parts[2], rows=rows)

    def _parse_row(self, row_line: str, expected_index: int) -> ExportRow:
        parts = row_line.split("|", 5)
        if len(parts) != 6 or parts[0] != "ROW" or parts[2] not in {"PASS", "FAIL"}:
            raise DeviceProtocolError(f"Unexpected row reply: {row_line}")

        row_index = self._parse_int(parts[1], "row index")
        if row_index != expected_index:
            raise DeviceProtocolError(
                f"Expected row index {expected_index}, but device replied with {row_index}."
            )

        return ExportRow(
            index=row_index,
            pass_fail=parts[2],
            test_name=parts[3],
            expected_result=parts[4],
            actual_result=parts[5],
        )

    def _raise_export_error(self, error_line: str) -> None:
        error_code = error_line.removeprefix("EXPORT_ERROR|")
        if error_code == "NOT_READY":
            raise DeviceRunningError("The NUCLEO export service reported that the test is not ready yet.")
        if error_code == "ALREADY_EXPORTED":
            raise DeviceAlreadyExportedError("This test run has already been exported.")
        if error_code == "BAD_SEQ":
            raise DeviceProtocolError("The NUCLEO rejected the requested run sequence.")
        raise DeviceProtocolError(f"Unexpected export error reply: {error_line}")

    def _send_line(self, port: serial.Serial, line: str) -> None:
        payload = f"{line}\n".encode("ascii")
        written = port.write(payload)
        port.flush()
        if written != len(payload):
            raise DeviceProtocolError(f"Failed to send the full command: {line}")

    def _read_line(self, port: serial.Serial) -> str:
        raw = port.readline()
        if not raw:
            raise DeviceProtocolError("Timed out waiting for the NUCLEO export response.")

        try:
            line = raw.decode("ascii").strip()
        except UnicodeDecodeError as exc:
            raise DeviceProtocolError("Received a non-ASCII export response from the NUCLEO.") from exc

        if not line:
            raise DeviceProtocolError("Received an empty export response from the NUCLEO.")

        return line

    def _parse_int(self, raw_value: str, label: str) -> int:
        try:
            return int(raw_value, 10)
        except ValueError as exc:
            raise DeviceProtocolError(f"Invalid {label} in export response: {raw_value}") from exc


def format_port_choices(ports: Iterable[PortChoice]) -> list[str]:
    return [port.display_name for port in ports]


def _looks_like_stlink(port: PortChoice) -> bool:
    joined = " ".join([port.device, port.description, port.manufacturer, port.hwid]).lower()
    markers = (
        "st-link",
        "stlink",
        "stmicroelectronics",
        "stm32 stlink",
        "virtual com port",
    )
    return any(marker in joined for marker in markers)


def looks_like_stlink_port(port: PortChoice) -> bool:
    """Return True if the port matches ST-LINK / NUCLEO VCP heuristics (same rules as auto-detect)."""
    return _looks_like_stlink(port)
