import os
import subprocess

Import("env")

os.makedirs("src/embed_web", exist_ok=True)

try:
    subprocess.run(
        ["xxd", "-i", "web/index.html", "src/embed_web/index.html.hex"], check=True
    )
    subprocess.run(
        ["uglifyjs", "web/app.js", "-o", "web/app.min.js", "-m"],
        check=True,
    )
    subprocess.run(
        ["xxd", "-i", "web/app.min.js", "src/embed_web/app.min.js.hex"],
        check=True,
    )
    subprocess.run(
        ["xxd", "-i", "web/style.css", "src/embed_web/style.css.hex"], check=True
    )
    subprocess.run(
        ["xxd", "-i", "web/header_ok", "src/embed_web/header_ok.hex"], check=True
    )
    subprocess.run(
        ["xxd", "-i", "web/header_error", "src/embed_web/header_error.hex"], check=True
    )

except subprocess.CalledProcessError as e:
    print(f"Processe returned error: {e}")
