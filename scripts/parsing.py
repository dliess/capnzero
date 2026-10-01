"""Load TOML syntax without target-language decisions."""
import toml

def load(path):
    return toml.load(path)
