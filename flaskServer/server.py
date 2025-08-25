from flask import Flask, request, jsonify
import os, time
from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.primitives.asymmetric import padding
from cryptography.hazmat.primitives import serialization

# Store issued challenges
challenges = {}
CHALLENGE_EXPIRY = 60  # seconds

# Load public key (each user has private key, server has public key)
with open("dinamo_pub.pem", "rb") as f:
    public_key = serialization.load_pem_public_key(f.read())

app = Flask(__name__)

@app.route("/get_challenge")
def get_challenge():
    uid = request.args.get("uid")
    if not uid:
        return jsonify({"error": "UID is required"}), 400

    # Generate random 128-bit challenge
    challenge = os.urandom(16)
    challenges[uid] = {"challenge": challenge, "timestamp": time.time()}

    return jsonify({"challenge": challenge.hex()})

@app.route("/verify", methods=["POST"])
def verify():
    data = request.get_json()
    if not data:
        return "Missing JSON", 400

    uid = data.get("uid")
    challenge_hex = data.get("challenge")
    signature_hex = data.get("signature")

    if not uid or not challenge_hex or not signature_hex:
        return "Missing fields", 400

    entry = challenges.get(uid)
    if not entry:
        return "Challenge not found", 404

    # Expiry check
    if time.time() - entry["timestamp"] > CHALLENGE_EXPIRY:
        del challenges[uid]
        return "Challenge expired", 403

    expected_challenge = entry["challenge"]
    received_challenge = bytes.fromhex(challenge_hex)

    if received_challenge != expected_challenge:
        return "Challenge mismatch", 400

    try:
        # Convert hex signature back to bytes
        signature = bytes.fromhex(signature_hex)

        # Verify signature of the challenge
        public_key.verify(
            signature,
            received_challenge,
            padding.PKCS1v15(),
            hashes.SHA256()
        )

        del challenges[uid]
        return jsonify({"status": "success"}), 200

    except Exception as e:
        return jsonify({"status": "invalid signature", "error": str(e)}), 401

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000)
