"""Bounded subprocess tests for the four client/server language pairings."""
import argparse
from pathlib import Path
import selectors
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[2]


def ready(process):
    with selectors.DefaultSelector() as selector:
        selector.register(process.stdout, selectors.EVENT_READ)
        if not selector.select(10):
            raise AssertionError('Server did not become ready')
        line = process.stdout.readline()
        if line != 'READY\n':
            raise AssertionError(f'Server failed to start: {line!r}')


def python_peer(role, generated, rpc, signals):
    sys.path[:0] = [str(ROOT / 'python'), generated]
    import Interop_capnzero as api
    if role == 'server':
        class Handler:
            calls = 0
            server = None
            def add(self, a, b): return api.AddResult(a + b)
            def direct(self, value): return value * 2
            def echo(self, text, data, color, fraction, flag):
                return api.EchoResult(text, data, color, fraction, flag)
            def native(self, message):
                with api.schema.NativeValue.from_bytes(message) as reader:
                    return api.schema.NativeValue.new_message(value=reader.value + 1).to_bytes()
            def count(self): return self.calls
            def notify(self):
                self.calls += 1
                self.server.emit_changed(42)
            def device__increment(self, value): return value + 1
        handler = Handler()
        with api.Server(handler, rpc, signals) as server:
            handler.server = server
            server.on_subscribe('changed', lambda: server.emit_changed(7))
            print('READY', flush=True)
            while True:
                server.process_subscription(1)
                server.process_request(1)
    else:
        with api.Client(rpc, signals) as client:
            seen = []
            client.on_changed(seen.append)
            assert client.poll_signal(5000) and seen == [7]
            assert client.add(17, 25) == api.AddResult(42)
            assert client.direct(21) == 42
            assert client.device__increment(41) == 42
            assert client.echo('hello', b'\x00\x01\x02\xff', api.Color.Green, 1.25, True) == api.EchoResult('hello', b'\x00\x01\x02\xff', api.Color.Green, 1.25, True)
            data = api.schema.NativeValue.new_message(value=41).to_bytes()
            with api.schema.NativeValue.from_bytes(client.native(data)) as reader:
                assert reader.value == 42
            client.notify()
            assert client.count() == 1
            assert client.poll_signal(5000) and seen == [7, 42]
        print('OK', flush=True)


def run_pair(server_command, client_command, directory):
    rpc = 'ipc://' + str(directory / 'rpc')
    signals = 'ipc://' + str(directory / 'signals')
    with tempfile.TemporaryFile(mode='w+') as errors:
        server = subprocess.Popen(server_command + [rpc, signals], stdout=subprocess.PIPE, stderr=errors, text=True)
        try:
            ready(server)
            result = subprocess.run(client_command + [rpc, signals], capture_output=True, text=True, timeout=15)
            if result.returncode != 0 or result.stdout.strip() != 'OK':
                raise AssertionError(f'Client failed ({result.returncode}): {result.stdout}\n{result.stderr}')
        except Exception:
            errors.seek(0)
            print(errors.read(), file=sys.stderr)
            raise
        finally:
            server.terminate()
            try:
                server.wait(timeout=3)
            except subprocess.TimeoutExpired:
                server.kill()
                server.wait()
            server.stdout.close()


def main():
    if len(sys.argv) > 1 and sys.argv[1] == '--peer':
        python_peer(*sys.argv[2:])
        return
    parser = argparse.ArgumentParser()
    parser.add_argument('--cpp-client', required=True)
    parser.add_argument('--cpp-server', required=True)
    parser.add_argument('--generator-python', default='python3')
    parser.add_argument('--capnp-executable', default='capnp')
    parser.add_argument('--cpp-only', action='store_true')
    args = parser.parse_args()
    with tempfile.TemporaryDirectory(prefix='capnzero-interop-') as tmp:
        directory = Path(tmp)
        if args.cpp_only:
            run_pair([args.cpp_server], [args.cpp_client], directory)
            print('C++ server / C++ client: OK')
            return
        subprocess.run([args.generator_python, str(ROOT / 'scripts/capnzeroc.py'),
                        '--descrfile=' + str(Path(__file__).with_name('Interop.toml')),
                        '--outdir=' + tmp, '--language=python',
                        '--capnp-executable=' + args.capnp_executable], check=True)
        py_server = [sys.executable, __file__, '--peer', 'server', tmp]
        py_client = [sys.executable, __file__, '--peer', 'client', tmp]
        for server, client, label in [([args.cpp_server], py_client, 'C++ / Python'),
                                       (py_server, [args.cpp_client], 'Python / C++'),
                                       (py_server, py_client, 'Python / Python')]:
            run_pair(server, client, directory)
            print(label + ': OK')


if __name__ == '__main__':
    main()
