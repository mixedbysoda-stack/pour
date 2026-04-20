#!/usr/bin/env node
// Generate a test license key for Pour.
// Usage: node generate_test_key.js [email]
//
// Reads pour_license_secret.txt (gitignored). The secret must match
// the POUR_LICENSE_SECRET env var set on Netlify for activation to succeed.

const crypto = require("crypto");
const fs = require("fs");
const path = require("path");

const secretPath = path.join(__dirname, "pour_license_secret.txt");
if (!fs.existsSync(secretPath)) {
    console.error("ERROR: pour_license_secret.txt not found. Generate one with:");
    console.error("  openssl rand -hex 32 > pour_license_secret.txt");
    process.exit(1);
}

const secret = fs.readFileSync(secretPath, "utf8").trim();
const email  = (process.argv[2] || "test@example.com").trim();
const sessionCreated = Math.floor(Date.now() / 1000);

const emailHash = crypto.createHash("sha256")
    .update(email.toLowerCase())
    .digest()
    .slice(0, 16);

const tsBuf = Buffer.alloc(8);
tsBuf.writeBigUInt64BE(BigInt(sessionCreated));

const payload = Buffer.concat([emailHash, tsBuf]);
const hmac = crypto.createHmac("sha256", Buffer.from(secret, "hex"))
    .update(payload)
    .digest()
    .slice(0, 8);

const raw = Buffer.concat([emailHash, tsBuf, hmac])
    .toString("hex")
    .toUpperCase();
const key = raw.match(/.{8}/g).join("-");

console.log("Pour test license key for " + email + ":");
console.log(key);
