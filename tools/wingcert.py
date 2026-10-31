#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from __future__ import annotations

import argparse
import hashlib
import mmap
import os
import shutil
import sys
from datetime import datetime, timedelta, timezone
from pathlib import Path

from cryptography import x509
from cryptography.exceptions import InvalidSignature
from cryptography.hazmat.primitives import hashes, serialization
from cryptography.hazmat.primitives.asymmetric import ed25519
from cryptography.x509.oid import NameOID

MAGIC = b"WINGSIG1"          # 8 bytes
SIG_SIZE = 64                # Ed25519 fixed signature size
TRAILER_SIZE = len(MAGIC) + SIG_SIZE
CHUNK_SIZE = 1024 * 1024     # 1 MiB


# ============================================================
# Helpers
# ============================================================

def is_merged_signed(path: Path) -> bool:
    """Check whether file ends with our trailer."""
    try:
        size = path.stat().st_size
    except FileNotFoundError:
        return False

    if size < TRAILER_SIZE:
        return False

    with path.open("rb") as f:
        f.seek(size - TRAILER_SIZE)
        magic = f.read(len(MAGIC))
        sig = f.read(SIG_SIZE)

    return magic == MAGIC and len(sig) == SIG_SIZE


def read_trailer(path: Path) -> tuple[bool, bytes]:
    """Return (has_trailer, signature_bytes)."""
    try:
        size = path.stat().st_size
    except FileNotFoundError:
        return False, b""

    if size < TRAILER_SIZE:
        return False, b""

    with path.open("rb") as f:
        f.seek(size - TRAILER_SIZE)
        magic = f.read(len(MAGIC))
        sig = f.read(SIG_SIZE)

    if magic != MAGIC or len(sig) != SIG_SIZE:
        return False, b""

    return True, sig


def load_private_key(path: Path) -> ed25519.Ed25519PrivateKey:
    key = serialization.load_pem_private_key(path.read_bytes(), password=None)
    if not isinstance(key, ed25519.Ed25519PrivateKey):
        raise RuntimeError("private key is not Ed25519")
    return key


def load_certificate(path: Path) -> x509.Certificate:
    return x509.load_pem_x509_certificate(path.read_bytes())


def cert_fingerprint(cert: x509.Certificate) -> str:
    return cert.fingerprint(hashes.SHA256()).hex().upper()


def derive_signed_path(path: Path) -> Path:
    return path.with_name(f"{path.stem}_signed{path.suffix}")


def sha256_bytes(data: bytes | memoryview) -> str:
    return hashlib.sha256(data).hexdigest()


def file_mmap(path: Path):
    """Open file as mmap, caller must use in with-statement."""
    fp = path.open("rb")
    try:
        mm = mmap.mmap(fp.fileno(), 0, access=mmap.ACCESS_READ)
    except Exception:
        fp.close()
        raise
    return fp, mm


# ============================================================
# Key generation
# ============================================================

def cmd_gen_keypair(args: argparse.Namespace) -> int:
    priv = Path(args.private_key)
    crt = Path(args.certificate)

    if priv.exists() and not args.force:
        print(f"error: private key exists: {priv}", file=sys.stderr)
        return 1
    if crt.exists() and not args.force:
        print(f"error: certificate exists: {crt}", file=sys.stderr)
        return 1

    key = ed25519.Ed25519PrivateKey.generate()

    subject = issuer = x509.Name([
        x509.NameAttribute(NameOID.COUNTRY_NAME, args.country),
        x509.NameAttribute(NameOID.ORGANIZATION_NAME, args.organization),
        x509.NameAttribute(NameOID.COMMON_NAME, args.common_name),
    ])

    now = datetime.now(timezone.utc)
    cert = (
        x509.CertificateBuilder()
        .subject_name(subject)
        .issuer_name(issuer)
        .public_key(key.public_key())
        .serial_number(x509.random_serial_number())
        .not_valid_before(now - timedelta(days=1))
        .not_valid_after(now + timedelta(days=args.valid_days))
        .sign(private_key=key, algorithm=None)   # required for Ed25519
    )

    priv.write_bytes(
        key.private_bytes(
            encoding=serialization.Encoding.PEM,
            format=serialization.PrivateFormat.PKCS8,
            encryption_algorithm=serialization.NoEncryption(),
        )
    )
    crt.write_bytes(cert.public_bytes(serialization.Encoding.PEM))

    print(f"generated private key: {priv}")
    print(f"generated certificate: {crt}")
    print(f"certificate sha256 fingerprint: {cert_fingerprint(cert)}")
    return 0


