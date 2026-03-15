# libAES67

libAES67 provides a comprehensive set of tools for streaming audio over IP in compliance with the
AES67 standard. The library supports RTP for transport, SDP for session description, and PTPv2
for precise clock synchronization, enabling reliable interoperability between devices and
applications. Its implementation follows the relevant RFCs [RFC 3550], [RFC 4566], [IEEE 1588-2008]
to ensure timing accuracy and consistent delivery. Designed for portability and integration,
libAES67 can be used across embedded systems, desktop platforms, and other networked environments.

## Requirements

- CMake 4.1 or later
- C/C++23 toolchain with Clang or GCC
- Platforms:
    - Linux (x86_64 / ARM64)
    - macOS 15.0+ (x86_64 / Apple Silicon)
    - Embedded (with POSIX sockets)

## Contributing

Contributions are welcome whether it’s fixing a bug, improving documentation, or suggesting new features.

If you’d like to get involved, please see the [CONTRIBUTING.md](./CONTRIBUTING.md) for how to get started.

Not sure where to begin? Check out the [issues](https://github.com/Soundform-Labs/libAES67/issues) or open a
[discussion](https://github.com/swiftaudiofoundation/libAES67/discussions).

## Project Status

Version `0.1.0` is the first official release of `libAES67`. Earlier versions have no source stability guarantees.

Because the `libAES67` library is under active development, source stability is only guaranteed within minor
versions (for example, between `0.1.1` and `0.1.2`).

Future minor versions of the package may introduce changes to these rules as needed.

## License

libAES67 is licensed under the [BSD-3-Clause License](https://opensource.org/license/bsd-3-clause).

Redistribution and use in source and binary forms, with or without modification, are permitted provided
that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and
   the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions
   and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or
   promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS “AS IS” AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

For more details, see the [LICENSE](LICENSE) file included in this repository.

_© 2025 - 2026 Nevio Hirani and the libAES67 contributors._

[RFC 3550]: https://datatracker.ietf.org/doc/html/rfc3550
[RFC 4566]: https://datatracker.ietf.org/doc/html/rfc4566
[IEEE 1588-2008]: https://standards.ieee.org/ieee/1588/4355/
