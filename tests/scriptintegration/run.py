"""Run certificate and MIME regressions with an isolated application directory."""
import os
import pathlib
import shutil
import subprocess
import sys
import tempfile

binary = pathlib.Path(sys.argv[1]).resolve()
openssl = sys.argv[2]
with tempfile.TemporaryDirectory(prefix="wing-integration-") as temporary:
    directory = pathlib.Path(temporary)
    executable = directory / binary.name
    shutil.copy2(binary, executable)
    certificates = directory / "certs"
    certificates.mkdir()
    key = directory / "key.pem"
    certificate = certificates / "test.crt"
    subprocess.run([openssl, "req", "-x509", "-newkey", "ed25519", "-nodes",
                    "-keyout", str(key), "-out", str(certificate), "-days", "1",
                    "-subj", "/CN=Signature Regression"], check=True, capture_output=True)
    certificate.chmod(0o444)
    payload = directory / "payload.bin"
    payload.write_bytes(b"signed\x00payload\xff")
    for name, data in [("payload", payload.read_bytes())]:
        source = directory / f"{name}.bin"
        source.write_bytes(data)
        subprocess.run([openssl, "pkeyutl", "-sign", "-rawin", "-inkey", str(key),
                        "-in", str(source), "-out", str(directory / f"{name}.sig")],
                       check=True, capture_output=True)
    environment = dict(os.environ, QT_QPA_PLATFORM="offscreen")
    result = subprocess.run([str(executable)], env=environment, check=False)
    certificate.chmod(0o644)
    raise SystemExit(result.returncode)
