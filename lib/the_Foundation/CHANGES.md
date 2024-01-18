# Changelog

## 1.7.1
* PtrArray: Added `back`, `popBack`, `popFront` methods.
* String: Added `quoteDelim` method.
* String: Fixed issue with the `mid` method when starting position is at or past the end.
* StringList: Implemented missing iterator `remove` and `take` methods.
* TlsRequest: TLS 1.2 is the minimum accepted protocol version.

## 1.7
* Native Windows build using MinGW64: Winsock2 for sockets, IP addresses.
* Minimum CMake version requirement is now 3.5.
* Mat4 (generic): Fixed translating a matrix with `translate_Mat4`.
* Mat4: Fixed `scale` and `scalef` methods.
* Math: Added `xy_F3`, `initRotate_Mat4`.
* SSE: Fixed incorrect order of components when calling `initv_F4`.
* StringArray: Added serialization methods, reverse const iterator.
* StringList: Added serialization methods.

## 1.6.1
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
