"""Runtime boundary tests; run with the interpreter containing Python bindings."""
from pathlib import Path
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]
sys.path[:0] = [str(ROOT / 'python'), sys.argv.pop(1)]
import Interop_capnzero as api
from capnzero.runtime import Field, Payload
import zmq


class RuntimeTests(unittest.TestCase):
    def test_fixed_size_data_is_checked_on_both_sides(self):
        spec = api._rpc_0_2.parameters
        with self.assertRaisesRegex(ValueError, 'expected 4 bytes'):
            spec.encode_arguments(('text', b'short', api.Color.Red, 1.25, True))
        message = api.schema.CAPNPParameterEcho.new_message(text='text', data=b'bad', color='red', fraction=1.25, flag=True)
        with self.assertRaisesRegex(ValueError, 'expected 4 bytes'):
            spec.decode_arguments(message.to_bytes())

    def test_malformed_payload_frame_presence(self):
        with self.assertRaisesRegex(ValueError, 'Missing payload'):
            api._rpc_0_0.parameters.decode_arguments(None)
        with self.assertRaisesRegex(ValueError, 'Unexpected payload'):
            api._rpc_0_4.parameters.decode_arguments(b'')

    def test_enum_and_owned_return_values_round_trip(self):
        result = api.EchoResult('hello', b'1234', api.Color.Green, 2.5, False)
        codec = api._rpc_0_2.returns
        self.assertEqual(codec.decode_result(codec.encode_result(result)), result)

    def test_timeout_closes_rpc_connection(self):
        with tempfile.TemporaryDirectory() as tmp, zmq.Context() as context:
            with api.Client('ipc://' + tmp + '/unbound', context=context, timeout_ms=5) as client:
                with self.assertRaises(TimeoutError):
                    client.add(1, 2)
                with self.assertRaisesRegex(RuntimeError, 'closed'):
                    client.add(3, 4)
            self.assertFalse(context.closed)

    def test_exact_signal_topics_and_frame_validation(self):
        with zmq.Context() as context:
            publisher = context.socket(zmq.XPUB)
            publisher.bind('inproc://runtime-signal')
            try:
                with api.Client(signal_address='inproc://runtime-signal', context=context) as client:
                    values = []
                    client.on_changed(values.append)
                    self.assertTrue(publisher.poll(1000))
                    self.assertEqual(publisher.recv(), b'\x01changed')
                    publisher.send_multipart([b'changedSuffix', b'unrelated'])
                    self.assertFalse(client.poll_signal(1000))
                    publisher.send_multipart([b'changed'])
                    with self.assertRaisesRegex(ValueError, 'frame count'):
                        client.poll_signal(1000)
                    publisher.send_multipart([b'changed', api._signal_0_0.encode_arguments((42,))])
                    self.assertTrue(client.poll_signal(1000))
                    self.assertEqual(values, [42])
            finally:
                publisher.close(linger=0)


if __name__ == '__main__':
    unittest.main()