# ============================================================
# Detached signature
# ============================================================

def sign_detached(args: argparse.Namespace) -> int:
    input_path = Path(args.input)
    priv_path = Path(args.private_key)
    out_path = Path(args.output)

    if not input_path.is_file():
        print(f"error: input not found: {input_path}", file=sys.stderr)
        return 1
    if not priv_path.is_file():
        print(f"error: private key not found: {priv_path}", file=sys.stderr)
        return 1
    if out_path.exists():
        print(f"error: output exists: {out_path}", file=sys.stderr)
        return 1
    if is_merged_signed(input_path):
        print("error: input file already appears to be merged-signed", file=sys.stderr)
        return 1

    key = load_private_key(priv_path)

    # Sign raw file bytes
    fp, mm = file_mmap(input_path)
    data = None
    try:
        data = memoryview(mm)
        signature = key.sign(data)
        checksum = sha256_bytes(data)
    finally:
        if data is not None:
            del data
        mm.close()
        fp.close()

    if len(signature) != SIG_SIZE:
        print("error: unexpected signature size", file=sys.stderr)
        return 1

    out_path.write_bytes(signature)

    print(f"signed: {out_path}")
    print(f"sha256: {checksum}")
    return 0


def verify_detached(args: argparse.Namespace) -> int:
    input_path = Path(args.input)
    crt_path = Path(args.certificate)
    sig_path = Path(args.signature)

    if not input_path.is_file():
        print(f"error: input not found: {input_path}", file=sys.stderr)
        return 1
    if not crt_path.is_file():
        print(f"error: certificate not found: {crt_path}", file=sys.stderr)
        return 1
    if not sig_path.is_file():
        print(f"error: signature not found: {sig_path}", file=sys.stderr)
        return 1

    signature = sig_path.read_bytes()
    if len(signature) != SIG_SIZE:
        print(f"error: signature size must be {SIG_SIZE} bytes", file=sys.stderr)
        return 1

    # If file is already merged-signed, detached verify is almost certainly wrong input.
    if is_merged_signed(input_path):
        print("error: input appears merged-signed; verify without --signature", file=sys.stderr)
        return 1

    cert = load_certificate(crt_path)
    pub = cert.public_key()

    fp, mm = file_mmap(input_path)
    data = None
    try:
        data = memoryview(mm)
        pub.verify(signature, data)
        print("verify: OK")
        return 0
    except InvalidSignature:
        print("verify: FAILED", file=sys.stderr)
        return 2
    finally:
        if data is not None:
            del data
        mm.close()
        fp.close()


# ============================================================
# Merged signature
# ============================================================

def sign_merged(args: argparse.Namespace) -> int:
    input_path = Path(args.input)
    priv_path = Path(args.private_key)

    if not input_path.is_file():
        print(f"error: input not found: {input_path}", file=sys.stderr)
        return 1
    if not priv_path.is_file():
        print(f"error: private key not found: {priv_path}", file=sys.stderr)
        return 1
    if is_merged_signed(input_path):
        print("error: input file already appears to be merged-signed", file=sys.stderr)
        return 1

    key = load_private_key(priv_path)

    # Sign raw file bytes
    fp, mm = file_mmap(input_path)
    data = None
    try:
        data = memoryview(mm)
        signature = key.sign(data)
        checksum = sha256_bytes(data)
    finally:
        if data is not None:
            del data
        mm.close()
        fp.close()

    if len(signature) != SIG_SIZE:
        print("error: unexpected signature size", file=sys.stderr)
        return 1

    # Preserve permissions/mtime by copying before appending.
    if args.overwrite:
        out_path = input_path
        tmp_path = input_path.with_name(input_path.name + ".tmp")
    else:
        out_path = derive_signed_path(input_path)
        if out_path.exists():
            print(f"error: output exists: {out_path}", file=sys.stderr)
            return 1
        tmp_path = out_path

    shutil.copy2(input_path, tmp_path)

    with tmp_path.open("ab") as dst:
        dst.write(MAGIC)
        dst.write(signature)

    if args.overwrite:
        os.replace(tmp_path, out_path)

    print(f"signed: {out_path}")
    print(f"sha256: {checksum}")
    return 0


