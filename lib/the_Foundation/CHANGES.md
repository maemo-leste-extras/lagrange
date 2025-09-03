# Changelog

## 1.9.1
* TlsCertificate: Calculate SHA256 fingerprint when initializing (it will never change).

## 1.9 - 2024-09-20
* Added CMake build option `TFDN_ENABLE_STATIC_LINK` for linking dependencies statically.
* CommandLine: Query executable path via Linux /proc/ filesystem.
* Datagram: Added `isConnected` and `openRandom` methods.
* Datagram: Fixed premature termination of the UDP worker thread when an error occurs with one socket.
* Datagram: Handle UDP-related errors on Linux (e.g., ICMP port unreachable).
* TlsCertificate: Added method for querying subject alternative names.
* TlsRequest: Replaced use of deprecated OpenSSL API function.
* TlsRequest: Fixed a possible memory leak.
* XmlDocument: The `<?xml` header line is no longer required by the parser.

## 1.8.1 - 2024-04-14
* Fixed library version number in the CMakeLists file.

## 1.8 - 2024-04-14
* PtrArray: Added `back`, `constBack`, `popBack`, and `popFront` methods.
* String: Added `concat`, `concatCStr`, and `quoteDelim` methods.
* String: Added a function for setting the locale for 8-bit charset conversions.
* String: Added a method for converting a `Rangecc` to the local charset.
* String: Fixed issue with the `mid` method when starting position is at or past the end.
* StringList: Implemented missing iterator `remove` and `take` methods.
* TlsRequest: TLS 1.2 is the minimum accepted protocol version.
* TlsRequest: Adjusted ifdefs for improved LibreSSL compatibility.
* pkg-config: Compiler flag for SSE 4.1 added into the .pc file (if enabled in the build).
* Fixed warnings (errors) abouts implicit conversions.

## 1.7 - 2023-09-09
* Native Windows build using MinGW64: Winsock2 for sockets, IP addresses.
* Minimum CMake version requirement is now 3.5.
* Mat4 (generic): Fixed translating a matrix with `translate_Mat4`.
* Mat4: Fixed `scale` and `scalef` methods.
* Math: Added `xy_F3`, `initRotate_Mat4`.
* SSE: Fixed incorrect order of components when calling `initv_F4`.
* StringArray: Added serialization methods, reverse const iterator.
* StringList: Added serialization methods.

## 1.6.1 - 2023-03-09
* Library SOVERSION uses the major version number only. Minor versions do not break ABI compatibility.

## 1.6 - 2023-01-21
* Block: Added a method to encode contents in Base64.
* PtrArray: Added `copy` method.
* Process: Minor changes in how I/O is handled with the child process. Calling `writeInput` more than once is now possible.
* Process: Added a test where processes are started concurrently from threads.
* TlsRequest: Avoid possible crash when reusing TLS sessions.
* TlsRequest: Added a method to query the SSL library name (OpenSSL/LibreSSL).

## 1.5 - 2022-11-25
* RegExp: PCRE2 can be used as an alternative to the older PCRE.

## 1.4 - 2022-04-28
* Block: Fixed crash when checking size of zero-initialized instance.
* Char: Added `width` method.

## 1.3 - 2022-04-05
* Archive: Opening as writable, serializing with compression.
* Block: Handle possible vprintf encoding errors.
* String: Upper/lower-casing in a specific language.
* Time: Added `max` method.
* TlsCertificate: Avoid repeated verification of the same certificate.
* TlsRequest: TLS session cache can be disabled per request.

## 1.2.1
* SOVERSION includes the minor version, since new symbols may be added in minor releases.

## 1.2 - 2022-03-05
* String: Added replacing/substituting with a regular expression.
* TlsCertificate: Fixed copying of X509 chains.
* TlsRequest: Added a session cache to avoid repeat handshakes.

## 1.1 - 2022-01-20
* Detect Android as a variant of Linux. Some features like starting child processes are disabled in Android.
* Block: gzip-compatible decompression.
* Char: Querying script of a Unicode code point.
* Process: Remember exit status code.

## 1.0.3
* TlsRequest: Set "valid from" time using UTC instead of local time.

## 1.0 - 2021-11-06
* Initial release.
