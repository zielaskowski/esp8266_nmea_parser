import os
import subprocess

Import("env")

os.makedirs("src/embed_web", exist_ok=True)

try:
    subprocess.run(
        ["xxd", "-i", "web/index.html", "src/embed_web/index.html.hex"], check=True
    )
    subprocess.run(
        ["uglifyjs", "web/app.js", "-o", "src/embed_web/app.min.js", "-m"],
        check=True,
    )
    subprocess.run(
        ["xxd", "-i", "src/embed_web/app.min.js", "src/embed_web/app.min.js.hex"],
        check=True,
    )
    subprocess.run(
        ["xxd", "-i", "web/style.css", "src/embed_web/style.css.hex"], check=True
    )
except subprocess.CalledProcessError as e:
    print(f"Processe returned error: {e}")