def verify_merged(args: argparse.Namespace) -> int:
    input_path = Path(args.input)
    crt_path = Path(args.certificate)

    if not input_path.is_file():
        print(f"error: input not found: {input_path}", file=sys.stderr)
        return 1
    if not crt_path.is_file():
        print(f"error: certificate not found: {crt_path}", file=sys.stderr)
        return 1

    has_trailer, signature = read_trailer(input_path)
    if not has_trailer:
        print("verify: FAILED (missing trailer)", file=sys.stderr)
        return 2

    size = input_path.stat().st_size
    original_size = size - TRAILER_SIZE

    cert = load_certificate(crt_path)
    pub = cert.public_key()

    fp, mm = file_mmap(input_path)
    data = None
    try:
        # memoryview slice also creates a new memoryview that holds a reference
        full = memoryview(mm)
        data = full[:original_size]
        pub.verify(signature, data)
        print("verify: OK")
        return 0
    except InvalidSignature:
        print("verify: FAILED", file=sys.stderr)
        return 2
    finally:
        # Clean up both memoryview objects
        if data is not None:
            del data
        if 'full' in locals():
            del full
        mm.close()
        fp.close()


# ============================================================
# Inspect
# ============================================================

def cmd_inspect(args: argparse.Namespace) -> int:
    path = Path(args.input)

    if not path.is_file():
        print(f"error: input not found: {path}", file=sys.stderr)
        return 1

    size = path.stat().st_size
    has_trailer, sig = read_trailer(path)

    print(f"file: {path}")
    print(f"size: {size}")

    if has_trailer:
        print("mode: merged-signed")
        print(f"magic: {MAGIC.decode('ascii')}")
        print(f"signature size: {len(sig)}")
    else:
        print("mode: detached-or-unsigned")
        print("magic: absent")
        print("signature size: 0")

    return 0


# ============================================================
# Dispatch
# ============================================================

def cmd_sign(args: argparse.Namespace) -> int:
    if args.single and args.output:
        print("error: --output is not allowed with --single", file=sys.stderr)
        return 1
    if not args.single and not args.output:
        print("error: --output is required unless --single is used", file=sys.stderr)
        return 1

    if args.single:
        return sign_merged(args)
    return sign_detached(args)


def cmd_verify(args: argparse.Namespace) -> int:
    if args.signature:
        return verify_detached(args)
    return verify_merged(args)


# ============================================================
# Parser
# ============================================================

def build_parser() -> argparse.ArgumentParser:
    p = argparse.ArgumentParser(
        prog="wingcert",
        description="Wing plugin certificate/signature tool",
    )
    sub = p.add_subparsers(dest="cmd", required=True)

    g = sub.add_parser("gen-keypair", help="generate Ed25519 private key and self-signed certificate")
    g.add_argument("--private-key", required=True, help="output private key PEM")
    g.add_argument("--certificate", required=True, help="output certificate PEM")
    g.add_argument("--common-name", default="Wing Plugin Signer")
    g.add_argument("--organization", default="Wing Plugin")
    g.add_argument("--country", default="CN")
    g.add_argument("--valid-days", type=int, default=36500)
    g.add_argument("--force", action="store_true", help="overwrite existing files")
    g.set_defaults(func=cmd_gen_keypair)

    s = sub.add_parser("sign", help="sign a file")
    s.add_argument("--input", required=True, help="input file to sign")
    s.add_argument("--private-key", required=True, help="Ed25519 private key PEM")
    s.add_argument("--output", help="detached signature output file")
    s.add_argument("--single", action="store_true", help="append signature trailer into a merged file")
    s.add_argument("--overwrite", action="store_true", help="overwrite input file in --single mode")
    s.set_defaults(func=cmd_sign)

    v = sub.add_parser("verify", help="verify a signature")
    v.add_argument("--input", required=True, help="input file to verify")
    v.add_argument("--certificate", required=True, help="certificate PEM")
    v.add_argument("--signature", help="detached signature file; omit for merged file")
    v.set_defaults(func=cmd_verify)

    i = sub.add_parser("inspect", help="inspect file mode")
    i.add_argument("--input", required=True, help="file to inspect")
    i.set_defaults(func=cmd_inspect)

    return p


def main() -> int:
    args = build_parser().parse_args()
    return args.func(args)


if __name__ == "__main__":
    raise SystemExit(main())
