from __future__ import annotations

import argparse
import sys
from pathlib import Path
from datetime import datetime, timedelta, timezone

from cryptography import x509
from cryptography.x509.oid import NameOID
from cryptography.hazmat.primitives import hashes, serialization
from cryptography.hazmat.primitives.asymmetric import rsa, padding


def generate_keypair(args: argparse.Namespace) -> int:
    private_key_path = Path(args.private_key)
    cert_path = Path(args.certificate)

    if private_key_path.exists() and not args.force:
        print(f"error: private key already exists: {private_key_path}", file=sys.stderr)
        return 1

    if cert_path.exists() and not args.force:
        print(f"error: certificate already exists: {cert_path}", file=sys.stderr)
        return 1

    key = rsa.generate_private_key(
        public_exponent=65537,
        key_size=args.key_size,
    )

    subject = issuer = x509.Name(
        [
            x509.NameAttribute(NameOID.COUNTRY_NAME, args.country),
            x509.NameAttribute(NameOID.ORGANIZATION_NAME, args.organization),
            x509.NameAttribute(NameOID.COMMON_NAME, args.common_name),
        ]
    )

    now = datetime.now(timezone.utc)
    if args.valid_days <= 0:
        not_after = datetime(9999, 12, 31, tzinfo=timezone.utc)
    else:
        not_after = now + timedelta(days=args.valid_days)

    cert_builder = (
        x509.CertificateBuilder()
        .subject_name(subject)
        .issuer_name(issuer)
        .public_key(key.public_key())
        .serial_number(x509.random_serial_number())
        .not_valid_before(now - timedelta(days=1))
        .not_valid_after(not_after)
        .add_extension(
            x509.BasicConstraints(ca=False, path_length=None),
            critical=True,
        )
    )

    cert = cert_builder.sign(
        private_key=key,
        algorithm=hashes.SHA256(),
    )

    private_key_pem = key.private_bytes(
        encoding=serialization.Encoding.PEM,
        format=serialization.PrivateFormat.PKCS8,
        encryption_algorithm=serialization.NoEncryption(),
    )

    cert_pem = cert.public_bytes(serialization.Encoding.PEM)

    private_key_path.write_bytes(private_key_pem)
    cert_path.write_bytes(cert_pem)

    fingerprint = cert.fingerprint(
        hashes.SHA256()
    ).hex().upper()

    print(f"generated private key: {private_key_path}")
    print(f"generated certificate: {cert_path}")
    print(f"sha256 fingerprint : {fingerprint}")
    return 0


def sign_file(args: argparse.Namespace) -> int:
    input_path = Path(args.input)
    key_path = Path(args.private_key)
    sig_path = Path(args.output)

    if not input_path.is_file():
        print(f"error: input file not found: {input_path}", file=sys.stderr)
        return 1

    if not key_path.is_file():
        print(f"error: private key not found: {key_path}", file=sys.stderr)
        return 1

    data = input_path.read_bytes()

    private_key = serialization.load_pem_private_key(
        key_path.read_bytes(),
        password=None,
    )

    signature = private_key.sign(
        data,
        padding.PSS(
            mgf=padding.MGF1(hashes.SHA256()),
            salt_length=padding.PSS.MAX_LENGTH,
        ),
        hashes.SHA256(),
    )

    sig_path.write_bytes(signature)

    print(f"signature written to: {sig_path}")
    return 0


def verify_file(args: argparse.Namespace) -> int:
    input_path = Path(args.input)
    cert_path = Path(args.certificate)
    sig_path = Path(args.signature)

    if not input_path.is_file():
        print(f"error: input file not found: {input_path}", file=sys.stderr)
        return 1

    if not cert_path.is_file():
        print(f"error: certificate not found: {cert_path}", file=sys.stderr)
        return 1

    if not sig_path.is_file():
        print(f"error: signature not found: {sig_path}", file=sys.stderr)
        return 1

    data = input_path.read_bytes()

    cert = x509.load_pem_x509_certificate(cert_path.read_bytes())
    public_key = cert.public_key()

    raw_sig = sig_path.read_bytes()

    try:
        public_key.verify(
            raw_sig,
            data,
            padding.PSS(
                mgf=padding.MGF1(hashes.SHA256()),
                salt_length=padding.PSS.MAX_LENGTH,
            ),
            hashes.SHA256(),
        )
        print("verify: OK")
        return 0
    except Exception as e:
        print(f"verify: FAILED ({e})", file=sys.stderr)
        return 2


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        prog="wingcert",
        description="Generate certificate/key, sign files, and verify signatures.",
    )

    subparsers = parser.add_subparsers(dest="command", required=True)

    # 1) generate keypair
    p_gen = subparsers.add_parser("gen-keypair", help="Generate private key and self-signed certificate")
    p_gen.add_argument("--private-key", required=True, help="Output path for private key PEM")
    p_gen.add_argument("--certificate", required=True, help="Output path for certificate PEM")
    p_gen.add_argument("--common-name", default="Wing Plugin Signer", help="Certificate CN")
    p_gen.add_argument("--organization", default="Wing Plugin", help="Certificate O")
    p_gen.add_argument("--country", default="CN", help="Certificate C")
    p_gen.add_argument("--key-size", type=int, default=3072, choices=[2048, 3072, 4096], help="RSA key size")
    p_gen.add_argument("--valid-days", type=int, default=36500, help="Certificate validity in days")
    p_gen.add_argument("--force", action="store_true", help="Overwrite existing files")
    p_gen.set_defaults(func=generate_keypair)

    # 2) sign
    p_sign = subparsers.add_parser("sign", help="Sign an input file with a private key")
    p_sign.add_argument("--input", required=True, help="File to sign")
    p_sign.add_argument("--private-key", required=True, help="Private key PEM")
    p_sign.add_argument("--output", required=True, help="Signature output file")
    p_sign.set_defaults(func=sign_file)

    # 3) verify
    p_verify = subparsers.add_parser("verify", help="Verify a file signature using a certificate")
    p_verify.add_argument("--input", required=True, help="File to verify")
    p_verify.add_argument("--certificate", required=True, help="Public certificate PEM")
    p_verify.add_argument("--signature", required=True, help="Signature file")
    p_verify.set_defaults(func=verify_file)

    return parser


def main() -> int:
    parser = build_parser()
    args = parser.parse_args()
    return args.func(args)


if __name__ == "__main__":
    raise SystemExit(main())
