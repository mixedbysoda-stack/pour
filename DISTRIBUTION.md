# Pour — Distribution Checklist

Every step needed to ship Pour v1.0.0 publicly. Mirrors the On Tap / De-Sipper / Carbonator pipeline.

## One-time setup (already done for your other plugins)

- Apple Developer ID Application cert: `45C26CF1655F48EBC8A21802BDA053514719E1F0`
- Apple notary keychain profile: `carbonator-notary`
- PACE account: `sodanswishers`, WCGUID `5F45D2A0-3560-11F1-B6A4-00505692AD3E`
- Netlify site hosting the activation functions
- `hello@carbonatedaudio.com` domain email via Resend

## Steps for Pour specifically

### 1. Deploy the Netlify activation function

- File already created: `Carbonator/website/netlify/functions/activate-pour.js`
- Set the env var on Netlify: `POUR_LICENSE_SECRET = <contents of pour_license_secret.txt>`
- Register Blobs store `pour-activations` (auto-created on first request; confirm it appears in Netlify dashboard)
- Deploy site — endpoint becomes `https://carbonatedaudio.com/.netlify/functions/activate-pour`

### 2. Register Pour on Stripe

- Create Stripe product "Pour" at $20 (same as On Tap launch price).
- Create a Stripe payment link → metadata `product=pour`.
- Existing `stripe-webhook.js` auto-generates a key on `checkout.session.completed` via the `pour` entry in `config.js` (already added).
- Update `stripe-webhook.js` email template to include a Pour branch if you want a Pour-specific subject line / download URL (the `PRODUCTS.pour.downloads` URLs already point to the right GitHub release path).

### 3. Build the macOS distribution

```bash
cd /Users/soda/Desktop/Pour
./distribute.sh 1.0.0
```

This:
1. Configures + builds universal (arm64 + x86_64)
2. Codesigns VST3, AU, Standalone with Developer ID
3. Submits to Apple notary and waits
4. Staples notarization tickets
5. PACE-signs the AAX (prompts for PACE password)
6. Stages everything at `dist/pour-v1.0.0/`

Then build the installer:

```bash
./build_installer.sh 1.0.0
```

Produces `installer/Pour-v1.0.0-Installer.pkg`, notarized and stapled.

### 4. Push to GitHub

Create the repo first at **https://github.com/mixedbysoda-stack/pour** (private), then:

```bash
cd /Users/soda/Desktop/Pour
git init
git add .
git commit -m "Initial commit — Pour v1.0.0"
git branch -M main
git remote add origin git@github.com:mixedbysoda-stack/pour.git
git push -u origin main
```

### 5. Trigger the Windows build

```bash
git tag v1.0.0
git push --tags
```

GitHub Actions runs `.github/workflows/build-windows.yml` and automatically attaches `Pour-Windows.zip` + `Pour-v1.0.0-Windows-Installer.exe` to the GitHub Release page.

### 6. Publish the GitHub Release

On the tag `v1.0.0` GitHub Release page, upload the macOS assets from `dist/pour-v1.0.0/`:
- `Pour-v1.0.0-Installer.pkg`
- Optional: zipped VST3, AU, AAX, Standalone folders

Filenames must match the URLs in `Carbonator/website/netlify/functions/config.js`.

### 7. Landing page

Clone from an existing product landing (De-Sipper or On Tap) in the website repo:
- `Carbonator/website/pour.html`
- Update Stripe payment link, copy, screenshots
- Add route in site nav

### 8. Post-launch

- Hourly monitor already watches all Stripe sales — it will auto-include Pour
- Carbonated Audio Bundle: once Pour is stable, raise bundle product to include Pour ($55 + for 4 plugins) per pricing notes

## Generating a test license key manually

Until a Stripe purchase is made, use the HMAC key generator to test activation. `pour_license_secret.txt` holds the secret this reads from:

```bash
node -e '
const crypto = require("crypto");
const fs = require("fs");
const secret = fs.readFileSync("pour_license_secret.txt", "utf8").trim();
const email = "test@example.com";
const ts = Math.floor(Date.now() / 1000);

const emailHash = crypto.createHash("sha256").update(email.toLowerCase().trim()).digest().slice(0, 16);
const tsBuf = Buffer.alloc(8); tsBuf.writeBigUInt64BE(BigInt(ts));
const payload = Buffer.concat([emailHash, tsBuf]);
const hmac = crypto.createHmac("sha256", Buffer.from(secret, "hex")).update(payload).digest().slice(0, 8);
const raw = Buffer.concat([emailHash, tsBuf, hmac]).toString("hex").toUpperCase();
console.log(raw.match(/.{8}/g).join("-"));
'
```

Paste the resulting key into the Pour activation dialog. Note: the Netlify function must be live with the same `POUR_LICENSE_SECRET` env var for this to succeed.

## Secrets stored locally

- `pour_license_secret.txt` — HMAC key for generating and validating license keys. Gitignored. **Must match** the `POUR_LICENSE_SECRET` Netlify env var.
- PACE password — prompted interactively during `distribute.sh`.
