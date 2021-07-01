# CapnZero
 _Lightweight easy to use middleware generator using [zeromq] for transport and [cap'nproto] for serialization_

> **But wait!!! Why don't you simply use the rpc mechanism provided by capnproto?**
As much as I loved the insanely powerful capnproto time-travel rpc implementation, I was struggling to get it integrated with other event loops (e.g. Qt-s event loop or just a simple select), so I decided to use zeromq that provides a simpler way of doing that.

The IDL allows definitions of **rpcs**, **signals** and **properties**. It generates c++ code for server and client. There is also the possibility to get a Qt Client generated, where rpc-s translate to Q_INVOKABLE-s, signals translate to qt-signals, properties to Q_PROPERTIES. This QObject can then be registered into a qml environment and can be used in **property bindings**..
## IDL
The interface of the service is defined as a **toml** file, where **rpcs** and **signals** can be defined. 
### simple example
__Calculator.toml__ --> the file basename is used as service name and namespace for all generated code
```
[services]
    [services._.rpc.add]
        parameter = { a = 'Int32', b = 'Int32' }
        returns = { ret = 'Int32' }
    [services._.rpc.sub]
        parameter = { a = 'Int32', b = 'Int32' }
        returns = { ret = 'Int32' }

    [services._.signal.batteryChargeChanged]
        parameter = { percentage = 'Float32' }
```
<ins>**rpc-s**</ins> can have input **parameter** and **return** values. If one of them is void, you can simply ommit it.
<ins>**signals**</ins> can only have input parameter, wich can be omitted when void

The used **types** can be one of:
    * Int8 Int16 Int32 Int64
    * UInt8 UInt16 UInt32 UInt64
    * Float32, Float64
    * Text
    * Span
    * Data< _Size_ >
    * Native CapnProto message

The underscore after after the "services" keyword means, that the following rpc or signal definition will live in the "main" Service and is not in a "subservice". 
A subservice can be defined for example like this:
```
[services.SubserviceName.rpc.add]
        parameter = { a = 'Int32', b = 'Int32' }
        returns = { ret = 'Int32' }
```
This will have the effect, that the rpc method name in the generated classes will be prefixed with SubserviceName__ and there will be a distinct rpc interface class that has to be implemented and registered to the server.

### Direct Return Value
_Documentation has to be written_

### Native capnproto mesage as parameter and return value
_Documentation has to be written_
### Using Properties
_Documentation has to be written_
### Qt Client
checkout: __tests/TemperatureQt__
_Documentation has to be written_

[zeromq]: https://zeromq.org/
[cap'nproto]: https://capnproto.org/
